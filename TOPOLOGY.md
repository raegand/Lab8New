[Topology file is setup as follows]

The first line in the topology file:
A B C D

----------------------------------------------
Info:
A [LINK] is communication via PIPES
A [SOCKET] is communication via PORTS
A [CONNECTION] is either a SOCKET or a LINK
----------------------------------------------

A is an INTEGER that corresponds to the NUMBER OF HOSTS
B is an INTEGER that corresponds to the NUMBER OF LINKS
C is an INTEGER that corresponds to the NUMBER OF SWITCHES
D is an INTEGER that corresponds to the NUMBER OF SOCKETS


-->The rest of the lines are used to setup links and connections between
components such


Setting up links:
A B CCCC DDDD

A is an INTEGER that corresponds to the SENDER of the CONNECTION
B is an INTEGER that corresponds to the RECEIVER of the CONNECTION
CCCC is a LISTENING PORT NUMBER (1 to 4 DIGIT INTEGER) of the SENDER
-> So in this case, say if A is a switch, then it listens to PORT CCCC for data
DDDD is a LISTENING PORT NUMBER (1 to 4 DIGIT INTEGER) of the RECEIVER

LINKS are assumed to be bidirectional so you do not need to define two links 

If you wish to define a PIPE based LINK instead of using sockets, simply use
the PORT NUMBER of 0. 



