CC=gcc
P=csview
#OBJECTS=csv.o csv-handler.o # Dependencies that need to be compiled first.
OUTDIR=./debug
RELDIR=./release
TESTS=./tests
ifeq ($(OS), Windows_NT)
	CFLAGS=-g -O3 # Don't have a lot of options with w64devkit, unfortunately.
	EXT=.exe
else
	EXT=
	CFLAGS=-fsanitize=address -g -ggdb -fno-omit-frame-pointer -Wall -O3
endif
# Setting EXT to ".exe" for compiling in Windows, keep it empty for Linux.

# GNU MAKE DOES NOT LIKE SPACES!  Need to use tabs.
# To replace all spaces with tabs in Vim:
# set noexpandtab
# %retab!

debug: $(P)
	@mkdir -p $(OUTDIR)
	@rm -rf $(OUTDIR)/$(P)$(EXT)
	@mv $(P)$(EXT) $(OUTDIR)

release: CFLAGS=
release: OUTDIR=$(RELDIR)
release: debug

$(P): $(OBJECTS)

# Run this with something like `make test CASE=csv-handler`.
test: $(OBJECTS)
	@mkdir -p $(TESTS)
	@$(CC) $(CASE)-test.c $(CFLAGS) $(OBJECTS) -o $(TESTS)/$(CASE)-test$(EXT)
