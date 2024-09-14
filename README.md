# SimpleNotepad: A Good Old Windows Notepad Clone

[![Build Status](https://github.com/SFINXVC/SimpleNotepad/actions/workflows/cmake-build.yml/badge.svg)](https://github.com/SFINXVC/SimpleNotepad/actions)
[![License](https://img.shields.io/github/license/SFINXVC/SimpleNotepad)](LICENSE)

## Overview

SimpleNotepad is a simplified reimplementation of the old Windows notepad. I don't know why i made this, but it's kinda cool actually!.
Built with C and using CMake, it's just because, im kinda lazy to use makefile, but im gonna add it soon!

## Features

- Basic Windows Notepad features
- Basic formatting options (such as changing font, etc)
- File saving and loading (it still had some bug probably)
- Discord Rich Presence (hell yeah)

## Installation

### Option 1: Download Pre-built Application

1. Visit the [Releases page](https://github.com/SFINXVC/SimpleNotepad/releases) to download the latest version.
2. Just open the executable to run it.

### Option 2: Build from Source

1. Clone this repository:
   ```
   git clone https://github.com/SFINXVC/SimpleNotepad.git
   cd SimpleNotepad
   ```

2. Install dependencies (but wait, THERES NO EXTERNAL DEPEDENCIES HERE).

3. Build the project:
   ```
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

4. And finally, just run the application:
   ```
   ./SimpleNotepad.exe
   ```
   
## Supported Compilers

SimpleNotepad supports the following compilers:

- Microsoft Visual C++ (MSVC)

(i only tested MSVC, you can give it a try to build it with another compiler if possible)

## Contributing

It's just a for-fun project, never meant to be serious. But feel free to submit issues or pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.