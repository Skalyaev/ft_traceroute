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

    pthread_mutex_destroy(&data.hops_mutex);
    pthread_mutex_destroy(&data.reached_mutex);
    pthread_mutex_destroy(&data.abort_mutex);
    return data.code;
}

int main(int ac, char** av) {

    setlocale(LC_ALL, "");
    if(ac < 2) {
        printf(USAGE, av[0]);
        return EXIT_USAGE;
    }
    getargs(ac, av);
    if(init() != EXIT_SUCCESS) return bye();

    ubyte sim_queries = data.opts.sim_queries;
    ubyte sent = 0;
    for(ubyte ttl = data.opts.first - 1; ttl < data.opts.max_hops; ttl++) {

        if(send_udp(&ttl, &sim_queries, &sent) != EXIT_SUCCESS) break;
        if(sim_queries && ttl < data.opts.max_hops - 1) continue;
        if(!sim_queries) ttl--;

        if(recv_icmp(&sent) != EXIT_SUCCESS) break;
        sim_queries = data.opts.sim_queries;
        sent = 0;
    }
    return bye();
}
