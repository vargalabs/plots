#  _________________________________________________________
#  Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
#  MIT License — see LICENSE
#  _________________________________________________________
#
# Example programs link plot::plot and emit .svg files. Target names are
# prefixed `example_` with OUTPUT_NAME set to the plain name.

foreach(ex line_chart scatter heatmap dashboard histogram bar density contour ohlc graph)
	add_executable(example_${ex} ${ex}.cpp)
	target_link_libraries(example_${ex} PRIVATE plot::plot)
	set_target_properties(example_${ex} PROPERTIES OUTPUT_NAME ${ex})
endforeach()
