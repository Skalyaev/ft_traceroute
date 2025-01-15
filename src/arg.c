#include "../include/header.h"

extern t_traceroute data;

static void failure(const byte code, char* const opt, char* const arg) {

    if(opt) free(opt);
    if(arg) free(arg);
    if(data.host) free(data.host);
    if(data.opts.interface) free(data.opts.interface);
    exit(code);
}

static void smallopt(int* const x,
                     char** const av,
                     char** const opt,
                     char** const arg) {
    if(!av[*x][1]) {

        *arg = strdup("-");
        if(!*arg) {

            fprintf(stderr, "memory allocation error\n");
            failure(EXIT_FAILURE, *opt, *arg);
        }
        *x += 1;
        return;
    }
    if(!av[*x][2]) {

        *opt = strdup(av[*x]);
        if(!*opt) {

            fprintf(stderr, "memory allocation error\n");
            failure(EXIT_FAILURE, *opt, *arg);
        }
        *x += 1;
        if(av[*x] && strcmp(*opt, "-F") && strcmp(*opt, "-n")) {

            *arg = strdup(av[*x]);
            if(!*arg) {

                fprintf(stderr, "memory allocation error\n");
                failure(EXIT_FAILURE, *opt, *arg);
            }
            *x += 1;
        }
        return;
    }
    *opt = strndup(av[*x], 2);
    if(!*opt) {

        fprintf(stderr, "memory allocation error\n");
        failure(EXIT_FAILURE, *opt, *arg);
    }
    *arg = strdup(av[*x] + 2);
    if(!*arg) {

        fprintf(stderr, "memory allocation error\n");
        failure(EXIT_FAILURE, *opt, *arg);
    }
    *x += 1;
    return;
}

static void largeopt(int* const x,
                     char** const av,
                     char** const opt,
                     char** const arg) {
    if(!av[*x][2]) {

        *x += 1;
        return;
    }
    for(size_t y = 0; y < strlen(av[*x]); y++) {

        if(av[*x][y] != '=') continue;

        *opt = strndup(av[*x], y);
        if(!*opt) {

            fprintf(stderr, "memory allocation error\n");
            failure(EXIT_FAILURE, *opt, *arg);
        }
        *arg = strdup(av[*x] + y + 1);
        if(!*arg) {

            fprintf(stderr, "memory allocation error\n");
            failure(EXIT_FAILURE, *opt, *arg);
        }
        *x += 1;
        return;
    }
    *opt = strdup(av[*x]);
    if(!*opt) {

        fprintf(stderr, "memory allocation error\n");
        failure(EXIT_FAILURE, *opt, *arg);
    }
    *x += 1;
    return;
}

static void parsearg(int* const x,
                     char** const av,
                     char** const opt,
                     char** const arg) {
    if(!av[*x]) {

        *arg = strdup("");
        if(!*arg) {

            fprintf(stderr, "memory allocation error\n");
            failure(EXIT_FAILURE, *opt, *arg);
        }
        *x += 1;
        return;
    }
    if(!strncmp(av[*x], "--", 2)) return largeopt(x, av, opt, arg);
    else if(!strncmp(av[*x], "-", 1)) return smallopt(x, av, opt, arg);

    *arg = strdup(av[*x]);
    if(!*arg) {

        fprintf(stderr, "memory allocation error\n");
        failure(EXIT_FAILURE, *opt, *arg);
    }
    *x += 1;
}

static bool is_numeric(const char* const str, const bool isfloat) {

    ssize_t idx = 0;
    while(str[idx] == ' '
            || str[idx] == '\t'
            || str[idx] == '\n'
            || str[idx] == '\r'
            || str[idx] == '\v'
            || str[idx] == '\f') idx++;

    if(str[idx] == '-' || str[idx] == '+') idx++;
    if(!str[idx]) return NO;
    if(str[idx] == '.') return NO;

    bool dot = NO;
    while(str[idx]) {

        if(str[idx] < '0' || str[idx] > '9') {

            if(!isfloat) return NO;
            if(dot) return NO;

            if(str[idx] == '.') dot = YES;
            else return NO;
        }
        idx++;
    }
    return YES;
}

static void missing(char* const opt,
                    const char* const smallopt,
                    char* const src1,
                    char* const src2,
                    const int ac) {
    char* req;
    if(!strcmp(opt, smallopt)) req = src1;
    else req = src2;

    fprintf(stderr, ERR_MISS_OPT, opt, ac, req);
    failure(EXIT_USAGE, opt, NULL);
}

void getargs(const int ac, char** const av) {

    for(int x = 1; x < ac; x++) {

        if(strcmp(av[x], "--help")) continue;
        printf(USAGE, av[0]);
        exit(EXIT_SUCCESS);
    }
    data.opts.fragment = YES;
    data.opts.first = 1;
    data.opts.max_hops = 30;
    data.opts.sim_queries = 1;
    data.opts.resolve = YES;
    data.opts.port = PORT_MIN;
    data.opts.wait = 0.1;
    data.opts.queries = 3;

    int x = 1;
    char* opt = NULL;
    char* arg = NULL;
    while(x < ac) {

        if(opt) free(opt);
        if(arg) free(arg);
        opt = NULL;
        arg = NULL;

        parsearg(&x, av, &opt, &arg);
        if(!opt) {
            if(!arg) continue;

            if(!data.host) data.host = strdup(arg);
            else if(!data.opts.packetlen) {

                if(!arg[0] || !is_numeric(arg, NO)) {

                    fprintf(stderr, ERR_CMD_ARG, "packetlen", arg, x - 1);
                    failure(EXIT_USAGE, opt, arg);
                }
                data.opts.packetlen = atoi(arg);
            }
            else {
                fprintf(stderr, ERR_XTRA_ARG, arg, x - 1);
                failure(EXIT_USAGE, opt, arg);
            }
            continue;
        }
        if(!strcmp(opt, "-F") || !strcmp(opt, "--dont-fragment")) {

            data.opts.fragment = NO;
            continue;
        }
        if(!strcmp(opt, "-f") || !strcmp(opt, "--first")) {
            if(!arg) {

                char* const src1 = "-f first_ttl";
                char* const src2 = "--first=first_ttl";
                missing(opt, "-f", src1, src2, x - 1);
            }
            if(!arg[0] || !is_numeric(arg, NO)) {

                fprintf(stderr, ERR_OPT_ARG, opt, arg, x - 1);
                failure(EXIT_USAGE, opt, arg);
            }
            data.opts.first = atoi(arg);
            if(!data.opts.first) {

                fprintf(stderr, "first hop out of range\n");
                failure(EXIT_USAGE, opt, arg);
            }
            continue;
        }
        if(!strcmp(opt, "-i") || !strcmp(opt, "--interface")) {
            if(!arg) {

                char* const src1 = "-i device";
                char* const src2 = "--interface=device";
                missing(opt, "-i", src1, src2, x - 1);
            }
            if(data.opts.interface) free(data.opts.interface);
            data.opts.interface = strdup(arg);
            continue;
        }
        if(!strcmp(opt, "-m") || !strcmp(opt, "--max-hops")) {
            if(!arg) {

                char* const src1 = "-m max_ttl";
                char* const src2 = "--max-hops=max_ttl";
                missing(opt, "-m", src1, src2, x - 1);
            }
            if(!arg[0] || !is_numeric(arg, NO)) {

                fprintf(stderr, ERR_OPT_ARG, opt, arg, x - 1);
                failure(EXIT_USAGE, opt, arg);
            }
            data.opts.max_hops = atoi(arg);
            if(!data.opts.max_hops) {

                fprintf(stderr, "first hop out of range\n");
                failure(EXIT_USAGE, opt, arg);
            }
            if(data.opts.max_hops > 255) {

                fprintf(stderr, "max hops cannot be more than 255\n");
                failure(EXIT_USAGE, opt, arg);
            }
            continue;
        }
        if(!strcmp(opt, "-n")) {

            data.opts.resolve = NO;
            continue;
        }
        if(!strcmp(opt, "-p") || !strcmp(opt, "--port")) {
            if(!arg) {

                char* const src1 = "-p port";
                char* const src2 = "--port=port";
                missing(opt, "-p", src1, src2, x - 1);
            }
            if(!arg[0] || !is_numeric(arg, NO)) {

                fprintf(stderr, ERR_OPT_ARG, opt, arg, x - 1);
                failure(EXIT_USAGE, opt, arg);
            }
            data.opts.port = atoi(arg);
            if(data.opts.port < PORT_MIN) data.opts.port = PORT_MIN;
            continue;
        }
        if(!strcmp(opt, "-q") || !strcmp(opt, "--queries")) {
            if(!arg) {

                char* const src1 = "-q nqueries";
                char* const src2 = "--queries=nqueries";
                missing(opt, "-q", src1, src2, x - 1);
            }
            if(!arg[0] || !is_numeric(arg, NO)) {

                fprintf(stderr, ERR_OPT_ARG, opt, arg, x - 1);
                failure(EXIT_USAGE, opt, arg);
            }
            data.opts.queries = atoi(arg);
            if(!data.opts.queries || data.opts.queries > 10) {

                fprintf(stderr, "no more than 10 probes per hop\n");
                failure(EXIT_USAGE, opt, arg);
            }
            continue;
        }
        fprintf(stderr, ERR_BAD_OPT, opt, x - 1);
        failure(EXIT_USAGE, opt, arg);
    }
    if(opt) free(opt);
    if(arg) free(arg);
    if(data.opts.first > data.opts.max_hops) {

        fprintf(stderr, "first hop out of range\n");
        failure(EXIT_USAGE, NULL, NULL);
    }
    if(!data.opts.packetlen) data.opts.packetlen = IP_SIZE + 40;
    else if(data.opts.packetlen < IP_SIZE + 8) data.opts.packetlen = IP_SIZE + 8;
    else if(data.opts.packetlen > BUFFER_SIZE - IP_SIZE - 8) {

        fprintf(stderr, "too big packet length specified\n");
        failure(EXIT_USAGE, NULL, NULL);
    }
    if(data.host) return;

    fprintf(stderr, "Specify \"host\" missing argument.\n");
    failure(EXIT_USAGE, NULL, NULL);
}
