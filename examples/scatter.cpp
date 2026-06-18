// Copyright (c) 2026 Steven Varga, Toronto, ON, Canada

#include <plot/all>

#include <vector>
#include <string>
#include <cmath>

int main(){
	plot::theme(plot::solarized_dark);

	std::vector<double> xs, ys;
	// deterministic pseudo-random cloud around y = 0.8*x + noise.
	unsigned s = 12345u;
	auto rng = [&]() {
		s = s*1664525u + 1013904223u;
		return (s >> 8) / 16777216.0;
	};

	for(int i=0;i<120;++i){
		double x = i * 0.5;
		double y = 0.8 * x + (rng() - 0.5) * 18.0;
		xs.push_back(x);
		ys.push_back(y);
	}

	plot::scatter("scatter.svg", xs, ys, // plots directly into file
		plot::title("scatter: y ~ 0.8x + noise"), plot::xlabel{"x"}, plot::ylabel{"y"},
		plot::width{640}, plot::height{420});

	return 0;
}
