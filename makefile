VERSION = 0.1
RELEASE = 1
DEV = 1

CFLAGS    = -Wall -pedantic -Werror -DVERSION=$(VERSION) -DRELEASE=$(RELEASE) -DDEV=$(DEV)
LFLAGS    = `sdl-config --libs` -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
OBJS      = animation.o audio.o collisions.o draw.o entity.o font.o game.o graphics.o init.o input.o inventory.o
OBJS     += lift.o main.o map.o player.o resources.o weapons.o weather.o sprites.o bat.o properties.o custom_actions.o
OBJS     += apple.o item.o crate.o status.o enemies.o hud.o random.o decoration.o sounds.o
ED_OBJS   = animation.o audio.o collisions.o draw_editor.o entity.o font.o game.o graphics.o init.o input.o inventory.o
ED_OBJS  += lift.o main_editor.o map.o player.o resources.o weapons.o weather.o sprites.o bat.o properties.o custom_actions.o
ED_OBJS  += apple.o item.o crate.o status.o cursor.o enemies.o hud.o random.o decoration.o sounds.o
PROG      = edgar
ED_PROG   = mapeditor
CXX       = gcc

# top-level rule to create the program.
all: $(PROG) $(ED_PROG)

# compiling other source files.
%.o: src/%.c src/%.h src/defs.h src/structs.h
	$(CXX) $(CFLAGS) -c -s $<

# linking the program.
$(PROG): $(OBJS)
	$(CXX) $(OBJS) -o $(PROG) $(LFLAGS)
	
# linking the program.
$(ED_PROG): $(ED_OBJS)
	$(CXX) $(ED_OBJS) -o $(ED_PROG) $(LFLAGS)

# cleaning everything that can be automatically recreated with "make".
clean:
	rm $(PROG) $(ED_PROG) *.o
