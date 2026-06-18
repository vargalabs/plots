/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::histogram(values, opts...) — a deferred view (plot::view) that bins a
 * vector of doubles into N bins and draws count bars over a cartesian panel.
 *
 * Bin count: plot::bins{n} named argument; default ~Sturges' rule
 * (ceil(log2(N)) + 1). Built on the #5 view pattern + impl::render_panel_into
 * cartesian core: factory returns a movable struct tagged tag::view_t, captures
 * data + options by value, exposes draw_into / natural / render / save.
 * Dependency-free (standard library only).
 */
#ifndef PLOT_HISTOGRAM_HPP
#define PLOT_HISTOGRAM_HPP

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
	// histogram bin count (order-independent named argument).
	struct bins { using value_type = tag::bins_t; std::size_t value; };
}

namespace plot::impl {
	// resolve the bin count: plot::bins{n} override, else Sturges' rule.
	template <class... opt_t>
	std::size_t bins_of(std::size_t n, const opt_t&... opts){
		using bins_t = typename arg::tpos<tag::bins_t, opt_t...>;
		if constexpr( bins_t::present ){
			auto tuple = std::forward_as_tuple(opts...);
			std::size_t b = std::get<bins_t::position>(tuple).value;
			return b ? b : 1;
		}
		if( n < 2 ) return 1;
		return static_cast<std::size_t>(std::ceil(std::log2(double(n)))) + 1;
	}

	template <class... opt_t>
	struct histogram_view {
		using value_type = tag::view_t;
		std::vector<double> values;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{640}, std::size_t{400}, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				const std::size_t nb = impl::bins_of(values.size(), o...);

				auto [vmn, vmx] = impl::minmax_of(values);
				if( vmx <= vmn ) vmx = vmn + 1.0;
				const double width = (vmx - vmn) / double(nb);

				std::vector<double> counts(nb, 0.0);
				for(double v : values){
					std::size_t b = static_cast<std::size_t>((v - vmn) / width);
					if( b >= nb ) b = nb - 1;
					counts[b] += 1.0;
				}
				double cmax = 0.0;
				for(double c : counts) if( c > cmax ) cmax = c;
				if( cmax <= 0.0 ) cmax = 1.0;

				impl::scale_t sx = impl::make_scale(vmn, vmx, false, 10.0);
				impl::scale_t sy = impl::make_scale(0.0, cmax, false, 10.0);
				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						std::uint32_t col = th.series.empty()? th.fg : th.series[0];
						const float y0 = py(0.0);   // pixel baseline for count 0
						for(std::size_t b=0;b<nb;++b){
							double lo = vmn + double(b)   * width;
							double hi = vmn + double(b+1) * width;
							float xl0 = px(lo), xr0 = px(hi);
							float yt = py(counts[b]);
							float bw = xr0 - xl0;
							if( bw < 1.0f ) bw = 1.0f;
							attribute_t a; a.color = plot::attribute::color_t{ col };
							// inset by 1px so adjacent bars read as separate columns.
							c.rect(xl0 + 0.5f, yt, bw - 1.0f, y0 - yt, 0, 0, a);
						}
					});
			}, opts);
		}
		void render(std::ostream& os) const { impl::render_view(os, *this); }
		void save(const std::string& file) const { impl::save_view(file, *this); }
	};
}

namespace plot {
	template <class... opt_t>
	impl::histogram_view<opt_t...> histogram(std::vector<double> values, opt_t... opts){
		return impl::histogram_view<opt_t...>{ std::move(values), std::make_tuple(opts...) };
	}
}
#endif
