#ifndef UTILITIES_H_
#define UTILITIES_H_

#define PIPE 0
#define SOCKET 1
#define DATA 0 
#define INFO 1
#define DNSNAME 2
#define DNSACK 3
#define DNSREQ 4
#define DNSREP 5
#define DLREQ 6

/* A collection of useful functions */

int asciiValue(char c);
void int2Ascii(char c[], int value);
int point2Word(char c[], int k);
void copyWord(char word[], char c[], int k);
void findWord(char word[], char c[], int k);
void appendWithSpace(char c1[], char c2[]);
int ascii2Int(char c[]);

#endif
