CC := g++
COMMON_FLAGS := -Wall -Wextra -std=c++17
DEBUG_FLAGS := -O0 -g
RELEASE_FLAGS := -O2
INCLUDE_DIR := include 
ARGPARSE_INCLUDE_DIR := ext/argparse/include/argparse
SRC_FILES := src/config.cpp src/asyd.cpp

build:
	$(CC) $(COMMON_FLAGS) $(RELEASE_FLAGS) -I$(INCLUDE_DIR) -I$(ARGPARSE_INCLUDE_DIR) $(SRC_FILES) -o out/asyd

debug:
	$(CC) $(COMMON_FLAGS) $(DEBUG_FLAGS) -I$(INCLUDE_DIR) -I$(ARGPARSE_INCLUDE_DIR) $(SRC_FILES) -o out/asyd
