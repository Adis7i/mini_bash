CXX = g++
CXXFLAGS = -std=c++17
SRC_DIR = src
INC_DIR = include
ROOT = .

.PHONY: all
all: main

main: main.o lib.o
	$(CXX) $^ -o $@

main.o: main.cpp
	$(CXX) -c $^ -o $@

lib.o: src/module/lsdir.cpp
	$(CXX) -c $^ -o $@
