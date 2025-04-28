#include "server.h"
#include "../common/include/protocol.h"
#include "../common/include/message.h"
#include "../common/include/utils.h"

bool server_authenticate(server_t *server, int client_index, const char *username, const char *password) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS || !username || !password) {
        return false;
    }
    if (server->clients[client_index].authenticated) {
        return true;
    }

    bool authenticated = db_authenticate_user(&server->db, username, password);
    if (authenticated) {
        pthread_mutex_lock(&server->clients_mutex);
        server->clients[client_index].authenticated = true;
        safe_strcpy(server->clients[client_index].username, username, MAX_USERNAME_LEN);
        pthread_mutex_unlock(&server->clients_mutex);
        
        log_message("User authenticated: %s", username);
    } else {
        log_message("Authentication failed for user: %s", username);
    }
    
    return authenticated;
}

int server_register_user(server_t *server, int client_index, const char *username, const char *password) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS || !username || !password) {
        return -1;
    }
    int result = db_register_user(&server->db, username, password);
    
    if (result > 0) {
        log_message("New user registered: %s", username);
    } else if (result == -2) {
        log_message("User already exists: %s", username);
    } else {
        log_message("Failed to register user: %s", username);
    }
    
    return result;
} 