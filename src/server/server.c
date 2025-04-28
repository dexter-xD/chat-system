#include "server.h"
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
#include <signal.h>
#include <errno.h>

server_t *g_server = NULL;

void handle_signal(int sig) {
    if (g_server) {
        server_stop(g_server);
    }
    exit(0);
}

int server_init(server_t *server, const char *db_path) {
    if (!server || !db_path) {
        return -1;
    }
    if (db_init(&server->db, db_path) != 0) {
        log_message("Failed to initialize database");
        return -1;
    }
    
    memset(server->clients, 0, sizeof(server->clients));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->clients[i].sockfd = -1;
        server->clients[i].authenticated = false;
        server->clients[i].connected = false;
    }
    
    if (pthread_mutex_init(&server->clients_mutex, NULL) != 0) {
        log_message("Failed to initialize mutex");
        db_close(&server->db);
        return -1;
    }
    
    server->running = false;
    server->server_sockfd = -1;
    g_server = server;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    return 0;
}

int server_start(server_t *server, int port) {
    if (!server || port <= 0) {
        return -1;
    }
    
    printf("Debug: Starting server on port %d\n", port);
    server->server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_sockfd < 0) {
        printf("Debug: Failed to create socket: %s\n", strerror(errno));
        log_message("Failed to create socket");
        return -1;
    }
    
    printf("Debug: Socket created: %d\n", server->server_sockfd);

    int opt = 1;
    if (setsockopt(server->server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("Debug: Failed to set socket options: %s\n", strerror(errno));
        log_message("Failed to set socket options");
        close(server->server_sockfd);
        return -1;
    }
    
    printf("Debug: Socket options set\n");
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    server_addr.sin_port = htons(port);
    
    printf("Debug: Binding to 127.0.0.1:%d\n", port);
    if (bind(server->server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Debug: Failed to bind socket: %s\n", strerror(errno));
        log_message("Failed to bind socket");
        close(server->server_sockfd);
        return -1;
    }
    
    printf("Debug: Socket bound successfully\n");
    if (listen(server->server_sockfd, 10) < 0) {
        printf("Debug: Failed to listen on socket: %s\n", strerror(errno));
        log_message("Failed to listen on socket");
        close(server->server_sockfd);
        return -1;
    }
    
    printf("Debug: Server listening on 127.0.0.1:%d\n", port);
    log_message("Server started on port %d", port);
    
    server->running = true;
    
    while (server->running) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        int client_sockfd = accept(server->server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd < 0) {
            if (server->running) {
                log_message("Failed to accept connection");
            }
            continue;
        }
        
        int client_index = server_add_client(server, client_sockfd, client_addr);
        if (client_index < 0) {
            log_message("Failed to add client");
            close(client_sockfd);
            continue;
        }
        
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void *)(intptr_t)client_index) != 0) {
            log_message("Failed to create thread for client");
            server_remove_client(server, client_index);
            continue;
        }
        
        server->clients[client_index].thread = thread;
        
        log_message("New client connected: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    
    return 0;
}

void server_stop(server_t *server) {
    if (!server) {
        return;
    }
    
    log_message("Stopping server...");
    
    server->running = false;
    if (server->server_sockfd >= 0) {
        close(server->server_sockfd);
        server->server_sockfd = -1;
    }
    
    pthread_mutex_lock(&server->clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].connected) {
            close(server->clients[i].sockfd);
            server->clients[i].connected = false;
            pthread_join(server->clients[i].thread, NULL);
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);
    pthread_mutex_destroy(&server->clients_mutex);
    db_close(&server->db);
    
    log_message("Server stopped");
}

int server_add_client(server_t *server, int sockfd, struct sockaddr_in addr) {
    if (!server || sockfd < 0) {
        return -1;
    }
    
    pthread_mutex_lock(&server->clients_mutex);
    
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!server->clients[i].connected) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        pthread_mutex_unlock(&server->clients_mutex);
        return -1;
    }
    
    server->clients[index].sockfd = sockfd;
    server->clients[index].addr = addr;
    server->clients[index].authenticated = false;
    server->clients[index].connected = true;
    server->clients[index].username[0] = '\0';
    server->clients[index].current_room_id[0] = '\0';
    
    pthread_mutex_unlock(&server->clients_mutex);
    
    return index;
}

void server_remove_client(server_t *server, int client_index) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS) {
        return;
    }
    
    pthread_mutex_lock(&server->clients_mutex);
    if (server->clients[client_index].sockfd >= 0) {
        close(server->clients[client_index].sockfd);
        server->clients[client_index].sockfd = -1;
    }
    
    server->clients[client_index].authenticated = false;
    server->clients[client_index].connected = false;
    pthread_mutex_unlock(&server->clients_mutex);
    log_message("Client disconnected: %s", server->clients[client_index].username);
}

int server_find_client_by_sockfd(server_t *server, int sockfd) {
    if (!server || sockfd < 0) {
        return -1;
    }
    
    pthread_mutex_lock(&server->clients_mutex);
    
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].connected && server->clients[i].sockfd == sockfd) {
            index = i;
            break;
        }
    }
    
    pthread_mutex_unlock(&server->clients_mutex);
    
    return index;
}

int main(int argc, char *argv[]) {
    const char *db_path = "../chat.db"; 
    int port = SERVER_PORT;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--db") == 0) {
            if (i + 1 < argc) {
                db_path = argv[i + 1];
                i++;
            }
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
                if (port <= 0) {
                    port = SERVER_PORT;
                }
                i++;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -d, --db PATH     Database path (default: %s)\n", db_path);
            printf("  -p, --port PORT   Port to listen on (default: %d)\n", SERVER_PORT);
            printf("  -h, --help        Show this help message\n");
            return 0;
        }
    }
    
    printf("Debug: Using database path: %s\n", db_path);
    server_t server;
    if (server_init(&server, db_path) != 0) {
        log_message("Failed to initialize server");
        return 1;
    }
    
    if (server_start(&server, port) != 0) {
        log_message("Failed to start server");
        return 1;
    }
    
    return 0;
} 