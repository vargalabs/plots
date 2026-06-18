/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Deferred plots (plot::view) + figure composition (plot::grid).
 *
 * Calling plot::line/scatter/heatmap WITHOUT a leading ostream/filename argument
 * returns a concrete, movable *view* that captures the data and named options BY
 * VALUE. A view knows how to draw itself into a REGION of an existing canvas
 * (`draw_into(canvas, x, y, w, h)`) and reports a natural default size. A view
 * renders standalone via `plot::save(file, view)` / `plot::render(os, view)`
 * (each opens one <svg> at the natural size), so the file/ostream driver
 * overloads are now thin sugar: build the view, then save/render.
 *
 * plot::grid(file|os, args...) is variadic and uses the SAME order-independent
 * tpos/static_for machinery as the rest of the layer: it collects every argument
 * whose value_type is tag::view_t (in order) and resolves plot::rows / plot::cols
 * / plot::width / plot::height / plot::use{theme} by tag, in any order. It opens
 * ONE <svg>, paints the figure background, and tiles the views into cells.
 *
 * No type erasure: views are concrete types templated on their option pack, and
 * the grid is a variadic template — identification is purely compile-time.
 * Dependency-free (standard library only).
 */
#ifndef PLOT_VIEW_HPP
#define PLOT_VIEW_HPP

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <type_traits>
#include <array>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <fstream>
#include <ostream>
#include <sstream>

#include "tags.hpp"
#include "meta.hpp"
#include "attributes.hpp"
#include "canvas.hpp"
#include "theme.hpp"
#include "gr.hpp"
#include "heatmap.hpp"

namespace plot {
	// ---- grid layout carriers (named, order-independent) --------------------
	struct rows { using value_type = tag::rows_t; std::size_t value; };
	struct cols { using value_type = tag::cols_t; std::size_t value; };
}

namespace plot::impl {
	// forward declarations: the views' render()/save() members delegate to these
	// shared helpers, which are defined after the view structs.
	template <class view_t> void render_view(std::ostream& os, const view_t& v);
	template <class view_t> void save_view(const std::string& file, const view_t& v);
}

namespace plot::impl {
	// natural plot size from width/height options, with defaults (W,H).
	template <class... opt_t>
	std::pair<std::size_t,std::size_t>
	natural_size(std::size_t dw, std::size_t dh, const opt_t&... opts){
		using width_t  = typename arg::tpos<tag::width_t,  opt_t...>;
		using height_t = typename arg::tpos<tag::height_t, opt_t...>;
		auto tuple = std::forward_as_tuple(opts...);
		std::size_t W = dw, H = dh;
		if constexpr( width_t::present )  W = std::get<width_t::position>(tuple).value;
		if constexpr( height_t::present ) H = std::get<height_t::position>(tuple).value;
		return {W, H};
	}
}

namespace plot::impl {
	// ---- line view (single series) ------------------------------------------
	template <class... opt_t>
	struct line_view {
		using value_type = tag::view_t;
		std::vector<double> xs, ys;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{640}, std::size_t{400}, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				auto [xlg, xb] = impl::xlog_of(o...);
				auto [ylg, yb] = impl::ylog_of(o...);
				auto [xmn,xmx] = impl::minmax_of(xs);
				auto [ymn,ymx] = impl::minmax_of(ys);
				impl::scale_t sx = impl::make_scale(xmn,xmx,xlg,xb);
				impl::scale_t sy = impl::make_scale(ymn,ymx,ylg,yb);
				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						std::vector<float> xpx, ypx;
						const std::size_t n = std::min(xs.size(), ys.size());
						for(std::size_t i=0;i<n;++i){
							xpx.push_back(px(sx.xform(xs[i])));
							ypx.push_back(py(sy.xform(ys[i])));
						}
						attribute_t a;
						a.color = plot::attribute::color_t{ th.series.empty()? th.fg : th.series[0] };
						a.stroke = plot::attribute::stroke_t{1.0f, 1.8f, {}, {}, {}};
						c.poly_line(xpx, ypx, a);
					});
			}, opts);
		}
	};

	// ---- multi-series line view ---------------------------------------------
	template <class... opt_t>
	struct mline_view {
		using value_type = tag::view_t;
		std::vector<double> xs;
		std::vector<impl::series_t> series;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{640}, std::size_t{400}, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				auto [xlg, xb] = impl::xlog_of(o...);
				auto [ylg, yb] = impl::ylog_of(o...);
				auto [xmn,xmx] = impl::minmax_of(xs);
				double ymn =  std::numeric_limits<double>::infinity();
				double ymx = -std::numeric_limits<double>::infinity();
				for(const auto& s : series){
					auto [a,b] = impl::minmax_of(s.data);
					if(a<ymn) ymn=a; if(b>ymx) ymx=b;
				}
				if( !(ymn<=ymx) ){ ymn=0; ymx=1; }
				impl::scale_t sx = impl::make_scale(xmn,xmx,xlg,xb);
				impl::scale_t sy = impl::make_scale(ymn,ymx,ylg,yb);
				std::vector<std::string> labels;
				for(const auto& s : series) labels.push_back(s.label);
				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, labels,
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						for(std::size_t k=0;k<series.size();++k){
							const auto& s = series[k];
							std::vector<float> xpx, ypx;
							const std::size_t n = std::min(xs.size(), s.data.size());
							for(std::size_t i=0;i<n;++i){
								xpx.push_back(px(sx.xform(xs[i])));
								ypx.push_back(py(sy.xform(s.data[i])));
							}
							std::uint32_t col = th.series.empty()? th.fg
											: th.series[k % th.series.size()];
							attribute_t a; a.color = plot::attribute::color_t{ col };
							a.stroke = plot::attribute::stroke_t{1.0f, 1.8f, {}, {}, {}};
							c.poly_line(xpx, ypx, a);
						}
					});
			}, opts);
		}
	};

	// ---- scatter view -------------------------------------------------------
	template <class... opt_t>
	struct scatter_view {
		using value_type = tag::view_t;
		std::vector<double> xs, ys;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{640}, std::size_t{400}, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				auto [xlg, xb] = impl::xlog_of(o...);
				auto [ylg, yb] = impl::ylog_of(o...);
				auto [xmn,xmx] = impl::minmax_of(xs);
				auto [ymn,ymx] = impl::minmax_of(ys);
				impl::scale_t sx = impl::make_scale(xmn,xmx,xlg,xb);
				impl::scale_t sy = impl::make_scale(ymn,ymx,ylg,yb);
				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						const std::size_t n = std::min(xs.size(), ys.size());
						std::uint32_t col = th.series.empty()? th.fg : th.series[0];
						for(std::size_t i=0;i<n;++i){
							attribute_t a; a.color = plot::attribute::color_t{ col };
							c.circle(px(sx.xform(xs[i])), py(sy.xform(ys[i])), 3.0f, a);
						}
					});
			}, opts);
		}
	};

	// ---- heatmap view -------------------------------------------------------
	// The heatmap's layout (cell grid, gutters) is fixed by its axis fonts, so its
	// natural size comes from heatmap_render. To fit an arbitrary cell rect we
	// render at natural size inside a translate+uniform-scale group.
	template <class T, class... opt_t>
	struct heatmap_view {
		using value_type = tag::view_t;
		T data;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			// a throwaway probe render reports the natural extent (small grids).
			std::ostringstream probe;
			std::array<float,4> M{5,5,5,5};
			impl::canvas_t scratch(probe, 1, 1, M);
			return std::apply([&](const auto&... o){
				return impl::heatmap_render(scratch, data, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			auto [nw, nh] = natural();
			float s = std::min( w / float(nw ? nw : 1), h / float(nh ? nh : 1) );
			if( s <= 0.0f ) s = 1.0f;
			// centre the scaled heatmap within the cell.
			float ox = x + (w - float(nw) * s) * 0.5f;
			float oy = y + (h - float(nh) * s) * 0.5f;
			cv.group_scaled(ox, oy, s, [&]{
				std::apply([&](const auto&... o){
					(void)impl::heatmap_render(cv, data, o...); }, opts);
			});
		}
	};

	// ---- standalone render/save helpers (shared by every view) --------------
	template <class view_t>
	void render_view(std::ostream& os, const view_t& v){
		auto [W,H] = v.natural();
		std::array<float,4> M{5,5,5,5};
		canvas_t cv(os, W, H, M);
		v.draw_into(cv, 0, 0, float(W), float(H));
	}
	template <class view_t>
	void save_view(const std::string& file, const view_t& v){
		std::ofstream ofs(file, std::ios::out | std::ios::trunc);
		render_view(ofs, v);
	}
}

namespace plot {
	// ---- standalone rendering --------------------------------------------------
	// Free-function form (replaces the per-view .save()/.render() members):
	//   plot::save("chart.svg", plot::line(x, y, ...));   -> file
	//   plot::render(std::cout,  plot::scatter(x, y));     -> stream
	// Filename/stream first, matching plot::grid(file, ...) / plot::line(file, ...).
	template <class V>
	void save(const std::string& file, const V& view){ impl::save_view(file, view); }
	template <class V>
	void render(std::ostream& os, const V& view){ impl::render_view(os, view); }

	// ---- deferred-view factories (no leading os/file argument) ---------------
	template <class X, class Y, class... opt_t>
	impl::line_view<opt_t...> line(const std::vector<X>& xs, const std::vector<Y>& ys, opt_t... opts){
		impl::line_view<opt_t...> v{ {}, {}, std::make_tuple(opts...) };
		v.xs.assign(xs.begin(), xs.end());
		v.ys.assign(ys.begin(), ys.end());
		return v;
	}
	template <class X, class... opt_t>
	impl::mline_view<opt_t...> line(const std::vector<X>& xs,
			const std::vector<impl::series_t>& series, opt_t... opts){
		impl::mline_view<opt_t...> v{ {}, series, std::make_tuple(opts...) };
		v.xs.assign(xs.begin(), xs.end());
		return v;
	}
	template <class X, class... opt_t>
	impl::mline_view<opt_t...> line(const std::vector<X>& xs,
			std::initializer_list<impl::series_t> series, opt_t... opts){
		return line(xs, std::vector<impl::series_t>(series), opts...);
	}
	template <class X, class Y, class... opt_t>
	impl::scatter_view<opt_t...> scatter(const std::vector<X>& xs, const std::vector<Y>& ys, opt_t... opts){
		impl::scatter_view<opt_t...> v{ {}, {}, std::make_tuple(opts...) };
		v.xs.assign(xs.begin(), xs.end());
		v.ys.assign(ys.begin(), ys.end());
		return v;
	}
	// the deferred heatmap factory must not shadow the os/filename overloads in
	// heatmap.hpp; SFINAE it out when the first argument is an ostream or a string.
	template <class T, class... opt_t,
		class = std::enable_if_t<
			!std::is_convertible_v<const T&, const std::string&> &&
			!std::is_base_of_v<std::ostream, std::decay_t<T>>>>
	impl::heatmap_view<T,opt_t...> heatmap(const T& data, opt_t... opts){
		return impl::heatmap_view<T,opt_t...>{ data, std::make_tuple(opts...) };
	}
}

namespace plot::impl {
	// ---- grid composition core ----------------------------------------------
	// Collect the view-typed arguments (value_type == tag::view_t) in order, then
	// tile them into cells. rows/cols/width/height/use resolved by tag (any order).
	template <class... arg_t>
	void grid_render(std::ostream& os, arg_t... args){
		auto tuple = std::forward_as_tuple(args...);

		// count the views.
		constexpr std::size_t N = ( 0 + ... +
			(impl::is_value_type<arg_t, tag::view_t>::value ? 1u : 0u) );
		static_assert( N > 0, "plot::grid needs at least one view argument" );

		// resolved layout.
		using rows_t   = typename arg::tpos<tag::rows_t,   arg_t...>;
		using cols_t   = typename arg::tpos<tag::cols_t,   arg_t...>;
		using width_t  = typename arg::tpos<tag::width_t,  arg_t...>;
		using height_t = typename arg::tpos<tag::height_t, arg_t...>;

		std::size_t C = 0, R = 0;
		if constexpr( cols_t::present ) C = std::get<cols_t::position>(tuple).value;
		if constexpr( rows_t::present ) R = std::get<rows_t::position>(tuple).value;
		// auto-layout: cols = ceil(sqrt(N)); rows = ceil(N/cols).
		if( C == 0 && R == 0 ){
			C = static_cast<std::size_t>(std::ceil(std::sqrt(double(N))));
			if( C == 0 ) C = 1;
			R = (N + C - 1) / C;
		} else if( C == 0 ){
			C = (N + R - 1) / R;
		} else if( R == 0 ){
			R = (N + C - 1) / C;
		}
		if( C == 0 ) C = 1;
		if( R == 0 ) R = 1;

		std::size_t W = 1200, H = 800;
		if constexpr( width_t::present )  W = std::get<width_t::position>(tuple).value;
		if constexpr( height_t::present ) H = std::get<height_t::position>(tuple).value;

		const theme_t& fig_th = impl::resolve_theme(args...);

		const float cellW = float(W) / float(C);
		const float cellH = float(H) / float(R);

		std::array<float,4> M{0,0,0,0};
		canvas_t canvas(os, W, H, M);
		// figure background with the resolved theme.
		{ plot::attribute::element_t bg; bg.color = plot::attribute::color_t{ fig_th.bg };
		  canvas.rect(0, 0, float(W), float(H), 0, 0, bg); }

		// tile views in argument order. static_for walks the full pack; a running
		// view index (incremented only for view-typed args) places each cell.
		std::size_t i = 0;
		impl::static_for<std::tuple<arg_t...>>(
			[&](auto idx){
				using A = std::tuple_element_t<idx.value, std::tuple<arg_t...>>;
				if constexpr( impl::is_value_type<A, tag::view_t>::value ){
					const std::size_t col = i % C;
					const std::size_t row = i / C;
					const float cx = float(col) * cellW;
					const float cy = float(row) * cellH;
					std::get<idx.value>(tuple).draw_into(canvas, cx, cy, cellW, cellH);
					++i;
				}
			});
	}
}

namespace plot {
	// ---- public grid drivers -------------------------------------------------
	template <class... arg_t>
	void grid(std::ostream& os, arg_t... args){
		impl::grid_render(os, args...);
	}
	template <class... arg_t>
	void grid(const std::string& filename, arg_t... args){
		std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
		impl::grid_render(ofs, args...);
	}
}
#endif
