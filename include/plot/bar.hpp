/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::bar(labels, values, opts...) — a deferred view (plot::view) drawing one
 * categorical bar per (label,value) pair over a cartesian panel. The x ticks are
 * the category labels (a hand-built impl::scale_t whose tick labels are the
 * category names rather than numbers), bar colours cycle theme.series.
 *
 * Built on the #5 view pattern + impl::render_panel_into. Dependency-free.
 */
#ifndef PLOT_BAR_HPP
#define PLOT_BAR_HPP

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

namespace plot::impl {
	template <class... opt_t>
	struct bar_view {
		using value_type = tag::view_t;
		std::vector<std::string> labels;
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
				const std::size_t n = std::min(labels.size(), values.size());

				double vmax = 0.0, vmin = 0.0;
				for(std::size_t i=0;i<n;++i){
					if( values[i] > vmax ) vmax = values[i];
					if( values[i] < vmin ) vmin = values[i];
				}
				if( vmax <= vmin ) vmax = vmin + 1.0;

				// categorical x scale: one tick per category, label = name.
				impl::scale_t sx;
				sx.lo = -0.5; sx.hi = double(n) - 0.5;
				for(std::size_t i=0;i<n;++i){ sx.ticks.push_back(double(i)); sx.labels.push_back(labels[i]); }
				impl::scale_t sy = impl::make_scale(vmin, vmax, false, 10.0);

				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						const float y0 = py(0.0);
						const float cellw = (px(1.0) - px(0.0));
						const float bw = std::max(1.0f, cellw * 0.7f);
						for(std::size_t i=0;i<n;++i){
							float cx = px(double(i));
							float yt = py(values[i]);
							std::uint32_t col = th.series.empty()? th.fg
											: th.series[i % th.series.size()];
							attribute_t a; a.color = plot::attribute::color_t{ col };
							float top = std::min(yt, y0);
							float hgt = std::abs(y0 - yt);
							c.rect(cx - bw*0.5f, top, bw, hgt, 0, 0, a);
						}
					});
			}, opts);
		}
	};
}

namespace plot {
	template <class... opt_t>
	impl::bar_view<opt_t...> bar(std::vector<std::string> labels,
			std::vector<double> values, opt_t... opts){
		return impl::bar_view<opt_t...>{ std::move(labels), std::move(values),
				std::make_tuple(opts...) };
	}
}
#endif
