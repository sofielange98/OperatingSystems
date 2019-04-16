
.PHONY: all clean

all: multi-lookup
multi-lookup: multi-lookup.o queue.o util.o
	gcc -pthread -Wall -Wextra $^ -o $@
util.o: util.c util.h
	gcc -c -g -Wall -Wextra $<
queue.o: queue.c queue.h
	gcc -c -g -Wall -Wextra $<
multi-lookup.o: multi-lookup.c
	gcc -c -g -Wall -Wextra $<

clean:
	rm -f multi-lookup
	rm -f *.o
	rm -f *~
	rm -f results.txt
	rm -f serviced.txt
