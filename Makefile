CXX = gcc
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -g \
           -Wnull-dereference -Wshadow -Wformat=2 \
           -Wuninitialized -std=c++11
LDFLAGS = -fsanitize=address -fsanitize=undefined
LDLIBS = -lGL -ldl -lm -lX11 -lXi -lXcursor -lstdc++ -lglfw
INCLUDES = -Ilib/imgui -Ilib/sokol -Ilib/sokol/util -Ilib/glm -Ilib/stb
TARGET=starterkit

IMGUI_CXXFLAGS=-std=c++11 -Ilib/imgui -Ilib/imgui/backends
IMGUI_SOURCES=$(wildcard lib/imgui/*.cpp) \
				lib/imgui/backends/imgui_impl_glfw.cpp \
				lib/imgui/backends/imgui_impl_opengl3.cpp
IMGUI_OBJECTS=$(IMGUI_SOURCES:.cpp=.o)

$(TARGET): src/main.cpp src/Camera.cpp src/Model.cpp src/Light.cpp src/Scene.cpp src/Texture.cpp src/Mesh.cpp src/State.cpp $(IMGUI_OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS) $(LDFLAGS) $(INCLUDES)

$(IMGUI_OBJECTS): %.o: %.cpp
	$(CXX) -c $< -o $@ $(IMGUI_CXXFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(LDLIBS) $(INCLUDES)

clean:
	@rm -f lib/imgui/*.o $(TARGET)

sokol-shdc:
	wget -q https://github.com/floooh/sokol-tools-bin/raw/master/bin/linux/sokol-shdc
	chmod +x sokol-shdc

shader: sokol-shdc
	./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l glsl430

bootstrap: sokol-shdc shader
	git submodule update --init --recursive

.PHONY: bootstrap clean
