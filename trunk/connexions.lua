os.setlocale ("", "time")

--------------------------------------
function display_logo ()
	mud.send_to_char ("    _   __               ______            __                \b\n")
	mud.send_to_char ("   / | / /___  ____     / ____/____  _____/ /____   _  __    \b\n")
        mud.send_to_char ("  /  |/ // _ \\/ __ \\   / /    / __ \\/ ___/ __/ _ \\ | |/_/    \b\n")
        mud.send_to_char (" / /|  //  __/ /_/ /  / /___ / /_/ / /  / /_/  __/_>  <      \b\n")
        mud.send_to_char ("/_/ |_/ \\___/\\____/   \\____/ \\____/_/   \\__/\\___//_/|_|      \b\n")
        mud.send_to_char ("                                                             \b\n")
        mud.send_to_char ("\b\n")
        mud.send_to_char ("		   _____            __                     \b\n")
        mud.send_to_char ("		  / ___/__  _______/ /____  ____ ___  _____\b\n")
        mud.send_to_char ("		  \\__ \\/ / / / ___/ __/ _ \\/ __ `__ \\/ ___/\b\n")
        mud.send_to_char ("		 ___/ / /_/ (__  ) /_/  __/ / / / / (__  ) \b\n")
        mud.send_to_char ("		/____/\\__, /____/\\__/\\___/_/ /_/ /_/____/  \b\n")
        mud.send_to_char ("			 /____/                                \b\n")
        mud.send_to_char ("			 \b\n")
end --end display logo
--------------------------------------


require "wait"
require "tprint"
require "serialize"
require "achievements"

function con_old_in ()

	local charinfo = mud.character_info ()

	--achievements.read_achiev_list()
	
	wait.make( function ()
	
	display_logo ()

	   mud.send_to_char ("Connection Established With NeoCortex Systems ...\b\n")
	
	   wait.wpause(1)
	
	   mud.send_to_char ("Please Wait ...\b\n")
       wait.wpause(1)
	   mud.send_to_char ("Welcome ...\b\n")
       wait.wpause(1)
	   mud.send_to_char ("Identification completed.\b\n")
	   mud.send_to_char ("Please Wait ...\b\n")
	   mud.send_to_char ("Creating Computer Personna ...\b\n")
       wait.wpause(1)
	   mud.send_to_char ("Personna Created Successfully.\b\n")
	   mud.send_to_char ("Loading ROM ...\b\n")
	   wait.wpause(1)
	   mud.send_to_char ("Rom Loaded Successfully.\b\n")
	   mud.send_to_char ("Loading RAM ...\b\n")
       wait.wpause(1)
	   mud.send_to_char ("RAM Loaded Successfully.\b\n")
	   mud.send_to_char ("Loading Constructs ...\b\n")
       wait.wpause(1)
	   mud.send_to_char ("Constructs Loaded Successfully.\b\n")
	   mud.send_to_char ("Loading User ...\b\n")
       wait.wpause(1)
	   mud.send_to_char ("#flash#\b\n")
       wait.wpause(1)
	 end)
    return
end --end con_old_in
  
function con_new_in() 

	wait.make( function ()
       mud.send_to_char ("You feel well, relaxed.\b\n")
       
       wait.wpause(0.5)
	
       mud.send_to_char ("You hear a distant countdown, while your nanobots are preparing the connection with the CyberSpace.\b\n")
       wait.wpause(1)
       mud.send_to_char ("3...\b\n")
       wait.wpause(1)
       mud.send_to_char ("2...\b\n")
       wait.wpause(1)
       mud.send_to_char ("1...\b\n")
       wait.wpause(1)

       display_logo ()

       mud.send_to_char ("Connection Established With NeoCortex Systems ...\b\n")
       mud.send_to_char ("Please Wait ...\b\n")
       wait.wpause(2)
       mud.send_to_char ("As your mind is transfered through a vortex of color into the \nprogram of your new computered avatar, you begin to discern the \nworld of CyberSpace.\b\n")
       wait.wpause(6)
       mud.send_to_char ("Your avatar is floating above a grid pulsating with dataflow, \nsurrounded by few computer personas watching the transfer.\b\n")
       wait.wpause(6)
       mud.send_to_char ("You gradually recover fully operating senses and watch the \nroof above you, of the color of a television tuned to a dead channel.\b\n")
       wait.wpause(6)
       mud.send_to_char ("Somebody is talking to you...\b\n")
       wait.wpause(1)
       mud.send_to_char ("[SAY] Somebody : 'Greetings, human.'\b\n")
       wait.wpause(4)
       mud.send_to_char ("A green flying octaedra appears in your field of vision\b\n")
       wait.wpause(1)
       mud.send_to_char ("[SAY] Octaedra : 'Shall we proceed ? '\b\n")
	wait.wpause(4)
       mud.send_to_char ("You look around you")

	mud.send_to_char ("[SAY] Octaedra : You just connected for the first time into the Netrunner Space")
	mud.send_to_char ("[SAY] Octaedra : Be welcome here, human")
	mud.send_to_char ("[SAY] Octaedra : I'm a Byte, I'm in charge of helping you to take control of your computed persona")
	mud.send_to_char ("[SAY] Byte : If you would be so nice to give me attention for a moment")
	mud.send_to_char ("[SAY] Byte : You can control your persona as a kind of great computer")
	mud.send_to_char ("[SAY] Byte : For instance, to see what is currently registered into your ROM")
	mud.send_to_char ("[SAY] Byte : You just have to type the command : INVENTORY")
	mud.send_to_char ("[SAY] Byte : But NeoCortex Systems have implemented a simplier alias for it : INV")
	mud.send_to_char ("[SAY] Byte : We will see together many commands you can use")
	mud.send_to_char ("[SAY] Byte : But first you need to know how defend your persona")
	mud.send_to_char ("[SAY] Byte : Please go through the door at the north (command : NORTH, alias : N)")
	mud.send_to_char ("[SAY] Byte : Have a good uptime in the Cyberspace, human. Share and enjoy !")

--wait north
--if not north : raler

--if north
--then


	end)
    return
end -- con_new_in
  
function con_death()
	wait.make( function ()
     mud.send_to_char ("  010001010111001001110010011011110111001000100000\b\n")
       mud.send_to_char ("001110100010000001001001011011100111001101110100\b\n")
       mud.send_to_char ("011000010110001001101001011011000110100101110100\b\n")
       mud.send_to_char ("Error : Instability Detected\b\n")
       wait.wpause(1)
       mud.send_to_char ("011110010010000001000100011001010111010001100101\b\n")
       mud.send_to_char ("011000110111010001100101011001000000110100001010\b\n")
       mud.send_to_char ("Unmounting Remote Filesystems ...\b\n")
       wait.wpause(1)
       mud.send_to_char ("000011010000101001010101011011100110110101101111\b\n")
       mud.send_to_char ("[WARN] System Integrity Compromised\b\n")
       wait.wpause(1)
       mud.send_to_char ("011101010110111001110100011010010110111001100111\b\n")
       mud.send_to_char ("001000000101001001100101011011010110111101110100\b\n")
       mud.send_to_char ("Beginning Dump of Memory ...\b\n")
       wait.wpause(1)
       mud.send_to_char ("011001010010000001000110011010010110110001100101\b\n")
       mud.send_to_char ("[WARN] Leak of Memory Detected. \b\n")
       wait.wpause(1)
       mud.send_to_char ("011100110111100101110011011101000110010101101101\b\n")
       mud.send_to_char ("011100110010000000101110001011100010111000001101\b\n")
       mud.send_to_char ("[WARN] Memory Crash Detected\b\n")
       wait.wpause(1)
       mud.send_to_char ("000010100101101101010111010000010101001001001110\b\n")
       mud.send_to_char ("010111010010000001010011011110010111001101110100\b\n")
       mud.send_to_char ("[WARN] Kernel Panic : 222\b\n")
       wait.wpause(1)
       mud.send_to_char ("011001010110110100100000010010010110111001110100\b\n")
       mud.send_to_char ("011001010110011101110010011010010111010001111001\b\n")
       mud.send_to_char ("001000000100001101101111011011010111000001110010\b\n")
       mud.send_to_char ("***STOP : 0x005A6B89 { 0x000000, 0xF73120AE, 0xC0000000}\b\n")
       mud.send_to_char ("***NEOCORT.SYS - Address FBE7676 base at FBEE5000, TimeStamp 3d6dd67c\b\n")
       wait.wpause(1)
       mud.send_to_char ("011011110110110101101001011100110110010101100100\b\n")
       mud.send_to_char ("000011010000101000001101000010100100001001100101\b\n")
       mud.send_to_char ("011001110110100101101110011011100110100101101110\b\n")
       mud.send_to_char ("The System Is Terminated ... NOW !!!\b\n")
       wait.wpause(1)
       mud.send_to_char ("010101000110100001100101001000000101001101111001\b\n")
       mud.send_to_char ("011100110111010001100101011011010010000001001001\b\n")
       mud.send_to_char ("011100110010000001010100011001010111001001101101\b\n")
       mud.send_to_char ("011010010110111001100001011101000110010101100100\b\n")
       mud.send_to_char ("001000000010111000101110001011100010000001001110\b\n")
       mud.send_to_char ("010011110101011100100000001000010010000100100001\b\n")
       wait.wpause(1)
       mud.send_to_char ("#flash#\b\n")
       wait.wpause(1)
       mud.send_to_char ("Connection Established With NeoCortex Systems ...\b\n")
       mud.send_to_char ("Please Wait ...\b\n")
       wait.wpause(1)

	display_logo ()

       mud.send_to_char ("Backup Procedure Processing ...\b\n")
       mud.send_to_char ("Loading... Please Wait ...\b\n")
       mud.send_to_char ("#flash#\b\n")
       wait.wpause(1)
       mud.send_to_char ("Dear User, Please Accept All NeoCortex Systems's Apologizes\b\n")
       mud.send_to_char ("It Seems That Your Last Session Terminated Abruptly For An Unknown Reason\b\n")
       mud.send_to_char ("Our Technical Team Is Investigating On This Problem\b\n")
       wait.wpause(1)
       mud.send_to_char ("What Could Be Restored From Your Last Session Is Now Loaded In Your New Persona\b\n")
       mud.send_to_char ("It Is Possible That Some Data Was Lost And Cannot Be Restored\b\n")
       mud.send_to_char ("Please Do Not Expose Your Computer Persona To Any Unreasonnable Stresses\b\n")
       wait.wpause(1)
       mud.send_to_char ("NeoCortex Systems Warranties Applie Only In Normal Case Uses Of The Persona\b\n")
	end)
  return
  end -- con_death
  
  function con_quit()
	wait.make( function ()
      mud.send_to_char ("Good bye runner\b\n")
		wait.wpause(1)
		mud.send_to_char ("You'll be back soon\b\n")
	end)
    return
end  -- con_quit



------------------------------------------------------------------------
