--TOPOLOGY CONFIGURATION DOCUMENTATION--

PART I. Setting up numbers/counters
      The initial line of the topology file needs to consists of four numbers
      corresponding to your desired topology

PART II. Setting up Connections
      The secondary part of the topology file consists of setting up the
      specific links that make up your network topology.

PART III. Setting up the DNS
      The last part of the topology file should contain the link or socket
      information for the DNS.


Part I. Setting up numbers/counters
==============================================

The first line in the topology file:
A B C D

----------------------------------------------
Info:
A [LINK] is communication via PIPES
A [SOCKET] is communication via PORTS
A [CONNECTION] is either a SOCKET or a LINK
----------------------------------------------

A is an INTEGER that corresponds to the NUMBER OF HOSTS
B is an INTEGER that corresponds to the NUMBER OF LINKS x 2
C is an INTEGER that corresponds to the NUMBER OF SWITCHES
D is an INTEGER that corresponds to the NUMBER OF SOCKETS x 2

** Since links are assumed to be BIDIRECTIONAL you need to set mumber of links
or sockets to be 2 times the number of "physical" links. 

==============================================

Part II. Setting up Links/Sockets e.g. Connections
==============================================

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

NOTE: Setting up links are done in numerical ORDER as a result, you need to
create links for the hosts where the HOST ID NUMBERS are the LEAST. For example
if I want a topology with 3 hosts and 2 switches, the IDs must be as the
   following: 0,1,2 are HOSTS, 3 and 4 are SWITCH IDS.

When writing the TOPOLOGY file, links should be created in numerical order
based on the value of A (the source).

==============================================


PART III. Setting up the Domain Name Server
==============================================

DNS should be DEFINED at the VERY END of the topology file for most cases.

It is assumed to have an ID of 100. This line basically looks the same as in
PART 2 except the A or B parameter has to be 100 for the DNS, AND this has to be the very last line of the topology file IF the source (A Parameter) is the DNS.

If you configured a switch to connect to the DNS and use the DNS initially as a
destination, then this line does not need to be at the end.

==============================================
