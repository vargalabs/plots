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
    std::vector<double> x{ 1e3, 4e3, 16e3, 64e3, 256e3, 1e6 },
      scalar{ 820, 1500, 2600, 3400, 3900, 4100 },
      simd  { 1900, 4100, 7200, 9800, 11500, 12200 };

    plot::line(std::string("throughput.svg"), x, {{ "scalar", scalar }, { "simd", simd }},
        plot::title("throughput vs payload size"),  plot::xlabel{"payload [bytes]"}, plot::ylabel{"throughput [MB/s]"},
        plot::xlog{10.0},  plot::width{720}, plot::height{440});
}
```
## Plots
  - `plot::line(os|"file.svg", xs, ys, opts...)` — single line series, `ys` vs `xs` (both `std::vector`); axes auto-scaled. Multi-series form: `plot::line(os|"file.svg", xs, {{"label", data}, ...}, opts...)` draws one line per series with a legend.
  - `plot::scatter(os|"file.svg", xs, ys, opts...)` — scatter of points, `ys` vs `xs`; axes auto-scaled.
  - `plot::heatmap(os|"file.svg", mat, plot::axis::x(xlabels), plot::axis::y(ylabels), opts...)` — 2-D grid colored by cell value. `mat` is a `plot::mat<T>{ ptr, rows, cols }` (row-major view); floating-point → continuous gradient, integral → categorical.
  `axis::x`/`axis::y` tick labels are required.
  
  Where the first argument is a `std::ostream&` OR a filename, followed by the data, then
  the named `opts...` (`title`, `xlabel`, `ylabel`, `xlog`, `ylog`, `width`, `height`,
  `use{theme}`) in any order. The active theme is the global `plot::theme(...)` default,
  or a per-call `plot::use{ plot::solarized_light }` override.
  
  Accuracy notes from the headers:
  - Each driver has a std::ostream& overload and a filename (std::string) overload — the filename one just opens a truncating ofstream.
  - plot::line is the only one with the {{"label", data}, …} multi-series overload (each series is a {std::string, std::vector<double>}); scatter/heatmap are single-data.
  - heatmap is the one that requires plot::axis::x/axis::y (static-asserted); line/scatter derive their axes from the data and instead take xlabel/ylabel.
 

## Options (pythonic syntax)

- `plot::title("Throughput vs size")` — chart title (string); optional 2nd arg `plot::position{x,y}` and a font.
- `plot::xlabel{"payload [bytes]"}` — x-axis label (string).
- `plot::ylabel{"throughput [MB/s]"}` — y-axis label (string).
- `plot::xlog{10.0}` — log-scale the x-axis; arg is the log base (default `10.0`, use `plot::xlog{2.0}` for log2). `plot::ylog{...}` is the y-axis analogue.
- `plot::width{720}` — canvas width in pixels.
- `plot::height{440}` — canvas height in pixels.
- `plot::axis::x(std::vector<std::string>{"1K","4K","16K","1M"})` — x-axis tick labels (heatmap).
- `plot::axis::y(std::vector<std::string>{"int","float","double"})` — y-axis tick labels (heatmap).

All of the above are passed as named options **in any order** after the positional arguments (`os`/filename, then the data). e.g.:
```cpp
  plot::line("bw.svg", xs, ys,
      plot::ylabel{"MB/s"}, plot::title("bandwidth"),   // any order
      plot::xlog{10.0}, plot::xlabel{"bytes"}, plot::width{720}, plot::height{440});
```
Two accuracy notes from the actual headers:
- Brace vs. parens: xlabel/ylabel/xlog/ylog/width/height are aggregates → brace-init {…}. title and axis::x/axis::y are constructed → use parens (…).
- Where each applies: title/xlabel/ylabel/xlog/width/height are for plot::line/plot::scatter (auto-scaled axes), where xlabel/ylabel name the axes. plot::heatmap uses plot::axis::x/y (the tick labels are required there) plus title. So axis::x/y and xlabel/ylabel are the heatmap-vs-line/scatter counterparts, not interchangeable.


## Features

- **GR-style API** — one call per chart (`plot::line` / `plot::scatter` / `plot::heatmap`) with order-free named arguments
  (`plot::title`, `plot::xlabel`, `plot::ylabel`, `plot::xlog`, `plot::width`, `plot::height`, `plot::axis::x/y`).
- **Solarized themes** — `plot::solarized_dark` and `plot::solarized_light`  presets; set a process-global default with `plot::theme(...)` or override a
  single render with `plot::use{...}`. A `plot::theme_t` carries structural   colours, a cycled categorical series palette, and a 3-stop continuous gradient
  used by the heatmap.

- **Dependency-free C++23** — standard library only; no runtime, no linking.  Output is a single static `.svg` you can open in any browser or embed inline.
- **`find_package` friendly** — header-only `plot::plot` INTERFACE target with an   installed CMake package config; just `find_package(plot)` and link
  `plot::plot`.


## Composing figures — `view` & `grid`

Each driver has a no-`os`/no-filename form that *defers* the render: it captures
the data and named options by value and returns a movable, composable **`view`**
instead of writing anything.

```cpp
auto v = plot::line(xs, ys, plot::title("series A"));   // a view, nothing written yet
plot::save("series.svg", v);                            // standalone <svg> at natural size
plot::render(os, v);                                    // … or into your own ostream
```

So `plot::line(xs, ys, opts...)` returns a view, and the familiar
`plot::line("series.svg", xs, ys, opts...)` (leading filename/`ostream`) is just
sugar for `plot::save("series.svg", plot::line(xs, ys, opts...))`.

`plot::grid(file|os, …)` tiles several views into a **single figure**. It is
variadic and order-independent: it collects every `view` argument (in order) and
resolves `plot::rows{r}` / `plot::cols{c}` / `plot::width{...}` /
`plot::height{...}` / `plot::use{...}` by tag, in any order. Omit `rows`/`cols`
and it lays out automatically (`cols = ceil(sqrt(N))`).

```cpp
plot::grid("dashboard.svg",
    plot::rows{2}, plot::cols{2},
    plot::line   (xs, ys,  plot::title("series A"), plot::xlabel{"t"}, plot::ylabel{"v"}),
    plot::scatter(sx, sy,  plot::title("scatter: y ~ 0.7x + noise")),
    plot::heatmap(field,   plot::axis::x(sizes), plot::axis::y(threads),
                           plot::title("throughput MB/s")),
    plot::line   (xs, {{ "rising", ys }, { "falling", ys2 }}, plot::title("two series")),
    plot::width{1200}, plot::height{800});
```

@image html dashboard.svg "A 2×2 figure: line, scatter, heatmap and multi-series line tiled by plot::grid"

**Themes.** A `view` carries no theme of its own: at render time each view
resolves the process-global `plot::theme(...)` default — unless it was built with
a per-view `plot::use{ ... }` override. `plot::grid` likewise paints the figure
background from the resolved theme (its own `plot::use{...}`, else the global
default), so one `plot::theme(...)` call styles the whole dashboard.

See `examples/dashboard.cpp` for the program that emits the figure above.

## Plot types

The shipping drivers, plus the gallery still being filled out. Every one has a
deferred `view` form (no leading `os`/filename) and the filename/`ostream` sugar
shown above.

- `plot::line(xs, ys, opts...)` — line series (multi-series: `plot::line(xs, {{"label", data}, …}, opts...)`).
- `plot::scatter(xs, ys, opts...)` — scatter of points.
- `plot::heatmap(mat, plot::axis::x(xs), plot::axis::y(ys), opts...)` — 2-D grid colored by cell value.
- `plot::histogram(values, opts...)` — binned frequency *(coming in the gallery)*.
- `plot::bar(labels, values, opts...)` — categorical bars *(coming in the gallery)*.
- `plot::density(values, opts...)` — smoothed (KDE) distribution *(coming in the gallery)*.
- `plot::contour(mat, opts...)` — iso-level contours of a scalar field *(coming in the gallery)*.
- `plot::ohlc(t, open, high, low, close, opts...)` — open/high/low/close candles *(coming in the gallery)*.
- `plot::graph(nodes, edges, opts...)` — node/edge network layout *(coming in the gallery)*.
- `plot::pie(labels, values, opts...)` — proportional wedges *(coming in the gallery)*.
- `plot::hexbin(xs, ys, opts...)` — hexagonal density binning *(coming in the gallery)*.

## Gallery

Every shipping driver, rendered with the Solarized theme. The image follows the
page theme — use the moon/sun toggle (top-right) to switch between the Solarized
**dark** and **light** renders.

|  |  |  |
|:-:|:-:|:-:|
| <img class="only-light" src="light/line_chart.svg" width="240"> <img class="only-dark" src="dark/line_chart.svg" width="240"> | <img class="only-light" src="light/scatter.svg" width="240"> <img class="only-dark" src="dark/scatter.svg" width="240"> | <img class="only-light" src="light/heatmap.svg" width="240"> <img class="only-dark" src="dark/heatmap.svg" width="240"> |
| <img class="only-light" src="light/histogram.svg" width="240"> <img class="only-dark" src="dark/histogram.svg" width="240"> | <img class="only-light" src="light/bar.svg" width="240"> <img class="only-dark" src="dark/bar.svg" width="240"> | <img class="only-light" src="light/density.svg" width="240"> <img class="only-dark" src="dark/density.svg" width="240"> |
| <img class="only-light" src="light/contour.svg" width="240"> <img class="only-dark" src="dark/contour.svg" width="240"> | <img class="only-light" src="light/ohlc.svg" width="240"> <img class="only-dark" src="dark/ohlc.svg" width="240"> | <img class="only-light" src="light/graph.svg" width="240"> <img class="only-dark" src="dark/graph.svg" width="240"> |
| <img class="only-light" src="light/pie.svg" width="240"> <img class="only-dark" src="dark/pie.svg" width="240"> | <img class="only-light" src="light/donut.svg" width="240"> <img class="only-dark" src="dark/donut.svg" width="240"> | <img class="only-light" src="light/hexbin.svg" width="240"> <img class="only-dark" src="dark/hexbin.svg" width="240"> |
| <img class="only-light" src="light/dashboard.svg" width="240"> <img class="only-dark" src="dark/dashboard.svg" width="240"> |  |  |

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
