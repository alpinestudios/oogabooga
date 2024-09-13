# !!! THIS VERY EXPERIMENTAL !!!


This is an example how how we can set up a oogabooga project where some code can be recompiled while the engine is still running.

To try this: 
1. Copy the files in this directory into the root project directory
2. Compile with `hotload_build_all.bat`
3. Run `build/launcher.exe`
4. Modify `build_game.c`
5. Recompile the game code only with `hotload_build_game.bat`
6. Go back to the application and press 'R'