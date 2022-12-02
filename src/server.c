/*
 ============================================================================
 Name        : Server.c
 Author      : Swajeet Swarnkar
 Version     :
 Copyright   : Copyright 2020 Swajeet Swarnkar
 Description : Server written in C
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

void sg_err(char* msg) {
    fprintf(stderr, "Err: %s, %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

int open_socket(void) {
    int sock_d = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_d == -1)
           sg_err("Unable to create socket");
    return sock_d;
}

void bind_port(int sock_d, int port) {
    struct sockaddr_in address;
    address.sin_family = PF_INET;
    address.sin_port = (in_port_t) htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    int reuse = 1;
    if(setsockopt(sock_d, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
        sg_err("Unable to set reusable port");
    if(bind(sock_d, (struct sockaddr*) &address, sizeof(address)) == -1)
        sg_err("Unable to bind to port");
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

int sock_d;
const char *head = "GET: RESPONSE-01";

void exit_server(int sig) {
    if(sock_d)
        close(sock_d);
    fprintf(stdout, "%s\n", "Shutting down server");
    exit(EXIT_SUCCESS);
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
    reg_handler(SIGINT, exit_server);
    sock_d = open_socket();
    bind_port(sock_d, 12345);
    if(listen(sock_d, 5) == -1)
        sg_err("Unable to listen for connections");
    struct sockaddr_storage client;
    unsigned int addr_size = sizeof(client);
    puts("Waiting for connection");
    char buf[256];
    while(1) {
        int client_d = accept(sock_d, (struct sockaddr*) &client, (socklen_t*) &addr_size);
        if(client_d == -1)
            sg_err("Unable to open client socket");
        read_msg(client_d, buf, 256);
        puts(buf);
        if(!strncasecmp(head, buf, strlen(head))) {
            send_msg(client_d, "RESPONSE 01\n");
        }
        close(client_d);
    }
	return EXIT_SUCCESS;
}
