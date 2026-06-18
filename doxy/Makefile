# plot Doxygen — run from this directory
# Outputs to ../docs/doxygen/html/  (relative to repo root: doxy/docs/doxygen/html)

DOXY    ?= doxygen
OUT     = docs/doxygen/html

# Gallery regeneration: dark renders use Nord (matched to the doxygen-awesome
# dark page background #2b303b); light renders use Solarized Light (matches the
# light page #fdf6e3). The default theme is selected at build time via the
# -DPLOT_DEFAULT_THEME macro, so the example sources stay theme-neutral.
DARK_THEME  ?= nord
LIGHT_THEME ?= solarized_light
EXAMPLES = line_chart scatter heatmap dashboard histogram bar density \
           contour ohlc graph pie hexbin

.PHONY: all view clean gallery

all:
	$(DOXY) Doxyfile 2>&1 | tee doxygen.log
	# doxygen 1.9.1 flattens HTML_EXTRA_FILES directories, so copy the
	# theme-aware gallery image dirs into the html output ourselves.
	cp -r dark light $(OUT)/
	# The gh-pages branch carries a repo .gitignore with `*.svg`; peaceiris
	# (git add) would skip the gallery SVGs. Ship an empty .gitignore in the
	# published dir so it overrides that rule and the SVGs get committed.
	: > $(OUT)/.gitignore

view: all
	xdg-open $(OUT)/index.html

# Rebuild the theme-aware gallery SVGs from the example programs. Builds the
# examples twice (dark + light default theme) and copies the renders into the
# dark/ and light/ dirs that `all` ships into the published HTML.
gallery:
	cmake -S .. -B ../build-dark  -DPLOTS_BUILD_EXAMPLES=ON -DCMAKE_CXX_FLAGS=-DPLOT_DEFAULT_THEME=$(DARK_THEME)
	cmake --build ../build-dark -j
	cd ../build-dark/examples  && for e in $(EXAMPLES); do ./$$e; done
	cp ../build-dark/examples/*.svg dark/
	cmake -S .. -B ../build-light -DPLOTS_BUILD_EXAMPLES=ON -DCMAKE_CXX_FLAGS=-DPLOT_DEFAULT_THEME=$(LIGHT_THEME)
	cmake --build ../build-light -j
	cd ../build-light/examples && for e in $(EXAMPLES); do ./$$e; done
	cp ../build-light/examples/*.svg light/

clean:
	$(RM) -rf docs/doxygen doxygen.log
