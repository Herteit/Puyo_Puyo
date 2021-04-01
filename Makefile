CXX = clang++
CXXFLAGS = -Wall -g -O2 -std=c++11
LDFLAGS = -lsfml-system -lsfml-window -lsfml-graphics

TARGETS = puyo

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
