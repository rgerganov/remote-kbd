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

#define fatal(msg...) { \
        fprintf(stderr, msg); \
        fprintf(stderr, " [%s(), %s:%u]\n", __FUNCTION__, __FILE__, __LINE__); \
        exit(1); \
        }

void usage(char *argv[])
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s recv [IP]:<port>\n", argv[0]);
    fprintf(stderr, "\t%s send <IP>:<port> <devname>\n", argv[0]);
    exit(1);
}

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

int create_server_socket(const char *hostname, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fatal("cannot create server socket");
    }
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fatal("Cannot resolve hostname '%s'", hostname);
    }
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memcpy(&serveraddr.sin_addr.s_addr, server->h_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        fatal("cannot bind server socket");
    }
    if (listen(sock, 5) < 0) {
        fatal("error on listen");
    }
    return sock;
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

int create_input_fd()
{
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        fatal("Cannot open /dev/uinput");
    }
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        fatal("UI_SET_EVBIT");
    }
    for (int i = 0 ; i < 256 ; i++) {
        if (ioctl(fd, UI_SET_KEYBIT, i) < 0) {
            fatal("UI_SET_KEYBIT");
        }
    }
    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "xakcop kbd");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0xdead;
    uidev.id.product = 0xbeef;
    uidev.id.version = 1;
    if (write(fd, &uidev, sizeof(uidev)) < 0) {
        fatal("cannot write to /dev/uinput");
    }
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        fatal("UI_DEV_CREATE");
    }
    return fd;
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

int main(int argc, char *argv[])
{
    char *host = NULL;
    int port = 0;
    int server_sock, client_sock, in_fd;

    if (argc < 3) {
        usage(argv);
    }
    if (strcmp(argv[1], "send") == 0) {
        if (argc < 4) {
            usage(argv); 
        }
        parse_host_port(argv[2], &host, &port);
        in_fd = open_input_fd(argv[3]);
        client_sock = connect_socket(host, port);
        struct input_event ev;
        while (1) {
            int ret = read(in_fd, &ev, sizeof(ev));
            if (ret != sizeof(ev)) {
                fprintf(stderr, "unexpected read count\n");
                continue;
            }
            if (ev.type == EV_KEY || ev.type == EV_SYN) {
                print_event(&ev);
                ret = send_event(client_sock, &ev);
                if (ret < 0) {
                    fatal("error on write");
                }
            }
        }
    } else if (strcmp(argv[1], "recv") == 0) {
        parse_host_port(argv[2], &host, &port);
        in_fd = create_input_fd();
        printf("Binding socket to %s:%d\n", host, port);
        server_sock = create_server_socket(host, port);
        while (1) {
            struct sockaddr_in clientaddr;
            int clientlen = sizeof(clientaddr);
            client_sock = accept(server_sock, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
            if (client_sock < 0) {
                fatal("error on accept");
            }
            printf("Client connected\n");
            while (1) {
                struct input_event ev;
                int ret = recv_event(client_sock, &ev);
                if (ret < 0) {
                    break;
                }
                print_event(&ev);
                write(in_fd, &ev, sizeof(ev));
            }
            close(client_sock);
        }
    } else {
        usage(argv); 
    }
    return 0;
}
