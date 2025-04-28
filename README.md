# Simple Chat Application

A simple TCP/IP-based chat application consisting of a server and client.

## Compilation

To compile both the server and client programs, run:

```
make
```

This will create two executables: `server` and `client`.

## Usage

### Starting the Server

```
./server
```

The server will start listening on port 8888. It can handle up to 10 simultaneous client connections.

### Connecting with Clients

```
./client
```

The client will attempt to connect to a server running on localhost (127.0.0.1) on port 8888.

Once connected, you can type messages which will be sent to all other connected clients.

## Features

- Multiple client support (up to 10 clients)
- Real-time message broadcasting
- Simple command line interface
- Automatic notification of client connections and disconnections

## Cleanup

To remove the compiled executables:

```
make clean
``` 

## Support the Project

If you find this project useful, consider buying me a coffee!

[![Buy Me A Coffee](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://buymeacoffee.com/trish07) 