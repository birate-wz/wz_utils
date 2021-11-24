#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "delay.h"
#define ECHO_SERVER_PORT 10000
#define LISTEN_BACKLOG 16
#define MAX_EVENT_COUNT 32
#define BUF_SIZE 2048

int main()
{
    int ret, i;
    int server_fd, client_fd, epoll_fd;
    int ready_count;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    struct epoll_event event;
    struct epoll_event *event_array;
    char *buf;

    event_array = (struct epoll_event *)
        malloc(sizeof(struct epoll_event) * MAX_EVENT_COUNT);
    int buf_size =  sizeof(t_delay_obj);;
    buf = (char *)malloc(sizeof(char) * buf_size);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(ECHO_SERVER_PORT);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("create socket failed.\n");
        return 1;
    }
    ret = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
    {
        perror("bind failed.\n");
        return 1;
    }
    ret = listen(server_fd, LISTEN_BACKLOG);
    if (ret == -1)
    {
        perror("listen failed.\n");
        return 1;
    }

    epoll_fd = epoll_create(1);
    if (epoll_fd == -1)
    {
        perror("epoll_create failed.\n");
        return 1;
    }
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl failed.\n");
        return 1;
    }

    while (1)
    {
        ready_count = epoll_wait(epoll_fd, event_array, MAX_EVENT_COUNT, -1);
        if (ready_count == -1)
        {
            perror("epoll_wait failed.\n");
            return 1;
        }
        for (i = 0; i < ready_count; i++)
        {
            if (event_array[i].data.fd == server_fd)
            {
                client_fd = accept(server_fd,
                                   (struct sockaddr *)&client_addr, &addr_len);
                printf("accept fd:%d\n", client_fd);
                if (client_fd == -1)
                {
                    perror("accept failed.\n");
                    return 1;
                }
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                if (ret == -1)
                {
                    perror("epoll_ctl failed.\n");
                    return 1;
                }
            }
            else
            {
                ret = recv(event_array[i].data.fd, buf, buf_size, 0);
                if (ret <= 0)
                {
                    close(event_array[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL,
                              event_array[i].data.fd, &event);
                    continue;
                }
                ret = send(event_array[i].data.fd, buf, (size_t)ret, 0);
                if (ret == -1)
                {
                    perror("send failed.\n");
                }
            }
        } // for each event
    }     // while(1)

    close(epoll_fd);
    close(server_fd);
    free(event_array);
    free(buf);

    return 0;
}