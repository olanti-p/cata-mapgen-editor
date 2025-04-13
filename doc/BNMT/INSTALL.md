# How to install

The editor works as a custom UI overlay on top of a loaded world.
As such, "uner the hood" it will usually behave like an ordinary instance of Cataclysm:
create a world, create a character, load it, validate files and generate the map.

You can use standard command line options to customize where the data and user directories are,
which the editor uses for data loading and temporary file storage. There are also editor-exclusive
options that streamline startup sequence (automatic world creation, loading project), see `--help`.

Like with the game, installation is simple. Get the latest release archive from here:

https://github.com/olanti-p/cata-mapgen-editor/releases

unpack it into an empty folder and run the executable, and that should be it.

Ensure you are using Ultica or Chibi Ultica tileset, and no overmap tileset.
Other tilesets (particularly MSX/UDP family) may also work, but no guarantees.

It is advised to use windowed mode in case the tool crashes.
