CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
SDLFLAGS = $(shell sdl2-config --cflags --libs)

TARGET = chip8
SOURCES = main.cpp chip8.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SDLFLAGS)

clean:
	rm -f $(TARGET)
