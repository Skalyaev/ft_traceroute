#include "../include/header.h"

t_traceroute data = {0};

static byte bye() {

    if(data.host) free(data.host);
    if(data.opts.interface) free(data.opts.interface);
    if(data.hops) {
        for(ubyte x = data.opts.first - 1; x < data.opts.max_hops; x++) {

            if(!data.hops[x]) continue;
            if(data.hops[x]->probes) {

                for(ubyte y = 0; y < data.opts.queries; y++)
                    if(data.hops[x]->probes[y]) free(data.hops[x]->probes[y]);
                free(data.hops[x]->probes);
            }
            free(data.hops[x]);
        }
        free(data.hops);
    }
    if(data.udp_socket) close(data.udp_socket);
    if(data.icmp_socket) close(data.icmp_socket);
    return data.code;
}

int main(int ac, char** av) {

    if(ac < 2) {
        printf(USAGE);
        return EXIT_USAGE;
    }
    getargs(ac, av);
    const size_t minsize = sizeof(t_ip) + sizeof(t_seq);
    const size_t size = data.opts.packetlen < minsize ? minsize : data.opts.packetlen;

    printf("traceroute to %s (%s), %u hops max, %lu byte packets\n",
           data.host, inet_ntoa(data.addr.sin_addr), data.opts.max_hops, size);

    ubyte sim_queries = data.opts.sim_queries;
    ubyte sent = 0;
    byte code;
    for(ubyte ttl = data.opts.first - 1; ttl < data.opts.max_hops; ttl++) {

        if(send_udp(&ttl, &sim_queries, &sent) != EXIT_SUCCESS) return bye();
        if(sim_queries && ttl < data.opts.max_hops - 1) continue;
        if(!sim_queries) ttl--;

        code = recv_icmp(&sent);
        if(code == EXIT_FAILURE) return bye();
        if(code == EXIT_REACHED) break;
        sim_queries = data.opts.sim_queries;
        sent = 0;
    }
    return bye();
}
