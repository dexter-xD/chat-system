#include "../include/message.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>

void init_message_header(message_header_t *header, uint8_t type, uint32_t length) {
    if (header) {
        header->type = type;
        header->length = length;
    }
}

auth_request_t *create_auth_request(const char *username, const char *password) {
    auth_request_t *req = (auth_request_t *)malloc(sizeof(auth_request_t));
    if (!req) {
        return NULL;
    }
    
    init_message_header(&req->header, MSG_AUTH_REQUEST, sizeof(auth_request_t));
    safe_strcpy(req->username, username, MAX_USERNAME_LEN);
    safe_strcpy(req->password, password, MAX_PASSWORD_LEN);
    
    return req;
}

auth_response_t *create_auth_response(uint8_t status) {
    auth_response_t *resp = (auth_response_t *)malloc(sizeof(auth_response_t));
    if (!resp) {
        return NULL;
    }
    
    init_message_header(&resp->header, MSG_AUTH_RESPONSE, sizeof(auth_response_t));
    resp->status = status;
    
    return resp;
}

register_request_t *create_register_request(const char *username, const char *password) {
    register_request_t *req = (register_request_t *)malloc(sizeof(register_request_t));
    if (!req) {
        return NULL;
    }
    
    init_message_header(&req->header, MSG_REGISTER_REQUEST, sizeof(register_request_t));
    safe_strcpy(req->username, username, MAX_USERNAME_LEN);
    safe_strcpy(req->password, password, MAX_PASSWORD_LEN);
    
    return req;
}

register_response_t *create_register_response(uint8_t status) {
    register_response_t *resp = (register_response_t *)malloc(sizeof(register_response_t));
    if (!resp) {
        return NULL;
    }
    
    init_message_header(&resp->header, MSG_REGISTER_RESPONSE, sizeof(register_response_t));
    resp->status = status;
    
    return resp;
}

create_room_request_t *create_room_request(const char *room_name) {
    create_room_request_t *req = (create_room_request_t *)malloc(sizeof(create_room_request_t));
    if (!req) {
        return NULL;
    }
    
    init_message_header(&req->header, MSG_CREATE_ROOM, sizeof(create_room_request_t));
    safe_strcpy(req->room_name, room_name, MAX_ROOM_NAME_LEN);
    
    return req;
}

create_room_response_t *create_room_response(uint8_t status, const char *room_id) {
    create_room_response_t *resp = (create_room_response_t *)malloc(sizeof(create_room_response_t));
    if (!resp) {
        return NULL;
    }
    
    init_message_header(&resp->header, MSG_CREATE_ROOM_RESPONSE, sizeof(create_room_response_t));
    resp->status = status;
    safe_strcpy(resp->room_id, room_id, MAX_ROOM_ID_LEN);
    
    return resp;
}

join_room_request_t *create_join_room_request(const char *room_id) {
    join_room_request_t *req = (join_room_request_t *)malloc(sizeof(join_room_request_t));
    if (!req) {
        return NULL;
    }
    
    init_message_header(&req->header, MSG_JOIN_ROOM, sizeof(join_room_request_t));
    safe_strcpy(req->room_id, room_id, MAX_ROOM_ID_LEN);
    
    return req;
}

join_room_response_t *create_join_room_response(uint8_t status, const char *room_name, const char *room_id) {
    join_room_response_t *resp = (join_room_response_t *)malloc(sizeof(join_room_response_t));
    if (!resp) {
        return NULL;
    }
    
    init_message_header(&resp->header, MSG_JOIN_ROOM_RESPONSE, sizeof(join_room_response_t));
    resp->status = status;
    safe_strcpy(resp->room_name, room_name, MAX_ROOM_NAME_LEN);
    safe_strcpy(resp->room_id, room_id, MAX_ROOM_ID_LEN);
    
    return resp;
}

leave_room_request_t *create_leave_room_request(const char *room_id) {
    leave_room_request_t *req = (leave_room_request_t *)malloc(sizeof(leave_room_request_t));
    if (!req) {
        return NULL;
    }
    
    init_message_header(&req->header, MSG_LEAVE_ROOM, sizeof(leave_room_request_t));
    safe_strcpy(req->room_id, room_id, MAX_ROOM_ID_LEN);
    
    return req;
}

chat_message_t *create_chat_message(const char *room_id, const char *username, const char *message) {
    chat_message_t *msg = (chat_message_t *)malloc(sizeof(chat_message_t));
    if (!msg) {
        return NULL;
    }
    
    init_message_header(&msg->header, MSG_CHAT_MESSAGE, sizeof(chat_message_t));
    safe_strcpy(msg->room_id, room_id, MAX_ROOM_ID_LEN);
    safe_strcpy(msg->username, username, MAX_USERNAME_LEN);
    safe_strcpy(msg->message, message, MAX_MESSAGE_LEN);
    
    return msg;
}

error_message_t *create_error_message(uint8_t error_code, const char *error_message) {
    error_message_t *err = (error_message_t *)malloc(sizeof(error_message_t));
    if (!err) {
        return NULL;
    }
    
    init_message_header(&err->header, MSG_ERROR, sizeof(error_message_t));
    err->error_code = error_code;
    safe_strcpy(err->error_message, error_message, MAX_MESSAGE_LEN);
    
    return err;
}

void free_message(void *message) {
    if (message) {
        free(message);
    }
} 