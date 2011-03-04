exp = {95 , 113 , 134 , 156 , 180 , 207 , 236 , 269 , 304 , 342 , 383 , 428 , 476 , 529 , 586 , 647 , 714 , 786 , 864 , 949 , 1040 , 1139 , 1246 , 1362 , 1488 , 1625 , 1774 , 1937 , 2115 , 2309 , 2523 , 2758 , 3017 , 3305 , 3625 , 3982 , 4383 , 4837 , 5355 , 5949 , 6640 , 7451 , 8421 , 9604 , 11084 , 13005 , 15636 , 19564 , 24791 , 31256} --made via a very strange function 

function levelup ()
local charinfo = mud.character_info ()
	if (charinfo.exp>=exp[actual_level]) then
		charinfo.level+=1
		return
		
	else
		return
	end -- endif
	
	
end --endlevelup


function gainxp (int gained_xp)
local charinfo = mud.character_info ()
	charinfo.exp+=gained_xp
	
	levelup()
	
end --endgainxp