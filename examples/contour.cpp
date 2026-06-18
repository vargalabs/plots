/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Marching-squares contour example -> contour.svg.
 */
#include <plot/all>
#include <vector>
#include <cmath>

int main(){
	const std::size_t R=40, C=40;
	std::vector<double> field(R*C);
	for(std::size_t i=0;i<R;++i) for(std::size_t j=0;j<C;++j){
		double x = (double(j)/C)*6.0 - 3.0;
		double y = (double(i)/R)*6.0 - 3.0;
		field[i*C+j] = std::sin(x)*std::cos(y);   // a saddle field
	}
	plot::mat<double> m{ field.data(), R, C };
	plot::save("contour.svg", plot::contour(m, plot::levels{12},
		plot::title("contour"), plot::xlabel{"x"}, plot::ylabel{"y"},
		plot::width{640}, plot::height{500}));
	return 0;
}
