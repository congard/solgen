cmake_minimum_required(VERSION 3.22)
project(solgen)

set(CMAKE_CXX_STANDARD 20)

add_executable(solgen
        src/main.cpp
        src/generator/Class.h
        src/types.h
        src/generator/Enum.h
        src/generator/Field.h
        src/generator/Function.h
        src/generator/Arg.h
        src/Parser.cpp src/Parser.h
        src/generator/Generator.cpp src/generator/Generator.h
        src/generator/Options.cpp src/generator/Options.h
        src/generator/Access.h
        src/utils/FileUtils.cpp src/utils/FileUtils.h
        src/utils/StringUtils.h
        src/Type.h src/Type.cpp
        src/CmdOptions.h
        src/Conf.cpp src/Conf.h
        src/ArgHandler/ArgHandler.cpp src/ArgHandler/ArgHandler.h
        src/ArgHandler/std_containers.hpp
        src/utils/format.cpp
        src/utils/format.h
        src/generator/Class.cpp
        src/generator/Arg.cpp
        src/generator/Generated.h
        src/utils/matches.cpp
        src/utils/matches.h
        src/generator/Enum.cpp
        src/generator/Construct.cpp
        src/generator/Construct.h
        src/generator/Function.cpp
        src/generator/Field.cpp)

target_link_libraries(solgen clang)
