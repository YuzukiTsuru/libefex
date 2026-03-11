function(add_executable_with_libraries target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE efex usb-1.0)

    # Copy libusb DLL to output directory when using shared library
    if(LIBEFEX_USE_SHARED_LIBUSB)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:usb-1.0>"
            "$<TARGET_FILE_DIR:${target_name}>"
        )
    endif()
endfunction()

