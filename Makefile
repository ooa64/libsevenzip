# Makefile for libsevenzip
# Alternative build system to CMake

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic -fPIC -Iinclude
LDFLAGS = -shared

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
LIB_DIR = lib
EXAMPLES_DIR = examples

# Source files
SOURCES = $(SRC_DIR)/libsevenzip.cpp \
          $(SRC_DIR)/exceptions.cpp \
          $(SRC_DIR)/archive.cpp

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Library name
LIBRARY = $(LIB_DIR)/libsevenzip.so
STATIC_LIB = $(LIB_DIR)/libsevenzip.a

# Example executables
EXTRACT_EXAMPLE = $(BUILD_DIR)/extract_example
COMPRESS_EXAMPLE = $(BUILD_DIR)/compress_example

# Default target
.PHONY: all
all: directories $(LIBRARY) $(STATIC_LIB) examples

# Create necessary directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build shared library
$(LIBRARY): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)

# Build static library
$(STATIC_LIB): $(OBJECTS)
	ar rcs $@ $(OBJECTS)

# Build examples
.PHONY: examples
examples: $(EXTRACT_EXAMPLE) $(COMPRESS_EXAMPLE)

$(EXTRACT_EXAMPLE): $(EXAMPLES_DIR)/extract_example.cpp $(LIBRARY)
	$(CXX) $(CXXFLAGS) -L$(LIB_DIR) $< -o $@ -lsevenzip

$(COMPRESS_EXAMPLE): $(EXAMPLES_DIR)/compress_example.cpp $(LIBRARY)
	$(CXX) $(CXXFLAGS) -L$(LIB_DIR) $< -o $@ -lsevenzip

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)

# Install library
.PHONY: install
install: all
	install -d $(DESTDIR)/usr/local/lib
	install -d $(DESTDIR)/usr/local/include/libsevenzip
	install -m 644 $(LIBRARY) $(DESTDIR)/usr/local/lib/
	install -m 644 $(STATIC_LIB) $(DESTDIR)/usr/local/lib/
	install -m 644 $(INC_DIR)/libsevenzip/*.hpp $(DESTDIR)/usr/local/include/libsevenzip/

# Uninstall library
.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)/usr/local/lib/libsevenzip.so
	rm -f $(DESTDIR)/usr/local/lib/libsevenzip.a
	rm -rf $(DESTDIR)/usr/local/include/libsevenzip

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all        - Build library and examples (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install library to system"
	@echo "  uninstall  - Remove library from system"
	@echo "  examples   - Build example programs"
	@echo "  help       - Show this help message"
