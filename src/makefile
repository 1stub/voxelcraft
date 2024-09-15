# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/sysprof-6 -pthread -std=c++17 #-fsanitize=address
LDFLAGS = -lfreetype -lGL -lglfw -lm

# Source files
SRCS = ../resources/stb_image.cpp block.cpp chunk.cpp chunkManager.cpp raycast.cpp main.cpp glad.c

# Output executable
TARGET = prog

# Build rule
$(TARGET): $(SRCS)
	$(CXX) -o $(TARGET) -g $(SRCS) $(CXXFLAGS) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)
