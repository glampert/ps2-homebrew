
# PlayStation 2 homebrew demos - Dungeon Game

----

Welcome to the coolest demo in this project, a full-scale dungeon crawling game!
This tiny game was inspired by classics like *Dungeon Siege* and *Diablo*. It features
third-person perspective camera, PS2 Controller input, melee combat between player and
enemies, particle effects, shadow-blobs and MD2 animations. No health items, but killing an enemy will
give you some HP back. Overall, the game is quite hard; this was not intended, it just happened `;)`.

## Tech:

This game, like the *framework* library and the other demos, is written in C++98. Code is very
straightforward but far from perfect. I've spent a lot of time just getting things to work on the PS2
without any aid of official documentation, so at the end little time was left for the actual
building of a game. Apart from this the game is playable, has no major bugs or crashes that I
know of and runs smoothly at 30fps on both the Console and Emulator. Two demo levels are provided plus an empty test level.

**How are the assets handled?** If you look into this directory, you should find a `raw_assets.zip`
file in there. This zip package has all the original MD2/OBJ models and PNG textures that were used
in this game demo. However, the game doesn't access this data directly. Instead, each of those assets was
converted to some format that could be built into the source code and compiled into the ELF executable.
If you check the subdirectories inside *dungeon_game* you should find several `.h` files that consist
of C-style arrays of bytes. Those are the game assets dumped to static C arrays so that they can be compiled
into the executable. So after compiling the game you end up with a single ELF file that is the whole
thing, no external dependencies. The executable is completely stand-alone and can be run on the Console
or Emulator. Why did I take all this trouble of embedding every asset? Because file IO did not work in
the Emulator. The Emulator is only capable of accessing files inside a CD/DVD or disk image, so my
homebrew apps can't open loose files in the local FS. Since I did 90% of my development and testing on the Emulator,
this was the only workaround I could think of. It is not very practical for anything much bigger than
this demo though. Compile times suffer with the processing of all that static data and the executable grows huge.
Not to mention that every asset chance involves a preprocessing step.

Geometry for the map tiles comes from OBJ models processed with a custom tool I call `obj2c` (https://github.com/glampert/obj2c).
A binary of this tool can be found inside `raw_assets.zip`. The rest of the data was dumped with the freely available
`bin2c` tool, which can be found [here](https://github.com/gwilymk/bin2c), but also ships with the PS2DEV SDK.

## Controls:

- Player movement controls:
 - **[UP]**
 - **[DOWN]**
 - **[RIGHT]**
 - **[LEFT]**
 - **[L_THUMBSTICK]**: Free movement in all directions.

- Camera rotation controls:
 - **[L1]**: Rotate camera to the left;
 - **[R1]**: Rotate camera to the right;
 - **[L2]**: Pitch the camera up;
 - **[R2]**: Pitch the camera down;
 - **[L3]**: Zoom in;
 - **[R3]**: Zoom out;
 - **[R_THUMBSTICK]**: Combines all the above.

- Miscellaneous:
 - **[SELECT]**: Toggles the developer console;
 - **[START]**: Toggles several debugging tools;
 - **[CROSS]**: Make the player attack;
 - **[TRIANGLE]**: Quit to main menu.

## In-game Screenshots:

![alt text](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen20.png "Our hero! [Yes, his face is green]")

![alt text](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen32.png "The Dungeons")

![alt text](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen38.png "Sneaky")

![alt text](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen40.png "Better luck next time!")

![alt text](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen7.png "Like a boss!")

----

More screen captures can be found inside the *screens* directory, at the root of this project.
I've also uploaded a few videos on YouTube showcasing the main game levels:

- [The Dungeons level](https://youtu.be/qrPz5AMEOUM)
- [The Graveyard level](https://youtu.be/pK5r_wBrzcM)
- [A simple test level](https://youtu.be/kM_C4iHzdNQ)

## Special Thanks and Credits:

- [Sitters Electronics](http://www.md2.sitters-electronics.nl/) for the awesome MD2 models and
textures I've used for the game entities, player and scene props.

- [OpenGameArt.org](http://opengameart.org) and the author of [this tileset](http://opengameart.org/content/3d-dungeon-tileset)
that served as walls and floor to my dungeons.

- Thanks to Sean Barrett for his [STBI image loading library](https://github.com/nothings/stb).

- And of course, huge thanks to the fellows that compiled the free PS2DEV SDK, without which
this project wouldn't be possible at all!

