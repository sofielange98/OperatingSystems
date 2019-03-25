To compile the program, run
make
clean:
make clean

then, run ./multi-lookup <num requesters> <num resolvers> <results.txt/ resolver log> <serviced.txt/ requester log> <list any name files containing domain names>

The output should be any errors produced in finding the domain, followed by the time taken to complete in microseconds

~~files included~~
multi-lookup.c
--Contains c code to resolve dns requests coming in from a file with one domain name per line using multi-threading and mutual exclusion of shared resources

multi-lookup.h
--Function prototypes and struct definitions

Makefile
--will compile all necessary files in order to execute multi-lookup

performance.txt
--evaluation of the performance of the program under given specifications

performance.png 
--graph of different numbers of requesters and resolvers and the corresponding run times

queue.c/queue.h
--contains an implementation of a queue data structure which is convenient for this shared buffer scenario, created by other students
