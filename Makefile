src = $(wildcard src/*.cpp) src/version.cpp
obj = $(src:.cpp=.o)

CXXFLAGS = -std=c++14 -g -O3 
LDFLAGS =  -lboost_system -lboost_filesystem
all: rapidhls

rapidhls: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

src/version.cpp:
	src/update_version.sh

.PHONY: clean version.cpp
clean:
	rm -f $(obj) rapidhls