In this directory are API NetWorks provided kernels for the API NetWorks CS20.
These kernel were built from version 2.2.17 with an updated Symbios SCSI driver
to support the CS20's onboard SCSI3 controller, and an updated Intel EEPro100
driver to properly support the two onboard NICs.

cs20boot-02.iso
	-An ISO9660 CD-ROM image with a bootable kernel and ramdisk for
	 RedHat 7.0 and 6.2. To install using this, burn the image to a CD-ROM
	 as is. Insert the CD-ROM into the CS20 and boot with the following 
	 command:
		
		for RedHat 7.0
	 P00>>b dqb1 -fl 0  
		OR for 6.2
	 P00>>b dqb1 -fl 1	

	 this assumes dqb1 is the CD-ROM device, substitute the correct device
	 name if it differs.

	 If installation is to be done from CD-ROM then change the API CD-ROM
	 to the RedHat CD-ROM at the language selection screen. All of the files 
	 below have also been included on this CD-ROM.

api-cs20-rh62-install.bootp
	-bootp loadable kernel for installing RedHat 6.2. This image
	 contains both the kernel and the ramdisk.img. Note that network
	 installs do not work on 6.2 as the installer seems unable to
	 handle multiple network interfaces.

api-cs20-rh70-install.bootp
	-bootp loadable kernel for installing RedHat 7.0. This image 
	 contains both the kernel and ramdisk.img file.

cs20-smp-2.2.17.bootp           
	-SMP bootp kernel without the ramdisk. Can be used
	 for booting after installation.

cs20-uni-2.2.17.bootp           
	-bootp kernel without the ramdisk. Can be used
	 for booting after installation.

cs20-smp-2.2.17.gz
	-Multi-Processor kernel for the CS20.

cs20-uni-2.2.17.gz
	-Uni-Processor kernel for the CS20.

kernel-2.2.17-cs20.alpha.rpm
	-RPM of the CS20 kernel for RedHat 6.2 and 7.0.

kernel-cs20-boot-2.2.17-cs20.alpha.rpm
	-RedHat boot RPM for bootp server, installs both 6.2 
	 and 7.0 kernels into /tftpboot

kernel-cs20-install-2.2.17-cs20.alpha.rpm
	-RedHat boot RPM for bootp server, installs both 6.2
	 and 7.0 kernels into /tftpboot. Kernel files contain
	 kernel and ramdisk for the particular version. 

kernel-source-2.2.17-cs20.alpha.rpm
	-Kernel source for the CS20 kernel RPMs.

kernel-2.2.17-cs20.src.rpm
	-Source RPM for the above kernels

Any problems should be reported to either customer.support@api-networks.com, or
entered into the API NetWorks Inc. bug tracking database at
http://bugs.api-networks.com

