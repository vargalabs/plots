/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Colour themes for the SVG plotting layer. A `theme_t` bundles the structural
 * colours (background, panel, foreground text, grid, axis), a categorical series
 * palette (cycled by line/scatter), and a 3-stop continuous gradient (wired into
 * the continuous heatmap). Two Solarized presets plus a set of popular VS Code
 * palettes (Dark+/Light+, Monokai, Dracula, Nord, One Dark, Gruvbox dark/light,
 * Tomorrow Night, Night Owl, Material, Tokyo Night) ship inline; a process-global
 * default current theme can be set with `plot::theme(...)`, and any driver call
 * may override it for a single render with the `plot::use{...}` named argument.
 * Dependency-free (standard library only).
 */
#ifndef PLOT_THEME_HPP
#define PLOT_THEME_HPP

#include <cstdint>
#include <array>
#include <vector>

#include "tags.hpp"

namespace plot {
	struct theme_t {
		std::uint32_t bg;                       // page / canvas background
		std::uint32_t panel;                    // plotting-area panel
		std::uint32_t fg;                       // default text / foreground
		std::uint32_t grid;                     // grid lines / axis ticks
		std::uint32_t axis;                     // axis lines
		std::vector<std::uint32_t> series;      // categorical palette (cycled)
		std::array<std::uint32_t,3> gradient;   // continuous 3-stop gradient
	};

	// Solarized (Ethan Schoonover) — exact hex.
	inline const theme_t solarized_dark = {
		/*bg   base03*/ 0x002b36,
		/*panel base02*/0x073642,
		/*fg   base0 */ 0x839496,
		/*grid base01*/ 0x586e75,
		/*axis base01*/ 0x586e75,
		/*series*/ { 0x268bd2, 0x859900, 0xb58900, 0xcb4b16,
		             0xdc322f, 0xd33682, 0x6c71c4, 0x2aa198 },
		/*gradient*/ { 0x268bd2, 0xb58900, 0xdc322f }
	};

	inline const theme_t solarized_light = {
		/*bg   base3 */ 0xfdf6e3,
		/*panel base2*/ 0xeee8d5,
		/*fg   base00*/ 0x657b83,
		/*grid base1 */ 0x93a1a1,
		/*axis base00*/ 0x657b83,
		/*series*/ { 0x268bd2, 0x859900, 0xb58900, 0xcb4b16,
		             0xdc322f, 0xd33682, 0x6c71c4, 0x2aa198 },
		/*gradient*/ { 0x268bd2, 0xb58900, 0xdc322f }
	};

	// ---- Popular VS Code theme presets (canonical palettes) -----------------

	// VS Code Dark+ (default dark).
	inline const theme_t dark_plus = {
		/*bg   */ 0x1E1E1E,
		/*panel*/ 0x252526,
		/*fg   */ 0xD4D4D4,
		/*grid */ 0x3C3C3C,
		/*axis */ 0x808080,
		/*series*/ { 0x569CD6, 0x4EC9B0, 0xCE9178, 0xDCDCAA,
		             0xC586C0, 0x9CDCFE, 0xB5CEA8, 0xD16969 },
		/*gradient*/ { 0x569CD6, 0xDCDCAA, 0xCE9178 }
	};

	// VS Code Light+ (default light).
	inline const theme_t light_plus = {
		/*bg   */ 0xFFFFFF,
		/*panel*/ 0xF3F3F3,
		/*fg   */ 0x1E1E1E,
		/*grid */ 0xD4D4D4,
		/*axis */ 0x808080,
		/*series*/ { 0x0000FF, 0x267F99, 0xA31515, 0x795E26,
		             0xAF00DB, 0x001080, 0x098658, 0xCD3131 },
		/*gradient*/ { 0x0000FF, 0x795E26, 0xA31515 }
	};

	// Monokai.
	inline const theme_t monokai = {
		/*bg   */ 0x272822,
		/*panel*/ 0x3E3D32,
		/*fg   */ 0xF8F8F2,
		/*grid */ 0x49483E,
		/*axis */ 0x75715E,
		/*series*/ { 0xF92672, 0xA6E22E, 0x66D9EF, 0xFD971F,
		             0xAE81FF, 0xE6DB74, 0x75715E, 0xF8F8F0 },
		/*gradient*/ { 0x66D9EF, 0xA6E22E, 0xF92672 }
	};

	// Dracula.
	inline const theme_t dracula = {
		/*bg   */ 0x282A36,
		/*panel*/ 0x44475A,
		/*fg   */ 0xF8F8F2,
		/*grid */ 0x44475A,
		/*axis */ 0x6272A4,
		/*series*/ { 0xFF79C6, 0xBD93F9, 0x50FA7B, 0x8BE9FD,
		             0xFFB86C, 0xFF5555, 0xF1FA8C, 0x6272A4 },
		/*gradient*/ { 0x8BE9FD, 0xBD93F9, 0xFF79C6 }
	};

	// Nord.
	inline const theme_t nord = {
		/*bg   */ 0x2E3440,
		/*panel*/ 0x3B4252,
		/*fg   */ 0xD8DEE9,
		/*grid */ 0x434C5E,
		/*axis */ 0x4C566A,
		/*series*/ { 0x88C0D0, 0x81A1C1, 0xA3BE8C, 0xEBCB8B,
		             0xBF616A, 0xB48EAD, 0xD08770, 0x8FBCBB },
		/*gradient*/ { 0x88C0D0, 0xEBCB8B, 0xBF616A }
	};

	// Atom One Dark.
	inline const theme_t one_dark = {
		/*bg   */ 0x282C34,
		/*panel*/ 0x21252B,
		/*fg   */ 0xABB2BF,
		/*grid */ 0x3B4048,
		/*axis */ 0x5C6370,
		/*series*/ { 0x61AFEF, 0x98C379, 0xE06C75, 0xE5C07B,
		             0xC678DD, 0x56B6C2, 0xD19A66, 0xABB2BF },
		/*gradient*/ { 0x61AFEF, 0x98C379, 0xE06C75 }
	};

	// Gruvbox Dark.
	inline const theme_t gruvbox_dark = {
		/*bg   */ 0x282828,
		/*panel*/ 0x3C3836,
		/*fg   */ 0xEBDBB2,
		/*grid */ 0x504945,
		/*axis */ 0x665C54,
		/*series*/ { 0x83A598, 0xB8BB26, 0xFB4934, 0xFABD2F,
		             0xD3869B, 0x8EC07C, 0xFE8019, 0xEBDBB2 },
		/*gradient*/ { 0x83A598, 0xFABD2F, 0xFB4934 }
	};

	// Gruvbox Light.
	inline const theme_t gruvbox_light = {
		/*bg   */ 0xFBF1C7,
		/*panel*/ 0xEBDBB2,
		/*fg   */ 0x3C3836,
		/*grid */ 0xD5C4A1,
		/*axis */ 0x7C6F64,
		/*series*/ { 0x076678, 0x79740E, 0x9D0006, 0xB57614,
		             0x8F3F71, 0x427B58, 0xAF3A03, 0x3C3836 },
		/*gradient*/ { 0x076678, 0xB57614, 0x9D0006 }
	};

	// Tomorrow Night.
	inline const theme_t tomorrow_night = {
		/*bg   */ 0x1D1F21,
		/*panel*/ 0x282A2E,
		/*fg   */ 0xC5C8C6,
		/*grid */ 0x373B41,
		/*axis */ 0x969896,
		/*series*/ { 0x81A2BE, 0xB5BD68, 0xCC6666, 0xF0C674,
		             0xB294BB, 0x8ABEB7, 0xDE935F, 0xC5C8C6 },
		/*gradient*/ { 0x81A2BE, 0xB5BD68, 0xCC6666 }
	};

	// Night Owl (Sarah Drasner).
	inline const theme_t night_owl = {
		/*bg   */ 0x011627,
		/*panel*/ 0x0B2942,
		/*fg   */ 0xD6DEEB,
		/*grid */ 0x122D42,
		/*axis */ 0x5F7E97,
		/*series*/ { 0x82AAFF, 0xADDB67, 0xEF5350, 0xFFCB8B,
		             0xC792EA, 0x7FDBCA, 0xF78C6C, 0xD6DEEB },
		/*gradient*/ { 0x82AAFF, 0xADDB67, 0xEF5350 }
	};

	// Material Theme (Mattia Astorino).
	inline const theme_t material = {
		/*bg   */ 0x263238,
		/*panel*/ 0x2E3C43,
		/*fg   */ 0xEEFFFF,
		/*grid */ 0x37474F,
		/*axis */ 0x546E7A,
		/*series*/ { 0x82AAFF, 0xC3E88D, 0xF07178, 0xFFCB6B,
		             0xC792EA, 0x89DDFF, 0xF78C6C, 0xEEFFFF },
		/*gradient*/ { 0x89DDFF, 0xC3E88D, 0xF07178 }
	};

	// Tokyo Night.
	inline const theme_t tokyo_night = {
		/*bg   */ 0x1A1B26,
		/*panel*/ 0x24283B,
		/*fg   */ 0xA9B1D6,
		/*grid */ 0x2F334D,
		/*axis */ 0x565F89,
		/*series*/ { 0x7AA2F7, 0x9ECE6A, 0xF7768E, 0xE0AF68,
		             0xBB9AF7, 0x7DCFFF, 0xFF9E64, 0xA9B1D6 },
		/*gradient*/ { 0x7AA2F7, 0x9ECE6A, 0xF7768E }
	};

	// process-global default current theme (header-only inline definition).
	inline theme_t current_theme = solarized_dark;

	// setter for the global default.
	inline void theme(const theme_t& t){ current_theme = t; }

	// per-call override: `plot::use{ plot::solarized_light }`. Carries a theme as
	// a named argument that line/scatter/heatmap honour over the global default.
	struct use {
		using value_type = tag::theme_t;
		theme_t value;
	};
}
#endif
