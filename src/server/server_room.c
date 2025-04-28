#include "server.h"
#include "../common/include/protocol.h"
#include "../common/include/message.h"
#include "../common/include/utils.h"
#include <stdio.h>

int server_create_room(server_t *server, int client_index, const char *room_name, char *room_id_out) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS || !room_name || !room_id_out) {
        return -1;
    }
    
    if (!server->clients[client_index].authenticated) {
        return -1;
    }
    
    int user_id = db_get_user_id(&server->db, server->clients[client_index].username);
    if (user_id <= 0) {
        return -1;
    }
    
    int result = db_create_room(&server->db, room_name, user_id, room_id_out);
    
    if (result == 0) {
        log_message("New room created: %s (ID: %s) by user %s", 
                   room_name, room_id_out, server->clients[client_index].username);
                   
        server_join_room(server, client_index, room_id_out);
    } else {
        log_message("Failed to create room: %s", room_name);
    }
    
    return result;
}

int server_join_room(server_t *server, int client_index, const char *room_id) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS || !room_id) {
        return -1;
    }
    if (!server->clients[client_index].authenticated) {
        return -1;
    }
    if (!db_room_exists(&server->db, room_id)) {
        return -1;
    }

    pthread_mutex_lock(&server->clients_mutex);
    safe_strcpy(server->clients[client_index].current_room_id, room_id, MAX_ROOM_ID_LEN);
    pthread_mutex_unlock(&server->clients_mutex);
    char room_name[MAX_ROOM_NAME_LEN];
    if (db_get_room_name(&server->db, room_id, room_name, sizeof(room_name)) != 0) {
        return -1;
    }
    
    log_message("User %s joined room: %s (ID: %s)", 
               server->clients[client_index].username, room_name, room_id);
    char system_message[MAX_MESSAGE_LEN];
    snprintf(system_message, sizeof(system_message), "User %s has joined the room.", 
            server->clients[client_index].username);
    server_broadcast_message(server, room_id, "SYSTEM", system_message);
    
    return 0;
}

int server_leave_room(server_t *server, int client_index) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS) {
        return -1;
    }
    if (server->clients[client_index].current_room_id[0] == '\0') {
        return 0; 
    }
    char room_name[MAX_ROOM_NAME_LEN];
    if (db_get_room_name(&server->db, server->clients[client_index].current_room_id, 
                        room_name, sizeof(room_name)) != 0) {
        return -1;
    }
    char system_message[MAX_MESSAGE_LEN];
    snprintf(system_message, sizeof(system_message), "User %s has left the room.", 
            server->clients[client_index].username);
    server_broadcast_message(server, server->clients[client_index].current_room_id, "SYSTEM", system_message);
    
    log_message("User %s left room: %s (ID: %s)", 
               server->clients[client_index].username, room_name, server->clients[client_index].current_room_id);
    
    pthread_mutex_lock(&server->clients_mutex);
    server->clients[client_index].current_room_id[0] = '\0';
    pthread_mutex_unlock(&server->clients_mutex);
    
    return 0;
} 