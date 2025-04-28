#ifndef SERVER_H
#define SERVER_H

#include "../common/include/database.h"
#include "../common/include/protocol.h"
#include <pthread.h>
#include <stdbool.h>
#include <netinet/in.h>

#define MAX_CLIENTS 100
#define MAX_ROOMS 50
#define SERVER_PORT 8080

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    char username[MAX_USERNAME_LEN];
    bool authenticated;
    pthread_t thread;
    char current_room_id[MAX_ROOM_ID_LEN];
    bool connected;
} client_t;

typedef struct {
    int server_sockfd;
    database_t db;
    client_t clients[MAX_CLIENTS];
    pthread_mutex_t clients_mutex;
    bool running;
} server_t;

int server_init(server_t *server, const char *db_path);
int server_start(server_t *server, int port);
void server_stop(server_t *server);
void *handle_client(void *arg);
bool server_authenticate(server_t *server, int client_index, const char *username, const char *password);
int server_register_user(server_t *server, int client_index, const char *username, const char *password);
int server_create_room(server_t *server, int client_index, const char *room_name, char *room_id_out);
int server_join_room(server_t *server, int client_index, const char *room_id);
int server_leave_room(server_t *server, int client_index);
int server_broadcast_message(server_t *server, const char *room_id, const char *username, const char *message);
int server_add_client(server_t *server, int sockfd, struct sockaddr_in addr);
void server_remove_client(server_t *server, int client_index);
int server_find_client_by_sockfd(server_t *server, int sockfd);

#endif