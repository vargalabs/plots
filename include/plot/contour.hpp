/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::contour(grid, opts...) — a deferred view (plot::view) drawing iso-contour
 * lines of a row-major plot::mat<double> field via the marching-squares
 * algorithm. plot::levels{n} sets the number of iso-levels (default 8); each
 * level is coloured by its fraction along theme.gradient. Every cell that the
 * iso-line crosses contributes one short <polyline> segment (linear interpolation
 * on the crossed edges).
 *
 * Built on the #5 view pattern + impl::render_panel_into. Dependency-free.
 */
#ifndef PLOT_CONTOUR_HPP
#define PLOT_CONTOUR_HPP

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
	// number of iso-levels for plot::contour (default 8).
	struct levels { using value_type = tag::levels_t; std::size_t value; };
}

namespace plot::impl {
	template <class... opt_t>
	std::size_t levels_of(const opt_t&... opts){
		using lv_t = typename arg::tpos<tag::levels_t, opt_t...>;
		if constexpr( lv_t::present ){
			auto tuple = std::forward_as_tuple(opts...);
			std::size_t l = std::get<lv_t::position>(tuple).value;
			return l ? l : 1;
		}
		return 8;
	}

	template <class... opt_t>
	struct contour_view {
		using value_type = tag::view_t;
		std::vector<double> data;        // owned copy, row-major
		std::size_t rows = 0, cols = 0;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{640}, std::size_t{400}, o...); }, opts);
		}
		double at(std::size_t i, std::size_t j) const { return data[i*cols + j]; }

		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				const std::size_t nlev = impl::levels_of(o...);

				double lo =  std::numeric_limits<double>::infinity();
				double hi = -std::numeric_limits<double>::infinity();
				for(double d : data){ if(d<lo) lo=d; if(d>hi) hi=d; }
				if( !(lo<=hi) ){ lo=0; hi=1; }
				if( hi <= lo ) hi = lo + 1.0;

				impl::scale_t sx = impl::make_scale(0.0, double(cols>0?cols-1:0), false, 10.0);
				impl::scale_t sy = impl::make_scale(0.0, double(rows>0?rows-1:0), false, 10.0);
				impl::render_panel_into(cv, x, y,
					static_cast<std::size_t>(w), static_cast<std::size_t>(h),
					th, sx, sy, title, xl, yl, {},
					[&](impl::canvas_t& c, auto px, auto py){
						using attribute_t = plot::attribute::element_t;
						if( rows < 2 || cols < 2 ) return;
						// linear interpolation of the crossing point between two
						// grid corners (a,b) at field values (va,vb) for level L.
						auto lerp = [](double a, double b, double va, double vb, double L){
							double d = (vb - va);
							double t = (std::abs(d) < 1e-12) ? 0.5 : (L - va)/d;
							if( t < 0 ) t = 0; else if( t > 1 ) t = 1;
							return a + t*(b - a);
						};
						for(std::size_t k=0;k<nlev;++k){
							double frac = (nlev==1) ? 0.5 : double(k)/double(nlev-1);
							double L = lo + frac*(hi-lo);
							std::uint32_t col = impl::gradient3(th.gradient, frac);
							attribute_t a; a.color = plot::attribute::color_t{ col };
							a.stroke = plot::attribute::stroke_t{1.0f, 1.4f, {}, {}, {}};
							for(std::size_t i=0;i+1<rows;++i)
							for(std::size_t j=0;j+1<cols;++j){
								// corner values: TL,TR,BR,BL of cell (i,j)
								double v00 = at(i,   j), v01 = at(i,   j+1);
								double v11 = at(i+1, j+1), v10 = at(i+1, j);
								// marching-squares case index.
								int cs = (v00>L?1:0) | (v01>L?2:0) | (v11>L?4:0) | (v10>L?8:0);
								if( cs==0 || cs==15 ) continue;
								// edge crossing points in grid coords (col=x, row=y):
								// top edge between (i,j)-(i,j+1), right (i,j+1)-(i+1,j+1),
								// bottom (i+1,j)-(i+1,j+1), left (i,j)-(i+1,j).
								double tx = lerp(double(j), double(j+1), v00, v01, L), ty = double(i);
								double rx = double(j+1), ry = lerp(double(i), double(i+1), v01, v11, L);
								double bx = lerp(double(j), double(j+1), v10, v11, L), by = double(i+1);
								double lx = double(j), ly = lerp(double(i), double(i+1), v00, v10, L);
								auto seg = [&](double ax,double ay,double bx2,double by2){
									std::vector<float> X{ px(ax), px(bx2) };
									std::vector<float> Y{ py(ay), py(by2) };
									c.poly_line(X, Y, a);
								};
								switch(cs){
									case 1: case 14: seg(lx,ly, tx,ty); break;
									case 2: case 13: seg(tx,ty, rx,ry); break;
									case 3: case 12: seg(lx,ly, rx,ry); break;
									case 4: case 11: seg(rx,ry, bx,by); break;
									case 6: case 9:  seg(tx,ty, bx,by); break;
									case 7: case 8:  seg(lx,ly, bx,by); break;
									case 5: seg(lx,ly, tx,ty); seg(rx,ry, bx,by); break;
									case 10: seg(tx,ty, rx,ry); seg(lx,ly, bx,by); break;
									default: break;
								}
							}
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
	impl::contour_view<opt_t...> contour(const mat<T>& grid, opt_t... opts){
		impl::contour_view<opt_t...> v{ {}, grid.rows, grid.cols, std::make_tuple(opts...) };
		v.data.assign(grid.begin(), grid.end());
		return v;
	}
}
#endif
