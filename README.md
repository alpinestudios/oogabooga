ooga booga

## TOC
- [Getting started](#getting-started)
- [What is ooga booga?](#what-is-ooga-booga)
- [Quickstart](#quickstart)
- [The "Build System"](#the-build-system)
- [Examples & Documentation](#examples--documentation)
- [Known bugs](#known-bugs)
- [Licensing](#licensing)

## Getting started
If you'd like to learn how to use the engine to build a game, there's a completely free course in the [Skool community](https://www.skool.com/game-dev)

You can find all tutorials and resources for getting started within the community.

## What is ooga booga?

Ooga booga, often referred to as a *game engine* for simplicity, is more so designed to be a new C Standard, i.e. a new way to develop software from scratch in C. Other than `<math.h>` we don't include a single C std header, but are instead writing a better standard library heavily optimized for developing games. Except for some image & audio file decoding, Ooga booga does not rely on any other third party code.

## Quickstart
1. Install clang, add to path
2. Clone repo to <project_dir>
3. Make a file my_file.c in <project_dir>
```
int entry(int argc, char **argv) {
	print("Ooga, booga!\n");
}
```
4. in build.c add this line to the bottom
```
#include "my_file.c"
```
5. Run `build.bat`
6. Run build/cgame.exe
7. profit

## The "Build System"

Our build system is a build.c and a build.bat which invokes the clang compiler on build.c. That's it. And we highly discourage anyone from introducing unnecessary complexity like a third party build system (cmake, premake) or to use header files at all whatsoever.

This might sound like we are breaking some law, but we're not. We're using a compiler to compile a file which includes all the other files, it doesn't get simpler. We are NOT using third party software to run the same compiler to compile the same files over and over again and write it all to disk to then try and link it together. That's what we call silly business (and unreasonably slow compile times, without any real benefit).
## Examples & Documentation

Documentation will come in the form of a lot of examples because that's the best way to learn and understand how everything works.

See [examples](oogabooga/examples). 

Simply add `#include "oogabooga/examples/some_example.c"` to build.c and compile & run to see the example code in action.

Other than examples, a great way to learn is to delve into the code of whatever module you're using. The codebase is written with this in mind.

## Known bugs
- Window positioning & sizing is fucky wucky

## Licensing
By default, the repository has an educational license that makes the engine free to use for personal projects.

[Educational license terms](https://github.com/alpinestudios/oogabooga/blob/master/LICENSE.md)

You can obtain the full commercial license by being an active member of the community and making your first game.

[Learn more here](https://www.skool.com/game-dev)
