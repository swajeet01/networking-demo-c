/*
 ============================================================================
 Name        : Client.c
 Author      : Swajeet Swarnkar
 Version     :
 Copyright   : Copyright 2020 Swajeet Swarnkar
 Description : Client written in C
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>

void sg_err(char* msg) {
    fprintf(stderr, "Err: %s, %s\n", msg, strerror(errno));
    exit(1);
}

int open_socket(char* hostname, char* port) {
    struct addrinfo address;
    struct addrinfo* address_ptr;
    memset(&address, 0, sizeof(address));
    address.ai_family = PF_UNSPEC;
    address.ai_socktype = SOCK_STREAM;
    if(getaddrinfo(hostname, port, &address, &address_ptr) == -1)
        sg_err("Unable to get address info");
    int sock_d = socket(address_ptr->ai_family, address_ptr->ai_socktype,
        address_ptr->ai_protocol);
    if(sock_d == -1)
        sg_err("Unable to create socket");
    int cn_status = connect(sock_d, address_ptr->ai_addr, address_ptr->ai_addrlen);
    freeaddrinfo(address_ptr);
    if (cn_status == -1)
    sg_err("Unable to connect to address");
    return sock_d;
}

int send_msg(int sock_d, char* msg) {
    int send_res = send(sock_d, msg, strlen(msg), 0);
    if(send_res == -1)
        fprintf(stderr, "Err: %s, %s\n", "Unable to send bytes", strerror(errno));
    return send_res;
}

int read_msg(int sock_d, char* buf, int len) {
    char *pstn = buf;
    int rem = len;
    int read = recv(sock_d, buf, len, 0);
    while((read > 0) && (pstn[read-1] != '\n')) {
        pstn += read;
        rem -= read;
        read = recv(sock_d, pstn, rem, 0);
    }
    if(read < 0)
        return read;
    else if(read == 0)
        buf[0] = '\0';
    else
        pstn[read-1] = '\0';
    return len - rem;
}

void reg_handler(int sig, void (*handler)(int)) {
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(sig, &action, NULL) == -1)
        sg_err("Unable to set sig handler");
}

int main(void) {
    int sock_d = open_socket("127.0.0.1", "12345");
    send_msg(sock_d, "GET: RESPONSE-01\n");
    char buf[256];
    read_msg(sock_d, buf, 256);
    puts(buf);
	return EXIT_SUCCESS;
}
