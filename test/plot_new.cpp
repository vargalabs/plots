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
#include <cmath>
#include <utility>

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

	// ---- density (KDE) ------------------------------------------------------
	{
		std::vector<double> v{1,2,2,3,3,3,4,4,5,2,3,4,1,5,3,3,2,4};
		auto out = render_of(plot::density(v, plot::bandwidth{0.7}, plot::title("kde")));
		if(!well_formed(out)) return 6;
		if(out.find("<polyline") == std::string::npos) return 7;  // smooth curve
	}

	// ---- contour (marching squares) -----------------------------------------
	{
		const std::size_t R=12, C=12;
		std::vector<double> field(R*C);
		for(std::size_t i=0;i<R;++i) for(std::size_t j=0;j<C;++j){
			double dx=double(j)-5.5, dy=double(i)-5.5;
			field[i*C+j] = std::exp(-(dx*dx+dy*dy)/12.0);   // a bump
		}
		plot::mat<double> m{ field.data(), R, C };
		auto out = render_of(plot::contour(m, plot::levels{6}, plot::title("contour")));
		if(!well_formed(out)) return 8;
		if(out.find("<polyline") == std::string::npos) return 9;  // iso segments
	}

	// ---- ohlc (candlestick) -------------------------------------------------
	{
		std::vector<double> tt{1,2,3,4,5}, op{10,11,10,12,11},
			hh{12,12,11,13,12}, ll{9,10,9,11,10}, cl{11,10,11,11,12};
		auto out = render_of(plot::ohlc(tt,op,hh,ll,cl, plot::title("ohlc")));
		if(!well_formed(out)) return 10;
		if(out.find("<line") == std::string::npos) return 11;   // wicks
		if(out.find("<rect") == std::string::npos) return 12;   // bodies
	}

	// ---- graph (circular node-link) -----------------------------------------
	{
		std::vector<std::string> nodes{"A","B","C","D","E"};
		std::vector<std::pair<std::size_t,std::size_t>> edges{{0,1},{1,2},{2,3},{3,4},{4,0},{0,2}};
		auto out = render_of(plot::graph(nodes, edges, plot::title("graph")));
		if(!well_formed(out)) return 13;
		if(out.find("<line")   == std::string::npos) return 14;   // edges
		if(out.find("<circle") == std::string::npos) return 15;   // nodes
		if(out.find(">A<")     == std::string::npos) return 16;   // node label
	}

	// ---- pie + donut --------------------------------------------------------
	{
		std::vector<std::string> labels{"x","y","z","w"};
		std::vector<double> vals{30,20,40,10};
		auto pie = render_of(plot::pie(labels, vals, plot::title("pie")));
		if(!well_formed(pie)) return 17;
		if(pie.find("<polyline") == std::string::npos) return 18;  // wedges
		if(pie.find(">x<")       == std::string::npos) return 19;  // slice label
		auto dn = render_of(plot::pie(labels, vals, plot::donut{0.5}, plot::title("donut")));
		if(!well_formed(dn)) return 20;
		if(dn.find("<polyline") == std::string::npos) return 21;   // ring wedges
	}

	// ---- hexbin (hexagonal heatmap) -----------------------------------------
	{
		const std::size_t R=8, C=8;
		std::vector<double> field(R*C);
		for(std::size_t i=0;i<R;++i) for(std::size_t j=0;j<C;++j)
			field[i*C+j] = double(i*C+j);
		plot::mat<double> m{ field.data(), R, C };
		auto out = render_of(plot::hexbin(m, plot::title("hex")));
		if(!well_formed(out)) return 22;
		if(out.find("<polyline") == std::string::npos) return 23;  // hexagons
		// honeycomb -> R*C hex cells.
		if(count(out, "<polyline") < R*C) return 24;
	}

	// ---- all eight new types compose into a single plot::grid figure --------
	{
		std::vector<double> v{1,2,2,3,3,4,5,2,3,4};
		std::vector<std::string> labels{"a","b","c"};
		std::vector<double> vals{3,5,2};
		std::vector<std::pair<std::size_t,std::size_t>> edges{{0,1},{1,2},{2,0}};
		const std::size_t R=6, C=6; std::vector<double> field(R*C);
		for(std::size_t i=0;i<R*C;++i) field[i] = double(i);
		plot::mat<double> m{ field.data(), R, C };
		std::vector<double> tt{1,2,3}, op{10,11,10}, hh{12,12,11}, ll{9,10,9}, cl{11,10,11};

		std::ostringstream os;
		plot::grid(os, plot::rows{2}, plot::cols{4},
			plot::histogram(v, plot::title("H")),
			plot::bar(labels, vals, plot::title("B")),
			plot::density(v, plot::title("D")),
			plot::contour(m, plot::title("C")),
			plot::ohlc(tt,op,hh,ll,cl, plot::title("O")),
			plot::graph(labels, edges, plot::title("G")),
			plot::pie(labels, vals, plot::title("P")),
			plot::hexbin(m, plot::title("X")),
			plot::width{1600}, plot::height{800});
		const std::string out = os.str();
		if(!well_formed(out)) return 25;
		if(count(out, "<svg")   != 1) return 26;   // exactly one outer document
		if(count(out, "</svg>") != 1) return 27;
	}

	return 0;
}
