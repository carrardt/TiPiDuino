# Create an INTERFACE library for our C module.
add_library(qrcodegen INTERFACE)

# Add our source files to the lib
target_sources(qrcodegen INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/qrcodegenmodule.c
    ${CMAKE_CURRENT_LIST_DIR}/qrcodegen.c
)

# Add the current directory as an include directory.
target_include_directories(qrcodegen INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE qrcodegen)

