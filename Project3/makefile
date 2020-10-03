LIB=-lpthread -lrt
CC=gcc
CCPP=g++

all: addem life

addem: addem.cpp
	 $(CCPP) addem.cpp mailbox.cpp -o addem $(LIB)

life: life.cpp
	 $(CCPP) life.cpp mailbox.cpp -o life $(LIB)

clean: 
	/bin/rm -f *.o core addem life