#include "client.h"
#include "../common/include/protocol.h"
#include "../common/include/message.h"
#include "../common/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h> 


static client_t *g_client = NULL;
void handle_signal(int sig) {
    if (g_client) {
        g_client->running = false;
        client_disconnect(g_client);
    }
    exit(0);
}

int client_init(client_t *client) {
    if (!client) {
        return -1;
    }
    memset(client, 0, sizeof(client_t));
    client->sockfd = -1;
    client->state = CLIENT_STATE_DISCONNECTED;
    client->running = false;
    if (pthread_mutex_init(&client->mutex, NULL) != 0) {
        printf("Failed to initialize mutex\n");
        return -1;
    }
    g_client = client;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    return 0;
}

int client_connect(client_t *client, const char *hostname, int port) {
    if (!client || !hostname || port <= 0) {
        printf("Invalid parameters for client_connect\n");
        return -1;
    }
    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->sockfd < 0) {
        return -1;
    }
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        close(client->sockfd);
        client->sockfd = -1;
        return -1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(port);   
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, ip_str, sizeof(ip_str));
    
    if (connect(client->sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(client->sockfd);
        client->sockfd = -1;
        return -1;
    }
    
    
    client->running = true;
    if (pthread_create(&client->recv_thread, NULL, client_receive_thread, client) != 0) {
        close(client->sockfd);
        client->sockfd = -1;
        client->running = false;
        return -1;
    }
    
    pthread_mutex_lock(&client->mutex);
    client->state = CLIENT_STATE_CONNECTED;
    pthread_mutex_unlock(&client->mutex);
    
    return 0;
}

void client_disconnect(client_t *client) {
    if (!client) {
        return;
    }
    
    client->running = false;
    if (client->sockfd >= 0) {
        close(client->sockfd);
        client->sockfd = -1;
    }
    if (client->recv_thread) {
        pthread_join(client->recv_thread, NULL);
        client->recv_thread = 0;
    }
    pthread_mutex_lock(&client->mutex);
    client->state = CLIENT_STATE_DISCONNECTED;
    client->username[0] = '\0';
    client->current_room_id[0] = '\0';
    client->current_room_name[0] = '\0';
    pthread_mutex_unlock(&client->mutex);
    pthread_mutex_destroy(&client->mutex);
}

int client_login(client_t *client, const char *username, const char *password) {
    if (!client || !username || !password || client->state < CLIENT_STATE_CONNECTED) {
        return -1;
    }
    auth_request_t *req = create_auth_request(username, password);
    if (!req) {
        return -1;
    }
    if (send_message(client->sockfd, req, sizeof(auth_request_t)) != 0) {
        perror("Failed to send authentication request");
        free_message(req);
        return -1;
    }
    
    free_message(req);
    return 0;
}

int client_register(client_t *client, const char *username, const char *password) {
    if (!client || !username || !password || client->state < CLIENT_STATE_CONNECTED) {
        return -1;
    }
    
    register_request_t *req = create_register_request(username, password);
    if (!req) {
        return -1;
    }
    
    if (send_message(client->sockfd, req, sizeof(register_request_t)) != 0) {
        perror("Failed to send register request");
        free_message(req);
        return -1;
    }
    
    free_message(req);
    return 0;
}

int client_create_room(client_t *client, const char *room_name) {
    if (!client || !room_name || client->state < CLIENT_STATE_AUTHENTICATED) {
        return -1;
    }
    
    create_room_request_t *req = create_room_request(room_name);
    if (!req) {
        return -1;
    }
    
    if (send_message(client->sockfd, req, sizeof(create_room_request_t)) != 0) {
        perror("Failed to send create room request");
        free_message(req);
        return -1;
    }
    
    free_message(req);
    return 0;
}

int client_join_room(client_t *client, const char *room_id) {
    if (!client || !room_id || client->state < CLIENT_STATE_AUTHENTICATED) {
        return -1;
    }
    
    join_room_request_t *req = create_join_room_request(room_id);
    if (!req) {
        return -1;
    }
    
    if (send_message(client->sockfd, req, sizeof(join_room_request_t)) != 0) {
        perror("Failed to send join room request");
        free_message(req);
        return -1;
    }
    
    free_message(req);
    return 0;
}

int client_leave_room(client_t *client) {
    if (!client || client->state < CLIENT_STATE_IN_ROOM) {
        return -1;
    }
    
    leave_room_request_t *req = create_leave_room_request(client->current_room_id);
    if (!req) {
        return -1;
    }
    
    if (send_message(client->sockfd, req, sizeof(leave_room_request_t)) != 0) {
        perror("Failed to send leave room request");
        free_message(req);
        return -1;
    }
    
    free_message(req);
    pthread_mutex_lock(&client->mutex);
    client->state = CLIENT_STATE_AUTHENTICATED;
    client->current_room_id[0] = '\0';
    client->current_room_name[0] = '\0';
    pthread_mutex_unlock(&client->mutex);
    
    return 0;
}

int client_send_message(client_t *client, const char *message) {
    if (!client || !message || client->state < CLIENT_STATE_IN_ROOM) {
        return -1;
    }
    
    chat_message_t *msg = create_chat_message(client->current_room_id, client->username, message);
    if (!msg) {
        return -1;
    }
    
    if (send_message(client->sockfd, msg, sizeof(chat_message_t)) != 0) {
        perror("Failed to send chat message");
        free_message(msg);
        return -1;
    }
    
    free_message(msg);
    
    return 0;
}

int main(int argc, char *argv[]) {
    const char *hostname = "127.0.0.1"; 
    int port = 8080;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--host") == 0) {
            if (i + 1 < argc) {
                hostname = argv[i + 1];
                i++;
            }
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
                if (port <= 0) {
                    port = 8080;
                }
                i++;
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --host HOST    Server hostname (default: %s)\n", hostname);
            printf("  -p, --port PORT    Server port (default: %d)\n", port);
            printf("  --help             Show this help message\n");
            return 0;
        }
    }
    
    client_t client;
    if (client_init(&client) != 0) {
        printf("Failed to initialize client\n");
        return 1;
    }
    printf("Connecting to %s:%d...\n", hostname, port);
    if (client_connect(&client, hostname, port) != 0) {
        printf("Failed to connect to server\n");
        return 1;
    }
    
    printf("Connected to server\n");
    client.running = true;
    while (client.running) {
        client_display_menu(&client);
        client_handle_input(&client);
    }
    client_disconnect(&client);
    
    return 0;
} 