.PHONY: all build clean

all: build

build: coolc

SRCS := cool.l.cc cool.y.cc ast.cc parser.cc main.cc
OBJS := $(patsubst %.cc,%.o,$(SRCS))

coolc: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

cool.l.cc cool.y.cc: cool.l cool.y
	$(BISON) cool.y
	$(FLEX) cool.l
	$(SED) -i -e 's@^#line@//#line@' cool.l.cc

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $^

clean:
	$(RM) cool.l.cc cool.y.cc cool.l.hh cool.y.hh location.hh $(OBJS) coolc
