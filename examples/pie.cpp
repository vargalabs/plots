// Copyright (c) 2026 Steven Varga, Toronto, ON, Canada

#include <plot/all>
#include <vector>
#include <string>

int main(){
	std::vector<std::string> labels{"Rust","C++","Kotlin","Python","Other"};
	std::vector<double> share{42, 28, 14, 10, 6};

	plot::save("pie.svg",
		plot::pie(labels, share, plot::title("language share"), plot::width{520}, plot::height{520}));
		
	plot::save("donut.svg", plot::donut(labels, share,
		plot::title("language share (donut)"), plot::width{520}, plot::height{520}));
	return 0;
}
