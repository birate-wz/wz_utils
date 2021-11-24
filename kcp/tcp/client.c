#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "delay.h"

#define SERVER_IP "114.215.169.66"
#define SERVER_PORT 10000
#define BUFSIZE 1024
#define DELAY_TEST1_N 100
#define DELAY_TEST2_N 5

void delay_test1(int fd) {
    // 初始化 100个 delay obj
    
    size_t obj_size = sizeof(t_delay_obj);
    t_delay_obj *objs = malloc(DELAY_TEST1_N * sizeof(t_delay_obj));


    for(int i = 0; i < DELAY_TEST1_N; i++) {
        delay_set_seqno_send_time(&objs[i], i);  
        if(send(fd, &objs[i], obj_size, 0) != obj_size) {
            printf("send %d seqno:%d failed\n", i, objs[i].seqno);
            return;
        } 
        else {
            //  printf("send %d seqno:%d ok\n", i, objs[i].seqno);
        }
    }

    for(int i = 0; i < DELAY_TEST1_N; i++) {
        int size = recv(fd, &objs[i], obj_size, 0);
        if(size != obj_size) {
            printf("recv %d seqno:%d failed, size:%d\n", i, objs[i].seqno, size);
            delay_print_rtt_time(objs, i);
            return;
        }
        delay_set_recv_time(&objs[i]);
    }
    delay_print_rtt_time(objs, DELAY_TEST1_N);
}

void delay_test2(int fd) {
    // 初始化 100个 delay obj
    
    size_t obj_size = sizeof(t_delay_obj);
    t_delay_obj *objs = malloc(DELAY_TEST2_N * sizeof(t_delay_obj));


    for(int i = 0; i < DELAY_TEST1_N; i++) {
        delay_set_seqno_send_time(&objs[i], i);  
        if(send(fd, &objs[i], obj_size, 0) != obj_size) {
            printf("send %d seqno:%d failed\n", i, objs[i].seqno);
            return;
        } 
        else {
            //  printf("send %d seqno:%d ok\n", i, objs[i].seqno);
        }
        int size = recv(fd, &objs[i], obj_size, 0);
        if(size != obj_size) {
            printf("recv %d seqno:%d failed, size:%d\n", i, objs[i].seqno, size);
            delay_print_rtt_time(objs, i);
            return;
        }
        delay_set_recv_time(&objs[i]);
        usleep(10000);
    }

    delay_print_rtt_time(objs, DELAY_TEST1_N);
}



int get_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        printf("Error, fd is -1\n");
    }
    return fd;
}

/**
 * 连接到server端，如果成功，返回fd，如果失败返回-1
 */
int connect_server()
{
    int fd = get_socket();
    printf("fd=%d\n", fd);
    if (-1 == fd)
    {
        printf("Error, connect_server() quit\n");
        return -1;
    }
    struct sockaddr_in remote_addr;                     //服务器端网络地址结构体
    memset(&remote_addr, 0, sizeof(remote_addr));       //数据初始化--清零
    remote_addr.sin_family = AF_INET;                   //设置为IP通信
    remote_addr.sin_addr.s_addr = inet_addr(SERVER_IP); //服务器IP地址
    remote_addr.sin_port = htons(SERVER_PORT);          //服务器端口号
    int con_result = connect(fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
    if (con_result < 0)
    {
        printf("Connect Error!\n");
        return -1;
    }
    printf("con_result=%d\n", con_result);
    return fd;
}


/**
*** 连接到server端，并且不断往server端发送心跳数据
*/
void *test_thread(void *arg)
{
    printf("test_thread started!\n");
    int fd = connect_server();
    if (fd < 0)
    {
        printf("Heart bead quit!\n");
        return 0;
    }
    // delay_test1(fd);
    delay_test2(fd);
    printf("test_thread finished!\n");
    return 0;
}

/**
*** 创建一个新线程，在新线程里调用heartBeat()函数
*/
void thread_call()
{
    pthread_t thread;
    pthread_create(&thread, NULL, test_thread, NULL);
    pthread_join(thread, NULL);
}
int main()
{
    printf("main started\n"); // prints Hello World!!!
    thread_call();
    printf("main finished\n");
    return 0;
}