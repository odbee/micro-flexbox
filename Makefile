# Define the compiler
CC = gcc
CXX = g++

# Compiler and linker flags
# -IInclude: Look for header files in the Include directory
# -std=c99: Enable C99 features for C files
# -std=c++11: Enable a modern C++ standard for C++ files (or c++14, c++17, c++20)
CFLAGS = -IInclude -Wall -Wextra -Wundef -g -O0 -std=c99 -DDEBUG
CXXFLAGS = -IInclude -Wall -Wextra -Wundef -std=c++2a

# Libraries to link
# Add -lopengl32 to link against the OpenGL library
LDFLAGS = -Lsrc/lib
LIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32

# The name of the final executable
TARGET = main


# The object files for the project
# This is a list of all your .o files
OBJS = main.o renderer.o microui_flex.o


# The default target. It depends on the object files.
# The command links the objects into the executable.
all: $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)


# The rule for compiling C++ files
# This is a built-in rule in make, but it's good practice to be explicit
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# The rule for compiling C files
# This is a built-in rule in make
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phony targets don't correspond to files
.PHONY: all clean

clean:
	rm -f $(OBJS) $(TARGET)