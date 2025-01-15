#include "../include/header.h"

extern t_traceroute data;

static byte init_udp() {

    if((data.udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {

        printf("socket SOCK_DGRAM: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_sockaddr_in bind_addr = {0};
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(data.opts.port - 1);
    if(bind(data.udp_socket, (t_sockaddr*)&bind_addr, SOCKADDRIN_SIZE) < 0) {

        printf("bind: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    if(data.opts.interface) {

        if(setsockopt(data.udp_socket, SOL_SOCKET, SO_BINDTODEVICE,
                      data.opts.interface, strlen(data.opts.interface)) < 0) {

            printf("setsockopt SO_BINDTODEVICE: %s\n", strerror(errno));
            data.code = errno;
            return EXIT_FAILURE;
        }
    }
    if(data.opts.fragment) {

        const int df = IP_PMTUDISC_DO;
        if(setsockopt(data.udp_socket, IPPROTO_IP, IP_MTU_DISCOVER,
                      &df, INT_SIZE) < 0) {

            printf("setsockopt IP_MTU_DISCOVER: %s\n", strerror(errno));
            data.code = errno;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static byte init_icmp() {

    if((data.icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {

        printf("socket SOCK_RAW: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_timeval tv = {0};
    tv.tv_sec = (int)data.opts.wait;
    tv.tv_usec = (data.opts.wait - (int)data.opts.wait) * 1000000;

    if(!tv.tv_sec && !tv.tv_usec) tv.tv_usec = 1;
    if(setsockopt(data.icmp_socket, SOL_SOCKET, SO_RCVTIMEO,
                  &tv, TIMEVAL_SIZE) < 0) {

        printf("setsockopt SO_RCVTIMEO: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

byte init() {

    t_addrinfo* res;
    t_addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if(getaddrinfo(data.host, 0, &hints, &res) < 0) {

        printf("getaddrinfo: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    data.addr.sin_family = AF_INET;
    data.addr.sin_addr = ((t_sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);

    const size_t packet_size = data.opts.packetlen < IP_SIZE
                               ? IP_SIZE : data.opts.packetlen;

    printf("traceroute to %s (%s), %u hops max, %lu byte packets\n",
           data.host, inet_ntoa(data.addr.sin_addr),
           data.opts.max_hops, packet_size);

    if(init_udp() != EXIT_SUCCESS) return EXIT_FAILURE;
    if(init_icmp() != EXIT_SUCCESS) return EXIT_FAILURE;

    const size_t size = data.opts.max_hops * PTR_SIZE;
    data.hops = malloc(size);
    if(!data.hops) {

        printf("malloc: %s\n", strerror(errno));
        data.code = errno;
        return EXIT_FAILURE;
    }
    memset(data.hops, 0, size);
    for(ubyte x = data.opts.first - 1; x < data.opts.max_hops; x++) {

        data.hops[x] = malloc(HOP_SIZE);
        if(!data.hops[x]) {

            printf("malloc: %s\n", strerror(errno));
            data.code = errno;
            return EXIT_FAILURE;
        }
        memset(data.hops[x], 0, HOP_SIZE);
    }
    pthread_mutex_init(&data.hops_mutex, 0);
    pthread_mutex_init(&data.reached_mutex, 0);
    pthread_mutex_init(&data.abort_mutex, 0);
    return EXIT_SUCCESS;
}
