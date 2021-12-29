CXX := g++ -std=c++11
CXXFLAGS := -ggdb
FLEX := flex
BISON := bison
RM := rm -rf
SED := sed

.EXPORT_ALL_VARIABLES:

.PHONY: all build clean test

all: build

build:
	$(MAKE) -C src build

clean:
	$(MAKE) -C src clean

test: build
	src/coolc example/hello.cl
