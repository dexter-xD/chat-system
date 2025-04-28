#include "client.h"
#include "../common/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void client_display_menu(client_t *client) {
    if (!client) {
        return;
    }
    
    clear_screen();
    
    printf("=============================================\n");
    printf("              CHAT CLIENT                   \n");
    printf("=============================================\n");
    printf("Status: ");
    switch (client->state) {
        case CLIENT_STATE_DISCONNECTED:
            printf("Disconnected\n");
            break;
        case CLIENT_STATE_CONNECTED:
            printf("Connected\n");
            break;
        case CLIENT_STATE_AUTHENTICATED:
            printf("Logged in as %s\n", client->username);
            break;
        case CLIENT_STATE_IN_ROOM:
            printf("In room: %s\n", client->current_room_name);
            break;
    }
    
    printf("\n");
    printf("Menu Options:\n");
    
    if (client->state == CLIENT_STATE_CONNECTED) {
        printf("1. Login\n");
        printf("2. Register\n");
        printf("7. Quit\n");
    } else if (client->state == CLIENT_STATE_AUTHENTICATED) {
        printf("3. Create room\n");
        printf("4. Join room\n");
        printf("7. Quit\n");
    } else if (client->state == CLIENT_STATE_IN_ROOM) {
        printf("5. Leave room\n");
        printf("6. Chat\n");
        printf("7. Quit\n");
    }
    
    printf("\nEnter your choice: ");
}

void get_input(const char *prompt, char *buffer, size_t buffer_size) {
    printf("%s", prompt);
    fgets(buffer, buffer_size, stdin);
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
}

void handle_login(client_t *client) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    get_input("Username: ", username, sizeof(username));
    get_input("Password: ", password, sizeof(password));
    
    if (client_login(client, username, password) == 0) {
        printf("Logging in...\n");
    } else {
        printf("Failed to send login request\n");
        printf("Press Enter to continue...");
        getchar();
    }
}

void handle_register(client_t *client) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char confirm_password[MAX_PASSWORD_LEN];
    
    get_input("Username: ", username, sizeof(username));
    get_input("Password: ", password, sizeof(password));
    get_input("Confirm Password: ", confirm_password, sizeof(confirm_password));
    
    if (strcmp(password, confirm_password) != 0) {
        printf("Passwords do not match\n");
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    
    if (client_register(client, username, password) == 0) {
        printf("Registering...\n");
    } else {
        printf("Failed to send register request\n");
        printf("Press Enter to continue...");
        getchar();
    }
}

void handle_create_room(client_t *client) {
    char room_name[MAX_ROOM_NAME_LEN];
    
    get_input("Room Name: ", room_name, sizeof(room_name));
    
    if (client_create_room(client, room_name) == 0) {
        printf("Creating room...\n");
    } else {
        printf("Failed to send create room request\n");
        printf("Press Enter to continue...");
        getchar();
    }
}

void handle_join_room(client_t *client) {
    char room_id[MAX_ROOM_ID_LEN];
    
    get_input("Room ID: ", room_id, sizeof(room_id));
    
    if (client_join_room(client, room_id) == 0) {
        printf("Joining room...\n");
    } else {
        printf("Failed to send join room request\n");
        printf("Press Enter to continue...");
        getchar();
    }
}

void handle_chat(client_t *client) {
    clear_screen();
    printf("=============================================\n");
    printf("              CHAT MODE                    \n");
    printf("=============================================\n");
    printf("Room: %s\n", client->current_room_name);
    printf("Type your message and press Enter to send.\n");
    printf("Type '/quit' to exit chat mode.\n");
    printf("=============================================\n\n");
    
    char message[MAX_MESSAGE_LEN];
    
    while (1) {
        get_input("> ", message, sizeof(message));
        
        if (strcmp(message, "/quit") == 0) {
            break;
        }
        
        if (message[0] != '\0') {
            if (client_send_message(client, message) != 0) {
                printf("Failed to send message\n");
            }
        }
    }
}

void client_handle_input(client_t *client) {
    if (!client) {
        return;
    }
    
    int choice;
    char input[10];
    
    fgets(input, sizeof(input), stdin);
    choice = atoi(input);
    
    switch (choice) {
        case MENU_LOGIN:
            if (client->state == CLIENT_STATE_CONNECTED) {
                handle_login(client);
            }
            break;
            
        case MENU_REGISTER:
            if (client->state == CLIENT_STATE_CONNECTED) {
                handle_register(client);
            }
            break;
            
        case MENU_CREATE_ROOM:
            if (client->state == CLIENT_STATE_AUTHENTICATED) {
                handle_create_room(client);
            }
            break;
            
        case MENU_JOIN_ROOM:
            if (client->state == CLIENT_STATE_AUTHENTICATED) {
                handle_join_room(client);
            }
            break;
            
        case MENU_LEAVE_ROOM:
            if (client->state == CLIENT_STATE_IN_ROOM) {
                if (client_leave_room(client) == 0) {
                    printf("Left room\n");
                } else {
                    printf("Failed to leave room\n");
                }
            }
            break;
            
        case MENU_CHAT:
            if (client->state == CLIENT_STATE_IN_ROOM) {
                handle_chat(client);
            }
            break;
            
        case MENU_QUIT:
            client->running = false;
            break;
            
        default:
            printf("Invalid choice\n");
            printf("Press Enter to continue...");
            getchar();
            break;
    }
} 