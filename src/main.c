#include "../include/header.h"

t_traceroute data = {0};

static int bye() {
    return data.code;
}

int main(int ac, char** const av) {

    signal(SIGINT, sigexit);
    signal(SIGQUIT, sigexit);
    signal(SIGTERM, sigexit);
    //getargs(ac, av);
    return bye();
}
