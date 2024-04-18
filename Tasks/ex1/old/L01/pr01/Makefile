# ***************************************************************************
#
# Simple project Makefile suitable for command line and IDE
#
# petr.olivka@vsb.cz, Dept. of Computer Science, FEECS, VSB-TU Ostrava, CZ
#
# ***************************************************************************

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	g++ -g -pthread -std=c++11 $^ -o $@

clean:
	rm -f $(TARGET)
	
a: b c
	echo $@
	
b:
	echo plnim $@
	
c:
	echo plnim $@
