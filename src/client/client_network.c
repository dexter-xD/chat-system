#include "client.h"
#include "../common/include/protocol.h"
#include "../common/include/message.h"
#include "../common/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void *client_receive_thread(void *arg) {
    client_t *client = (client_t *)arg;
    if (!client) {
        return NULL;
    }
    
    char buffer[2048];
    
    while (client->running) {
        int recv_size = receive_message(client->sockfd, buffer, sizeof(buffer));
        if (recv_size <= 0) {
            if (client->running) {
                printf("\nDisconnected from server\n");
                client->running = false;
            }
            break;
        }
        
        message_header_t *header = (message_header_t *)buffer;
        
        switch (header->type) {
            case MSG_AUTH_RESPONSE: {
                auth_response_t *resp = (auth_response_t *)buffer;
                
                if (resp->status == RESP_SUCCESS) {
                    pthread_mutex_lock(&client->mutex);
                    client->state = CLIENT_STATE_AUTHENTICATED;
                    pthread_mutex_unlock(&client->mutex);
                    
                    printf("\nLogin successful\n");
                } else {
                    printf("\nLogin failed: Invalid username or password\n");
                }
                
                printf("Press Enter to continue...");
                break;
            }
            
            case MSG_REGISTER_RESPONSE: {
                register_response_t *resp = (register_response_t *)buffer;
                
                if (resp->status == RESP_SUCCESS) {
                    printf("\nRegistration successful\n");
                } else if (resp->status == RESP_USER_EXISTS) {
                    printf("\nRegistration failed: Username already exists\n");
                } else {
                    printf("\nRegistration failed: Internal error\n");
                }
                
                printf("Press Enter to continue...");
                break;
            }
            
            case MSG_CREATE_ROOM_RESPONSE: {
                create_room_response_t *resp = (create_room_response_t *)buffer;
                
                if (resp->status == RESP_SUCCESS) {
                    pthread_mutex_lock(&client->mutex);
                    client->state = CLIENT_STATE_IN_ROOM;
                    safe_strcpy(client->current_room_id, resp->room_id, MAX_ROOM_ID_LEN);
                    pthread_mutex_unlock(&client->mutex);
                    
                    printf("\nRoom created successfully\n");
                    printf("Room ID: %s\n", resp->room_id);
                } else {
                    printf("\nFailed to create room\n");
                }
                
                printf("Press Enter to continue...");
                break;
            }
            
            case MSG_JOIN_ROOM_RESPONSE: {
                join_room_response_t *resp = (join_room_response_t *)buffer;
                
                if (resp->status == RESP_SUCCESS) {
                    pthread_mutex_lock(&client->mutex);
                    client->state = CLIENT_STATE_IN_ROOM;
                    safe_strcpy(client->current_room_name, resp->room_name, MAX_ROOM_NAME_LEN);
                    safe_strcpy(client->current_room_id, resp->room_id, MAX_ROOM_ID_LEN);
                    pthread_mutex_unlock(&client->mutex);
                    
                    printf("\nJoined room: %s\n", resp->room_name);
                    printf("Room ID: %s\n", resp->room_id);
                } else {
                    printf("\nFailed to join room: Room not found\n");
                }
                
                printf("Press Enter to continue...");
                break;
            }
            
            case MSG_CHAT_MESSAGE: {
                chat_message_t *msg = (chat_message_t *)buffer;
                if (client->state == CLIENT_STATE_IN_ROOM) {
                    printf("\n[%s]: %s\n> ", msg->username, msg->message);
                    fflush(stdout);
                }
                break;
            }
            
            case MSG_ERROR: {
                error_message_t *err = (error_message_t *)buffer;
                
                printf("\nError: %s\n", err->error_message);
                printf("Press Enter to continue...");
                break;
            }
            
            default:
                break;
        }
    }
    
    return NULL;
} 