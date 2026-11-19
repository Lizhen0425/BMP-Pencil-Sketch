# BMP Pencil Sketch (C++)

A low-level image-processing course project using C++ and manual BMP pixel operations. The original source and sample results are preserved in this repository.

## What it does

1. Reads an uncompressed 24-bit BMP image and its headers.
2. Converts RGB pixels to grayscale using weighted luminance.
3. Inverts and applies a 3×3 mean blur.
4. Produces a pencil-sketch effect using color dodge.

The cleaned version handles BMP row padding, validates image dimensions, and avoids division by zero. The **original student source** is preserved unchanged under `archive/`. The original file contained duplicate `doFilter` declarations and therefore did not compile as uploaded; the current `src/main.cpp` is a repaired, independently testable version, **not the unmodified historical submission**.

## Build & run

Requires a C++17 compiler. From the repository root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra src/main.cpp -o bmp-sketch
./bmp-sketch samples/input.bmp output
```

On Windows PowerShell with MinGW:

```powershell
g++ -std=c++17 -O2 src/main.cpp -o bmp-sketch.exe
.\bmp-sketch.exe samples/input.bmp output
```

This generates `output-gray.bmp` and `output-sketch.bmp` in the current directory. You can replace the input with another uncompressed 24-bit BMP file.

## Repository layout

- `src/main.cpp` — compilable, fixed implementation
- `samples/input.bmp` — provided sample input
- `samples/original-gray.bmp` and `samples/original-sketch.bmp` — historical outputs from the uploaded archive
- `archive/original-s1112037.cpp` — untouched original code
- `archive/*.vcxproj*` — original Visual Studio project metadata

## Scope and limitations

This example only supports uncompressed 24-bit BMP files, not PNG/JPEG or all BMP formats. The sketch result may differ from the provided historical output because the code was repaired. The original development date was not specified and is intentionally left unstated.
