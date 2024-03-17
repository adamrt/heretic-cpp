CXX = gcc
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -g \
           -Wnull-dereference -Wshadow -Wformat=2 \
           -Wuninitialized -std=c++11
LDFLAGS = -fsanitize=address -fsanitize=undefined
LDLIBS = -lGL -ldl -lm -lX11 -lXi -lXcursor -lstdc++ -lglfw
INCLUDES = -Ilib/imgui -Ilib/sokol -Ilib/sokol/util
TARGET=starterkit

IMGUI_CXXFLAGS=-std=c++11 -Ilib/imgui -Ilib/imgui/backends
IMGUI_SOURCES=$(wildcard lib/imgui/*.cpp) \
				lib/imgui/backends/imgui_impl_glfw.cpp \
				lib/imgui/backends/imgui_impl_opengl3.cpp
IMGUI_OBJECTS=$(IMGUI_SOURCES:.cpp=.o)

$(TARGET): src/main.cpp $(IMGUI_OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS) $(INCLUDES)

$(IMGUI_OBJECTS): %.o: %.cpp
	$(CXX) -c $< -o $@ $(IMGUI_CXXFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(LDLIBS) $(INCLUDES)

clean:
	@rm -f lib/imgui/*.o $(TARGET)

bootstrap:
	git submodule update --init --recursive

.PHONY: bootstrap clean
