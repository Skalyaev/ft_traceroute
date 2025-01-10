#ifndef STRUCT_H
#define STRUCT_H

typedef struct s_opts {
    bool fragment;
    ubyte first;
    char* interface;
    ushort max_hops;
    ubyte sim_queries;
    bool resolve;
    ushort port;
    double wait;
    ubyte queries;
    ushort packetlen;
} t_opts;

typedef struct s_probe {
    t_timeval start;
    ushort port;
    char ip[INET_ADDRSTRLEN];
    char host[NI_MAXHOST];
    double rtt;
    bool lost;
    bool printed;
} t_probe;

typedef struct s_hop {
    t_probe** probes;
} t_hop;

typedef struct s_traceroute {
    t_opts opts;
    char* host;
    byte code;
    int udp_socket;
    int icmp_socket;
    t_sockaddr_in addr;
    t_hop** hops;
    byte reached;
    bool abort;
    pthread_mutex_t hops_mutex;
    pthread_mutex_t reached_mutex;
    pthread_mutex_t abort_mutex;
} t_traceroute;

#endif
