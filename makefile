net367: host.o utilities.o link.o man.o main.o net.o table.o queue.o switch.o dns.o ntable.o
	gcc -o net367 host.o utilities.o link.o man.o main.o net.o table.o queue.o switch.o dns.o ntable.o

main.o: main.c
	gcc -c main.c

host.o: host.c 
	gcc -c host.c  

man.o:  man.c
	gcc -c man.c

net.o:  net.c
	gcc -c net.c

utilities.o: utilities.c
	gcc -c utilities.c

link.o:  link.c
	gcc -c link.c

table.o: table.c
	gcc -c table.c

ntable.o: ntable.c
	gcc -c ntable.c

dns.o: dns.c
	gcc -c dns.c

queue.o: queue.c
	gcc -c queue.c

switch.o: switch.c
	gcc -c switch.c

clean:
	rm -f *.o

real_clean: clean
	rm -f net367


