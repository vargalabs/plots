/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Exercises the popular VS Code theme presets added for issue #14. For every
 * preset this renders a small line chart (both via the per-call plot::use{...}
 * override and via the global plot::theme(...) default) into an in-memory
 * stream, asserts the result is a well-formed standalone <svg> ... </svg>
 * document, and asserts the theme's own background colour (uppercase hex) is
 * painted. The Solarized presets are still checked to keep prior behaviour.
 */
#include <plot/all>

#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>

namespace {
	bool well_formed(const std::string& s){
		return s.rfind("<svg", 0) == 0 && s.find("</svg>") != std::string::npos;
	}
	// the drivers paint colours as uppercase #RRGGBB hex.
	std::string hex(std::uint32_t c){
		static const char* d = "0123456789ABCDEF";
		std::string s = "#";
		for(int sh = 20; sh >= 0; sh -= 4) s += d[(c >> sh) & 0xF];
		return s;
	}
}

int main(){
	std::vector<double> xs{ 1, 2, 3, 4, 5 };
	std::vector<double> ys{ 2, 1, 4, 3, 6 };

	struct named { const char* name; const plot::theme_t& t; };
	const std::vector<named> presets = {
		{ "solarized_dark",  plot::solarized_dark  },
		{ "solarized_light", plot::solarized_light },
		{ "dark_plus",       plot::dark_plus       },
		{ "light_plus",      plot::light_plus      },
		{ "monokai",         plot::monokai         },
		{ "dracula",         plot::dracula         },
		{ "nord",            plot::nord            },
		{ "one_dark",        plot::one_dark        },
		{ "gruvbox_dark",    plot::gruvbox_dark    },
		{ "gruvbox_light",   plot::gruvbox_light   },
		{ "tomorrow_night",  plot::tomorrow_night  },
		{ "night_owl",       plot::night_owl       },
		{ "material",        plot::material        },
		{ "tokyo_night",     plot::tokyo_night     },
	};

	int code = 1;   // distinct non-zero exit per preset on failure
	for(const auto& p : presets){
		const std::string bg = hex(p.t.bg);

		// ---- per-call override ----------------------------------------------
		{
			std::ostringstream os;
			plot::line(os, xs, ys, plot::use{ p.t },
					plot::title(p.name), plot::width{400}, plot::height{260});
			const std::string out = os.str();
			if(!well_formed(out)) return code;
			if(out.find("<polyline") == std::string::npos) return code + 1;
			if(out.find(bg) == std::string::npos) return code + 2;   // theme bg painted
		}

		// ---- global default -------------------------------------------------
		{
			plot::theme(p.t);
			std::ostringstream os;
			plot::scatter(os, xs, ys,
					plot::title(p.name), plot::width{400}, plot::height{260});
			const std::string out = os.str();
			if(!well_formed(out)) return code + 3;
			if(out.find("<circle") == std::string::npos) return code + 4;
			if(out.find(bg) == std::string::npos) return code + 5;
		}

		code += 6;
	}

	return 0;
}
