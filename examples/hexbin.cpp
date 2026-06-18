/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Hexagonal heatmap example -> hexbin.svg.
 */
#include <plot/all>
#include <vector>
#include <cmath>

int main(){
	const std::size_t R=18, C=22;
	std::vector<double> field(R*C);
	for(std::size_t i=0;i<R;++i) for(std::size_t j=0;j<C;++j){
		double dx = double(j) - 11.0, dy = double(i) - 9.0;
		field[i*C+j] = std::exp(-(dx*dx + dy*dy)/40.0);   // a radial bump
	}
	plot::mat<double> m{ field.data(), R, C };
	plot::hexbin(m,
		plot::title("hexbin heatmap"),
		plot::width{680}, plot::height{520}).save("hexbin.svg");
	return 0;
}
