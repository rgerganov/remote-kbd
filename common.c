#include <stdio.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "common.h"

void parse_host_port(const char *s, char **host, int *port)
{
    char *str = strdup(s);
    char *ind = strchr(str, ':');
    if (ind == NULL) {
        fatal("Cannot parse host and port from '%s'", str);
    }
    *ind = 0;
    *host = str;
    if (**host == 0) {
        *host = strdup("0.0.0.0");
    }
    *port = atoi(ind+1);
}

int recv_event(int fd, struct input_event *ev)
{
    uint32_t tcv[3];
    int r = read(fd, &tcv, 12);
    if (r != 12) {
        return -1;
    }
    ev->type = ntohl(tcv[0]);
    ev->code = ntohl(tcv[1]);
    ev->value = ntohl(tcv[2]);
    return 0;
}

int send_event(int fd, struct input_event *ev)
{
    uint32_t tcv[3] = {htonl(ev->type), htonl(ev->code), htonl(ev->value)};
    int w = write(fd, &tcv, 12);
    if (w != 12) {
        return -1;
    }
    return 0;
}

void print_event(struct input_event *ev)
{
    printf("type: %d code: %d value: %d\n", ev->type, ev->code, ev->value);
}

int connect_socket(char *hostname, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fatal("cannot create client socket");
    }
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fatal("Cannot resolve hostname '%s'", hostname);
    }
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memcpy(&serveraddr.sin_addr.s_addr, server->h_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        fatal("error connecting to server");
    }
    return sock;
}

int open_input_fd(const char *devname)
{
    int fd = open(devname, O_RDONLY);
    if (fd < 0) {
        fatal("Cannot open %s", devname);
    }
    struct input_id id;
    int ret = ioctl(fd, EVIOCGID, &id);
    if (ret < 0) {
        fatal("EVIOCGID");
    }
    printf("Input device info:\n");
    printf("\tbustype=0x%x, vendor=0x%x, product=0x%x, version=0x%x\n", id.bustype, id.vendor, id.product, id.version);

    char name[256];
    ret = ioctl(fd, EVIOCGNAME(256), name);
    if (ret < 0) {
        fatal("EVIOCGNAME");
    }
    printf("\tname=%s\n", name);
    return fd;
}

