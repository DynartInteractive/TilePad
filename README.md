TilePad
=======

Padding creator for Tiled tilesets. You can easily prevent the stripped tiles with it when you use display resolution independent rendering.

### Install on Windows

[Download the Win64 installer here](https://github.com/goph-R/TilePad/releases/download/0.5.0/tilepad-setup-0.5.0.exe)

### Build from source

**Requirements:**
- Qt 6.x
- CMake 3.16+
- C++17 compatible compiler

**Build steps:**

```
mkdir build
cd build
cmake ..
cmake --build .
```

On Linux, make sure you have the Qt6 development packages installed:

```
sudo apt install qt6-base-dev cmake build-essential
```

### How to use

Drop the tileset, you want to add padding to, on the grey area. Set the export path and hit export.

![Screenshot](https://raw.githubusercontent.com/goph-R/TilePad/master/images/screenshot.png)

Set the margin and spacing values in Tiled. The margin equals the padding set in TilePad, spacing should be the doubled value of the margin.

![Screenshot](https://raw.githubusercontent.com/goph-R/TilePad/master/images/tiled_settings.png)

### Reprocess

After dropping a tileset, you can change any settings and click the **Reprocess** button to re-apply padding with the new settings without having to drop the file again.

### Watch file for changes

Check the **Watch file** checkbox to automatically reprocess the tileset whenever the source image file changes on disk. This is useful when editing the tileset in an external image editor and wanting TilePad to update the result in real time.

### CLI usage

TilePad can be used from the command line without the GUI. If any flags are passed, it runs in headless mode:

```
TilePad --input tileset.png --output padded.png --tile-width 16 --tile-height 16 --padding 2 --force-pot --transparent
```

To remove padding:

```
TilePad --input padded.png --output original.png --tile-width 16 --tile-height 16 --padding 2 --remove
```

Run `TilePad --help` for a full list of options.

### How to remove padding

Check the "Remove padding" checkbox, drop the padded tileset image on the grey area. Set the export path and hit export.
