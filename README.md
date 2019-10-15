# Improved Alpha-Tested Magnfication for Vector Textures and Special Effects

OpenGL implementation of Chris Green's paper : 
__Improved alpha-tested magnification for vector textures and special effects__.

Link to paper : https://dl.acm.org/citation.cfm?id=1281665

Requierements for the demo :
- [SFML](https://www.sfml-dev.org/index.php) ( version > 2 )
- C++ 11

The interface is done thanks to [ImGui](https://github.com/ocornut/imgui) and [ImGui SFML](https://github.com/eliasdaler/imgui-sfml).

## Build 

### Install SFML
To install the SFML on your system, there are 
[tutorials](https://www.sfml-dev.org/tutorials/2.5/#top) provided.

### Linux
Once the repository is cloned or copied :
```bash
cd ValveSignedDistanceField
mkdir build
cd build
cmake ..
make
```

### Windows

You can install the SFML in the ``Program Files`` and the ``Program Files (x86)`` directory directly,
or set the environment variable ``SFML_ROOT`` where you installed the SFML (or simply set this variable via CMake).

__Caution__ : if the ``bin`` folder of the SFML isn't in your ``path`` environment variable, don't forget
to copy the ``.dll`` files next to your executable.

This demo needs the following ``.dll`` :
- ``sfml-graphics-2`` (``sfml-graphics-d-2`` for the debug).
- ``sfml-window-2`` (``sfml-window-d-2`` for the debug).
- ``sfml-system-2`` (``sfml-system-d-2`` for the debug).

#### Using Visual Studio tool

If you have installed the ``Visual C++ tools for CMake`` component for Visual Studio, you can directly
open the cloned or copied folder in Visual Studio.

If you haven't set ``SFML_ROOT`` as an environment variable or installed SFML in ``Program Files``
directories, you can specify the path to SFML inside the ``CMakeSettings.json`` file by adding
a ``variable``.

Below is an example of what to do if the SFML folder is next to the cloned or copied repository see the ``variables`` part) :
```json
"name": "x64-Release",
"generator": "Ninja",
"configurationType": "Release",
"inheritEnvironments": [
    "msvc_x64_x64"
],
"buildRoot": "${projectDir}\\build\\${name}",
"installRoot": "${projectDir}\\install\\${name}",
"cmakeCommandArgs": "",
"buildCommandArgs": "-v",
"ctestCommandArgs": "",
"variables": [
    {
        "name": "SFML_ROOT",
        "value": "${projectDir}\\..\\SFML-2.5.1"
    }
]
```

#### Using CMake

Here is an example to build and execute the project with Visual Studio in 64 bits 
and in release mode :
```bash
cd ValveSignedDistanceField
mkdir build
cd build
# Generate Visual Studio 2017 project files (64 bits).
cmake -G "Visual Studio 15 2017 Win64" ..
# Build the project in release mode.
cmake --build . --config Release
# Execute the generated executable.
Release\ValveSignedDistanceField.exe
```

If you haven't set ``SFML_ROOT`` as an environment variable or installed SFML in ``Program Files``
directories, you can specify the path to SFML when calling the cmake generation : 
```bash
cmake -G "Visual Studio 15 2017 Win64" -DSFML_ROOT=YOUR_PATH_TO_SFML ..
```

### MacOS
Unfortunately, I don't have a Mac so I can't test it, 
but the SFML and ImGui work on MacOS, so hopefuly, this project too.
