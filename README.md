# arcanelock
Arcane Lock Password Keeper

## Requirements

To build and run ArcaneLock, you will need the following:

*   **CMake**: Version 3.16 or higher. Used as the build system generator.
*   **Qt 6 Development Libraries**: Specifically, Qt Widgets for C++.
*   **libsodium Development Libraries**: A modern, easy-to-use crypto library for encryption.

## Installation of Requirements (Development Libraries)

### Linux (Debian/Ubuntu/Mint based distributions)

Open your terminal and run:

```bash
sudo apt update
sudo apt install cmake qt6-base-dev libsodium-dev
```

### macOS

Open your terminal and install Homebrew if you haven't already:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then, install the requirements:

```bash
brew install cmake qt@6 libsodium
```

### Windows

Installing dependencies on Windows can be more involved. You will typically need:

1.  **CMake**: Download and install from [cmake.org](https://cmake.org/download/).
2.  **A C++ Compiler**: Like MSVC (from Visual Studio) or MinGW-w64.
3.  **Qt 6**: Download and install the Qt Online Installer from [qt.io](https://www.qt.io/download). Make sure to select the Qt 6 version for your chosen compiler (e.g., MSVC 2019 64-bit or MinGW 64-bit).
4.  **libsodium**:
    *   Download pre-compiled binaries from the `libsodium` GitHub releases page (look for `libsodium-X.Y.Z-msvc.zip` or `libsodium-X.Y.Z-mingw.zip` depending on your compiler).
    *   Extract the archive to a convenient location (e.g., `C:\libsodium`).
    *   You may need to set environment variables or pass `CMAKE_PREFIX_PATH` to CMake during configuration so it can find `libsodium`. For example, `cmake -DCMAKE_PREFIX_PATH=C:/libsodium ..` if `libsodium` is extracted to `C:\libsodium`.

## Building the Project

Once all requirements are installed, navigate to the root directory of the ArcaneLock project in your terminal.

1.  **Run the build script:**

    ```bash
    ./build.sh
    ```

    This script will create a `build` directory, configure CMake, and compile the project.

2.  **If the build is successful, you can run the application:**

    ```bash
    ./run.sh
    ```

    This script executes the compiled `arcanelock` executable located in the `build` directory.