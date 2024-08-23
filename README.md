# C++ Raytracer using ImGUI, GLM, Vulkan and GLFW

## Requirements

### C++ Compiler

- **Ubuntu**: Install a compiler and essential build Tools using the following command:
```bash
  sudo apt update && sudo apt install build-essential
```

- **Windows**: Download the [MinGW Windows Binary](https://sourceforge.net/projects/mingw/)
Or use [Visual Studio](https://visualstudio.microsoft.com/) with its built-in MSVC compiler.

### CMake 3.30

- **Ubuntu**: Install CMake using this command:
```bash
  sudo apt update && sudo apt install cmake
```

- **Windows**: Download the [CMake Windows binary](https://cmake.org/download/)

### Ninja 1.12

- **Ubuntu**: Install Ninja using this command:
```bash
  sudo apt update && sudo apt install ninja-build
```

- **Windows**: Download the [Ninja Windows Binary](https://github.com/ninja-build/ninja/releases)

### VulkanSDK 1.3

- **Ubuntu**: Install the VulkanSDK using this command:
```bash
  sudo apt update && sudo apt install vulkan-sdk
```
Alternatively, you can download the [VulkanSDK archive for Linux](https://vulkan.lunarg.com/sdk/home#linux) on the website
and follow the instructions in the CMakeLists at the root of the project

- **Windows**: Download the [Windows VulkanSDK Binary](https://vulkan.lunarg.com/sdk/home#windows)

## To simply build and run the project, you can run the Following command in Gitbash, PowerShell, or Ubuntu's terminal at the root of the project :
```console
./run.sh
```

## Or you can open the project in an IDE like Visual Studio using this command at the root of the project:
```console
cmake -S . -B build/ -G "Visual Studio 17 2022"
```