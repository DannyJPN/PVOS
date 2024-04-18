SOURCES=$(wildcard *.cpp)
OBJS=$(SOURCES:%.cpp=%.o)
TARGETS=$(SOURCES:%.cpp=%)

CPPFLAGS=-g -pthread -std=c++11 -Wall -fsanitize=address
LDFLAGS=-g -pthread -lrt -fsanitize=address

.PHONY: all clean 

all: $(TARGETS)

%.o: %.cpp
	g++ -c $(CPPFLAGS) $^ -o $@
	
$(TARGETS): %: %.o
	g++  $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o $(TARGETS)
