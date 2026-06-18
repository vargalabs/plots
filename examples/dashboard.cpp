/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Figure composition: four deferred plots (plot::view) tiled into one 2×2 figure
 * via plot::grid. Each call to plot::line/scatter/heatmap WITHOUT a leading
 * os/filename argument returns a movable view that captures its data + options by
 * value; plot::grid opens a single <svg>, paints the figure background, and draws
 * each view into its cell with the SAME order-independent tpos dispatch the rest
 * of the layer uses (rows/cols/width/height resolved by tag, any order).
 *
 * Build with -DPLOTS_BUILD_EXAMPLES=ON; run; open dashboard.svg.
 */
#include <plot/all>

#include <vector>
#include <string>
#include <cmath>
#include <cstddef>

int main(){
	plot::theme(plot::solarized_dark);

	// ---- panel 1 + 4 data: a couple of curves -------------------------------
	std::vector<double> xs{ 1, 2, 3, 4, 5, 6, 7, 8 };
	std::vector<double> ys{ 2, 5, 4, 8, 7, 11, 9, 13 };
	std::vector<double> ys2{ 13, 9, 11, 7, 8, 4, 5, 2 };

	// ---- panel 2 data: a noisy scatter cloud --------------------------------
	std::vector<double> sx, sy;
	unsigned s = 2246u;
	auto rng = [&]() { s = s*1664525u + 1013904223u; return (s >> 8) / 16777216.0; };
	for(int i=0;i<80;++i){
		double x = i * 0.25;
		sx.push_back(x);
		sy.push_back(0.7 * x + (rng() - 0.5) * 6.0);
	}

	// ---- panel 3 data: a heatmap field --------------------------------------
	std::vector<std::string> sizes  { "1K", "4K", "16K", "64K", "256K" };
	std::vector<std::string> threads{ "1", "2", "4", "8", "16" };
	const std::size_t cols = sizes.size(), rows = threads.size();
	std::vector<double> z(rows * cols);
	for(std::size_t r=0;r<rows;++r)
		for(std::size_t c=0;c<cols;++c)
			z[r*cols+c] = 200.0 * (1.0 + double(r)) * std::log2(double(c)+2.0);
	plot::mat<double> field{ z.data(), rows, cols };
	const plot::position htitle{ std::size_t{4}, std::size_t{10} };

	// ---- one 2×2 figure: line, scatter, heatmap, multi-series line ----------
	plot::grid(std::string("dashboard.svg"),
		plot::rows{2}, plot::cols{2},
		plot::line(xs, ys, plot::title("series A"),
			plot::xlabel{"t"}, plot::ylabel{"v"}),
		plot::scatter(sx, sy, plot::title("scatter: y ~ 0.7x + noise"),
			plot::xlabel{"x"}, plot::ylabel{"y"}),
		plot::heatmap(field,
			plot::axis::x(sizes), plot::axis::y(threads),
			plot::title(std::string("throughput MB/s"), htitle)),
		plot::line(xs,
			{ { "rising", ys }, { "falling", ys2 } },
			plot::title("two series")),
		plot::width{1200}, plot::height{800});

	return 0;
}
