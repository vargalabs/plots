/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::hexbin(grid, opts...) — a deferred view (plot::view): a HEXAGONAL variant
 * of the continuous heatmap. Each cell of a row-major plot::mat<double> renders
 * as a flat-topped hexagon (a closed <polyline> of 6 vertices — no new canvas
 * method) coloured along theme.gradient by the cell's value normalised across the
 * grid's [min,max]. Alternate rows are offset by half a hex width for the classic
 * honeycomb tessellation.
 *
 * A separate header from heatmap.hpp (left untouched). Built on the #5 view
 * pattern (draws straight into the cell). Dependency-free.
 */
#ifndef PLOT_HEXBIN_HPP
#define PLOT_HEXBIN_HPP

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "tags.hpp"
#include "meta.hpp"
#include "attributes.hpp"
#include "canvas.hpp"
#include "theme.hpp"
#include "gr.hpp"
#include "heatmap.hpp"   // plot::mat<T>
#include "view.hpp"

namespace plot {
	// style marker selecting the hexagonal heatmap layout (e.g. heatmap(..,hex{})).
	struct hex { using value_type = tag::hex_t; };
}

namespace plot::impl {
	template <class... opt_t>
	struct hexbin_view {
		using value_type = tag::view_t;
		std::vector<double> data;     // owned, row-major
		std::size_t rows = 0, cols = 0;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{560}, std::size_t{460}, o...); }, opts);
		}
		double at(std::size_t i, std::size_t j) const { return data[i*cols + j]; }

		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				using attribute_t = plot::attribute::element_t;

				double lo =  std::numeric_limits<double>::infinity();
				double hi = -std::numeric_limits<double>::infinity();
				for(double d : data){ if(d<lo) lo=d; if(d>hi) hi=d; }
				if( !(lo<=hi) ){ lo=0; hi=1; }
				const double span = (hi>lo) ? (hi-lo) : 1.0;

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
						if( rows == 0 || cols == 0 ) return;

						const float pad = 8.0f;
						const float availW = w - 2*pad;
						const float availH = (h - top) - 2*pad;
						// pointy-top hex grid: horizontal pitch = sqrt(3)*r, with a
						// half-pitch offset on odd rows; vertical pitch = 1.5*r.
						// solve r from the available box for (cols + 0.5) columns and
						// (rows*1.5 + 0.5) rows.
						const float sqrt3 = 1.7320508075688772f;
						float rW = availW / (sqrt3 * (float(cols) + 0.5f));
						float rH = availH / (1.5f * float(rows) + 0.5f);
						float r = std::max(2.0f, std::min(rW, rH));
						float hpitch = sqrt3 * r;
						float vpitch = 1.5f * r;
						float ox = pad + hpitch*0.5f;
						float oy = top + pad + r;

						for(std::size_t i=0;i<rows;++i)
						for(std::size_t j=0;j<cols;++j){
							float cx = ox + float(j)*hpitch + ((i & 1) ? hpitch*0.5f : 0.0f);
							float cy = oy + float(i)*vpitch;
							double t = (at(i,j) - lo)/span;
							std::uint32_t col = impl::gradient3(th.gradient, t);
							// pointy-top hexagon: 6 vertices at 30°,90°,...,330°.
							std::vector<float> X, Y;
							for(int k=0;k<6;++k){
								double a = (3.141592653589793/180.0)*(60.0*double(k) + 30.0);
								X.push_back(cx + r*float(std::cos(a)));
								Y.push_back(cy + r*float(std::sin(a)));
							}
							X.push_back(X.front()); Y.push_back(Y.front());   // close
							attribute_t ha; ha.color = plot::attribute::color_t{ col };
							ha.stroke = plot::attribute::stroke_t{1.0f, 1.0f, {}, {}, {}};
							cv.poly_line(X, Y, ha);
						}
					});
			}, opts);
		}
		void render(std::ostream& os) const { impl::render_view(os, *this); }
		void save(const std::string& file) const { impl::save_view(file, *this); }
	};
}

namespace plot {
	template <class T, class... opt_t,
		class = std::enable_if_t<std::is_arithmetic_v<T>>>
	impl::hexbin_view<opt_t...> hexbin(const mat<T>& grid, opt_t... opts){
		impl::hexbin_view<opt_t...> v{ {}, grid.rows, grid.cols, std::make_tuple(opts...) };
		v.data.assign(grid.begin(), grid.end());
		return v;
	}
}
#endif
