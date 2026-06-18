/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * plot::graph(nodes, edges, opts...) — a deferred view (plot::view) drawing a
 * node-link diagram. `nodes` is a std::vector<std::string> of labels; `edges` is
 * a std::vector<std::pair<std::size_t,std::size_t>> of node-index pairs.
 *
 * Layout: a simple CIRCULAR layout — nodes are placed evenly on a circle
 * inscribed in the cell. (A force-directed / spring layout would give nicer
 * spacing for dense graphs and can be added later behind a layout option; the
 * circular layout is deterministic, dependency-free and needs no iteration.)
 * Edges draw as <line>, nodes as <circle> + a <text> label; node colours cycle
 * theme.series.
 *
 * Built on the #5 view pattern (draws straight into the cell, no cartesian
 * panel). Dependency-free.
 */
#ifndef PLOT_GRAPH_HPP
#define PLOT_GRAPH_HPP

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
	struct graph_view {
		using value_type = tag::view_t;
		std::vector<std::string> nodes;
		std::vector<std::pair<std::size_t,std::size_t>> edges;
		std::tuple<opt_t...> opts;

		std::pair<std::size_t,std::size_t> natural() const {
			return std::apply([](const auto&... o){
				return impl::natural_size(std::size_t{500}, std::size_t{500}, o...); }, opts);
		}
		void draw_into(canvas_t& cv, float x, float y, float w, float h) const {
			std::apply([&](const auto&... o){
				const theme_t& th = impl::resolve_theme(o...);
				auto [title, xl, yl] = impl::texts(o...);
				using attribute_t = plot::attribute::element_t;

				cv.group(static_cast<std::size_t>(x), static_cast<std::size_t>(y), attribute_t{},
					[&](){
						// background panel for the cell.
						{ attribute_t bg; bg.color = plot::attribute::color_t{ th.bg };
						  cv.rect(0,0, w, h, 0,0, bg); }
						float top = title.empty() ? 8.0f : 26.0f;
						if( !title.empty() ){
							attribute_t a; a.color = plot::attribute::color_t{ th.fg };
							a.font = plot::attribute::font_t{"Arial, sans-serif", "bold", 13u};
							a.align = plot::attribute::align_t::center;
							cv.text(title, std::size_t(w/2), std::size_t(16), a);
						}
						const std::size_t n = nodes.size();
						if( n == 0 ) return;
						float cx = w*0.5f, cy = top + (h - top)*0.5f;
						float R = std::max(10.0f, std::min(w, h - top)*0.5f - 28.0f);

						auto pos = [&](std::size_t i)->std::pair<float,float>{
							double ang = -1.5707963267948966 + 2.0*3.141592653589793*double(i)/double(n);
							return { cx + R*float(std::cos(ang)), cy + R*float(std::sin(ang)) };
						};

						// edges first so nodes sit on top.
						attribute_t ea; ea.color = plot::attribute::color_t{ th.grid };
						ea.stroke = plot::attribute::stroke_t{1.0f, 1.2f, {}, {}, {}};
						for(auto [a,b] : edges){
							if( a >= n || b >= n ) continue;
							auto [ax,ay] = pos(a); auto [bx,by] = pos(b);
							cv.line(ax, ay, bx, by, ea);
						}
						// nodes + labels.
						for(std::size_t i=0;i<n;++i){
							auto [nx,ny] = pos(i);
							std::uint32_t col = th.series.empty()? th.fg
											: th.series[i % th.series.size()];
							attribute_t na; na.color = plot::attribute::color_t{ col };
							cv.circle(nx, ny, 7.0f, na);
							attribute_t ta; ta.color = plot::attribute::color_t{ th.fg };
							ta.font = plot::attribute::font_t{"Arial, sans-serif", "normal", 10u};
							ta.align = plot::attribute::align_t::center;
							// push labels radially outward so they clear the node disc.
							float lx = nx + (nx - cx)*0.16f, ly = ny + (ny - cy)*0.16f;
							cv.text(nodes[i], std::size_t(lx < 0 ? 0 : lx), std::size_t(ly + 3), ta);
						}
					});
			}, opts);
		}
	};
}

namespace plot {
	template <class... opt_t>
	impl::graph_view<opt_t...> graph(std::vector<std::string> nodes,
			std::vector<std::pair<std::size_t,std::size_t>> edges, opt_t... opts){
		return impl::graph_view<opt_t...>{ std::move(nodes), std::move(edges),
				std::make_tuple(opts...) };
	}
}
#endif
