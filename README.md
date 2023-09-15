# Solgen

Solgen – C++ code generator for creating Lua bindings using [sol2](https://github.com/ThePhD/sol2) library.

> [!WARNING]  
> There is no any guarantee that this project will work for you as you expect.
> **solgen** was created as a part of [algine](https://github.com/congard/algine)
> infrastructure, so its main goal is to automate Lua bindings generation for this
> project. However, **solgen** is open to contributions.

## Motivation

I created this code generator <u>mainly for my project [algine](https://github.com/congard/algine)</u> since I noticed,
that writing class bindings by hand is too boring, takes a lot of time and hard to maintain
(especially when you have advanced inheritance).

Note: it is very possible that something will not work for you.

## Features

1. Lua bindings generation **for C++ classes** (inner classes, constructors, operators, methods, fields, enums)
2. Ability to generate custom factories instead of constructors for selected classes
3. Ability to bind custom implementations for selected methods
4. Properties generation based on getters & setters
5. Operators bindings (temporary limited)
6. Default function arguments support
7. "Bad" (i.e. not auto-convertible from / to Lua table) arguments detector (e.g. such as `const std::vector<T> &`)
8. You can specify additional options for classes, methods, fields etc by using `///` or `//!` comments or by creating conf file.
9. Can rebuild only changed files (by default). See `--regenerate`, `--regenerate-derived` for more info
10. For more info see `--help`

## Dependencies

Solgen has only one dependency - `libclang`.

### Linux

Fedora: `sudo dnf install clang-devel`

### Windows

In order to build `solgen` on Windows, you should install [LLVM](https://releases.llvm.org/download.html).

> [!NOTE]
> If you have installed LLVM in the default location (i.e., `$Env:ProgramW6432\LLVM`),
> there is no need to specify the LLVM location.

## Requirements

C++20 compatible compiler.

### Windows

After the building process, a symlink to the `libclang.dll` will be created in order
to ensure that `solgen` can be executed even when LLVM is not included in the PATH.

By default, in Windows, symlinks can be created only with elevated privileges. To change
this, you need to enable Developer mode in the settings.

**For Windows 11:** _Settings -> Privacy & security -> For developers -> Developer Mode (on)_.

## Flags

| Flag        | Description             | Platform | Mandatory | Default                  |
|-------------|-------------------------|----------|-----------|--------------------------|
| `LLVM_PATH` | Specifies LLVM location | Windows  | No        | `$Env:ProgramW6432\LLVM` |

## Support

Tested on:

- Fedora 38
- Windows 11, clang version 16.0.0, target: x86_64-pc-windows-msvc

## Notes

It's important to run solgen from your project's root and pass absolute paths or relative from the project root.

If you see the error `stddef.h: No such file or directory` (or something similar),
you should add your compiler's include directories to solgen includes (`--includes`).
You can get that directories using the following commands:

- ```bash
  clang++ -E -x c++ - -v < /dev/null
  ```

- ```bash
  gcc -E -x c++ - -v < /dev/null
  ```

## Limitations

Most limitations are coming from the sol2 library itself.
For example, you can't bind `std::set` or `std::vector` in some cases.
Possible workarounds: you can provide your own custom implementations for these types
(i.e. convert them to lua table and vice versa) or you can just ignore them. Possible
solution - detect these types and generate custom bindings for them.

## Roadmap

1. [x] **High Priority**: refactor: move functionality from `SolGen.cpp` to the corresponding 
       classes (`Class`, `Enum` etc)
2. [ ] Add templates support
3. [x] Add Windows support
4. [ ] Add ability to disable properties (flag `--disable-properties` and option `prop_disable`)
5. [ ] Register types that were used in class (args, fields, return types etc)
6. [ ] Link getters of type `const std::string&` and setters of type `std::string_view`
7. [ ] Add multiline support for conf
8. [x] Generate bindings for enums located in namespaces
9. [ ] Detect deleted constructors
10. [ ] Complete TODOs in the project
11. [ ] Make conf files less type sensitive (?)

I will update this list when I will need more features & functionality for my own projects.

## How to use

Solgen generates only source (`cpp`) files which contain specialized templates.
To use them you can do something like that:

```cpp
// NOTE: you can specify OutputNamespace by using --output-namespace
// By default it is solgen
namespace OutputNamespace {
template<typename T> void registerLuaUsertype(sol::table &table);
}

void registerMyTypes(sol::table &table) {
    registerLuaUsertype<Foo>(table);
    registerLuaUsertype<Bar>(table);
    registerLuaUsertype<Baz>(table);
}
```

You can get list of generated files by setting `--print-paths` flag.
This option is very useful for build systems like CMake.

## Conf file syntax

```conf
[canonical-signature-1]
option1     value1
option2     value2
switcher0

# comment

[canonical-signature-2]
option1     value1
option2     value2
switcher0
```

Canonical signature means that you should write full type names, e.g.:

```cpp
const std::string& MyClass::modify(std::string_view str)
// becomes
const std::basic_string<char> &MyNamespace::MyClass::modify(std::basic_string_view<char>)
```

Available options you can find in `--help-options`
