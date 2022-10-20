
# PlayStation 2 homebrew demos

----

## About:

This repository contains a number of homebrew PS2 demos and simple games that I've
developed using the freely available [PS2DEV](https://github.com/ps2dev) unofficial SDK.

I have written a few blog posts about this endeavor, you can read them [here](http://glampert.com/2015/04-24/ps2-homebrew-a-dungeon-game/).

## Directory Structure:

    +-ps2dev-tests/
     |
     +-source/     => Soure code for the PS2 demos.
      |
      +--demos/    => Source code and assets for the individual demo applications.
      |  |
      |  +-bin/    => Pre-built ELF binaries ready for use in the PS2 or Emulator.
      |
     ++-framework/ => A set of reusable classes and code shared by all demos.
     |
     +-screens/    => Screenshots of some of the demos for eye candy.
     |
     +-extras/     => Zip packages with a Mac OSX compatible version of the PCSX2 Emulator and a tuned PS2DEV_SDK that builds and install on OSX.

----

Inside each main directory you can find a more detailed *README* file with information
on how to build and run each of the demos plus any other relevant info about the source code.

## Special Thanks:

A big thanks to the guys of [Sitters Electronics](http://www.md2.sitters-electronics.nl/) for the awesome
collection of freely available MD2 models that I have used for the "Dungeons" game demo.

Big thanks to Dr. Fortuna and his [website on PS2 dev](http://www.hsfortuna.pwp.blueyonder.co.uk/).

Thanks to [Lukasz.dk](http://lukasz.dk/playstation-2-programming/an-introduction-to-ps2dev/)
and the whole PS2DEV community for making it possible for us mere mortals without an official devkit
to develop PS2 Console Games.

## License:

This project's source code is released under the [MIT License](http://opensource.org/licenses/MIT).

## Screenshots:

![Rotating cubes](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen42.png "Rotating cubes demo")

![The Graveyard](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen13.png "The Graveyard - Dungeon Game demo")

![Boss with minions](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen15.png "Boss with minions - Dungeon Game demo")

![The Dungeons](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen39.png "The Dungeons - Dungeon Game demo")

![Debug view](https://bytebucket.org/glampert/ps2dev-tests/raw/f3a137f9030c823d554080ec8bd97b262904bdb9/screens/screen22.png "Debug view - Dungeon Game demo")

----

More screenshot are available inside the *screens* directory.
I've also uploaded a few videos on YouTube showcasing the "Dungeons" demo game:

- [The Dungeons level](https://youtu.be/qrPz5AMEOUM)
- [The Graveyard level](https://youtu.be/pK5r_wBrzcM)
- [A simple test level](https://youtu.be/kM_C4iHzdNQ)

