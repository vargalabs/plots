/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::pie(labels, values, opts...) — a deferred view (plot::view) drawing a
 * pie chart, or a donut when plot::donut{r} (inner-radius fraction in (0,1)) is
 * given. Each slice is a CLOSED <polyline> wedge: the arc is segmented into short
 * chords (no new canvas method needed), centre→arc→centre for a pie, or
 * outer-arc→inner-arc for a donut ring. Slice colours cycle theme.series; a
 * label is placed at the slice's mid-angle.
 *
 * Built on the #5 view pattern (draws straight into the cell). Dependency-free.
 */
#ifndef PLOT_PIE_HPP
#define PLOT_PIE_HPP

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "tags.hpp"
#include "meta.hpp"
#include "attributes.hpp"
#include "canvas.hpp"
#include "theme.hpp"
#include "gr.hpp"
#include "view.hpp"

namespace plot {
	// donut inner-radius fraction in (0,1); 0 (or absent) ⇒ a full pie.
	struct donut { using value_type = tag::donut_t; double value; };
}

namespace plot::impl {
	template <class... opt_t>
	double donut_of(const opt_t&... opts){
		using d_t = typename arg::tpos<tag::donut_t, opt_t...>;
		if constexpr( d_t::present ){
			auto tuple = std::forward_as_tuple(opts...);
			double r = std::get<d_t::position>(tuple).value;
			if( r < 0 ) r = 0; else if( r > 0.95 ) r = 0.95;
			return r;
		}
		return 0.0;
	}

	template <class... opt_t>
	struct pie_view {
		using value_type = tag::view_t;
		std::vector<std::string> labels;
		std::vector<double> values;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{500}, std::size_t{500}, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				const double inner = impl::donut_of(o...);
				using attribute_t = plot::attribute::element_t;

				const std::size_t n = std::min(labels.size(), values.size());
				double total = 0.0; for(std::size_t i=0;i<n;++i) if(values[i]>0) total += values[i];
				if( total <= 0.0 ) total = 1.0;

				cv.group(static_cast<std::size_t>(x), static_cast<std::size_t>(y), attribute_t{},
					[&](){
						{ attribute_t bg; bg.color = plot::attribute::color_t{ th.bg };
						  cv.rect(0,0, w, h, 0,0, bg); }
						float top = title.empty() ? 8.0f : 26.0f;
						if( !title.empty() ){
							attribute_t a; a.color = plot::attribute::color_t{ th.fg };
							a.font = plot::attribute::font_t{"Arial, sans-serif", "bold", 13u};
							a.align = plot::attribute::align_t::center;
							cv.text(title, std::size_t(w/2), std::size_t(16), a);
						}
						float cx = w*0.5f, cy = top + (h - top)*0.5f;
						float Ro = std::max(8.0f, std::min(w, h - top)*0.5f - 30.0f);
						float Ri = float(inner) * Ro;

						double a0 = -1.5707963267948966;   // start at 12 o'clock
						const double TWO_PI = 6.283185307179586;
						for(std::size_t i=0;i<n;++i){
							double frac = (values[i] > 0 ? values[i] : 0.0) / total;
							double a1 = a0 + frac*TWO_PI;
							// segment the arc into ~enough chords for smoothness.
							std::size_t segs = std::max<std::size_t>(2,
									static_cast<std::size_t>(std::ceil(frac*64.0)));
							std::vector<float> X, Y;
							// outer arc a0 -> a1
							for(std::size_t s=0;s<=segs;++s){
								double a = a0 + (a1-a0)*double(s)/double(segs);
								X.push_back(cx + Ro*float(std::cos(a)));
								Y.push_back(cy + Ro*float(std::sin(a)));
							}
							if( Ri > 0.5f ){
								// inner arc a1 -> a0 (reverse) for a ring wedge.
								for(std::size_t s=0;s<=segs;++s){
									double a = a1 + (a0-a1)*double(s)/double(segs);
									X.push_back(cx + Ri*float(std::cos(a)));
									Y.push_back(cy + Ri*float(std::sin(a)));
								}
							} else {
								X.push_back(cx); Y.push_back(cy);   // pie apex
							}
							// close the wedge.
							X.push_back(X.front()); Y.push_back(Y.front());

							std::uint32_t col = th.series.empty()? th.fg
											: th.series[i % th.series.size()];
							attribute_t pa; pa.color = plot::attribute::color_t{ col };
							pa.stroke = plot::attribute::stroke_t{1.0f, 1.2f, {}, {}, {}};
							cv.poly_line(X, Y, pa);

							// label at the slice mid-angle, just outside the wedge.
							double am = 0.5*(a0+a1);
							float lr = Ro + 12.0f;
							attribute_t ta; ta.color = plot::attribute::color_t{ th.fg };
							ta.font = plot::attribute::font_t{"Arial, sans-serif", "normal", 10u};
							ta.align = plot::attribute::align_t::center;
							float lx = cx + lr*float(std::cos(am));
							float ly = cy + lr*float(std::sin(am));
							cv.text(labels[i], std::size_t(lx<0?0:lx), std::size_t(ly+3), ta);
							a0 = a1;
						}
					});
			}, opts);
		}
	};
}

namespace plot {
	template <class... opt_t>
	impl::pie_view<opt_t...> pie(std::vector<std::string> labels,
			std::vector<double> values, opt_t... opts){
		return impl::pie_view<opt_t...>{ std::move(labels), std::move(values),
				std::make_tuple(opts...) };
	}
}
#endif
