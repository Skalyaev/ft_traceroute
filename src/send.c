#include "../include/header.h"

extern t_traceroute data;

byte send_udp(const ubyte* const idx,
              ubyte* const sim_queries,
              ubyte* const sent) {

    const ubyte ttl = *idx + 1;
    if(setsockopt(data.udp_socket, SOL_IP, IP_TTL, &ttl, TTL_SIZE) < 0) {

        printf("setsockopt IP_TTL: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_hop* hop = data.hops[*idx];
    if(!hop->probes) {

        const size_t size = data.opts.queries * PTR_SIZE;
        hop->probes = malloc(size);
        if(!hop->probes) {

            printf("malloc: %s\n", strerror(errno));
            data.code = errno;
            return EXIT_FAILURE;
        }
        memset(hop->probes, 0, size);
    }
    const size_t sendbuffsize = data.opts.packetlen - IP_SIZE;
    char sendbuff[sendbuffsize];
    memset(sendbuff, 0, sendbuffsize);

    for(ubyte x = 0; x < data.opts.queries; x++) {

        if(hop->probes[x]) continue;
        data.addr.sin_port = htons(data.opts.port);

        if(sendto(data.udp_socket, sendbuff, sendbuffsize, 0,
                  (t_sockaddr*)&data.addr, SOCKADDRIN_SIZE) < 0) {

            printf("sendto: %s\n", strerror(errno));
            data.code = errno;
            return EXIT_FAILURE;
        }
        t_timeval start;
        gettimeofday(&start, NULL);

        hop->probes[x] = malloc(PROBE_SIZE);
        if(!hop->probes[x]) {

            printf("malloc: %s\n", strerror(errno));
            data.code = errno;
            return EXIT_FAILURE;
        }
        memset(hop->probes[x], 0, PROBE_SIZE);

        hop->probes[x]->port = data.opts.port;
        hop->probes[x]->lost = YES;
        hop->probes[x]->start = start;

        data.opts.port++;
        if(data.opts.port < PORT_MIN) data.opts.port = PORT_MIN;

        (*sent)++;
        (*sim_queries)--;
        if(!*sim_queries) break;
    }
    return EXIT_SUCCESS;
}
