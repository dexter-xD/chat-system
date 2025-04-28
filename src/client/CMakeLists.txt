add_executable(chat_client
    client.c
    client_ui.c
    client_network.c
)

target_link_libraries(chat_client
    PRIVATE
        common
        ${CMAKE_THREAD_LIBS_INIT}
) 