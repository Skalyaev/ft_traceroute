#include "../include/header.h"

extern t_traceroute data;

static void* output() {

    char hostbuf[NI_MAXHOST];
    t_sockaddr_in sin;

    t_hop* hop;
    ubyte x = data.opts.first - 1;
    ubyte y;
    while(x < data.opts.max_hops) {

        pthread_mutex_lock(&data.reached_mutex);
        if(data.reached >= 0 && x > data.reached) break;
        pthread_mutex_unlock(&data.reached_mutex);

        hop = data.hops[x];
        if(!hop || !hop->probes) break;
        y = 0;
        while(y < data.opts.queries) {

            if(!hop->probes[y]) break;
            if(hop->probes[y]->printed) {
                y++;
                continue;
            }
            pthread_mutex_lock(&data.hops_mutex);
            if(hop->probes[y]->lost) {

                pthread_mutex_lock(&data.abort_mutex);
                if(!data.abort) {

                    pthread_mutex_unlock(&data.abort_mutex);
                    pthread_mutex_unlock(&data.hops_mutex);
                    continue;
                }
                pthread_mutex_unlock(&data.abort_mutex);
            }
            if(!y) printf("%2u  ", x + 1);
            if(hop->probes[y]->lost) {

                pthread_mutex_unlock(&data.hops_mutex);
                printf("* ");
            }
            else {
                pthread_mutex_unlock(&data.hops_mutex);
                if(!y || strcmp(hop->probes[y]->ip, hop->probes[y - 1]->ip)) {

                    if(data.opts.resolve) {
                        memset(hostbuf, 0, NI_MAXHOST);
                        memset(&sin, 0, SOCKADDRIN_SIZE);

                        sin.sin_family = AF_INET;
                        sin.sin_addr.s_addr = inet_addr(hop->probes[y]->ip);
                        if(!getnameinfo((t_sockaddr*)&sin, SOCKADDRIN_SIZE,
                                        hostbuf, NI_MAXHOST, NULL, 0, 0)) {

                            printf("%s ", hostbuf);
                            printf("(%s)  ", hop->probes[y]->ip);
                        } else printf("%s  ", hop->probes[y]->ip);
                    } else printf("%s  ", hop->probes[y]->ip);
                }
                pthread_mutex_lock(&data.hops_mutex);
                printf("%.3f ms  ", hop->probes[y]->rtt);
                pthread_mutex_unlock(&data.hops_mutex);
            }
            hop->probes[y]->printed = YES;
            if(++y == data.opts.queries) printf("\n");
            else fflush(stdout);
        }
        x++;
    }
    return NULL;
}

byte recv_icmp(const ubyte* const sent) {

    data.reached = -1;
    data.abort = NO;

    pthread_t printer;
    pthread_create(&printer, NULL, output, NULL);

    t_ip* iphdr;
    t_ip* orig_iphdr;
    t_icmphdr* icmphdr;
    t_udphdr* udphdr;
    int iphdr_len, orig_iphdr_len;

    ushort port;
    t_probe* probe;
    t_hop* hop;

    byte idx;
    char* ptr;
    char recvbuff[BUFFER_SIZE];

    char src[INET_ADDRSTRLEN + 1];
    char org_dst[INET_ADDRSTRLEN + 1];
    memset(src, 0, INET_ADDRSTRLEN + 1);
    memset(org_dst, 0, INET_ADDRSTRLEN + 1);

    ssize_t ret;
    ubyte received = 0;
    while(received < *sent) {

        t_sockaddr_in sockaddr;
        socklen_t sockaddrlen = SOCKADDRIN_SIZE;
        memset(recvbuff, 0, BUFFER_SIZE);

        ret = recvfrom(data.icmp_socket, recvbuff, BUFFER_SIZE, 0,
                       (t_sockaddr*)&sockaddr, &sockaddrlen);
        t_timeval end;
        gettimeofday(&end, NULL);
        if(ret < 0) break;

        iphdr = (t_ip*)recvbuff;
        iphdr_len = iphdr->ip_hl * 4;
        if((size_t)ret < iphdr_len + ICMPHDR_SIZE) continue;

        icmphdr = (t_icmphdr*)(recvbuff + iphdr_len);
        if(icmphdr->type != ICMP_TIME_EXCEEDED
                && icmphdr->type != ICMP_DEST_UNREACH) continue;

        ptr = recvbuff + iphdr_len + ICMPHDR_SIZE;
        orig_iphdr = (t_ip*)ptr;
        orig_iphdr_len = orig_iphdr->ip_hl * 4;

        ptr += orig_iphdr_len;
        udphdr = (t_udphdr*)ptr;
        port = ntohs(udphdr->uh_dport);

        idx = data.opts.first - 1;
        probe = NULL;
        for(ubyte x = data.opts.first - 1; x < data.opts.max_hops; x++) {
            idx = x;
            hop = data.hops[x];
            if(!hop || !hop->probes) continue;
            for(ubyte y = 0; y < data.opts.queries; y++) {

                if(!hop->probes[y]) continue;
                if(hop->probes[y]->lost == NO) continue;
                if(hop->probes[y]->port != port) continue;

                probe = hop->probes[y];
                break;
            }
            if(probe) break;
        }
        if(!probe) continue;

        pthread_mutex_lock(&data.hops_mutex);
        probe->lost = NO;
        probe->rtt = (end.tv_sec - probe->start.tv_sec) * 1000.0
                     + (end.tv_usec - probe->start.tv_usec) / 1000.0;

        strcpy(probe->ip, inet_ntoa(iphdr->ip_src));
        pthread_mutex_unlock(&data.hops_mutex);

        strncpy(src, inet_ntoa(iphdr->ip_src), INET_ADDRSTRLEN);
        strncpy(org_dst, inet_ntoa(orig_iphdr->ip_dst), INET_ADDRSTRLEN);

        received++;
        if(strcmp(src, org_dst)) continue;
        if(data.reached >= 0 && idx > data.reached) continue;

        pthread_mutex_lock(&data.reached_mutex);
        data.reached = idx;
        pthread_mutex_unlock(&data.reached_mutex);
    }
    pthread_mutex_lock(&data.abort_mutex);
    data.abort = YES;
    pthread_mutex_unlock(&data.abort_mutex);

    pthread_join(printer, NULL);
    idx = data.reached;
    if(idx >= 0 && data.hops[(int)idx] && data.hops[(int)idx]->probes)
        if(!data.hops[(int)idx]->probes[data.opts.queries - 1]) printf("\n");

    return idx < 0 ? EXIT_SUCCESS : EXIT_REACHED;
}
