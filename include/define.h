#ifndef DEFINE_H
#define DEFINE_H

#define USAGE "Usage:\n"\
"  traceroute [ -ifmqn ] [ -i interface ] [ -f first_ttl ]"\
" [ -m max_hops ] [ -q queries ] host [ packetlen ]\n"\
"Options:\n"\
"  -F  --dont-fragment                  Do not fragment packets\n"\
"  -f first_ttl  --first=first_ttl      Start from the first_ttl hop (instead from 1)\n"\
"  -i device  --interface=device        Specify a network interface to operate with\n"\
"  -m max_ttl  --max-hops=max_ttl       Set the max number of hops (max TTL to be reached)."\
                                        " Default is 30\n"\
"  -N squeries  --sim-queries=squeries  Set the number of probes to be tried simultaneously."\
                                        " Default is 16\n"\
"  -n                                   Do not resolve IP addresses to their domain names\n"\
"  -p port  --port=port                 Set the initial destination port to use."\
                                        " Default is 33434\n"\
"  -q nqueries  --queries=nqueries      Set the number of probes per each hop. Default is 3\n"\
"  --help                               Read this help and exit\n"\
"\n"\
"Arguments:\n"\
"  host         The host to traceroute to\n"\
"  packetlen    The full packet length (default is the length of an IP\n"\
"               header, plus 40). Can be ignored or increased to a minimal allowed value\n"

#define ERR_BAD_OPT "Bad option `%s' (argc %d)\n"
#define ERR_MISS_OPT "Option `%s' (argc %d) requires an argument: `%s'\n"

#define ERR_OPT_ARG "Cannot handle `%s' option with arg `%s' (argc %d)\n"
#define ERR_CMD_ARG "Cannot handle \"%s\" cmdline arg `%s' on position 2 (argc %d)\n"
#define ERR_XTRA_ARG "Extra arg `%s' (position 3, argc %d)\n"

#define EXIT_REACHED -1
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EXIT_USAGE 2

#define YES 1
#define NO 0

#define PORT_MIN 33434

#define BUFFER_SIZE 1024
#define PTR_SIZE sizeof(void*)
#define INT_SIZE sizeof(int)
#define TIMEVAL_SIZE sizeof(t_timeval)
#define SOCKADDR_SIZE sizeof(t_sockaddr)
#define SOCKADDRIN_SIZE sizeof(t_sockaddr_in)
#define ICMPHDR_SIZE sizeof(t_icmphdr)
#define UDPHDR_SIZE sizeof(t_udphdr)
#define IP_SIZE sizeof(t_ip)
#define TTL_SIZE sizeof(ubyte)
#define PROBE_SIZE sizeof(t_probe)
#define HOP_SIZE sizeof(t_hop)

typedef unsigned char bool;
typedef unsigned char ubyte;
typedef char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef struct ip t_ip;
typedef struct sockaddr t_sockaddr;
typedef struct sockaddr_in t_sockaddr_in;
typedef struct addrinfo t_addrinfo;
typedef struct timeval t_timeval;
typedef struct icmphdr t_icmphdr;
typedef struct udphdr t_udphdr;

#endif
