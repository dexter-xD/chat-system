#include "server.h"
#include "../common/include/protocol.h"
#include "../common/include/message.h"
#include "../common/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern server_t *g_server;
int server_broadcast_message(server_t *server, const char *room_id, const char *username, const char *message) {
    if (!server || !room_id || !username || !message) {
        return -1;
    }
    
    chat_message_t *chat_msg = create_chat_message(room_id, username, message);
    if (!chat_msg) {
        return -1;
    }
    
    pthread_mutex_lock(&server->clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].connected && 
            server->clients[i].authenticated && 
            strcmp(server->clients[i].current_room_id, room_id) == 0) {
            
            send_message(server->clients[i].sockfd, chat_msg, sizeof(chat_message_t));
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);
    free_message(chat_msg);   
    return 0;
}

void *handle_client(void *arg) {
    int client_index = (int)(intptr_t)arg;
    server_t *server = g_server;
    
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS) {
        return NULL;
    }
    
    int sockfd = server->clients[client_index].sockfd;
    char buffer[2048]; 
    log_message("Handling client %d", client_index);
    
    while (server->running && server->clients[client_index].connected) {
        int recv_size = receive_message(sockfd, buffer, sizeof(buffer));
        if (recv_size <= 0) {
            break;
        }
        

        message_header_t *header = (message_header_t *)buffer;  
        switch (header->type) {
            case MSG_AUTH_REQUEST: {
                auth_request_t *req = (auth_request_t *)buffer;
                bool success = server_authenticate(server, client_index, req->username, req->password);
                auth_response_t *resp = create_auth_response(success ? RESP_SUCCESS : RESP_AUTH_FAILED);
                send_message(sockfd, resp, sizeof(auth_response_t));
                free_message(resp);
                break;
            }
            
            case MSG_REGISTER_REQUEST: {
                register_request_t *req = (register_request_t *)buffer;
                int result = server_register_user(server, client_index, req->username, req->password);
                uint8_t status = (result > 0) ? RESP_SUCCESS : 
                                (result == -2) ? RESP_USER_EXISTS : RESP_INTERNAL_ERROR;
                register_response_t *resp = create_register_response(status);
                send_message(sockfd, resp, sizeof(register_response_t));
                free_message(resp);
                break;
            }
            
            case MSG_CREATE_ROOM: {
                if (!server->clients[client_index].authenticated) {
                    error_message_t *err = create_error_message(RESP_AUTH_FAILED, 
                                                              "You must be logged in to create a room");
                    send_message(sockfd, err, sizeof(error_message_t));
                    free_message(err);
                    break;
                }
                
                create_room_request_t *req = (create_room_request_t *)buffer;
                char room_id[MAX_ROOM_ID_LEN];
                int result = server_create_room(server, client_index, req->room_name, room_id);
                create_room_response_t *resp = create_room_response(
                    result == 0 ? RESP_SUCCESS : RESP_INTERNAL_ERROR, 
                    result == 0 ? room_id : "");
                send_message(sockfd, resp, sizeof(create_room_response_t));
                free_message(resp);
                break;
            }
            
            case MSG_JOIN_ROOM: {
                if (!server->clients[client_index].authenticated) {
                    error_message_t *err = create_error_message(RESP_AUTH_FAILED, 
                                                              "You must be logged in to join a room");
                    send_message(sockfd, err, sizeof(error_message_t));
                    free_message(err);
                    break;
                }
                
                join_room_request_t *req = (join_room_request_t *)buffer;
                if (server->clients[client_index].current_room_id[0] != '\0') {
                    server_leave_room(server, client_index);
                }
                
                int result = server_join_room(server, client_index, req->room_id);
                char room_name[MAX_ROOM_NAME_LEN] = "";
                if (result == 0) {
                    db_get_room_name(&server->db, req->room_id, room_name, sizeof(room_name));
                }
                join_room_response_t *resp = create_join_room_response(
                    result == 0 ? RESP_SUCCESS : RESP_ROOM_NOT_FOUND, 
                    room_name,
                    req->room_id);
                send_message(sockfd, resp, sizeof(join_room_response_t));
                free_message(resp);
                break;
            }
            
            case MSG_LEAVE_ROOM: {
                if (!server->clients[client_index].authenticated) {
                    error_message_t *err = create_error_message(RESP_AUTH_FAILED, 
                                                              "You must be logged in to leave a room");
                    send_message(sockfd, err, sizeof(error_message_t));
                    free_message(err);
                    break;
                }
                
                int result = server_leave_room(server, client_index);
                break;
            }
            
            case MSG_CHAT_MESSAGE: {
                if (!server->clients[client_index].authenticated) {
                    error_message_t *err = create_error_message(RESP_AUTH_FAILED, 
                                                              "You must be logged in to send messages");
                    send_message(sockfd, err, sizeof(error_message_t));
                    free_message(err);
                    break;
                }
                
                chat_message_t *msg = (chat_message_t *)buffer;
                if (strcmp(server->clients[client_index].current_room_id, msg->room_id) != 0) {
                    error_message_t *err = create_error_message(RESP_ROOM_NOT_FOUND, 
                                                              "You are not in this room");
                    send_message(sockfd, err, sizeof(error_message_t));
                    free_message(err);
                    break;
                }
                safe_strcpy(msg->username, server->clients[client_index].username, MAX_USERNAME_LEN);
                server_broadcast_message(server, msg->room_id, msg->username, msg->message);
                break;
            }
            
            default: {
                error_message_t *err = create_error_message(RESP_INTERNAL_ERROR, 
                                                          "Unknown message type");
                send_message(sockfd, err, sizeof(error_message_t));
                free_message(err);
                break;
            }
        }
    }
    
    server_remove_client(server, client_index);
    return NULL;
} 