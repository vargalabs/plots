// Copyright (c) 2026 Steven Varga, Toronto, ON, Canada

#include <plot/all>
#include <vector>
#include <string>

int main(){
	plot::theme(plot::solarized_dark); // (per-call override is also possible  via plot::use{ plot::solarized_light }).

	std::vector<double> sizes { 1'000, 4'000, 16'000, 64'000, 256'000, 1'000'000 }, // payload sizes (bytes) — spans 3 decades, so log-x reads naturally.
		scalar { 820, 1'500, 2'600, 3'400, 3'900, 4'100 },  // two synthetic throughput curves (MB/s).	
		simd { 1'900, 4'100, 7'200, 9'800, 11'500, 12'200 };

	plot::line(std::string("line_chart.svg"), sizes, {{ "scalar", scalar }, { "simd", simd } },
		plot::title("throughput vs payload size"), plot::xlabel{"payload [bytes]"}, plot::ylabel{"throughput [MB/s]"}, 
		plot::xlog{10.0}, plot::width{720}, plot::height{440});

	return 0;
}
