/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Histogram example: bin a sample into counts and emit histogram.svg.
 */
#include <plot/all>
#include <vector>
#include <cmath>

int main(){
	std::vector<double> v;
	// a rough bell shape from a deterministic sum-of-uniforms.
	unsigned s = 12345u;
	auto rnd = [&]{ s = s*1664525u + 1013904223u; return double(s>>8 & 0xFFFF)/65536.0; };
	for(int i=0;i<2000;++i){
		double g = 0; for(int k=0;k<6;++k) g += rnd();
		v.push_back(g);   // ~Irwin-Hall, mean 3
	}
	plot::histogram(v, plot::bins{24},
		plot::title("histogram"), plot::xlabel{"value"}, plot::ylabel{"count"},
		plot::width{640}, plot::height{400}).save("histogram.svg");
	return 0;
}
