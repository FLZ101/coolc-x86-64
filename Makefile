CXX := g++ -std=c++11
CXXFLAGS := -ggdb
FLEX := flex
BISON := bison
RM := rm -rf
SED := sed

SHELL=/bin/bash

.EXPORT_ALL_VARIABLES:

.PHONY:: all build test clean clean-build clean-test

all: build

build:
	$(MAKE) -C src build

clean: clean-build clean-test

clean-build:
	$(MAKE) -C src clean

define clean-it =
	rm -f test/$(name) test/$(name).s
endef

define test-it =
@echo "test $(name)" && \
	src/coolc test/$(name) test/$(name).cl && \
	diff <(test/$(name) 2>&1) <(cat test/$(name).cl | sed -n -E -e 's@^.*-- (.*)$$@\1@p')
endef

define test-it-with-input =
@echo "test $(name)" && \
	src/coolc test/$(name) test/$(name).cl && \
	diff <(echo "$(input)" | test/$(name) 2>&1) <(cat test/$(name).cl | sed -n -E -e 's@^.*-- (.*)$$@\1@p')
endef

.PHONY test:: test-io
test-io: name=io
test-io: input=the quick brown fox jumps over the lazy dog
test-io: build
	$(test-it-with-input)

.PHONY clean-test:: clean-test-io
clean-test-io: name=io
clean-test-io:
	$(clean-it)

.PHONY test:: test-string
test-string: name=string
test-string: build
	$(test-it)

.PHONY clean-test:: clean-test-string
clean-test-string: name=string
clean-test-string:
	$(clean-it)

.PHONY test:: test-int
test-int: name=int
test-int: build
	$(test-it)

.PHONY clean-test:: clean-test-int
clean-test-int: name=int
clean-test-int:
	$(clean-it)

.PHONY test:: test-let
test-let: name=let
test-let: build
	$(test-it)

.PHONY clean-test:: clean-test-let
clean-test-let: name=let
clean-test-let:
	$(clean-it)

.PHONY test:: test-if
test-if: name=if
test-if: build
	$(test-it)

.PHONY clean-test:: clean-test-if
clean-test-if: name=if
clean-test-if:
	$(clean-it)

.PHONY test:: test-while
test-while: name=while
test-while: build
	$(test-it)

.PHONY clean-test:: clean-test-while
clean-test-while: name=while
clean-test-while:
	$(clean-it)

.PHONY test:: test-recursion
test-recursion: name=recursion
test-recursion: build
	$(test-it)

.PHONY clean-test:: clean-test-recursion
clean-test-recursion: name=recursion
clean-test-recursion:
	$(clean-it)

.PHONY test:: test-isvoid
test-isvoid: name=isvoid
test-isvoid: build
	$(test-it)

.PHONY clean-test:: clean-test-isvoid
clean-test-isvoid: name=isvoid
clean-test-isvoid:
	$(clean-it)

.PHONY test:: test-new
test-new: name=new
test-new: build
	$(test-it)

.PHONY clean-test:: clean-test-new
clean-test-new: name=new
clean-test-new:
	$(clean-it)

.PHONY test:: test-inheritance
test-inheritance: name=inheritance
test-inheritance: build
	$(test-it)

.PHONY clean-test:: clean-test-inheritance
clean-test-inheritance: name=inheritance
clean-test-inheritance:
	$(clean-it)

.PHONY test:: test-polymorphism
test-polymorphism: name=polymorphism
test-polymorphism: build
	$(test-it)

.PHONY clean-test:: clean-test-polymorphism
clean-test-polymorphism: name=polymorphism
clean-test-polymorphism:
	$(clean-it)

.PHONY test:: test-case
test-case: name=case
test-case: build
	$(test-it)

.PHONY clean-test:: clean-test-case
clean-test-case: name=case
clean-test-case:
	$(clean-it)

.PHONY test:: test-abort
test-abort: name=abort
test-abort: build
	$(test-it)

.PHONY clean-test:: clean-test-abort
clean-test-abort: name=abort
clean-test-abort:
	$(clean-it)

.PHONY test:: test-invoke_on_void
test-invoke_on_void: name=invoke_on_void
test-invoke_on_void: build
	$(test-it)

.PHONY clean-test:: clean-test-invoke_on_void
clean-test-invoke_on_void: name=invoke_on_void
clean-test-invoke_on_void:
	$(clean-it)

.PHONY test:: test-case_no_match
test-case_no_match: name=case_no_match
test-case_no_match: build
	$(test-it)

.PHONY clean-test:: clean-test-case_no_match
clean-test-case_no_match: name=case_no_match
clean-test-case_no_match:
	$(clean-it)

.PHONY test:: test-case_on_void
test-case_on_void: name=case_on_void
test-case_on_void: build
	$(test-it)

.PHONY clean-test:: clean-test-case_on_void
clean-test-case_on_void: name=case_on_void
clean-test-case_on_void:
	$(clean-it)
