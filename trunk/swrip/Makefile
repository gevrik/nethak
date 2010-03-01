CC      = g++
PROF    = 
NOCRYPT =
#Uncomment the next line if you want MCCP support
#DBUGFLG = -DMCCP
C_FLAGS = -g3 -Wall $(PROF) $(NOCRYPT) $(DBUGFLG)
L_FLAGS = $(PROF) -lcrypt -lz

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	  build.o clans.o comm.o comments.o const.o db.o fight.o \
	  handler.o hashstr.o id.o interp.o magic.o makeobjs.o \
	  misc.o mud_comm.o mud_prog.o krearena.o player.o requests.o \
	  reset.o save.o shops.o skills.o special.o tables.o track.o update.o \
	  space.o space2.o bounty.o swskills.o alias.o grub.o mapper.o templateparse.o trivia.o 

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c \
	  build.c clans.c comm.c comments.c const.c db.c fight.c \
	  handler.c hashstr.c id.c interp.c magic.c makeobjs.c \
	  misc.c mud_comm.c mud_prog.c krearena.c player.c requests.c \
	  reset.c save.c shops.c skills.c special.c tables.c track.c update.c \
	  space.c space2.c bounty.c swskills.c alias.c grub.c mapper.c templateparse.c trivia.c

H_FILES = mud.h bet.h

all:
#	co $(H_FILES)
	make swrip
#	rm -f $(H_FILES)
	mv swrip ../bin/swrip

swrip: $(O_FILES)
	rm -f swrip
	$(CC) $(L_FLAGS) -o swrip $(O_FILES) -lm
	chmod g+w swrip
	chmod g+w $(O_FILES)

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f $(O_FILES)
