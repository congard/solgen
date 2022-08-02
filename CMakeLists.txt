cmake_minimum_required(VERSION 3.22)
project(solgen)

set(CMAKE_CXX_STANDARD 17)

add_executable(solgen
    src/main.cpp
    src/Class.h
    src/types.h
    src/Enum.h
    src/Field.h
    src/Function.h
    src/Arg.h
    src/PTBuilder.cpp src/PTBuilder.h
    src/SolGen.cpp src/SolGen.h
    src/GenOptions.cpp src/GenOptions.h
    src/Access.h
    src/FileUtils.cpp src/FileUtils.h
    src/StringUtils.h
    src/Type.h src/Type.cpp
    src/CmdOptions.h
    src/Conf.cpp src/Conf.h)

target_link_libraries(solgen clang)
