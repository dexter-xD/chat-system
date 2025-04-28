#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdbool.h>

typedef struct {
    sqlite3 *db;
    char *db_path;
} database_t;

typedef struct {
    int id;
    char username[32];
    char password_hash[64];
} user_t;

typedef struct {
    char id[37];       
    char name[64];
    int owner_id;
} room_t;

int db_init(database_t *db, const char *db_path);
void db_close(database_t *db);
int db_register_user(database_t *db, const char *username, const char *password);
bool db_authenticate_user(database_t *db, const char *username, const char *password);
int db_get_user_id(database_t *db, const char *username);
int db_create_room(database_t *db, const char *name, int owner_id, char *room_id_out);
bool db_room_exists(database_t *db, const char *room_id);
int db_get_room_name(database_t *db, const char *room_id, char *name_out, int name_out_size);
int db_list_rooms(database_t *db, room_t **rooms, int *count);

#endif