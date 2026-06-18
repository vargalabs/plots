add_executable(plot_gr plot_gr.cpp)
target_link_libraries(plot_gr PRIVATE plot::plot)
add_test(NAME plot_gr COMMAND plot_gr)

add_executable(plot_grid plot_grid.cpp)
target_link_libraries(plot_grid PRIVATE plot::plot)
add_test(NAME plot_grid COMMAND plot_grid)

add_executable(plot_new plot_new.cpp)
target_link_libraries(plot_new PRIVATE plot::plot)
add_test(NAME plot_new COMMAND plot_new)
