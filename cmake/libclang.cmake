# Note: this file must be included after the target creation

if (WIN32)
    message(DEBUG "Platform: Windows")

    if (NOT LLVM_PATH)
        message(DEBUG "LLVM path is not specified, falling back to default")
        set(LLVM_PATH "$ENV{ProgramW6432}\\LLVM")
    endif()

    if (NOT EXISTS "${LLVM_PATH}")
        message(FATAL_ERROR "\"${LLVM_PATH}\" does not exist.")
    endif()

    set(LibClang_LIBRARIES "${LLVM_PATH}\\lib\\libclang.lib")
    set(LibClang_INCLUDES "${LLVM_PATH}\\include")

    # Create a symlink instead of copying.
    # This step ensures that solgen can be executed
    # even when LLVM is not included in the PATH.
    # You can read more here: https://bit.ly/3ZjQmvk
    add_custom_command(TARGET solgen POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${LLVM_PATH}\\bin\\libclang.dll"
            "${CMAKE_CURRENT_BINARY_DIR}\\libclang.dll")
elseif (UNIX)
    message(DEBUG "Platform: UNIX")

    set(LibClang_LIBRARIES clang)
    set(LibClang_INCLUDES "")
else()
    message(FATAL_ERROR "Unsupported platform. Only Windows and Unix are supported.")
endif()
