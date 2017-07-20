#makefile to compile Rodent III on Linux
PREFIX=/usr/local

#define the directory for the executable file
BINDIR = $(PREFIX)/bin

#define the directory for the data files
DATADIR = $(PREFIX)/share/rodentIII


# define the C compiler to use
CXX = g++

# define features
# Other options are -DBOOKGEN -DNO_THREADS
CFG = -DNDEBUG -DUSEGEN -DUSE_THREADS -DNEW_THREADS -DBOOKPATH=$(DATADIR)
CFG1 = -DWRITEDEBUGFILE -DUSEGEN -DUSE_THREADS -DNEW_THREADS -DBOOKPATH=$(DATADIR)

# define the compile-time flags
CXXFLAGS = -g -std=c++14 -Wall -Wextra -O3 -msse -fno-rtti $(CFG)
CXX1FLAGS = -g -std=c++14 -Wall -Wextra $(CFG1)

# define the link options
LDFLAGS = -s -lpthread -Wl,--no-as-needed,--gc-sections
LD1FLAGS = -lm -lpthread

#command line switches
lto = yes
ifeq ($(lto),yes)
	CXXFLAGS += -flto
	LDFLAGS += -flto
endif

# define outpout name and settings file
EXENAME= rodentIII
CONFIGFILE = basic.ini

.PHONY: clean install update remove help

default: build

build:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)

build-popcnt:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) -msse3 -mpopcnt $(LDFLAGS) -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)

clang-profiled:
	@echo "Type make help for additional options"
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -fprofile-instr-generate -o $(EXENAME) src/*.cpp
	@echo bench 12 | ./rodentIII > /dev/null
	@llvm-profdata merge -output=default.profdata default.profraw
	$(CXX) $(CXXFLAGS) -fprofile-instr-use=default.profdata $(LDFLAGS) -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)
	@rm -f *.profdata *.profraw

clang-popcnt-profiled:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) -msse3 -mpopcnt -fprofile-instr-generate $(LDFLAGS) -o $(EXENAME) src/*.cpp
	@echo bench 12 | ./rodentIII > /dev/null
	@llvm-profdata merge -output=default.profdata default.profraw
	$(CXX) $(CXXFLAGS) -msse3 -mpopcnt -fprofile-instr-use=default.profdata $(LDFLAGS) -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)
	@rm -f *.profdata *.profraw

gcc-profiled:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) -fprofile-generate $(LDFLAGS) -lgcov -o $(EXENAME) src/*.cpp
	@echo bench 12 | ./rodentIII > /dev/null
	$(CXX) $(CXXFLAGS) -fprofile-use $(LDFLAGS) -lgcov -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)
	@rm -f *.gcda

gcc-popcnt-profiled:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) -msse3 -mpopcnt -fprofile-generate $(LDFLAGS) -lgcov -o $(EXENAME) src/*.cpp
	@echo bench 12 | ./rodentIII > /dev/null
	$(CXX) $(CXXFLAGS) -msse3 -mpopcnt -fprofile-use $(LDFLAGS) -lgcov -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)
	@rm -f *.gcda

build-static:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -static -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)

build-debug:
	@echo "Type make help for additional options"
	$(CXX) $(CXX1FLAGS) $(LD1FLAGS) -o $(EXENAME) src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)
	
bookgen:
	@echo "Type make help for additional options"
	$(CXX) $(CXXFLAGS) -DBOOKGEN $(LDFLAGS) -o $(EXENAME)-bookgen src/*.cpp
	@echo "SHOW_OPTIONS" > $(CONFIGFILE)
	@./$(EXENAME)-bookgen
	@rm -f $(EXENAME)-bookgen 

clean:
	rm -f $(EXENAME) $(EXENAME).exe book_gen.h basic.ini *.profdata *.profraw *.gcda

install:
	mkdir -p $(BINDIR)
	mkdir -p $(DATADIR)
	cp $(EXENAME) $(BINDIR)
	cp $(CONFIGFILE) $(DATADIR)
	find ../books/ -name '*.bin' -type f -exec cp '{}' $(DATADIR) \;
	find ../personalities/ -name '*.ini' -type f -exec cp '{}' $(DATADIR) \;
	chmod 755 $(BINDIR)/$(EXENAME)
	chmod 644 $(DATADIR)/*

# this just performs a copy of rodentIII engine
update:
	cp $(EXENAME) $(BINDIR)
	
remove:
	rm -f $(BINDIR)/$(EXENAME)
	rm -rf $(DATADIR)
	
help:
	@echo ""
	@echo "To compile RodentIII type:"
	@echo ""
	@echo "make build			> Build Rodent III"
	@echo "make build-popcnt		> Build with fast popcnt"
	@echo "make clang-profiled		> Build with pgo optimizations (clang++ required)"
	@echo "make clang-popcnt-profiled	> Build with pgo+popcnt optimizations (clang++ required)"
	@echo "make gcc-profiled		> Build with pgo optimizations (g++ required)"
	@echo "make gcc-popcnt-profiled	> Build with pgo+popcnt optimizations (g++ required)"
	@echo "make build-static		> Build a static binary"
	@echo "make build-debug		> Build a binary with Logfile support"
	@echo "make bookgen			> Build internal book"
	@echo "make clean			> Clean up"
	@echo "make install			> Install RodentIII (root privileges required)"
	@echo "make update			> Update RodenIII engine (root privileges required)"
	@echo "make remove			> Remove RodenIII from your system (root privileges required)"
