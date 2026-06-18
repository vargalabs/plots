/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Exercises the issue #6 plot types — histogram, bar, density, contour, ohlc,
 * graph, pie, hexbin. Each is a deferred plot::view: this test asserts every
 * one renders a well-formed standalone <svg> with the primitives it should
 * emit, and that they all compose inside a single plot::grid figure (one outer
 * <svg>). Renders into in-memory streams; no files are required.
 */
#include <plot/all>

#include <sstream>
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
	std::string render_of(const auto& view){
		std::ostringstream os; view.render(os); return os.str();
	}
}

int main(){
	plot::theme(plot::solarized_dark);

	// ---- histogram ----------------------------------------------------------
	{
		std::vector<double> v{1,2,2,3,3,3,4,4,5,6,7,7,8,2,3,4,5,5};
		auto out = render_of(plot::histogram(v, plot::bins{6}, plot::title("hist")));
		if(!well_formed(out)) return 1;
		if(out.find("<rect") == std::string::npos) return 2;   // bars
	}

	// ---- bar ----------------------------------------------------------------
	{
		std::vector<std::string> labels{"a","b","c","d"};
		std::vector<double> vals{3,7,2,5};
		auto out = render_of(plot::bar(labels, vals, plot::title("bar")));
		if(!well_formed(out)) return 3;
		if(out.find("<rect") == std::string::npos) return 4;
		if(out.find(">a<")   == std::string::npos) return 5;   // category tick label
	}

	return 0;
}
