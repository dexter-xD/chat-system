#include "../include/database.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SQL_CREATE_USERS_TABLE \
    "CREATE TABLE IF NOT EXISTS users (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "username TEXT UNIQUE NOT NULL," \
    "password_hash TEXT NOT NULL);"

#define SQL_CREATE_ROOMS_TABLE \
    "CREATE TABLE IF NOT EXISTS rooms (" \
    "id TEXT PRIMARY KEY," \
    "name TEXT NOT NULL," \
    "owner_id INTEGER NOT NULL," \
    "FOREIGN KEY(owner_id) REFERENCES users(id));"

#define SQL_INSERT_USER \
    "INSERT INTO users (username, password_hash) VALUES (?, ?);"

#define SQL_GET_USER_BY_USERNAME \
    "SELECT id, username, password_hash FROM users WHERE username = ?;"

#define SQL_CREATE_ROOM \
    "INSERT INTO rooms (id, name, owner_id) VALUES (?, ?, ?);"

#define SQL_GET_ROOM_BY_ID \
    "SELECT id, name, owner_id FROM rooms WHERE id = ?;"

#define SQL_LIST_ROOMS \
    "SELECT id, name, owner_id FROM rooms;"

int db_init(database_t *db, const char *db_path) {
    if (!db || !db_path) {
        return -1;
    }
    int rc = sqlite3_open(db_path, &db->db);
    if (rc != SQLITE_OK) {
        log_message("Cannot open database: %s", sqlite3_errmsg(db->db));
        sqlite3_close(db->db);
        return -1;
    }
    db->db_path = strdup(db_path);
    if (!db->db_path) {
        sqlite3_close(db->db);
        return -1;
    }
    char *err_msg = NULL;
    rc = sqlite3_exec(db->db, SQL_CREATE_USERS_TABLE, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        log_message("SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db->db);
        free(db->db_path);
        return -1;
    }
    
    rc = sqlite3_exec(db->db, SQL_CREATE_ROOMS_TABLE, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        log_message("SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db->db);
        free(db->db_path);
        return -1;
    }
    
    return 0;
}

void db_close(database_t *db) {
    if (db) {
        if (db->db) {
            sqlite3_close(db->db);
            db->db = NULL;
        }
        
        if (db->db_path) {
            free(db->db_path);
            db->db_path = NULL;
        }
    }
}

int db_register_user(database_t *db, const char *username, const char *password) {
    if (!db || !db->db || !username || !password) {
        return -1;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    
    rc = sqlite3_prepare_v2(db->db, SQL_GET_USER_BY_USERNAME, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -2;
    }
    
    sqlite3_finalize(stmt);
    char password_hash[65];
    hash_password(password, password_hash, sizeof(password_hash));
    
    rc = sqlite3_prepare_v2(db->db, SQL_INSERT_USER, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);   
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        log_message("Failed to insert user: %s", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    return sqlite3_last_insert_rowid(db->db);
}

bool db_authenticate_user(database_t *db, const char *username, const char *password) {
    if (!db || !db->db || !username || !password) {
        return false;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    
    rc = sqlite3_prepare_v2(db->db, SQL_GET_USER_BY_USERNAME, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    const char *stored_hash = (const char *)sqlite3_column_text(stmt, 2);
    bool result = verify_password(password, stored_hash);
    
    sqlite3_finalize(stmt);
    return result;
}

int db_get_user_id(database_t *db, const char *username) {
    if (!db || !db->db || !username) {
        return -1;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    
    rc = sqlite3_prepare_v2(db->db, SQL_GET_USER_BY_USERNAME, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC); 
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int user_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt); 
    return user_id;
}

int db_create_room(database_t *db, const char *name, int owner_id, char *room_id_out) {
    if (!db || !db->db || !name || !room_id_out || owner_id <= 0) {
        return -1;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    generate_uuid(room_id_out, 37);
    rc = sqlite3_prepare_v2(db->db, SQL_CREATE_ROOM, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, room_id_out, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, owner_id); 
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        log_message("Failed to create room: %s", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

bool db_room_exists(database_t *db, const char *room_id) {
    if (!db || !db->db || !room_id) {
        return false;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    
    rc = sqlite3_prepare_v2(db->db, SQL_GET_ROOM_BY_ID, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, room_id, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    
    sqlite3_finalize(stmt);
    return exists;
}

int db_get_room_name(database_t *db, const char *room_id, char *name_out, int name_out_size) {
    if (!db || !db->db || !room_id || !name_out || name_out_size <= 0) {
        return -1;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    
    rc = sqlite3_prepare_v2(db->db, SQL_GET_ROOM_BY_ID, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, room_id, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    const char *room_name = (const char *)sqlite3_column_text(stmt, 1);
    safe_strcpy(name_out, room_name, name_out_size);
    sqlite3_finalize(stmt);
    return 0;
}

int db_list_rooms(database_t *db, room_t **rooms, int *count) {
    if (!db || !db->db || !rooms || !count) {
        return -1;
    }
    
    sqlite3_stmt *stmt;
    int rc;
    
    rc = sqlite3_prepare_v2(db->db, SQL_LIST_ROOMS, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db->db));
        return -1;
    }
    
    int room_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        room_count++;
    }
    
    sqlite3_reset(stmt);
    *rooms = (room_t *)malloc(room_count * sizeof(room_t));
    if (!*rooms) {
        sqlite3_finalize(stmt);
        return -1;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        int owner_id = sqlite3_column_int(stmt, 2);
        
        safe_strcpy((*rooms)[i].id, id, sizeof((*rooms)[i].id));
        safe_strcpy((*rooms)[i].name, name, sizeof((*rooms)[i].name));
        (*rooms)[i].owner_id = owner_id;
        
        i++;
    }
    
    *count = room_count;
    sqlite3_finalize(stmt);
    
    return 0;
} 