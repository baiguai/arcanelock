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

1.  **CMake**: Download and install from [cmake.org](https://cmake.org/download/) and ensure it's added to your system PATH.
2.  **A C++ Compiler**: Choose one and install it:
    *   **Visual Studio**: Install the free [Visual Studio Community Edition](https://visualstudio.microsoft.com/vs/community/). During installation, select the "Desktop development with C++" workload. This provides the MSVC compiler.
    *   **MinGW-w64**: A GCC/G++ compiler for Windows. You can install it via MSYS2 ([msys2.org](https://www.msys2.org/)) or other distributions. Ensure it's added to your system PATH.
3.  **Qt 6**: Download and install the Qt Online Installer from [qt.io](https://www.qt.io/download).
    *   **Crucially**, during installation, select the Qt 6 version that matches your chosen C++ compiler (e.g., "MSVC 2019 64-bit" or "MinGW 64-bit").
    *   Note the installation path (e.g., `C:\Qt\6.x.x\msvc2019_64` or `C:\Qt\6.x.x\mingw_64`).
4.  **libsodium**:
    *   Download pre-compiled binaries from the `libsodium` GitHub releases page (look for `libsodium-X.Y.Z-msvc.zip` or `libsodium-X.Y.Z-mingw.zip` depending on your compiler).
    *   Extract the archive to a convenient location (e.g., `C:\libsodium`).

### Building on Windows

After installing the requirements, you can use the provided `build_windows.bat` script.

1.  **Open a Command Prompt or PowerShell**: Navigate to the root directory of the ArcaneLock project.
2.  **Edit `build_windows.bat`**: Open `build_windows.bat` in a text editor.
    *   **Set `QT_INSTALL_DIR`**: Modify the `set QT_INSTALL_DIR=C:\path\to\your\Qt\6.x.x` line to point to your actual Qt 6 installation directory (e.g., `C:\Qt\6.x.x\msvc2019_64` or `C:\Qt\6.x.x\mingw_64`).
    *   **Set `LIBSODIUM_INSTALL_DIR`**: Modify the `set LIBSODIUM_INSTALL_DIR=C:\path\to\your\libsodium` line to point to where you extracted `libsodium` (e.g., `C:\libsodium`).
    *   **Choose CMake Generator**: Uncomment the appropriate `set CMAKE_GENERATOR="..."` line for your chosen C++ compiler (e.g., "Visual Studio 17 2022" for MSVC or "MinGW Makefiles" for MinGW).
3.  **Run the build script**:
    ```cmd
    build_windows.bat
    ```
    This script will create a `build_windows` directory, configure CMake, and compile the project.
4.  **Run the application**: If the build is successful, the executable will be located in `build_windows\<Config>\arcanelock.exe` (for Visual Studio) or `build_windows\arcanelock.exe` (for MinGW). You can run it directly.

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