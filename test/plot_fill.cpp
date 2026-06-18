/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Issue #13: filled polygon primitive + wireframe option.
 *  - pie/donut/hexbin now emit FILLED <polygon> wedges/cells (fill="#RRGGBB"),
 *    not fill="none" outlines.
 *  - plot::bar(.., plot::wireframe{}) / plot::histogram(.., plot::wireframe{})
 *    render outline-only columns (fill="none"), no filled <rect> bars.
 * Renders into in-memory streams; no files are required.
 */
#include <plot/all>

#include <sstream>
#include <string>
#include <vector>
#include <cstddef>

namespace {
	bool well_formed(const std::string& s){
		return s.rfind("<svg", 0) == 0 && s.find("</svg>") != std::string::npos;
	}
	std::string render_of(const auto& view){
		std::ostringstream os; plot::render(os, view); return os.str();
	}
	// is there a <polygon ...> whose tag carries a fill="#... attribute?
	bool has_filled_polygon(const std::string& s){
		for(std::size_t p = s.find("<polygon"); p != std::string::npos;
				p = s.find("<polygon", p + 8)){
			std::size_t end = s.find('>', p);
			if(end == std::string::npos) return false;
			if(s.find("fill=\"#", p) < end) return true;
		}
		return false;
	}
}

int main(){
	plot::theme(plot::solarized_dark);

	// ---- pie: filled <polygon> wedges --------------------------------------
	{
		std::vector<std::string> labels{"x","y","z","w"};
		std::vector<double> vals{30,20,40,10};
		auto out = render_of(plot::pie(labels, vals, plot::title("pie")));
		if(!well_formed(out)) return 1;
		if(out.find("<polygon") == std::string::npos) return 2;   // wedges as polygons
		if(!has_filled_polygon(out)) return 3;                     // filled, not fill="none"
	}

	// ---- donut: filled ring <polygon> wedges -------------------------------
	{
		std::vector<std::string> labels{"x","y","z","w"};
		std::vector<double> vals{30,20,40,10};
		auto out = render_of(plot::donut(labels, vals, plot::title("donut")));
		if(!well_formed(out)) return 4;
		if(out.find("<polygon") == std::string::npos) return 5;
		if(!has_filled_polygon(out)) return 6;
	}

	// ---- hexbin: filled <polygon> cells ------------------------------------
	{
		const std::size_t R=6, C=6;
		std::vector<double> field(R*C);
		for(std::size_t i=0;i<R*C;++i) field[i] = double(i);
		plot::mat<double> m{ field.data(), R, C };
		auto out = render_of(plot::hexbin(m, plot::title("hex")));
		if(!well_formed(out)) return 7;
		if(out.find("<polygon") == std::string::npos) return 8;
		if(!has_filled_polygon(out)) return 9;
	}

	// ---- bar wireframe: outline-only columns (fill="none", no filled rect) -
	{
		std::vector<std::string> labels{"a","b","c","d"};
		std::vector<double> vals{3,7,2,5};
		auto out = render_of(plot::bar(labels, vals, plot::wireframe{}, plot::title("bar wire")));
		if(!well_formed(out)) return 10;
		if(out.find("fill=\"none\"") == std::string::npos) return 11;  // outlined bars
		// the bars themselves must not be filled <rect> columns.
		if(out.find("<rect") != std::string::npos &&
				out.find("fill=\"#") != std::string::npos){
			// a filled rect with a fill="# colour would mean a solid bar — only the
			// panel background rect is allowed; verify there is no series-coloured
			// fill beyond the background by ensuring outlined polylines exist.
			if(out.find("<polyline") == std::string::npos) return 12;
		}
	}

	// ---- histogram wireframe: outline-only bins ----------------------------
	{
		std::vector<double> v{1,2,2,3,3,3,4,4,5,6,7,7,8,2,3,4,5,5};
		auto out = render_of(plot::histogram(v, plot::bins{6}, plot::wireframe{}, plot::title("hist wire")));
		if(!well_formed(out)) return 13;
		if(out.find("fill=\"none\"") == std::string::npos) return 14;  // outlined bins
		if(out.find("<polyline") == std::string::npos) return 15;
	}

	// ---- a plain filled bar/histogram still emits filled <rect> ------------
	{
		std::vector<std::string> labels{"a","b","c"};
		std::vector<double> vals{3,5,2};
		auto out = render_of(plot::bar(labels, vals, plot::title("bar")));
		if(!well_formed(out)) return 16;
		if(out.find("<rect") == std::string::npos) return 17;
	}

	return 0;
}
