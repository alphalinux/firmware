Summary: The Linux kernel (customized for use on the CS20)
Name:      kernel
Version:   2.2.17
Release:   cs20
Copyright: GPL
Group: System Environment/Kernel
Source:    http://www.api-networks.com/products/downloads/customer_support/CS20/source/linux-%{version}-%{release}.tar.gz
BuildRoot: /var/tmp/linux-%{version}-%{release}
Packager:  Peter Petrakis <peter.petrakis@api-networks.com>

%description
The kernel package contains the Linux kernel (vmlinux), the core of your
operating system.  These sources also include support for the required hardware
in the CS20 to bootstrap itself ,various driver fixes, and configures
the CS20 for console operation over a serial terminal. Included are smp and uni
gzipped kernels.


%package cs20-install
Summary: uniprocessor BOOTP kernels with attached initrd images for installing RedHat-6.2 and 7.0. 
Group: System Environment/Kernel

%description cs20-install
Provides BOOTP images with attached initrd for installing RH-6.2 and
RH-7.0. Simply install the images to your BOOTP capable server
and call them by name from the SRM prompt through a networked
connected CS20. See the System Console Guide for detailed directions
on installing Linux on the CS20.

%package cs20-boot
Summary: uni and smp BOOTP kernels for use after installation or emergancy use.
Group: System Environment/Kernel

%description cs20-boot
Provides SMP and UNI processor BOOTP kernels for use to bootstrap an
existing installed disk that does not have the required kernel support
immediatly available. For example, You would use these BOOTP images
to mount your local disk after you have installed Linux. Install
these images to your BOOTP capabile server. See the System Console Guide 
for detailed directions on installing Linux on the CS20.


%package source
Summary: source code to the linux kernel and all patches used. 
Group: System Environment/Kernel 

%description source
The source code of the Linux kernel. Including all the patches provided
and applied by API NetWorks.


# Get down to business


%prep -n  linux-%{version}-%{release} 
%setup -n linux-%{version}-%{release}


%build

# Stupid RPM won't see functions unless they're defined in the same macro scope 
# they where defined in. This is going to be messy.


PrepFS ()
{
	rm -rf   $RPM_BUILD_ROOT/*
	mkdir -p $RPM_BUILD_ROOT/lib/modules
	mkdir    $RPM_BUILD_ROOT/boot
	mkdir    $RPM_BUILD_ROOT/tftpboot
	mkdir -p $RPM_BUILD_ROOT/usr/src
	mkdir -p $RPM_BUILD_ROOT/linux-%{version}-%{release}
}


BuildCS20InstallRamdisk ()

{
        cd /usr/src/redhat/BUILD/linux-%{version}-%{release}
        cp configs/cs20-uni-base.config .config
# make RH-6.2 BOOTP image
        cp -f Makefile.rh62 Makefile
        make oldconfig
        make dep
        make clean
# note to self. add code to detect number of CPU's and fork accordingly
        make boot
        make bootimage
        make bootpfile
        mv arch/alpha/boot/bootpfile arch/alpha/boot/cs20-rh62-install.bootp
# clean up time for next build
        make distclean

# make RH-7.0 BOOTP image
        cp configs/cs20-uni-base.config .config
        cp -f Makefile.rh70 Makefile
        make oldconfig
        make dep
        make clean
# note to self. add code to detect number of CPU's and fork accordingly                                  
        make boot
        make bootimage
        make bootpfile
        mv arch/alpha/boot/bootpfile arch/alpha/boot/cs20-rh70-install.bootp
# clean up time for next build
        make distclean
} 

BuildCS20BOOTP ()
{
        cd /usr/src/redhat/BUILD/linux-%{version}-%{release}
# build uniprocessor BOOTP kernel
        cp configs/cs20-uni-base.config .config
        cp -f Makefile.org Makefile
        make oldconfig
        make dep
        make clean
# note to self. add code to detect number of CPU's and fork accordingly
        make boot
        make bootimage
        make bootpfile
        mv arch/alpha/boot/bootpfile arch/alpha/boot/cs20-uni-2.2.17.bootp
# clean up time for next build
        make distclean

# build smp BOOTP kernel
        cp configs/cs20-smp-base.config .config
        cp -f Makefile.org Makefile

# add SMP to versioning
	sed s/CS20/CS20-smp/g Makefile > Makefile.tmp
	mv -f Makefile.tmp Makefile

        make oldconfig
        make dep
        make clean
# note to self. add code to detect number of CPU's and fork accordingly                                  
        make boot
        make bootimage
        make bootpfile
        mv arch/alpha/boot/bootpfile arch/alpha/boot/cs20-smp-2.2.17.bootp
# clean up time for next build
        make distclean
}

# Always run me LAST
BuildCS20VMLINUX ()
{
        cd /usr/src/redhat/BUILD/linux-%{version}-%{release}
# build uniprocessor GZ kernel
        cp configs/cs20-uni-base.config .config
        cp -f Makefile.org Makefile
        make oldconfig
        make dep
        make clean
# note to self. add code to detect number of CPU's and fork accordingly
        make boot
        make modules
        make INSTALL_MOD_PATH=$RPM_BUILD_ROOT modules_install
        mv arch/alpha/boot/vmlinux.gz arch/alpha/boot/cs20-uni-2.2.17.gz
# clean up time for next build
        make distclean

# build smp BOOTP kernel
        cp configs/cs20-smp-base.config .config
        cp -f Makefile.org Makefile

# add SMP to versioning
        sed s/CS20/CS20-smp/g Makefile > Makefile.tmp
        mv -f Makefile.tmp Makefile

        make oldconfig
        make dep
        make clean
# note to self. add code to detect number of CPU's and fork accordingly                                  
        make boot
        make modules
        make INSTALL_MOD_PATH=$RPM_BUILD_ROOT modules_install
        mv arch/alpha/boot/vmlinux.gz  arch/alpha/boot/cs20-smp-2.2.17.gz

# before we nuke everything, lets save the System map.
	mv System.map System.map.save

# clean up time for next build
        make distclean
}

# begin build process

if [ -x /usr/bin/kgcc ]; then
    KERNELCC=kgcc
else
    KERNELCC=gcc
fi

PrepFS
BuildCS20InstallRamdisk
BuildCS20BOOTP
BuildCS20VMLINUX

%install







RecordSources ()
{
        # might as well do this before I build anything. Use sed/awk magic
        # to record every file in the source tree and generate a filelist 
        # which I can later reference in the "files" section. 
        # Blatantly ripped off from the KDE guys :-) They write nice specs.


cd $RPM_BUILD_ROOT/usr
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > \
        $RPM_BUILD_ROOT/file.list.%{name}

find . -type f | sed -e 's,^\.,\%attr(-\,root\,root) ,' \
        -e '/\/config\//s|^|%config|' >> \
        $RPM_BUILD_ROOT/file.list.%{name}

find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> \
        $RPM_BUILD_ROOT/file.list.%{name}

cat $RPM_BUILD_ROOT/file.list.%{name} | sed s,'src/','usr/src/',g > $RPM_BUILD_ROOT/file.list.%{name}.tmp

rm -f $RPM_BUILD_ROOT/file.list.%{name}

mv $RPM_BUILD_ROOT/file.list.%{name}.tmp $RPM_BUILD_ROOT/file.list.%{name}

}

RecordSourcesModules ()
{
        # might as well do this before I build anything. Use sed/awk magic
        # to record every file in the source tree and generate a filelist 
        # which I can later reference in the "files" section. 
        # Blatantly ripped off from the KDE guys :-) They write nice specs.


cd $RPM_BUILD_ROOT/lib
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > \
        $RPM_BUILD_ROOT/file.list.%{name}-modules

find . -type f | sed -e 's,^\.,\%attr(-\,root\,root) ,' \
        -e '/\/config\//s|^|%config|' >> \
        $RPM_BUILD_ROOT/file.list.%{name}-modules

find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> \
        $RPM_BUILD_ROOT/file.list.%{name}-modules

cat $RPM_BUILD_ROOT/file.list.%{name}-modules | sed s,'modules/','lib/modules/',g > $RPM_BUILD_ROOT/file.list.%{name}-modules.tmp

rm -f $RPM_BUILD_ROOT/file.list.%{name}-modules

mv $RPM_BUILD_ROOT/file.list.%{name}-modules.tmp $RPM_BUILD_ROOT/file.list.%{name}-modules

}

# install BOOTP/RAMDISK images to /tftpboot

install -m 755 arch/alpha/boot/cs20-rh62-install.bootp $RPM_BUILD_ROOT/tftpboot/cs20-rh62-install.bootp
install -m 755 arch/alpha/boot/cs20-rh70-install.bootp $RPM_BUILD_ROOT/tftpboot/cs20-rh70-install.bootp

# install plain BOOTP files to /tftpboot
install -m 755 arch/alpha/boot/cs20-uni-2.2.17.bootp $RPM_BUILD_ROOT/tftpboot/cs20-uni-2.2.17.bootp
install -m 755 arch/alpha/boot/cs20-smp-2.2.17.bootp $RPM_BUILD_ROOT/tftpboot/cs20-smp-2.2.17.bootp

# install gzipped kernels and System map to /boot

install -m 755 arch/alpha/boot/cs20-uni-2.2.17.gz $RPM_BUILD_ROOT/boot/cs20-uni-2.2.17.gz
install -m 755 arch/alpha/boot/cs20-smp-2.2.17.gz $RPM_BUILD_ROOT/boot/cs20-smp-2.2.17.gz
install -m 755 System.map.save $RPM_BUILD_ROOT/boot/System-%{version}-%{release}.map

# fix the build symlink in /lib/modules

rm -f $RPM_BUILD_ROOT/lib/modules/2.2.17-CS20/build
rm -f $RPM_BUILD_ROOT/lib/modules/2.2.17-CS20-smp/build

cd $RPM_BUILD_ROOT/lib/modules/2.2.17-CS20/
ln -s /usr/src/linux-2.2.17-cs20 build

cd $RPM_BUILD_ROOT/lib/modules/2.2.17-CS20-smp/
ln -s /usr/src/linux-2.2.17-cs20 build

# Get the file list for installed modules
RecordSourcesModules

# install source code

tar -xzf $RPM_SOURCE_DIR/linux-%{version}-%{release}.tar.gz -C $RPM_BUILD_ROOT/usr/src
RecordSources


%post 
# base package. configure serial console and secure tty's

#configure /etc/securetty

grep ttyS0 /etc/securetty >/dev/null 2>&1
if [ $? = 0 ]
then
        echo "skipping securetty scripts"
else
        echo "modifing securetty"
        echo ttyS0 >> /etc/securetty
        echo ttyS1 >> /etc/securetty
fi

# check inittab


grep ttyS0 /etc/inittab >/dev/null 2>&1
if [ $? = 1 ]
then
        echo "modifing inittab"
        rm -f /tmp/inittab.wrk
        sed s/[1-6]:2345/'#&'/g /etc/inittab > /tmp/inittab.wrk
        mv -f /etc/inittab /etc/inittab.old
        mv -f /tmp/inittab.wrk /etc/inittab

        # add getty for serial ports
        echo "S0:2345:respawn:/sbin/mingetty ttyS0" >> /etc/inittab
        echo "S1:2345:respawn:/sbin/mingetty ttyS1" >> /etc/inittab
else
        echo "inittab all set"
fi

# check aboot

grep uni /etc/aboot.conf

if [ $? = 0 ]
then
        echo "skipping aboot mods"
else
        echo "applying aboot mods"

        #RH-6.2 aboot config
        sed s/vmlinuz-2.2.14-6.0smp/'cs20-smp-2.2.17.gz console=ttyS0'/ /etc/aboot.conf | sed s/vmlinuz-2.2.14-6.0/'cs20-uni-2.2.17.gz console=ttyS0'/ > /tmp/aboot.conf

        mv -f /boot/etc/aboot.conf /boot/etc/aboot.config.save
        mv -f /tmp/aboot.conf /boot/etc/aboot.conf

        #RH-7.0 aboot config

        sed s/vmlinuz-2.2.17-4smp/'cs20-smp-2.2.17.gz console=ttyS0'/ /etc/aboot.conf | sed s/vmlinuz-2.2.17-4/'cs20-uni-2.2.17.gz console=ttyS0'/ > /tmp/aboot.conf

        mv -f /boot/etc/aboot.conf /boot/etc/aboot.config.save
        mv -f /tmp/aboot.conf /boot/etc/aboot.conf
fi



# setup the system map

rm -f /boot/System.map
ln -s /boot/System-%{version}-%{release}.map /boot/System.map

%post source

AdjustMakefileVersion ()
{
#!/bin/sh
export PATH=$PATH:/usr/bin:/bin:/sbin:/usr/sbin

grep "cpus" /proc/cpuinfo | awk '{
   if ( $4 != 1 ) {
      if (system ("sed s/CS20/CS20-smp/g /usr/src/linux/Makefile > /tmp/mf.tmp") != 0)
        print "uni-processor system"
      else
        print "smp system. adjusting /usr/src/linux/Makefile"
      system ("mv -f /tmp/mf.tmp /usr/src/linux/Makefile")
   }
   else
      print "uniprocessor system, no changes needed"
}'

}


#setup linux symlink

cd /usr/src/
mv -f linux linux.old
ln -s linux-2.2.17-cs20 linux

# are you SMP? 
AdjustMakefileVersion


%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{buildroot}/file.list.%{name}-modules
/boot/cs20-uni-2.2.17.gz
/boot/cs20-smp-2.2.17.gz
/boot/System-%{version}-%{release}.map


%files cs20-install
/tftpboot/cs20-rh62-install.bootp
/tftpboot/cs20-rh70-install.bootp


%files cs20-boot
/tftpboot/cs20-uni-2.2.17.bootp
/tftpboot/cs20-smp-2.2.17.bootp


%files source -f %{buildroot}/file.list.%{name}

%changelog
* Mon Mar 12 2001 Peter Petrakis <peter.petrakis@api-networks.com>
- removed awk from logic structures and used bash instead. scripts
- evaluate system config before applying mods.

* Thu Mar 08 2001 Peter Petrakis <peter.petrakis@api-networks.com>
- fixed system.map symlink
- added function that detects how many cpus present and corrects 
- the installed makefile weither to mark it as SMP or not.
- created backup files of all files modified 

* Wed Mar 07 2001 Peter Petrakis <peter.petrakis@api-networks.com>
- fixed a symlink problem in /lib/modules
- fixed linux symlink in /usr/src

* Thu Mar 01 2001 Peter Petrakis <peter.petrakis@api-networks.com>
- initial spec creation. it works but could be more elegent.

