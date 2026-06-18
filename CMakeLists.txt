#  _________________________________________________________
#  Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
#  MIT License — see LICENSE
#  _________________________________________________________

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(plot VERSION 0.1.0 LANGUAGES CXX)

add_library(plot INTERFACE)
add_library(plot::plot ALIAS plot)

target_include_directories(plot INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(plot INTERFACE cxx_std_23)

option(PLOTS_BUILD_TESTS "Build tests" ON)
if(PLOTS_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

option(PLOTS_BUILD_EXAMPLES "Build examples" ON)
if(PLOTS_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# Doxygen single-page documentation (output under doxy/docs/doxygen/html).
# Build with: cmake --build build --target docs-doxygen
find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
	add_custom_target(docs-doxygen
		COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doxy
		COMMENT "Generating Doxygen documentation"
		VERBATIM
	)
endif()

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS plot EXPORT plot-targets
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)
set(PLOT_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/cmake/plot)
configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/plot-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/plot-config.cmake
	INSTALL_DESTINATION ${PLOT_INSTALL_CMAKEDIR}
)
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/plot-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
	ARCH_INDEPENDENT
)
install(EXPORT plot-targets
	FILE plot-targets.cmake
	NAMESPACE plot::
	DESTINATION ${PLOT_INSTALL_CMAKEDIR}
)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/plot-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/plot-config-version.cmake
	DESTINATION ${PLOT_INSTALL_CMAKEDIR}
)
