CXX=g++
CXXFLAGS=-Wall -Wextra -O2 -pthread

all: p5

p5: p5.cpp
	$(CXX) $(CXXFLAGS) -o p5 p5.cpp

clean:
	rm -f p5