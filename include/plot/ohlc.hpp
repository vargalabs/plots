/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::ohlc(times, open, high, low, close, opts...) — a deferred view
 * (plot::view) drawing a candlestick chart from five parallel std::vector<double>
 * series. Each bar is a vertical <line> from low to high plus a body <rect>
 * spanning open↔close. Up bars (close >= open) and down bars use distinct theme
 * series colours (green = series[1], red = series[4] in the Solarized palette,
 * with foreground fallbacks).
 *
 * Built on the #5 view pattern + impl::render_panel_into. Dependency-free.
 */
#ifndef PLOT_OHLC_HPP
#define PLOT_OHLC_HPP

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "tags.hpp"
#include "meta.hpp"
#include "attributes.hpp"
#include "canvas.hpp"
#include "theme.hpp"
#include "gr.hpp"
#include "view.hpp"

namespace plot::impl {
	template <class... opt_t>
	struct ohlc_view {
		using value_type = tag::view_t;
		std::vector<double> t, o, hi, lo, c;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... oo){
				return impl::natural_size(std::size_t{640}, std::size_t{400}, oo...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... oo){
				const theme_t& th = impl::resolve_theme(oo...);
				auto [title, xl, yl] = impl::texts(oo...);
				const std::size_t n = std::min({t.size(), o.size(), hi.size(), lo.size(), c.size()});

				auto [xmn, xmx] = impl::minmax_of(t);
				double ymn =  std::numeric_limits<double>::infinity();
				double ymx = -std::numeric_limits<double>::infinity();
				for(std::size_t i=0;i<n;++i){
					ymn = std::min({ymn, lo[i]}); ymx = std::max({ymx, hi[i]});
				}
				if( !(ymn<=ymx) ){ ymn=0; ymx=1; }

				impl::scale_t sx = impl::make_scale(xmn, xmx, false, 10.0);
				impl::scale_t sy = impl::make_scale(ymn, ymx, false, 10.0);

				// up = green-ish series[1], down = red-ish series[4], with fallbacks.
				std::uint32_t up   = th.series.size() > 1 ? th.series[1] : th.fg;
				std::uint32_t down = th.series.size() > 4 ? th.series[4] : th.fg;

				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& cc, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						// candle half-width in pixels: ~40% of the inter-bar spacing.
						float spacing = (n > 1) ? std::abs(px(sx.xform(t[1])) - px(sx.xform(t[0]))) : 10.0f;
						float bw = std::max(2.0f, spacing * 0.4f);
						for(std::size_t i=0;i<n;++i){
							bool upbar = c[i] >= o[i];
							std::uint32_t col = upbar ? up : down;
							float X = px(sx.xform(t[i]));
							// high-low wick.
							attribute_t la; la.color = plot::attribute::color_t{ col };
							la.stroke = plot::attribute::stroke_t{1.0f, 1.2f, {}, {}, {}};
							cc.line(X, py(hi[i]), X, py(lo[i]), la);
							// open-close body.
							float yo = py(o[i]), yc = py(c[i]);
							float top = std::min(yo, yc);
							float bh  = std::max(1.0f, std::abs(yc - yo));
							attribute_t ra; ra.color = plot::attribute::color_t{ col };
							cc.rect(X - bw, top, bw*2.0f, bh, 0, 0, ra);
						}
					});
			}, opts);
		}
	};
}

namespace plot {
	template <class... opt_t>
	impl::ohlc_view<opt_t...> ohlc(std::vector<double> times,
			std::vector<double> open, std::vector<double> high,
			std::vector<double> low, std::vector<double> close, opt_t... opts){
		return impl::ohlc_view<opt_t...>{ std::move(times), std::move(open),
				std::move(high), std::move(low), std::move(close),
				std::make_tuple(opts...) };
	}
}
#endif
