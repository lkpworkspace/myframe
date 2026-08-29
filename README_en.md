# myframe

English | [中文](README.md)

## Overview

A component-based programming framework implemented in C++. Applications consist of actors and workers.  
Actors are message-driven and can exchange messages with one another.  
Workers are self-driven and can interact with actors through messages.  
It is suitable for building medium- to large-scale projects.

## Development and Runtime Environment

| C++ Standard |
| ------------ |
| C++17        |

| Supported Operating Systems |
| --------------------------- |
| Linux                       |
| Windows                     |
| macOS                       |

## GitHub Builds

* [GitHub CI: Linux](.github/workflows/linux.yml)
* [GitHub CI: Windows](.github/workflows/windows.yml)
* [GitHub CI: macOS](.github/workflows/macos.yml)

## Quick Local Build

```sh
# Download, build, and install dependencies
cmake -S 3rd -B build_3rd -DCMAKE_INSTALL_PREFIX=output
cmake --build build_3rd -j --config Release
# Build and install
cmake --preset release
cmake --build --preset release
# Test and pack
ctest --preset release
cpack --preset release
```

### Hello, World API Example

- [API example](test/test_hello.cpp)

### Hello, World Component Example

- [Component code example](examples/example_actor_helloworld.cpp)
- [Component configuration example](examples/example_actor_helloworld.json)

## Program Interfaces

- [Examples](examples)
- [Actor module](myframe/actor.h)
- [Worker module](myframe/worker.h)
- [Message module](myframe/msg.h)

## Documentation

- [Development Guide](doc/development_guide.md)
