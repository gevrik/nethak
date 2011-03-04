flag = false

print("\n\n\nReading orgdata.txt ...")

textdata = file.Read("..\\clan\\"..org_name)

print(org_name.." is read. Data: \n" .. textdata)
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
	
	if stream contient Members then
		cut Members&nbsp&nbsp&nbsp
		if stream==0 --org abandonnee
			flag = true			
		end
	end
	
	--erase org file
    file.Delete("..\\clan\\"..org_name)
	check all planet
		replace org_name by Unknown
	erase org place
	update planet.lst
	
end
print("\n\n")