Firmware updates from the SRM console now require two floppies.


To "disk1" copy the following files:

	pc264fw.txt
	pc264nt.sys
	pc264srm.sys



To do the update from the SRM console:

 - Insert disk1 into the floppy drive on the ds20(e), type "lfu" at the 
   console. 

 - The update utility will run and say that files were not found on CD or 
   floppy, but will then ask on which device the files are located.

 - Type "dva0"

 - The LFU will then ask for the name of the firmware files list. 

 - Type "pc264fw.txt"

 - The LFU will then report as it copies each file. When the files on disk1 are
   copied, the LFU will ask for the next floppy.

 - type "done"

 - When you get to the UPD> prompt, perform updates as normal. It would be a 
   good idea to do a "list" command to see that the images succesfully copied
   and are listed with the correct revision.

