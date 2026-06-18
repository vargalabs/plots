# plot Doxygen — run from this directory
# Outputs to ../docs/doxygen/html/  (relative to repo root: doxy/docs/doxygen/html)

DOXY    ?= doxygen
OUT     = docs/doxygen/html

.PHONY: all view clean

all:
	$(DOXY) Doxyfile 2>&1 | tee doxygen.log
	# doxygen 1.9.1 flattens HTML_EXTRA_FILES directories, so copy the
	# theme-aware gallery image dirs into the html output ourselves.
	cp -r dark light $(OUT)/

view: all
	xdg-open $(OUT)/index.html

clean:
	$(RM) -rf docs/doxygen doxygen.log
