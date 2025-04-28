#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define MSG_AUTH_REQUEST     1
#define MSG_AUTH_RESPONSE    2
#define MSG_REGISTER_REQUEST 3
#define MSG_REGISTER_RESPONSE 4
#define MSG_CREATE_ROOM      5
#define MSG_CREATE_ROOM_RESPONSE 6
#define MSG_JOIN_ROOM        7
#define MSG_JOIN_ROOM_RESPONSE 8
#define MSG_LEAVE_ROOM       9
#define MSG_CHAT_MESSAGE     10
#define MSG_ERROR            255

#define RESP_SUCCESS         0
#define RESP_AUTH_FAILED     1
#define RESP_USER_EXISTS     2
#define RESP_ROOM_NOT_FOUND  3
#define RESP_INTERNAL_ERROR  255

#define MAX_USERNAME_LEN     32
#define MAX_PASSWORD_LEN     64
#define MAX_ROOM_NAME_LEN    64
#define MAX_MESSAGE_LEN      1024
#define MAX_ROOM_ID_LEN      37  

#pragma pack(1)

typedef struct {
    uint8_t type;        
    uint32_t length;     
} message_header_t;

typedef struct {
    message_header_t header;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} auth_request_t;

typedef struct {
    message_header_t header;
    uint8_t status;     
} auth_response_t;

typedef struct {
    message_header_t header;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} register_request_t;

typedef struct {
    message_header_t header;
    uint8_t status;    
} register_response_t;

typedef struct {
    message_header_t header;
    char room_name[MAX_ROOM_NAME_LEN];
} create_room_request_t;

typedef struct {
    message_header_t header;
    uint8_t status;    
    char room_id[MAX_ROOM_ID_LEN];
} create_room_response_t;

typedef struct {
    message_header_t header;
    char room_id[MAX_ROOM_ID_LEN];
} join_room_request_t;

typedef struct {
    message_header_t header;
    uint8_t status;    
    char room_name[MAX_ROOM_NAME_LEN];
    char room_id[MAX_ROOM_ID_LEN];
} join_room_response_t;

typedef struct {
    message_header_t header;
    char room_id[MAX_ROOM_ID_LEN];
} leave_room_request_t;

typedef struct {
    message_header_t header;
    char room_id[MAX_ROOM_ID_LEN];
    char username[MAX_USERNAME_LEN];
    char message[MAX_MESSAGE_LEN];
} chat_message_t;

typedef struct {
    message_header_t header;
    uint8_t error_code;
    char error_message[MAX_MESSAGE_LEN];
} error_message_t;

#pragma pack()

int send_message(int sockfd, const void *message, size_t length);
int receive_message(int sockfd, void *buffer, size_t buffer_size);

#endif