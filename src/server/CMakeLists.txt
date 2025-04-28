add_executable(chat_server
    server.c
    server_auth.c
    server_room.c
    server_client.c
)

target_link_libraries(chat_server
    PRIVATE
        common
        ${CMAKE_THREAD_LIBS_INIT}
) 