cmake_minimum_required(VERSION 4.0)

project(PokerStack)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(TgBot REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
else()
    message(STATUS "Boost found: ${Boost_VERSION}")
endif()

if(NOT TgBot_FOUND)
    message(FATAL_ERROR "TgBot library not found!")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()



add_executable(PokerStack main.cpp mybot.cpp room.cpp player.cpp database.cpp common.cpp)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PQXX REQUIRED libpqxx)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${TGBOT_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::system
    ${PQXX_LINK_LIBRARIES}
    TgBot::TgBot
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${Threads_LIBRARIES}
)

list(FILTER SOURCES EXCLUDE REGEX ".*CompilerId.*")
list(FILTER SOURCES EXCLUDE REGEX ".*CMakeCXX.*")
