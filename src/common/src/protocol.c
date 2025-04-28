#include "../include/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

int send_message(int sockfd, const void *message, size_t length) {
    
    const message_header_t *header = (const message_header_t *)message;
    uint8_t type = header->type;
    uint32_t net_length = htonl(header->length);
    message_header_t net_header;
    net_header.type = type;
    net_header.length = net_length;
    ssize_t sent = send(sockfd, &net_header, sizeof(message_header_t), 0);
    if (sent != sizeof(message_header_t)) {
        return -1;
    }
    if (length > sizeof(message_header_t)) {
        sent = send(sockfd, (char*)message + sizeof(message_header_t), 
                   length - sizeof(message_header_t), 0);
        if (sent != (ssize_t)(length - sizeof(message_header_t))) {
            return -1;
        }
    }
    
    return 0;
}

int receive_message(int sockfd, void *buffer, size_t buffer_size) {
    message_header_t header;
    ssize_t received = recv(sockfd, &header, sizeof(message_header_t), 0);
    
    if (received <= 0) {
        return received; 
    }    
    if (received != sizeof(message_header_t)) {
        return -1; 
    }
    
    header.length = ntohl(header.length);
    if (header.length > buffer_size) {
        return -1;
    }
    
    memcpy(buffer, &header, sizeof(message_header_t));
    size_t body_size = header.length - sizeof(message_header_t);
    if (body_size > 0) {
        received = recv(sockfd, (char*)buffer + sizeof(message_header_t), body_size, 0);
        if (received != (ssize_t)body_size) {
            return -1;
        }
    }
    
    return header.length;
} 