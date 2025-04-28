add_library(common STATIC
    src/message.c
    src/protocol.c
    src/database.c
    src/utils.c
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SQLite3 REQUIRED)
target_link_libraries(common PRIVATE SQLite::SQLite3) 