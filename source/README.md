
# PlayStation 2 homebrew demos - Source code

----

This directory is subdivided into two sub-projects:

- __framework/*__
- __demos/*__

----

The __*framework*__ directory contains a set of reusable classes and miscellaneous code
used by all of the demo applications. This is a tiny helper library that abstracts the
lower-level aspects of PlayStation 2 development and provides a higher level and friendlier
C++ interface to the hardware and the [PS2DEV C libraries](https://github.com/ps2dev/ps2sdk).

There is no makefile for the *framework*. Each executable built against it must add
all the relevant files to its own project makefile. This facilitates the selection
of required modules, making it easy to discard unneeded stuff.

The *framework* is written in C++98 flavor, imposed by the compiler (GCCv3).
Very vanilla C-with-classes style, still common in the games industry to this date.
The only external dependency is the PS2DEV unofficial SDK and the minimal C runtime
that comes with it. If you have the SDK installed properly this should compile
without errors or warnings. If your are interested on knowing how to install
the PS2DEV SDK on a Mac OSX machine, I've written a [blog post about it](http://glampert.com/2015/02-27/ps2-homebrew-setting-up-the-environment/).

----

The __*demos*__ directory is subdivided into one directory for each demo application.
The *dungeon_game* is the most interesting one, being a rudimentary but complete
third-person dungeon crawling game. Check the README inside its directory for details.
The other demos are simple tests for specific features of the *framework* library or
the hardware.

- __console/*__: Tiny test for the in-game developer console and debugging tools
provided by my framework. Also tests the PS2 controller if one is connected.

- __cubes/*__: A few spinning cubes. This demos tests 3D rendering,
camera, game controller input and texture mapping.

- __play_adpcm/*__: Tests the Sound Processing Unit and AUDSRV library by playing a
looping sound. Prints some debug info to the screen.

- __dungeon_game/*__: A simple third person dungeon crawling action game.
See the README inside for further details.

- __bin/*__: Pre-built ELF binaries for the demos above. Ready to be executed on the Emulator or the Console.
For details on how to run homebrew software on a PS2, see the [uLaunchELF](http://ps2ulaunchelf.pbworks.com/w/page/19520134/FrontPage) Wiki.

----

**In-source build:**

Unfortunately the demos will be built "in-source", meaning that the makefile for
each project can be found residing inside each directory together with the source code.
`make`ing each demo will output the intermediate object files and the ELF binary directly
inside the current dir, mixing all the generate stuff with the source files. Some people
find this annoying, myself included, however I could not provide a simple fix for this problem
because the makefile of each project depends on an external makefile provided by the PS2DEV SDK.
That plus my short knowledge of makefiles has resulted in this suboptimal setup. This is a future FIXME.

----

**`.h/.hpp` file convention:**

This is *almost* a convention in this project: Files with the `.hpp` extension are human-generate
C++ header files while file with the `.h` extension are (almost always) machine-generated raw textual includes.
This project makes heavy use of built-in assets for things like fonts and bitmaps, dumped as C-style arrays
of bytes and then embedded into the source code. For such files, the `.h` extension is used to differentiate
them from programmer-written C++ includes. This is an inconsistent notation though, some `.h` files in this project
are in fact programmer-written C header files. More of a hint rather than a rule.

