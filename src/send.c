#include "../include/header.h"

extern t_traceroute data;

static byte init() {

    t_addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    t_addrinfo* res;
    if(getaddrinfo(data.host, 0, &hints, &res) < 0) {

        strerror(errno);
        data.code = errno;
        return EXIT_FAILURE;
    }
    data.addr.sin_family = AF_INET;
    data.addr.sin_port = htons(data.opts.port);
    data.addr.sin_addr = ((t_sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);

    if((data.udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {

        strerror(errno);
        data.code = errno;
        return EXIT_FAILURE;
    }
    if(data.opts.interface) {
        if(setsockopt(data.udp_socket, SOL_SOCKET,
                      SO_BINDTODEVICE, data.opts.interface,
                      strlen(data.opts.interface)) < 0) {

            strerror(errno);
            data.code = errno;
            return EXIT_FAILURE;
        }
    }
    if(data.opts.fragment) {
        const int df = IP_PMTUDISC_DO;
        if(setsockopt(data.udp_socket, IPPROTO_IP,
                      IP_MTU_DISCOVER, &df, sizeof(df)) < 0) {

            strerror(errno);
            data.code = errno;
            return EXIT_FAILURE;
        }
    }
    size_t size = data.opts.max_hops * sizeof(t_hop*);
    data.hops = malloc(size);
    if(!data.hops) {

        strerror(errno);
        data.code = errno;
        return EXIT_FAILURE;
    }
    size = sizeof(t_hop);
    for(ubyte x = data.opts.first - 1; x < data.opts.max_hops; x++) {

        data.hops[x] = malloc(size);
        if(!data.hops[x]) {

            strerror(errno);
            data.code = errno;
            return EXIT_FAILURE;
        }
        memset(data.hops[x], 0, size);
    }
    return EXIT_SUCCESS;
}

byte send_udp(const ubyte* const idx,
              ubyte* const sim_queries,
              ubyte* const sent) {

    if(!data.udp_socket && init() != EXIT_SUCCESS) return EXIT_FAILURE;

    static const size_t probesize = sizeof(t_probe);
    static const size_t ttlsize = sizeof(ubyte);
    static ubyte seq = 0;

    const ubyte ttl = *idx + 1;
    if(setsockopt(data.udp_socket, SOL_IP, IP_TTL, &ttl, ttlsize) < 0) {

        strerror(errno);
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_hop* hop = data.hops[*idx];
    if(!hop->probes) {

        const size_t size = data.opts.queries * sizeof(t_probe*);
        hop->probes = malloc(size);
        if(!hop->probes) {

            strerror(errno);
            data.code = errno;
            return EXIT_FAILURE;
        }
        memset(hop->probes, 0, size);
    }
    const size_t sendbuffsize = data.opts.packetlen - sizeof(t_ip);
    char sendbuff[sendbuffsize];
    memset(sendbuff, 0, sendbuffsize);

    for(ubyte x = 0; x < data.opts.queries; x++) {

        if(hop->probes[x]) continue;
        ((t_seq*)sendbuff)->seq = htons(seq);

        t_timeval start;
        gettimeofday(&start, NULL);
        if(sendto(data.udp_socket, sendbuff, sendbuffsize, 0,
                  (t_sockaddr*)&data.addr, sizeof(t_sockaddr_in)) < 0) {

            strerror(errno);
            data.code = errno;
            return EXIT_FAILURE;
        }
        printf("sent probe %u\n", seq);
        hop->probes[x] = malloc(probesize);
        if(!hop->probes[x]) {

            strerror(errno);
            data.code = errno;
            return EXIT_FAILURE;
        }
        memset(hop->probes[x], 0, probesize);

        hop->probes[x]->seq = seq;
        hop->probes[x]->lost = YES;
        hop->probes[x]->start = start;

        seq++;
        *sent += 1;
        *sim_queries -= 1;
        if(!*sim_queries) break;
    }
    return EXIT_SUCCESS;
}
