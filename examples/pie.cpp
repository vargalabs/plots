/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Pie + donut example -> pie.svg and donut.svg.
 */
#include <plot/all>
#include <vector>
#include <string>

int main(){
	std::vector<std::string> labels{"Rust","C++","Kotlin","Python","Other"};
	std::vector<double> share{42, 28, 14, 10, 6};
	plot::pie(labels, share,
		plot::title("language share"),
		plot::width{520}, plot::height{520}).save("pie.svg");
	plot::pie(labels, share, plot::donut{0.55},
		plot::title("language share (donut)"),
		plot::width{520}, plot::height{520}).save("donut.svg");
	return 0;
}
