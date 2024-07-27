
ooga booga

## TOC
- [What is ooga booga?](#what-is-ooga-booga)
	- [A new C Standard](#a-new-c-standard)
	- [SIMPLICITY IS KING](#simplicity-is-king)
	- [The "Build System"](#the-build-system)
- [Course: From Scratch to Steam](#course-from-scratch-to-steam)
- [Quickstart](#quickstart)
- [Examples & Documentation](#examples--documentation)
- [Known bugs](#known-bugs)
- [Licensing](#licensing)
- [Contributions](#contributions)

## What is ooga booga?

Ooga booga, often referred to as a *game engine* for simplicity, is more so designed to be a new C Standard, i.e. a new way to develop software from scratch in C. Other than `<math.h>` we don't include a single C std header, but are instead writing a better standard library heavily optimized for developing games. Except for some image & audio file decoding, Ooga booga does not rely on any other third party code.

### A new C Standard

Let's face it. The C standard is terrible. Don't even get me started on `string.h`. To be fair, any mainstream language standard is terrible. 

So what if we could strip out the nonsense standard of C and slap on something that's specifically made for video games, prioritizing speed and *simplicity*?

That's exactly what oogabooga sets out to do.

### SIMPLICITY IS KING

Ooga booga is designed to keep things simple, and let you solve video game problems the simplest way possible.

What we mean by simple, is twofold:

1. <b>Simple to use</b><br>
	Performing SIMPLE and TRIVIAL tasks should be ... SIMPLE. If you want to draw a rectangle, there should be a single procedure to draw a rectangle. If you want to play an audio clip, there should be a single procedure to play an audio clip. Etc. This is something OS & Graphics API's tend to be fascinatingly terrible at even for the most trivial of tasks, and that is a big chunk of what we set out to solve.

2. <b>Simple to understand</b><br>
	When you need to do something more complicated, you need to understand the library you're working with. For some reason, it seems like it's a standard for libraries today to obscure the implementation details as much as possible spread out in layers and layers of procedure calls and abstractions. This is terrible.
	In Oogabooga, there is none of that. We WANT you to delve into our implementations and see exactly what we do. We do not hide ANYTHING from you. We do not impose RESTRICTIONS on how you solve problems. If you need to know what a procedure does, you search for the symbol and look at the implementation code. That's it.

	
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
	
	window.title = STR("Minimal Game Example");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

	while (!window.should_close) {
		reset_temporary_storage();
		
		os_update(); 
		gfx_update();
	}

	return 0;
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

You can obtain the full commercial license by being an active member of the community and making your first game.

[Learn more here](https://www.skool.com/game-dev)

## Contributions
- Open PR's with `dev` as the base branch
- Keep it simple, no multi-layer abstractions
- Keep the implementation code readable, comment confusing code
- If you're introducing a new file/module, document the API and how to use it at the top of the file
- Add tests in tests.c if it makes sense to test
- Run tests (#define RUN_TESTS 1) before submitting PR
- Don't submit PR's for:
	- the sake of submitting PR's
	- Small polishing/tweaks that doesn't really affect the people making games
- When you submit a PR, please answer these prompts (if you're submitting a bugfix then you can skip this):
	- What feature/bugfix does this PR implement?
	- Why do we need this?
	- Describe at least one specific and practical problem this solves for people developing a game
	- Does this add complexity/friction for people making games? If so, how do you justify that?