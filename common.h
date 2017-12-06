#ifndef __COMMON_H__
#define __COMMON_H__

#include <linux/input.h>

#define fatal(msg...) { \
        fprintf(stderr, msg); \
        fprintf(stderr, " [%s(), %s:%u]\n", __FUNCTION__, __FILE__, __LINE__); \
        exit(1); \
        }

void parse_host_port(const char *s, char **host, int *port);
int recv_event(int fd, struct input_event *ev);
int send_event(int fd, struct input_event *ev);
void print_event(struct input_event *ev);
int connect_socket(char *hostname, int port);
int open_input_fd(const char *devname);

#endif
