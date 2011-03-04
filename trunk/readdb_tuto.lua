print("\n\n\nReading playerdata.txt ...")
 
textdata = file.Read("playerdata.txt")
print("playerdata.txt is read. Data: \n" .. textdata)
print("")
 
print("Seperating lines ...")
lines = string.Explode("\n", textdata)
print("Lines split. The lines are:")
PrintTable( lines )
 
// Now let's loop through all the lines so we can split those too
for i, line in ipairs(lines) do
	// line is the current line, and i is the current line number
 
	data = string.Explode(";", line)
 
	playersteam = data[1]
	playername = data[2]
	playertotaltime = data[3]
 
	print(playername .. " (" .. playersteam .. ") has played on this server " .. playertotaltime .. " seconds")
end
print("\n\n")