cmake_minimum_required(VERSION 3.11)
project(TwtichChatBot CXX)
set(CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0091 NEW)

add_compile_definitions(
    _WIN32_WINNT=0x0601
    BOOST_ALL_NO_LIB=1
    SPDLOG_NO_EXCEPTIONS=1
)

include_directories(src)

if(MSVC)
    add_compile_options(/bigobj)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(Boost_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(Boost 1.84.0 REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

############################################# libraries

add_library(Logger STATIC 
    src/logging.h
    src/logging.cpp
)

target_link_libraries(Logger PUBLIC
    spdlog::spdlog
)

target_include_directories(Logger PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
#    ${spdlog_INCLUDE_DIRS}
#    ${fmt_INCLUDE_DIRS}
)

########################################## ssl serts loader

add_library(SSLCertsLoader STATIC 
    src/ca_sertificates_loader.h 
    src/ca_sertificates_loader.cpp
)

target_include_directories(SSLCertsLoader PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(SSLCertsLoader PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    ZLIB::ZLIB
)

if(WIN32)
    target_link_libraries(SSLCertsLoader PUBLIC ws2_32 crypt32 user32)
endif()


########################################## Connection

add_library(Connection STATIC 
    src/connection.h
    src/connection.cpp
)

target_link_libraries(Connection PUBLIC 
    SSLCertsLoader
    Logger
    Boost::system
    Boost::thread
    ZLIB::ZLIB
)

########################################## Chat bot

add_library(ChatBot STATIC 
    src/command.h 
    src/command.cpp
    src/command_executor.h 
    src/command_executor.cpp
    src/user_validator.h
    src/user_validator.cpp
    src/chat_bot.h 
    src/chat_bot.cpp
)

target_link_libraries(ChatBot PUBLIC
    Connection
    Logger
)

########################################## irc client

add_library(IRCClient STATIC
    src/irc_client.h

    src/message.h
    src/message.cpp
    src/message_handler.h
    src/message_processor.h 
    src/message_processor.cpp

    src/domain.h

    src/auth_data.h
    src/auth_data.cpp
)

target_link_libraries(IRCClient PUBLIC 
    Connection
    Logger
)

add_executable(TestTwitchIRCClient 
    src/main.cpp
)

target_link_libraries(TestTwitchIRCClient PUBLIC
    IRCClient
    ChatBot
)
