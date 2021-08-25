cmake_minimum_required(VERSION 3.12)

project(aes_key_generator)

set(CMAKE_EXPORT_COMPILE_COMMANDS "YES")

set(MbedTLS_DIR "/home/tangjia/Jackistang/mbedtls/build/cmake")

find_package(MbedTLS)

set(src 
    "main.c"
)

add_executable(mbed_example ${src})

target_link_libraries(mbed_example
    PUBLIC
    MbedTLS::mbedtls
    MbedTLS::mbedcrypto
    MbedTLS::mbedx509 
)

