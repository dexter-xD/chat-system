# Chat System

A simple, real-time chat system with client-server architecture built in C. The system allows users to register, authenticate, create chat rooms, join existing rooms, and exchange messages with other users in the same room.

## Features

- User registration and authentication
- Chat room creation and management
- Real-time messaging within rooms
- Multi-client support
- Command-line interface for both client and server

## Project Structure

```
.
├── src/                    # Source code
│   ├── client/             # Client application
│   │   ├── client.c        # Main client implementation
│   │   ├── client.h        # Client header file
│   │   ├── client_ui.c     # Client user interface
│   │   ├── client_network.c # Client network handling
│   │   └── CMakeLists.txt  # Client build configuration
│   ├── server/             # Server application
│   │   ├── server.c        # Main server implementation
│   │   ├── server.h        # Server header file
│   │   ├── server_room.c   # Server room management
│   │   ├── server_client.c # Server client handling
│   │   ├── server_auth.c   # Server authentication logic
│   │   └── CMakeLists.txt  # Server build configuration
│   ├── common/             # Shared code between client and server
│   │   ├── include/        # Common header files
│   │   │   ├── database.h  # Database interface
│   │   │   ├── message.h   # Message handling
│   │   │   ├── protocol.h  # Communication protocol
│   │   │   └── utils.h     # Utility functions
│   │   ├── src/            # Common source files
│   │   │   ├── database.c  # Database implementation
│   │   │   ├── message.c   # Message creation and parsing
│   │   │   ├── protocol.c  # Protocol implementation
│   │   │   └── utils.c     # Utility functions
│   │   └── CMakeLists.txt  # Common build configuration
│   └── CMakeLists.txt      # Main source build configuration
└── CMakeLists.txt          # Main project build configuration
```

## Requirements

- C compiler (GCC or Clang)
- CMake (version 3.10 or higher)
- SQLite3 development libraries
- POSIX-compliant operating system (Linux, macOS, or WSL for Windows)
- pthread library

## Building

1. Clone the repository
2. Create a build directory and navigate to it:
   ```bash
   mkdir -p build
   cd build
   ```
3. Configure the project with CMake:
   ```bash
   cmake ..
   ```
4. Build the project:
   ```bash
   make
   ```

The executables will be created in the `bin` directory.

## Running the Server

```bash
./bin/chat_server [options]
```

Options:
- `-d, --db PATH` - Database path (default: `../chat.db`)
- `-p, --port PORT` - Port to listen on (default: `8080`)
- `-h, --help` - Show help message

Example:
```bash
./bin/chat_server -p 9000 -d /path/to/custom.db
```

## Running the Client

```bash
./bin/chat_client [options]
```

Options:
- `-h, --host HOST` - Server hostname (default: `127.0.0.1`)
- `-p, --port PORT` - Server port (default: `8080`)
- `--help` - Show help message

Example:
```bash
./bin/chat_client -h chat.example.com -p 9000
```

## Usage

### Server
1. Start the server with desired options
2. The server will automatically create the database if it doesn't exist
3. Keep the server running to allow clients to connect

### Client
1. Start the client and connect to the server
2. Register a new account or login with existing credentials
3. Create a new chat room or join an existing one
4. Start chatting with other users in the same room

## Communication Protocol

The client and server communicate using a custom binary protocol with different message types:
- Authentication requests/responses
- Registration requests/responses
- Room creation requests/responses
- Room joining/leaving requests/responses
- Chat messages
- Error messages

Each message has a header specifying the message type and length, followed by message-specific data.

## License

[MIT License](LICENSE)

## Support the Project

If you find this project useful, consider buying me a coffee!

[![Buy Me A Coffee](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://buymeacoffee.com/trish07) 