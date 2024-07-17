
ooga booga

## TOC
- [What is ooga booga?](#what-is-ooga-booga)
	- [SIMPLICITY IS KING](#simplicity-is-king)
	- [The "Build System"](#the-build-system)
- [Course: From Scratch to Steam](#course-from-scratch-to-steam)
- [Quickstart](#quickstart)
- [Examples & Documentation](#examples--documentation)
- [Known bugs](#known-bugs)
- [Licensing](#licensing)

## What is ooga booga?

Ooga booga, often referred to as a *game engine* for simplicity, is more so designed to be a new C Standard, i.e. a new way to develop software from scratch in C. Other than `<math.h>` we don't include a single C std header, but are instead writing a better standard library heavily optimized for developing games. Except for some image & audio file decoding, Ooga booga does not rely on any other third party code.

### SIMPLICITY IS KING

Ooga booga is designed to keep things simple, and let you solve video game problems the simplest way possible.

Performing SIMPLE and TRIVIAL tasks should be ... SIMPLE.
If you want to draw a rectangle, there should be a single procedure to draw a rectangle.
If you want to play an audio clip, there should be a single procedure to play an audio clip.
Etc.
This is something OS & Graphics API's tend to be fascinatingly terrible at even for the most trivial of tasks.
Thankfully, this is a main problem which oogabooga seeks to solve with a thin-as-possible layer of abstraction over the 
If you need to do something more complicated, you should be able to focus on that problem alone.
We aim to give you tools at a low enough level where there are no constraints to how you can go about solving the problems which arise for your game.
If you wonder what any of the oogabooga procedures do, you can search for that symbol, go to the definition, and see & digest the exact implementation.
Almost all implementations are code written by us with this in mind (with the exception of 3 nothings stb headers).
	
	
### The "Build System"

Our build system is a build.c and a build.bat which invokes the clang compiler on build.c. That's it. And we highly discourage anyone from introducing unnecessary complexity like a third party build system (cmake, premake) or to use header files at all whatsoever.

This might sound like we are breaking some law, but we're not. We're using a compiler to compile a file which includes all the other files, it doesn't get simpler. We are NOT using third party software to run the same compiler to compile the same files over and over again and write it all to disk to then try and link it together. That's what we call silly business (and unreasonably slow compile times, without any real benefit).

Oogabooga is made to be used in Unity builds. The idea is that you only include oogabooga.c somewhere in your project, specify the entry (see build.c) and now it's a Oogabooga project. Oogabooga is meant to replace the C standard, so it is not tested with projects which include standard C headers, so that will probably cause issues.

## Course: From Scratch to Steam

This project was started to be used in a course detailing the full ride from starting out making a game to publishing it to Steam. If you're keen on going all-in on getting a small game published to steam within 2-3 months, then check it out for free in our [Skool Community](https://www.skool.com/game-dev).

## Quickstart
Currently, we only support Windows x64 systems.
1. Make sure Windows SDK is installed
2. Install clang, add to path
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

## Examples & Documentation

Documentation will come in the form of a lot of examples because that's the best way to learn and understand how everything works.

See [examples](oogabooga/examples). 

Simply add `#include "oogabooga/examples/some_example.c"` to build.c and compile & run to see the example code in action.

Other than examples, a great way to learn is to delve into the code of whatever module you're using. The codebase is written with this in mind.

## Known bugs
- Window positioning & sizing is fucky wucky
- Converting 24-bit audio files doesn't really work

## Licensing
By default, the repository has an educational license that makes the engine free to use for personal projects.

[Educational license terms](https://github.com/alpinestudios/oogabooga/blob/master/LICENSE.md)

When you're ready to take the next step and work on a commercial game, you can upgrade to the full commercial license.

Here are the benefits of obtaining the full license:
- Permanent Ownership: You completely own the source code for life.
- No Recurring Fees or Royalties: Just an affordable one-time payment.
- It qualifies you to enter the private Skool community, where there's daily calls with Randy & Charlie, to help speedrun your game's development

You can [contact us](https://randy.gg/contact) to find out more.