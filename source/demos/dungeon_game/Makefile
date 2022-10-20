
# ---------------------------------------------------------

ifndef SOURCE_PATH
SOURCE_PATH = /Users/Lampert/repos/ps2dev-tests/source
endif

ifndef INSTALL_PATH
INSTALL_PATH = /Applications/pcsx2.app/Contents/Resources/pcsx2
endif

ifndef RUN_CMD
RUN_CMD = open -a pcsx2
endif

ifndef EE_BIN
EE_BIN = dungeon_game.elf
endif

# ---------------------------------------------------------

EE_OBJS =                                             \
	$(SOURCE_PATH)/framework/ps2_math/math_funcs.o    \
	$(SOURCE_PATH)/framework/memory.o                 \
	$(SOURCE_PATH)/framework/common.o                 \
	$(SOURCE_PATH)/framework/texture.o                \
	$(SOURCE_PATH)/framework/renderer.o               \
	$(SOURCE_PATH)/framework/sound.o                  \
	$(SOURCE_PATH)/framework/ingame_console.o         \
	$(SOURCE_PATH)/framework/particle_emitter.o       \
	$(SOURCE_PATH)/framework/md2_model.o              \
	$(SOURCE_PATH)/framework/game_pad.o               \
	$(SOURCE_PATH)/framework/game_time.o              \
	$(SOURCE_PATH)/framework/third_person_camera.o    \
	$(SOURCE_PATH)/framework/first_person_camera.o    \
	$(SOURCE_PATH)/demos/dungeon_game/audsrv_irx.o    \
	$(SOURCE_PATH)/demos/dungeon_game/tile_map.o      \
	$(SOURCE_PATH)/demos/dungeon_game/render_entity.o \
	$(SOURCE_PATH)/demos/dungeon_game/game_entity.o   \
	$(SOURCE_PATH)/demos/dungeon_game/game_world.o    \
	$(SOURCE_PATH)/demos/dungeon_game/game_main.o

# ---------------------------------------------------------

EE_LIBS =     \
	-lc       \
	-ldma     \
	-lgraph   \
	-ldraw    \
	-lpatches \
	-lmf      \
	-laudsrv

# ---------------------------------------------------------

GLOBAL_DEFINES =                      \
	-I$(SOURCE_PATH)                  \
	-Dnullptr=NULL                    \
	-DLOG_PRINTF_ADD_MESSAGE_PREFIX=1 \
	-DUSE_CUSTOM_ASSERT=1

#
# NOTE:
#  Defining `DISABLE_ASSERTIONS` will turn every assertion
#  used in the game and framework into a no-op. This is unlikely
#  to have any measurable performance impact, but could be
#  done if you wish to squeeze every cycle out of the machine...
#

# ---------------------------------------------------------

EE_CXXFLAGS +=                \
	-O3                       \
	-funroll-loops            \
	-fexpensive-optimizations \
	-fno-exceptions           \
	-fno-rtti                 \
	-Wall                     \
	-Wformat=2                \
	-Wmissing-braces          \
	-Wparentheses             \
	-Wpointer-arith           \
	-Wreturn-type             \
	-Wsequence-point          \
	-Wswitch                  \
	-Wuninitialized           \
	-Wunknown-pragmas         \
	-Wwrite-strings           \
	$(GLOBAL_DEFINES)

# ---------------------------------------------------------

# We build `audsrv.irx` into the application
# as a raw binary dump of the module, inside an ASM file.
#
# This dependency rule will generate `audsrv_irx.s` using `bin2s`
# from the pre-built module provided by the PS2DEV SDK.
#
.SUFFIXES: .s
%.s::
	bin2s $(PS2SDK)/iop/irx/audsrv.irx audsrv_irx.s audsrv_irx

# ---------------------------------------------------------

all: $(EE_BIN)
	ee-strip --strip-all $(EE_BIN)

# ---------------------------------------------------------

clean:
	rm -f *.elf *.o *.a
	rm -f $(INSTALL_PATH)/$(EE_BIN)
	cd $(SOURCE_PATH)/framework               && rm -f *.o *.a
	cd $(SOURCE_PATH)/framework/builtin_fonts && rm -f *.o *.a
	cd $(SOURCE_PATH)/framework/ps2_math      && rm -f *.o *.a
	cd $(SOURCE_PATH)/framework/third_party   && rm -f *.o *.a
	cd $(SOURCE_PATH)/demos/dungeon_game

# ---------------------------------------------------------

run: $(EE_BIN)
	cp $(EE_BIN) $(INSTALL_PATH)/$(EE_BIN)
	$(RUN_CMD)

# ---------------------------------------------------------

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

# ---------------------------------------------------------

