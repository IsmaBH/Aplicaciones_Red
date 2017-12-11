#define CONNMAX 100
#define MAXBYTES 1024

char *ROOT;
int sockfd, clients[CONNMAX];
void error (char *);
void startServer (char *);
void respond (int);
