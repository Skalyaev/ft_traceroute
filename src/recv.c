#include "../include/header.h"

extern t_traceroute data;

static byte init() {

    data.icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (data.icmp_socket < 0) {

        strerror(errno);
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_timeval tv = {0};
    tv.tv_sec = (int)data.opts.wait;
    tv.tv_usec = (data.opts.wait - (int)data.opts.wait) * 1000000;

    if(!tv.tv_sec && !tv.tv_usec) tv.tv_usec = 1;
    if (setsockopt(data.icmp_socket, SOL_SOCKET,
                   SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {

        strerror(errno);
        data.code = errno;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

byte recv_icmp(const ubyte* const sent) {

    if (!data.icmp_socket && init() != EXIT_SUCCESS) return EXIT_FAILURE;

    t_sockaddr_in sockaddr;
    socklen_t sockaddrlen = sizeof(sockaddr);

    static const size_t icmphdr_len = sizeof(t_icmphdr);
    static const size_t udphdr_len = sizeof(t_udphdr);
    static const size_t recvbuffsize = 1024;

    char hostbuf[NI_MAXHOST] = {0};
    char recvbuff[recvbuffsize];
    memset(recvbuff, 0, recvbuffsize);

    t_ip* iphdr;
    t_ip* orig_iphdr;
    int iphdr_len, orig_iphdr_len;
    char* udp_ptr;

    t_probe* probe;
    t_seq* seqhdr;
    ubyte seq, idx, received = 0;
    while(received < *sent) {

        if (recvfrom(data.icmp_socket, recvbuff, recvbuffsize, 0,
                     (t_sockaddr*)&sockaddr, &sockaddrlen) < 0) break;

        t_timeval end;
        gettimeofday(&end, NULL);

        iphdr = (t_ip*)recvbuff;
        iphdr_len = iphdr->ip_hl * 4;

        orig_iphdr = (t_ip*)(recvbuff + iphdr_len + icmphdr_len);
        orig_iphdr_len = orig_iphdr->ip_hl * 4;
        udp_ptr = (char*)orig_iphdr + orig_iphdr_len;

        seqhdr = (t_seq*)(udp_ptr + udphdr_len);
        seq = ntohs(seqhdr->seq);
        idx = seq % data.opts.queries;
        printf("recv seq %u\n", seq);

        probe = NULL;
        for (ubyte x = 0; x < data.opts.queries; x++) {

            if (data.hops[idx]->probes[x]->seq != seq) continue;
            probe = data.hops[idx]->probes[x];
            break;
        }
        if (!probe || !probe->lost) continue;
        probe->lost = NO;
        probe->rtt = (end.tv_sec - probe->start.tv_sec) * 1000.0
                     + (end.tv_usec - probe->start.tv_usec) / 1000.0;

        strcpy(probe->ip, inet_ntoa(iphdr->ip_src));
        if (data.opts.resolve) {

            memset(hostbuf, 0, NI_MAXHOST);
            if (getnameinfo((t_sockaddr*)&iphdr->ip_src, sockaddrlen,
                            hostbuf, NI_MAXHOST, NULL, 0, 0) == 0) {

                strcpy(probe->host, hostbuf);
            } else strncpy(probe->host, probe->ip, NI_MAXHOST);
        } else strncpy(probe->host, probe->ip, NI_MAXHOST);
        received++;
    }
    t_hop* hop;
    for(ubyte x = data.opts.first - 1; x < data.opts.max_hops; x++) {

        hop = data.hops[x];
        if(!hop || !hop->probes) continue;

        if(!hop->probes[0]->printed) printf(" %d  ", x + 1);
        for(ubyte y = 0; y < data.opts.queries; y++) {

            probe = hop->probes[y];
            if(!probe || probe->printed) continue;
            probe->printed = YES;

            if(probe->lost) printf("* ");
            else printf("%s (%s)  %.3f ms  ", probe->host, probe->ip, probe->rtt);
            if(y == data.opts.queries - 1) printf("\n");
        }
        fflush(stdout);
    }
    return EXIT_SUCCESS;
}
