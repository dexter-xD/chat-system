#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdbool.h>

void generate_uuid(char *uuid_str, size_t uuid_len);
void hash_password(const char *password, char *hash_out, size_t hash_out_size);
bool verify_password(const char *password, const char *hash);
size_t safe_strcpy(char *dest, const char *src, size_t dest_size);
char *trim_string(char *str);
void log_message(const char *format, ...);

#endif