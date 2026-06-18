/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Exercises deferred plots (plot::view) + figure composition (plot::grid).
 * Renders a 2×2 grid (line, scatter, heatmap, multi-series line) into an
 * in-memory stream and asserts:
 *   - exactly ONE outer <svg> document wraps the whole figure;
 *   - the expected number of cell translate-groups are present (one per panel,
 *     plus the heatmap's translate+scale fit group);
 *   - the subplot primitives appear: <polyline> (line), <circle> (scatter),
 *     <rect> (panels + heatmap cells);
 *   - auto-layout (no rows/cols) still produces one <svg> with all primitives;
 *   - a standalone plot::line(x,y).save(...)-style view renders a valid .svg.
 */
#include <plot/all>

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cstddef>

namespace {
	std::size_t count(const std::string& hay, const std::string& needle){
		std::size_t n = 0;
		for(std::size_t p = hay.find(needle); p != std::string::npos;
				p = hay.find(needle, p + needle.size())) ++n;
		return n;
	}
	bool well_formed(const std::string& s){
		return s.rfind("<svg", 0) == 0 && s.find("</svg>") != std::string::npos;
	}
}

int main(){
	plot::theme(plot::solarized_dark);

	std::vector<double> xs{ 1, 2, 3, 4, 5 };
	std::vector<double> ys{ 2, 1, 4, 3, 6 };

	std::vector<std::string> hx{ "a", "b", "c" };
	std::vector<std::string> hy{ "p", "q", "r" };
	std::vector<double> z{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	plot::mat<double> field{ z.data(), 3, 3 };

	// ---- 2×2 grid: line, scatter, heatmap, multi-series line ----------------
	{
		std::ostringstream os;
		plot::grid(os, plot::rows{2}, plot::cols{2},
			plot::line(xs, ys, plot::title("L")),
			plot::scatter(xs, ys, plot::title("S")),
			plot::heatmap(field, plot::axis::x(hx), plot::axis::y(hy),
				plot::title(std::string("H"))),
			plot::line(xs,
				{ { "a", std::vector<double>{1,2,3,4,5} },
				  { "b", std::vector<double>{5,4,3,2,1} } },
				plot::title("M")),
			plot::width{800}, plot::height{600});
		const std::string out = os.str();

		if(!well_formed(out)) return 1;
		// exactly one outer <svg> document and one close.
		if(count(out, "<svg")   != 1) return 2;
		if(count(out, "</svg>") != 1) return 3;

		// translate groups: one per panel (4) + the heatmap's fit group (1) plus
		// the heatmap's own internal content group → at least 5 cell groups.
		if(count(out, "transform=\"translate") < 5) return 4;

		// the panel cells anchor at their cell corners (cellW=400, cellH=300).
		// line@(0,0), scatter@(400,0), multi-line@(400,300) are integral-anchored
		// panel groups; the heatmap@(0,300) is centred inside its cell via a
		// translate+scale fit group (asserted below), so it isn't a bare corner.
		if(out.find("translate(0 0)")     == std::string::npos) return 5;
		if(out.find("translate(400 0)")   == std::string::npos) return 6;
		if(out.find("translate(400 300)") == std::string::npos) return 7;

		// subplot primitives: line panel -> polyline (the multi-series adds 2 more
		// so >= 3 total), scatter -> circle, panels + heatmap -> rect.
		if(count(out, "<polyline") < 3) return 8;
		if(out.find("<circle")  == std::string::npos) return 9;
		if(out.find("<rect")    == std::string::npos) return 10;

		// the heatmap is fitted into its cell via a translate+scale group.
		if(out.find("scale(") == std::string::npos) return 11;
	}

	// ---- auto-layout (no rows/cols): cols=ceil(sqrt(3))=2, rows=2 -----------
	{
		std::ostringstream os;
		plot::grid(os,
			plot::line(xs, ys),
			plot::scatter(xs, ys),
			plot::line(xs, ys),
			plot::width{600}, plot::height{400});
		const std::string out = os.str();
		if(!well_formed(out)) return 12;
		if(count(out, "<svg") != 1) return 13;
		if(out.find("<polyline") == std::string::npos) return 14;
		if(out.find("<circle")   == std::string::npos) return 15;
	}

	// ---- standalone deferred view: plot::save(file, plot::line(x,y)) --------
	{
		std::ostringstream os;
		plot::render(os, plot::line(xs, ys, plot::title("standalone"),
				plot::width{400}, plot::height{260}));
		const std::string out = os.str();
		if(!well_formed(out)) return 16;
		if(out.find("<polyline") == std::string::npos) return 17;

		const std::string file = "plot_grid_standalone.svg";
		plot::save(file, plot::scatter(xs, ys, plot::width{320}, plot::height{220}));
		std::ifstream ifs(file);
		std::string body((std::istreambuf_iterator<char>(ifs)),
				std::istreambuf_iterator<char>());
		if(!well_formed(body)) return 18;
		if(body.find("<circle") == std::string::npos) return 19;
	}

	return 0;
}
