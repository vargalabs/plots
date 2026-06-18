# plot
> Header-only C++ SVG charts — heatmap, line, scatter — GR-style API + Solarized themes.

[![CI](https://github.com/vargalabs/plots/actions/workflows/ci.yml/badge.svg)](https://github.com/vargalabs/plots/actions/workflows/ci.yml)
[![MIT License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/docs-stable-blue)](https://vargalabs.github.io/plots/)

| OS / Compiler | GCC 13        | GCC 14        | Clang 18     | Clang 20     | Apple Clang | MSVC         |
|---------------|---------------|---------------|--------------|--------------|-------------|--------------|
| Ubuntu 22.04  | ![gcc13][200] | ![NA][NA]     | ![cl18][201] | ![NA][NA]    | ![NA][NA]   | ![NA][NA]    |
| Ubuntu 24.04  | ![NA][NA]     | ![gcc14][300] | ![NA][NA]    | ![cl20][301] | ![NA][NA]   | ![NA][NA]    |
| macOS 15      | ![NA][NA]     | ![NA][NA]     | ![NA][NA]    | ![NA][NA]    | ![ac][400]  | ![NA][NA]    |
| Windows       | ![NA][NA]     | ![NA][NA]     | ![NA][NA]    | ![NA][NA]    | ![NA][NA]   | ![msvc][500] |

> Badge and documentation URLs resolve only once **GitHub Pages is enabled** for this
> (private) repository — the CI badge job publishes to the `gh-pages` branch under
> `badges/` (release) and `badges-staging/` (staging).

## Quick start

```cpp
#include <plot/all>
#include <vector>

int main() {
  plot::theme(plot::solarized_dark);          // process default; per-call: plot::use{t}

  std::vector<double> x{1, 2, 3, 4, 5};
  plot::line("throughput.svg", x,
      { {"scalar", {820, 1500, 2600, 3400, 3900}},
        {"simd",   {1900, 4100, 7200, 9800, 11500}} },
      plot::title("throughput"), plot::xlabel{"n"}, plot::ylabel{"MB/s"});
}
```

## Features

- **GR-style API** — `plot::line`, `plot::scatter`, and `plot::heatmap`, each
  taking an output stream or filename plus order-independent named options
  (`title`, `xlabel`/`ylabel`, `xlog`/`ylog`, `legend`, `width`/`height`,
  `margin`).
- **Composable `view` + `grid`** — the no-`os` form (`plot::line(x, y, …)`)
  returns a deferred, movable `view` rendered via `plot::save("f.svg", view)` / `plot::render(os, view)`;
  `plot::grid(file|os, plot::rows{r}, plot::cols{c}, view…)` tiles several views
  into one figure (auto layout if `rows`/`cols` omitted).
- **Solarized themes** — `plot::solarized_dark` / `plot::solarized_light`; set the
  global default with `plot::theme(t)` or override per call with `plot::use{t}`.
- **Dependency-free C++23** — standard library only; no HDF5, zlib, boost, or
  third-party plotting/matrix/formatting libraries.
- **CMake-friendly** — `find_package(plot)` exposes the `plot::plot` target.

## Build & install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build              # plot_gr
```

plot is header-only — drop `include/plot` on your include path, or consume it via
CMake:

```cmake
find_package(plot CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE plot::plot)
```

## Examples

`-DPLOTS_BUILD_EXAMPLES=ON` builds the demos under `examples/`; each writes an
`.svg`:

- `examples/line_chart` → `line_chart.svg`
- `examples/scatter` → `scatter.svg`
- `examples/heatmap` → `heatmap.svg`
- `examples/dashboard` → `dashboard.svg` (a 2×2 `view`/`grid` figure)

## Documentation

Full API reference: [vargalabs.github.io/plots](https://vargalabs.github.io/plots/)

## License

MIT — see [LICENSE](LICENSE).

[NA]: https://vargalabs.github.io/plots/badges/na.svg

<!-- Ubuntu 22.04 -->
[200]: https://vargalabs.github.io/plots/badges/ubuntu-22.04-gcc-13.svg
[201]: https://vargalabs.github.io/plots/badges/ubuntu-22.04-clang-18.svg

<!-- Ubuntu 24.04 -->
[300]: https://vargalabs.github.io/plots/badges/ubuntu-24.04-gcc-14.svg
[301]: https://vargalabs.github.io/plots/badges/ubuntu-24.04-clang-20.svg

<!-- macOS 15 -->
[400]: https://vargalabs.github.io/plots/badges/macos-15-apple-clang.svg

<!-- Windows -->
[500]: https://vargalabs.github.io/plots/badges/windows-latest-msvc.svg
