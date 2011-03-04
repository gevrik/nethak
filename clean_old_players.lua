flag = false

print("\n\n\nReading playerdata.txt ...")

textdata = file.Read("..\\player\\"..firstletter.."\\"..playername)

print(playername.." is read. Data: \n" .. textdata)
print("")
 
print("Seperating lines ...")
lines = string.Explode("\n", textdata)
print("Lines split. The lines are:")
PrintTable( lines )
 
// Now let's loop through all the lines so we can split those too
for i, line in ipairs(lines) do
	// line is the current line, and i is the current line number
 
	data = string.Explode(";", line)
	stream = data[1]
	
	if string.find(stream,"Logouttime") then
		cut Logouttime&nbsp&nbsp&nbsp
		if stream>=actual timestamp-3mois --vieille connection
			flag true			
		end
	end
	
	if string.find(stream,"Clan") then --appartient a une org
		
		cut Clan&nbsp&nbsp&nbsp
		keep name of org
		
		--remove player file
		file.Delete("..\\player\\"..firstletter.."\\"..playername)
		
		--appeler C void do_resign( CHAR_DATA *ch, char *argument )

			remove leader
	end
end
print("\n\n")