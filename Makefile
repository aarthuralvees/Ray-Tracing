CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

all: render

render: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o render

test: main.cpp
	$(CXX) $(CXXFLAGS) -DRUN_TESTS main.cpp -o test_runner
	./test_runner

image: render
	./render > image.ppm

clean:
	rm -f render test_runner image.ppm
