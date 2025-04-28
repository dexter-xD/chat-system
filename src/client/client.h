#ifndef CLIENT_H
#define CLIENT_H

#include "../common/include/protocol.h"
#include <pthread.h>
#include <stdbool.h>

typedef enum {
    CLIENT_STATE_DISCONNECTED,
    CLIENT_STATE_CONNECTED,
    CLIENT_STATE_AUTHENTICATED,
    CLIENT_STATE_IN_ROOM
} client_state_t;

typedef enum {
    MENU_NONE,
    MENU_LOGIN,
    MENU_REGISTER,
    MENU_CREATE_ROOM,
    MENU_JOIN_ROOM,
    MENU_LEAVE_ROOM,
    MENU_CHAT,
    MENU_QUIT
} client_menu_t;

typedef struct {
    int sockfd;
    client_state_t state;
    char username[MAX_USERNAME_LEN];
    char current_room_id[MAX_ROOM_ID_LEN];
    char current_room_name[MAX_ROOM_NAME_LEN];
    bool running;
    pthread_t recv_thread;
    pthread_mutex_t mutex;
} client_t;

int client_init(client_t *client);
int client_connect(client_t *client, const char *hostname, int port);
void client_disconnect(client_t *client);
int client_login(client_t *client, const char *username, const char *password);
int client_register(client_t *client, const char *username, const char *password);
int client_create_room(client_t *client, const char *room_name);
int client_join_room(client_t *client, const char *room_id);
int client_leave_room(client_t *client);
int client_send_message(client_t *client, const char *message);
void *client_receive_thread(void *arg);
void client_display_menu(client_t *client);
void client_handle_input(client_t *client);

#endif