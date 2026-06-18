/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Categorical bar chart example -> bar.svg.
 */
#include <plot/all>
#include <vector>
#include <string>

int main(){
	std::vector<std::string> labels{"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
	std::vector<double> sales{12, 19, 7, 22, 30, 25, 14};
	plot::bar(labels, sales,
		plot::title("weekly sales"), plot::xlabel{"day"}, plot::ylabel{"units"},
		plot::width{640}, plot::height{400}).save("bar.svg");
	return 0;
}
