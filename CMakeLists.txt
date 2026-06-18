#  _________________________________________________________
#  Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
#  MIT License — see LICENSE
#  _________________________________________________________

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(plot VERSION 1.0.0 LANGUAGES CXX)

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

# ─── Single-header amalgamation (plot.hpp) ───────────────────────────────────
# Generate a self-contained plot.hpp from include/plot/ (order taken from the
# `all` umbrella). This is the payload shipped in the release packages. The
# target is built as part of ALL when a Python 3 interpreter is available;
# without one the rest of the build (tests/examples/find_package) is unaffected.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
	file(GLOB PLOT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/plot/*.hpp)
	set(PLOT_AMALGAMATED_HEADER ${CMAKE_CURRENT_BINARY_DIR}/plot.hpp)
	add_custom_command(
		OUTPUT ${PLOT_AMALGAMATED_HEADER}
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/amalgamate.py
		        ${CMAKE_CURRENT_SOURCE_DIR}/include/plot ${PLOT_AMALGAMATED_HEADER}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/amalgamate.py
		        ${CMAKE_CURRENT_SOURCE_DIR}/include/plot/all ${PLOT_HEADERS}
		COMMENT "Amalgamating plot headers into plot.hpp"
		VERBATIM
	)
	add_custom_target(plot-amalgamate ALL DEPENDS ${PLOT_AMALGAMATED_HEADER})
	# Ship plot.hpp alongside the tree so #include <plot.hpp> works ambiently.
	install(FILES ${PLOT_AMALGAMATED_HEADER} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else()
	message(WARNING "Python3 not found — plot.hpp amalgamation disabled.")
endif()

# LICENSE travels in every package (and find_package install).
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})

# ─── Full header tree + CMake package config (find_package(plot)) ────────────
# ON for source installs; the release packages set -DPLOT_INSTALL_FULL=OFF so
# the package payload is just the single header + LICENSE.
option(PLOT_INSTALL_FULL "Install the full include/plot tree and CMake package config" ON)
if(PLOT_INSTALL_FULL)
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
endif()

# ─── CPack: H5CPP-style release packages ─────────────────────────────────────
# Header-only ⇒ arch-independent (DEB all / RPM noarch). The package payload is
# the single header + LICENSE (configure the release build with
# -DPLOT_INSTALL_FULL=OFF). Generators: DEB, RPM, NSIS (Windows), productbuild
# (macOS), plus TGZ + ZIP archives.
set(CPACK_PACKAGE_NAME                "plot-dev")
set(CPACK_PACKAGE_VENDOR              "Varga Labs")
set(CPACK_PACKAGE_CONTACT             "steven.varga@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Header-only, dependency-free C++ SVG plotting library")
set(CPACK_PACKAGE_HOMEPAGE_URL        "https://vargalabs.github.io/plots")
set(CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR       "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR       "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH       "${PROJECT_VERSION_PATCH}")

# productbuild (macOS) rejects license files without a .rtf/.html/.txt
# extension; the canonical LICENSE is extensionless, so stage a .txt copy.
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
	"${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt"
	COPYONLY)
set(CPACK_RESOURCE_FILE_LICENSE       "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
set(CPACK_STRIP_FILES                 OFF)

# Headers land at <prefix>/include/plot.hpp — ambient, no -I needed. Linux uses
# /usr (apt/yum convention); macOS uses /usr/local (Apple/Homebrew). Windows
# NSIS ignores this and lets the user pick a target dir.
if(APPLE)
	set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
else()
	set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
endif()

# DEB (Debian/Ubuntu)
set(CPACK_DEBIAN_PACKAGE_NAME         "plot-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION      "libdevel")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")     # header-only
set(CPACK_DEBIAN_FILE_NAME            "DEB-DEFAULT")

# RPM (RedHat/Fedora)
set(CPACK_RPM_PACKAGE_NAME            "plot-devel")
set(CPACK_RPM_PACKAGE_LICENSE         "MIT")
set(CPACK_RPM_PACKAGE_GROUP           "Development/Libraries")
set(CPACK_RPM_PACKAGE_ARCHITECTURE    "noarch")  # header-only
set(CPACK_RPM_FILE_NAME               "RPM-DEFAULT")

# NSIS (Windows installer)
set(CPACK_NSIS_PACKAGE_NAME           "plot ${PROJECT_VERSION}")
set(CPACK_NSIS_DISPLAY_NAME           "plot ${PROJECT_VERSION}")
set(CPACK_NSIS_URL_INFO_ABOUT         "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# productbuild (macOS .pkg)
set(CPACK_PRODUCTBUILD_IDENTIFIER     "org.vargalabs.plot")

# Distinct, lowercase per-OS filenames for NSIS/productbuild/TGZ/ZIP. DEB/RPM
# keep their own conventional *-DEFAULT names set above.
set(CPACK_PACKAGE_FILE_NAME "plot-v${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
string(TOLOWER "${CPACK_PACKAGE_FILE_NAME}" CPACK_PACKAGE_FILE_NAME)

include(CPack)
