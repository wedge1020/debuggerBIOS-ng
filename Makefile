# ****************************************************************************
# *  debuggerBIOS-ng -- GNU Makefile
# *
# *  The BIOS build is driven by the transpiler-generated Make.sh (or
# *  Make.bat): running v32c++ without --no-build-scripts writes both
# *  scripts, and they carry the whole pipeline (transpile -> compile ->
# *  assemble -> packrom) with all byproducts in obj/ and the final BIOS in
# *  bin/. This Makefile makes sure the scripts exist -- initializing them
# *  with v32c++ if they are missing -- and calls the one that is there.
# *
# *  Targets:
# *    all       (default) the BIOS in bin/
# *    bios      the same, explicitly (via Make.sh / Make.bat)
# *    clean     remove bin/ obj/ put/ and the project .zip
# *    archive   a zip of the project; its paths are relative, so it
# *              extracts straight into the project base
# *    put       a put/ bundle for reloading the context library:
# *              Makefile as Makefile.txt, every source, header and .md
# *              file, and the test scripts and CARTs, all flat
# *
# *  The transpiler is $(V32CXX) with $(V32CXX_FLAGS) (used only to create
# *  the build scripts when they are missing); override with e.g.
# *    make V32CXX=/opt/v32/v32c++ V32CXX_FLAGS="-b --other"
# * ****************************************************************************

PROJECT      := debuggerBIOS-ng
BIOS         := bin/$(PROJECT).v32

V32CXX       := v32c++
V32CXX_FLAGS := -b

ARCHIVE      := $(PROJECT)-project.zip

SOURCES      := $(PROJECT).cpp $(wildcard src/*.cpp)
HEADERS      := $(wildcard inc/*.hpp)
ASSETS       := $(wildcard assets/*)

.DEFAULT_GOAL := all
.PHONY: all bios clean archive put

all: bios

bios: $(BIOS)

# The BIOS build: call the existing script. If there is none, run the
# transpiler once -- it writes Make.sh and Make.bat -- then call the script
# it wrote.
$(BIOS): $(SOURCES) $(HEADERS) $(ASSETS)
	@mkdir -p obj bin
	@if [ -f Make.sh ]; then \
	    sh ./Make.sh; \
	elif [ -f Make.bat ]; then \
	    cmd //c Make.bat; \
	else \
	    echo "No Make.sh/Make.bat: initializing them with $(V32CXX)"; \
	    $(V32CXX) $(V32CXX_FLAGS) -o obj/$(PROJECT).c $(PROJECT).cpp || exit 1; \
	    if [ -f Make.sh ]; then \
	        sh ./Make.sh; \
	    elif [ -f Make.bat ]; then \
	        cmd //c Make.bat; \
	    else \
	        echo "ERROR: $(V32CXX) did not generate Make.sh/Make.bat"; \
	        exit 1; \
	    fi; \
	fi

# remove everything the build and the put target made; the generated
# Make.sh / Make.bat stay (the transpiler owns them)
clean:
	rm -rf bin obj put *.zip ../$(ARCHIVE)

# a source archive: obj/, bin/, put/ and other zips stay out, and the
# entries are relative, so it unpacks directly into the project base
archive:
	rm -f ../$(ARCHIVE)
	zip -r ../$(ARCHIVE) . -x 'obj/*' 'bin/*' 'put/*' '.git/*' '*.zip'
	@echo "../$(ARCHIVE) written"

# the context-library reload bundle: Makefile as text, every source, header
# and .md file, and the tests (tests/README.md would clash with README.md,
# so it goes in as tests-README.md). Everything lands flat in put/.
put:
	@mkdir -p              put
	@cp -v $(PROJECT).cpp  put/
	@cp -v src/*.cpp       put/
	@cp -v inc/*.hpp       put/
	@cp -v Makefile        put/Makefile.txt
	@cp -v *.md docs/*.md  put/
	@cp -v tests/README.md put/tests-README.md
	@cp -v tests/scripts/* put/
	@cp -v tests/carts/*.c put/
	@echo "put/ ready: Makefile.txt, sources, headers, .md files and tests"
