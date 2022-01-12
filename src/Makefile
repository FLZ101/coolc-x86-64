.PHONY: all build clean

all: build

build: coolc

SRCS := cool.l.cc cool.y.cc util.cc ast.cc parser.cc main.cc sa.cc cg.cc
OBJS := $(patsubst %.cc,%.o,$(SRCS))
DEPS := $(patsubst %.cc,%.d,$(SRCS))

-include $(DEPS)

coolc: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

cool.l.cc cool.y.cc: cool.l cool.y
	$(BISON) cool.y
	$(FLEX) cool.l
	$(SED) -i -e 's@^#line@//#line@' cool.l.cc

%.o: %.cc
	$(CXX) $(CXXFLAGS) -MMD -c $<

clean:
	$(RM) cool.l.cc cool.y.cc cool.l.hh cool.y.hh location.hh *.gch $(OBJS) $(DEPS) coolc
