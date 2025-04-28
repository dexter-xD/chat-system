#ifndef MESSAGE_H
#define MESSAGE_H

#include "protocol.h"
#include <stdint.h>

auth_request_t *create_auth_request(const char *username, const char *password);
auth_response_t *create_auth_response(uint8_t status);
register_request_t *create_register_request(const char *username, const char *password);
register_response_t *create_register_response(uint8_t status);
create_room_request_t *create_room_request(const char *room_name);
create_room_response_t *create_room_response(uint8_t status, const char *room_id);
join_room_request_t *create_join_room_request(const char *room_id);
join_room_response_t *create_join_room_response(uint8_t status, const char *room_name, const char *room_id);
leave_room_request_t *create_leave_room_request(const char *room_id);
chat_message_t *create_chat_message(const char *room_id, const char *username, const char *message);
error_message_t *create_error_message(uint8_t error_code, const char *error_message);
void init_message_header(message_header_t *header, uint8_t type, uint32_t length);
void free_message(void *message);

#endif 