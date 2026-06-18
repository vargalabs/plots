/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Node-link graph (circular layout) example -> graph.svg.
 */
#include <plot/all>
#include <vector>
#include <string>
#include <utility>

int main(){
	std::vector<std::string> nodes{"core","io","gr","heatmap","view","theme","canvas","tags"};
	std::vector<std::pair<std::size_t,std::size_t>> edges{
		{0,1},{0,2},{2,4},{3,4},{4,6},{5,2},{5,3},{7,0},{7,5},{1,6},{2,3}
	};
	plot::graph(nodes, edges,
		plot::title("module graph"),
		plot::width{560}, plot::height{560}).save("graph.svg");
	return 0;
}
