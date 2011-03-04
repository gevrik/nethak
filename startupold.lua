os.setlocale ("", "time")

-- player-specific data - this will be saved to state file

current_quests = {}
completed_quests = {}


-- helper functions

-- send to player - with newline
function send (...)
  mud.send_to_char (table.concat {...} .. "\n\r")
end -- send

-- formatted send to player - with newline
function fsend (s, ...)
  mud.send_to_char (string.format (s, ...) .. "\n\r")
end -- send

-- send to player - without newline
function send_nocr (...)
  mud.send_to_char (table.concat {...})
end -- send_nocr

require "tprint"
require "serialize"

systeminfo = mud.system_info ()  -- get file names, directories

-- work out file name of player state file
function get_file_name ()
local charinfo = mud.character_info ()
  return systeminfo.PLAYER_DIR .. 
        string.lower (string.sub (charinfo.name, 1, 1)) .. 
        "/" ..
        charinfo.name ..
        ".lua"
end -- get_file_name

-- player has entered game 
function entered_game (name)
end -- entered_game

-- player has entered room 'vnum'
function entered_room (vnum)
end -- entered_room

-- player has killed mob 'vnum'
function killed_mob (vnum)
end -- killed_mob

-- player has received object 'vnum'
function got_object (vnum)
end -- got_object

-- player is looking around
function looking (arg)
end -- looking

-- player has entered game 
function reconnected (name)
   dofile (get_file_name ())  -- load player state
end -- reconnected

-- new player has joined game
function new_player (name)
end -- new_player

-- periodic update (each minute)
function char_update ()
end -- char_update

-- player is being saved
function saving ()
local charinfo = mud.character_info ()
local fname = get_file_name ()

   local f = assert (io.open (fname, "w"))
   f:write (os.date ("-- Saved at: %c\n\n"))
   f:write (string.format ("-- Extra save file for %s\n\n", charinfo.name))
   
   f:write ((serialize.save ("current_quests")), "\n")
   f:write ((serialize.save ("completed_quests")), "\n")
   f:close ()

   fsend ("*** Saved into file %s", get_file_name ())
end -- saving
