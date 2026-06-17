@mainpage plot — header-only C++ SVG charts

**plot** is a dependency-free, header-only C++23 charting library that emits clean,
self-contained **SVG** files. It offers a GR-style call API — `plot::line`,
`plot::scatter`, `plot::heatmap` — with named arguments for titles, labels, log
axes and canvas size, and ships with Solarized light/dark themes out of the box.

```cpp
#include <plot/all>
#include <vector>
#include <string>

int main(){
    plot::theme(plot::solarized_dark);            // process-global default theme

    std::vector<double> x{ 1e3, 4e3, 16e3, 64e3, 256e3, 1e6 };
    std::vector<double> scalar{ 820, 1500, 2600, 3400, 3900, 4100 };
    std::vector<double> simd  { 1900, 4100, 7200, 9800, 11500, 12200 };

    plot::line(std::string("throughput.svg"), x,
        { { "scalar", scalar }, { "simd", simd } },
        plot::title("throughput vs payload size"),
        plot::xlabel{"payload [bytes]"}, plot::ylabel{"throughput [MB/s]"},
        plot::xlog{10.0},
        plot::width{720}, plot::height{440});
}
```

## Features

- **GR-style API** — one call per chart (`plot::line` / `plot::scatter` /
  `plot::heatmap`) with order-free named arguments
  (`plot::title`, `plot::xlabel`, `plot::ylabel`, `plot::xlog`,
  `plot::width`, `plot::height`, `plot::axis::x/y`).
- **Solarized themes** — `plot::solarized_dark` and `plot::solarized_light`
  presets; set a process-global default with `plot::theme(...)` or override a
  single render with `plot::use{...}`. A `plot::theme_t` carries structural
  colours, a cycled categorical series palette, and a 3-stop continuous gradient
  used by the heatmap.
- **Dependency-free C++23** — standard library only; no runtime, no linking.
  Output is a single static `.svg` you can open in any browser or embed inline.
- **`find_package` friendly** — header-only `plot::plot` INTERFACE target with an
  installed CMake package config; just `find_package(plot)` and link
  `plot::plot`.

## Examples

The three programs under `examples/` (build with `-DPLOTS_BUILD_EXAMPLES=ON`)
each emit one of the charts below.

### Line chart — `plot::line`

Multi-series throughput on a log-x axis, Solarized dark.

@image html line_chart.svg "Multi-series line chart on a log-x axis"

### Scatter — `plot::scatter`

A noisy linear cloud, Solarized light.

@image html scatter.svg "Scatter plot of a noisy linear relationship"

### Heatmap — `plot::heatmap`

A throughput field over a payload-size × thread-count grid, continuous gradient.

@image html heatmap.svg "Heatmap of a throughput field with continuous gradient"

## Building & integrating

```sh
cmake -S . -B build -DPLOTS_BUILD_EXAMPLES=ON
cmake --build build -j
./build/examples/line_chart   # writes line_chart.svg
```

In a downstream project:

```cmake
find_package(plot REQUIRED)
target_link_libraries(my_app PRIVATE plot::plot)
```

---

Copyright &copy; 2026 Steven Varga — MIT License.
