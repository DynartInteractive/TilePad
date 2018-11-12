TilePad
=======

Padding creator for Tiled tilesets. You can easily prevent the stripped tiles with it when you use display resolution indepent rendering.

### Install on Windows:

[Download the Win32 installer here](https://github.com/goph-R/TilePad/releases/download/release-0.4.0/tilepad-setup.exe)

### Compile on Ubuntu (18.04) install qtcreator:

```
sudo apt install build-essentials
sudo apt install qtcreator
```

Then create the make file and run make:

```
qmake -makefile -o Makefile TilePad.pro
make
```

The TilePad executable should have been created so run it:

```
./TilePad
```

### How to use

Drop the tileset, you want to has padding, on the grey area. Set the export path, and hit export.

![Screenshot](https://raw.githubusercontent.com/goph-R/TilePad/master/images/screenshot.png)

Set the margin and spacing values in Tiled. The margin equals with the padding set in TilePad, spacing should be the doubled value of the margin.

![Screenshot](https://raw.githubusercontent.com/goph-R/TilePad/master/images/tiled_settings.png)
