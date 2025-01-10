#ifndef HEADER_H
#define HEADER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "define.h"
#include "struct.h"

void getargs(const int ac, char** const av);
byte init();
byte send_udp(const ubyte* const idx,
              ubyte* const sim_queries,
              ubyte* const sent);
byte recv_icmp(const ubyte* const sent);

#endif
