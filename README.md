# plot

A small, header-only C++ library that renders **SVG** charts — heatmaps, line
charts, and scatter plots — with a GR.jl-style functional API and Solarized
themes. Dependency-free (C++23 standard library only): no third-party plotting,
matrix, or formatting libraries.

```cpp
#include <plot/all>

int main() {
  plot::theme(plot::solarized_dark);                 // global default; per-call: plot::use{...}

  std::vector<double> x{1'000, 4'000, 16'000, 64'000, 256'000, 1'000'000};
  plot::line("throughput.svg", x,
      { {"scalar", {820,1500,2600,3400,3900,4100}},
        {"simd",   {1900,4100,7200,9800,11500,12200}} },
      plot::title("throughput vs payload"), plot::xlabel{"bytes"},
      plot::ylabel{"MB/s"}, plot::xlog{10.0}, plot::width{720}, plot::height{440});
}
```

## API

- `plot::line(os|file, xs, ys, opts...)` — single series; or multi-series with a
  legend via `plot::line(os, xs, {{"label", data}, ...}, opts...)`.
- `plot::scatter(os|file, xs, ys, opts...)` — points.
- `plot::heatmap(os|file, mat<T>, plot::axis::x{...}, plot::axis::y{...}, opts...)`
  — continuous-gradient (arithmetic) or categorical (integral) 2-D field.

Order-independent named options: `title`, `xlabel`, `ylabel`, `xlog{base}`,
`ylog{base}`, `legend`, `width`, `height`, `margin`, and `use{theme}`.

## Themes

`plot::solarized_dark` / `plot::solarized_light` — set the process default with
`plot::theme(t)`, or override per call with `plot::use{t}`. A theme drives the
background, panel, grid, axis/text colors, the categorical series palette, and
the continuous gradient stops.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build            # plot_gr
./build/examples/line_chart       # writes line_chart.svg
```

Header-only: drop `include/plot` on your include path, or `find_package(plot CONFIG)`
and link `plot::plot`.

## License

MIT — see [LICENSE](LICENSE).
