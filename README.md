# SimpleNotepad: A Good Old Windows Notepad Clone

[![Build Status](https://github.com/SFINXVC/SimpleNotepad/actions/workflows/cmake-build.yml/badge.svg)](https://github.com/SFINXVC/SimpleNotepad/actions)  [![License](https://img.shields.io/github/license/SFINXVC/SimpleNotepad)](LICENSE)

## Overview

SimpleNotepad is a simplified reimplementation of the old Windows Notepad. I don’t know why I made this, but it's kinda cool actually.
I made this project using C, and CMake as the build system (I actually wanted to use "make" for the build system, but I’m kinda lazy, so I might add it later).

## Features

- Basic Windows Notepad features
- Basic formatting options (like changing fonts, etc.)
- File saving and loading (still has some bugs, probably)
- Discord Rich Presence (hell yeah)

## Installation

### Option 1: Download Pre-built Application

1. Visit the [Releases page](https://github.com/SFINXVC/SimpleNotepad/releases) to download the latest version.
2. Just open the executable to run it.

### Option 2: Build from Source

1. Clone this repository:
   ```bash
   git clone https://github.com/SFINXVC/SimpleNotepad.git
   cd SimpleNotepad
   ```

2. Install dependencies (wait, there are **no external dependencies** here).

3. Build the project:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

4. Finally, just run the application:
   ```bash
   ./SimpleNotepad.exe
   ```

## Supported Compilers

SimpleNotepad supports the following compilers:

- Microsoft Visual C++ (MSVC)

(I only tested MSVC. Feel free to try building it with another compiler if possible.)

## Contributing

This is just a for-fun project, never meant to be serious. But feel free to submit issues or pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.