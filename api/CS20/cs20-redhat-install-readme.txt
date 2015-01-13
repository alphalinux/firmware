Steps to install Redhat 6.2 or 7.0 into a CS20




1) Create a CS20 Boot CD from the ISO located at

http://www.apinetworks.com/products/downloads/customer_support/CS20/Kernels/cs20boot-02.iso 


2) Put the Cs20 boot cd in CS20

P00>Show dev    ! To find out drive names

3) b CDROM-drive (b Dqb0)  ! To boot and begin REDHAT installation

4) Swap in REDHAT CD's when prompted	


5) At completion on installation - halt system


6) Show dev	! To refresh your memory on the drive names

7) remove redhat CD and replace with CS20 Boot CD

P00>b harddrive -fl i (b dka0 -fl i)
aboot>l		! To verify root directory, if aboot does not come up; BSD label is missing
		! Erase partition information and reinstall redhat. (subnote 2)
write down partion info (root=/dev/sda2)
ctrl p

P00>> b dqb1 -fi kernels/cs20-uni-2.2.17.gz -fl "root=/dev/sda2 console=ttyS0 single" 
(boot kernel off cd, into disk, set console to serial port single user)

8) System will boot now to Bash

Sh-2.04# mount /dev/cdrom ! Mount the CS20 Boot disk

cd mnt/cdrom/RPMS	  ! cd to location of RPM

rpm -Fvh kernel-2.2.17-cs20.alpha.rpm ! Does all the fixes and setups

shutdown -h now

remove cd

9) boot harddrive (b dka0 -fl 0) ! Final boot up should boot to login prompt

Localhost Login:

(Note: If this boot stalls at "Setting up Dummy interface" do not be alarmed.
 This is a Redhat issue with the installer. The network dummy interface will timeout
 and finish load.)

Subnote 2

If you need to erase the disk partition information to successfully recreate a BSD label for the harddrive, then...

1) P00>> b dqb0 -fi kernels/cs20-uni-2.2.17.gz -fl "root=/dev/sda2 console=ttyS0 single" 

2) Sh-2.04# dd if=/dev/zero of=/dev/sda bs=512 count=4

3) halt system and start at top of page



