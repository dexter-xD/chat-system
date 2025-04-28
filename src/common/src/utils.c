#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

void generate_uuid(char *uuid_str, size_t uuid_len) {
    if (uuid_len < 37) {
        return;
    }
    
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    sprintf(uuid_str, 
            "%08x-%04x-%04x-%04x-%04x%08x",
            rand() & 0xffffffff,
            rand() & 0xffff,
            ((rand() & 0x0fff) | 0x4000), 
            ((rand() & 0x3fff) | 0x8000),
            rand() & 0xffff,
            rand() & 0xffffffff);
}

void hash_password(const char *password, char *hash_out, size_t hash_out_size) {
    if (hash_out_size < 64) {
        return;
    }
    
    unsigned char hash[32];
    size_t password_len = strlen(password);
    
    for (size_t i = 0; i < 32; i++) {
        hash[i] = (password[i % password_len] ^ ((i * 13) & 0xFF)) + 7;
    }
    
    for (size_t i = 0; i < 32; i++) {
        sprintf(hash_out + (i * 2), "%02x", hash[i]);
    }
    hash_out[64] = '\0';
}

bool verify_password(const char *password, const char *hash) {
    char computed_hash[65];
    hash_password(password, computed_hash, sizeof(computed_hash));
    return strcmp(computed_hash, hash) == 0;
}

size_t safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (dest_size == 0) {
        return 0;
    }
    
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    
    return i;
}

char *trim_string(char *str) {
    if (!str) return NULL;
    
    while (isspace((unsigned char)*str)) {
        str++;
    }
    
    if (*str == 0) {
        return str; 
    }
    
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';
    return str;
}

void log_message(const char *format, ...) {
    time_t now;
    struct tm *timeinfo;
    char timestamp[20];
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    printf("[%s] ", timestamp);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
} 