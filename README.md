TilePad
=======

Padding creator for Tiled tilesets. You can easily prevent the stripped tiles with it when you use display resolution independent rendering.

### Install on Windows

[Download the Win64 installer here](https://github.com/DynartInteractive/TilePad/releases/download/0.6.0/tilepad-setup-0.6.0.exe)

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

### Getting started

When you launch TilePad, a startup dialog lets you create a new project or open an existing one. Recent projects are listed for quick access.

![Startup](https://raw.githubusercontent.com/DynartInteractive/TilePad/master/images/startup.png)

### How to use

Drop tilesets onto the preview area or use **File > Import** to add files. Each file gets its own tab. Set the tile settings, then click **Reprocess** to generate the padded result. Set the export path and hit **Export**.

![Screenshot](https://raw.githubusercontent.com/DynartInteractive/TilePad/master/images/screenshot_dark.png)

Set the margin and spacing values in Tiled. The margin equals the padding set in TilePad, spacing should be the doubled value of the margin.

![Screenshot](https://raw.githubusercontent.com/DynartInteractive/TilePad/master/images/tiled_settings.png)

### Project system

TilePad uses a project-based workflow. Projects save your tile settings, file list, and export paths to a `.tilepad` file.

- **File > New Project** — start a fresh project with default settings
- **File > Open Project** — open an existing `.tilepad` file
- **File > Save Project / Save As** — save the current project
- **File > Import Files** — add images to the project (or use the **+** button on the tab bar)
- **Recent Projects** — quickly reopen previous projects from the File menu

### Multi-file workflow

Work with multiple tilesets at once. Each imported file appears as a tab. Switching tabs updates the source/result preview and export path. You can also drag and drop multiple files onto the preview area.

### Export

Each file has its own export path. By default, exports use the `.export.` tag (e.g., `tileset.export.png`). You can set a custom export path per file using the **Browse** button.

Set a project-level **Export Directory** to change where new files are exported by default. This setting is saved with the project.

- **Export** — exports the current file
- **Export All** — exports all files that have pending changes

### Reprocess

After importing a tileset, you can change any settings and click the **Reprocess** button to re-apply padding with the new settings. Reprocessing auto-exports to the file's export path.

### Watch file for changes

Check the **Watch file** checkbox to automatically reprocess the tileset whenever the source image file changes on disk. This is useful when editing the tileset in an external image editor and wanting TilePad to update the result in real time.

### Themes

TilePad supports dark, light, and system-following themes. Change the theme from **View > Theme**.

![Light theme](https://raw.githubusercontent.com/DynartInteractive/TilePad/master/images/screenshot_light.png)

### How to remove padding

Check the "Remove padding" checkbox, drop the padded tileset image on the preview area. Set the export path and hit export.

### CLI usage

TilePad can be used from the command line without the GUI. When any flags are passed, it runs in headless mode. Running without flags launches the GUI.

**Add padding:**

```
TilePad -i tileset.png -o padded.png --tile-width 16 --tile-height 16 -p 2 --transparent
```

**Add padding with Force PoT and tile reordering:**

```
TilePad -i tileset.png -o padded.png --tile-width 16 --tile-height 16 -p 2 --force-pot --reorder --transparent
```

**Add padding with custom background color:**

```
TilePad -i tileset.png -o padded.png --tile-width 32 --tile-height 32 -p 1 --bg-color FF00FF
```

**Remove padding:**

```
TilePad -i padded.png -o original.png --tile-width 16 --tile-height 16 -p 2 --remove
```

**CLI options:**

| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--input` | `-i` | Input image file path (required) | |
| `--output` | `-o` | Output image file path (required) | |
| `--tile-width` | | Tile width in pixels | 16 |
| `--tile-height` | | Tile height in pixels | 16 |
| `--padding` | `-p` | Padding in pixels | 1 |
| `--force-pot` | | Force power of two output dimensions | off |
| `--reorder` | | Reorder tiles (use with --force-pot) | off |
| `--transparent` | | Use transparent padding | on |
| `--bg-color` | | Background color hex (e.g. FF00FF) | FF00FF |
| `--remove` | | Remove padding instead of adding | off |
| `--help` | `-h` | Show help | |
| `--version` | `-v` | Show version | |

Supported image formats: PNG, JPG, JPEG.
