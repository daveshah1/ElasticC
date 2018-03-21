src = $(wildcard src/*.cpp) $(wildcard src/hdl/*.cpp) $(wildcard src/timing/*.cpp) src/version.cpp
obj = $(src:.cpp=.o)

CXXFLAGS = -std=c++17 -g -O0 -Isrc/ -DDEBUG
LDFLAGS =  -lboost_system -lboost_filesystem -lboost_filesystem -lboost_program_options
all: bin/elasticc

bin/elasticc: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

src/version.cpp:
	src/update_version.sh

test: bin/elasticc
	$(MAKE) -C tests/ test

.PHONY: clean src/version.cpp test
clean:
	rm -f $(obj) bin/elasticc
