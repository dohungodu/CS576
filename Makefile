
CC = gcc
TARGETS= test panserver panclient panclient audclient newserver
RM		= rm -rf 

all: $(TARGETS)

test: test.cpp
	g++ -std=c++11 test.cpp -o test

panserver: panserver.cpp
	g++ -std=c++11 panserver.cpp -o panserver	

panclient:  panclient.o
	$(CC)  -o panclient panclient.c $(LIBS)
	
audclient:  audclient.o
	$(CC)  -o audclient audclient.c $(LIBS)

newserver:  newserver.cpp
	g++ -std=c++11 newserver.cpp -o newserver	
	
clean:;
	$(RM) $(PROGS) *.o core 