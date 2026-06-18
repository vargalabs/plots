/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Gaussian KDE density example -> density.svg.
 */
#include <plot/all>
#include <vector>

int main(){
	std::vector<double> v;
	unsigned s = 777u;
	auto rnd = [&]{ s = s*1664525u + 1013904223u; return double(s>>8 & 0xFFFF)/65536.0; };
	// a bimodal sample.
	for(int i=0;i<400;++i){ double g=0; for(int k=0;k<4;++k) g+=rnd(); v.push_back(g); }
	for(int i=0;i<400;++i){ double g=0; for(int k=0;k<4;++k) g+=rnd(); v.push_back(g+4.0); }
	plot::density(v,
		plot::title("density (KDE)"), plot::xlabel{"value"}, plot::ylabel{"density"},
		plot::width{640}, plot::height{400}).save("density.svg");
	return 0;
}
