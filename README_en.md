# myframe

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
cmake -S . -B build_proj -DCMAKE_INSTALL_PREFIX=output -DCMAKE_PREFIX_PATH=output -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
cmake --build build_proj -j --config Release --target install
```

### Hello, World API Example

- [API example](test/hello_test.cpp)

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
