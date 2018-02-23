src = $(wildcard src/*.cpp) $(wildcard src/hdl/*.cpp) $(wildcard src/timing/*.cpp) src/version.cpp
obj = $(src:.cpp=.o)

CXXFLAGS = -std=c++14 -g -O3 -Isrc/
LDFLAGS =  -lboost_system -lboost_filesystem -lboost_filesystem -lboost_program_options
all: bin/elasticc

bin/elasticc: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

src/version.cpp:
	src/update_version.sh

.PHONY: clean src/version.cpp
clean:
	rm -f $(obj) bin/elasticc
