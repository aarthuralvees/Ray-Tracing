CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

ifeq ($(OS),Windows_NT)
    EXE    = render.exe
    RUNNER = render.exe
    TESTER = test_runner.exe
    CLEAN  = del /f render.exe test_runner.exe image.ppm 2>nul & exit 0
else
    EXE    = render
    RUNNER = ./render
    TESTER = ./test_runner
    CLEAN  = rm -f render test_runner image.ppm
endif

all: $(EXE)

$(EXE): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(EXE)

test: main.cpp
	$(CXX) $(CXXFLAGS) -DRUN_TESTS main.cpp -o $(TESTER)
	$(TESTER)

image: $(EXE)
	$(RUNNER) utils/input/test.json > image.ppm

clean:
	$(CLEAN)
