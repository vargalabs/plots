/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Empty tag types used to identify the order-independent named arguments of the
 * SVG plotting layer. Ported from plot:: into plot::. Dependency-free.
 */
#ifndef PLOT_TAGS_HPP
#define PLOT_TAGS_HPP

namespace plot::tag {
	struct width_t{};
	struct height_t{};
	struct position_t{};
	struct font_t{};
	struct color_t{};
	struct layout_t{};
	struct stroke_t{};
	struct opacity_t{};
	struct marker_t{};
	namespace axis {
		struct x_t{};
		struct y_t{};
	}
	struct text_t{};
	struct title_t{};
	struct footnote_t{};
	struct legend_t{};
	struct id_t{};
	struct style_t{};
	struct degree_t{};
	struct radian_t{};
	struct cw_t{};
	struct ccw_t{};
	struct horizontal_t{};
	struct vertical_t{};
	struct margin_t{};
	// GR.jl-style line/scatter named arguments
	struct xlabel_t{};
	struct ylabel_t{};
	struct xlog_t{};
	struct ylog_t{};
	struct theme_t{};   // per-call theme override (plot::use{...})
	// deferred-plot composition (plot::view + plot::grid)
	struct view_t{};    // identifies a deferred view in a variadic grid(...)
	struct rows_t{};    // grid row count
	struct cols_t{};    // grid column count
	// new plot-type named arguments (issue #6)
	struct bins_t{};      // histogram bin count
	struct bandwidth_t{}; // KDE bandwidth
	struct levels_t{};    // contour iso-level count
	struct donut_t{};     // pie inner-radius fraction (0 = full pie)
	struct hex_t{};       // hexagonal heatmap style marker
	struct wireframe_t{}; // outline-only bars/bins (no fill) — issue #13
}
#endif
