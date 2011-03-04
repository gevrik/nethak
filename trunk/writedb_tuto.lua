Putting the data in a text file is a lot easier. You make a new table and put your values in and then use the string.Implode function. Like so:

function file.AppendLine(filename, addme)
	data = file.Read(filename)
	if ( data ) then
		file.Write(filename, data .. "\n" .. tostring(addme))
	else
		file.Write(filename, tostring(addme))
	end
end
 
 
print("Getting local player data ...")
ply = LocalPlayer()
print(ply)
 
values = { ply:GetName(), ply:GetName(), 0 } // create a new table with our required values
 
print("Building data string ... ")
data = string.Implode(";", values)
 
print("Data string: " .. data)
 
file.AppendLine("playerdata.txt", data)

Output:

Getting local player data ...
Player [1][Manadar]
Building data string ... 
Data string: STEAM_0:1:14884029;Manadar;0

and our playerdata.txt now contains:

STEAM_0:1:14884029;Manadar;60
STEAM_0:0:11956651;Overv;240
STEAM_0:1:14884029;Manadar;0