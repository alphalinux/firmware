Known issues with the UP1000 system:

1. Linux boot via floppy disk drive:
        When booting Linux via APB, it takes about 5 minutes for APB to load the kernel image from a floppy device. 		Booting from a hard disk takes a matter of seconds.

2. 2x AGP card:
        a. At the present time, 2x AGP driver for Linux is not available so 2x mode has not been tested.
        b. The following cards have been used in 1x AGP mode: cards with Permedia-2 controller: Fire
	GL1000, ELSA Synergy, E&S, Matrox Millenium G200 and Stealth 2000.
        c. 3Dfx Rage Pro Turbo card does not work because the option ROM on the AGP card is
	not properly emulated by Alpha BIOS.

3. Linux installation:
	a. To install Linux, please click the  "Utilities" menu in the Alpha BIOS setup mode and the 
	"Run Maintenance Program". And follow the Linux installation procedure that API provides. 
	b. Please do not use the "Install Linux" menu in the "Operating System" menu. 