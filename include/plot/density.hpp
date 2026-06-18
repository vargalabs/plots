/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::density(values, opts...) — a deferred view (plot::view) drawing a
 * Gaussian kernel-density estimate as a smooth polyline over an x-grid. The
 * bandwidth is plot::bandwidth{h}; the default is Silverman's rule of thumb
 * h = 1.06 * sigma * n^(-1/5).
 *
 * Built on the #5 view pattern + impl::render_panel_into. Dependency-free.
 */
#ifndef PLOT_DENSITY_HPP
#define PLOT_DENSITY_HPP

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
	// KDE bandwidth override (h <= 0 ⇒ fall back to Silverman's rule).
	struct bandwidth { using value_type = tag::bandwidth_t; double value; };
}

namespace plot::impl {
	template <class... opt_t>
	double bandwidth_of(const std::vector<double>& v, const opt_t&... opts){
		using bw_t = typename arg::tpos<tag::bandwidth_t, opt_t...>;
		if constexpr( bw_t::present ){
			auto tuple = std::forward_as_tuple(opts...);
			double h = std::get<bw_t::position>(tuple).value;
			if( h > 0 ) return h;
		}
		// Silverman: 1.06 * sigma * n^(-1/5).
		const std::size_t n = v.size();
		if( n < 2 ) return 1.0;
		double mean = 0; for(double d : v) mean += d; mean /= double(n);
		double var = 0; for(double d : v) var += (d-mean)*(d-mean); var /= double(n-1);
		double sigma = std::sqrt(var > 0 ? var : 1.0);
		double h = 1.06 * sigma * std::pow(double(n), -1.0/5.0);
		return h > 0 ? h : 1.0;
	}

	template <class... opt_t>
	struct density_view {
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
				const double bw = impl::bandwidth_of(values, o...);

				auto [vmn, vmx] = impl::minmax_of(values);
				// pad the grid by 3 bandwidths so the tails are visible.
				double xlo = vmn - 3.0*bw, xhi = vmx + 3.0*bw;
				if( xhi <= xlo ) xhi = xlo + 1.0;

				const std::size_t G = 200;
				std::vector<double> gx(G), gy(G);
				constexpr double two_pi = 6.283185307179586;
				const double inv = 1.0 / (std::sqrt(two_pi) * bw * double(values.size() ? values.size() : 1));
				double dmax = 0.0;
				for(std::size_t g=0; g<G; ++g){
					double xq = xlo + (xhi - xlo) * double(g)/double(G-1);
					double acc = 0.0;
					for(double v : values){
						double u = (xq - v)/bw;
						acc += std::exp(-0.5*u*u);
					}
					double dens = acc * inv;
					gx[g] = xq; gy[g] = dens;
					if( dens > dmax ) dmax = dens;
				}
				if( dmax <= 0.0 ) dmax = 1.0;

				impl::scale_t sx = impl::make_scale(xlo, xhi, false, 10.0);
				impl::scale_t sy = impl::make_scale(0.0, dmax, false, 10.0);
				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						std::vector<float> xpx, ypx;
						for(std::size_t g=0; g<G; ++g){ xpx.push_back(px(gx[g])); ypx.push_back(py(gy[g])); }
						attribute_t a;
						a.color = plot::attribute::color_t{ th.series.empty()? th.fg : th.series[0] };
						a.stroke = plot::attribute::stroke_t{1.0f, 1.8f, {}, {}, {}};
						c.poly_line(xpx, ypx, a);
					});
			}, opts);
		}
	};
}

namespace plot {
	template <class... opt_t>
	impl::density_view<opt_t...> density(std::vector<double> values, opt_t... opts){
		return impl::density_view<opt_t...>{ std::move(values), std::make_tuple(opts...) };
	}
}
#endif
