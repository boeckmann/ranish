# Ranish Partition Manager
This is a fork of Ranish Partition Manager 2.37.11 developed until 1998
by Mikhail Ranish. Version 2.37.11 was the last version of the software with
source code distributed under public domain.

The public domain statement for the source code of RANISH is given in the
original README.TXT file. The README.TXT also states that the binaries are
freeware. Because they are not public domain I did not include the binaries
in the repository. I gave the original version git tag v2.37.11-original.

I plan to make some small modifications to the source code and have put it
under Creative Commons CC-0 1.0 license.

The the source in the trunk branch was converted to Watcom C and Netwide
Assember. The original version was targeted at Turbo C / Turbo Assembler.


## Large disk support
In contrast to version 2.37.99 the trunk version of Ranish contains
experimental support for LBA and disks larger than 8 GiB, which is enabled
by default. If you encounter issues please file a bug report.

To disable large disk support and revert to standard INT13 BIOS functions
run the partition manager with `-x` as an command line argument.

The compact and advanced boot managers are not yet adapted to use LBA.


## Minimum System Requirements
These are the minimum requirements for the trunk version of Ranish. It may
or may not run on less capable hardware.

 * IBM-AT compatible PC
 * minimum 512K RAM
 * IDE compatible controller supporting INT 13h BIOS functions 00h-0Ah
 * MS-DOS 3.3 compatible operating system
 * Color- or monochrome graphics adapter


## Installation
RANISH comes as a ZIP archive. You may extract it into a directory of your
choice.


## Making a rescue floppy disk
Before using the software it is HIGHLY recommended to create a bootable
rescue disk and copy RANISH along with partition layout information onto
it.

You may proceed as follows to perform these steps at the DOS / Win9x command
prompt.

Format the floppy disk in drive A: and install system files:

    format a: /u /s /V:PART_MANAGER

Navigate to the installation directory of RANISH and copy the program
along with its help file onto the floppy:

    copy part.exe a:\
    copy part.htm a:\

Save the partition information of your disks. If you have only one disk
only the first line is needed:

    part -d 1 -s a:\disk1.mbr
    part -d 2 -s a:\disk2.mbr
    part -d 3 -s a:\disk3.mbr
    part -d 4 -s a:\disk4.mbr

Alternatively, the last step may be performed in interactive mode.
Start `part.exe` without command line arguments and press `S` to save
the partition data onto your floppy disk. Make sure the correct 
hard disk is selected!

After saving the partition layout make sure you can boot from the
created floppy disk.


## Partition layout recovery
In case you messed up your partition layout you can restore it from the
saved data on the rescue disk.

If you installed the Advanced Boot Manager after making the backup you
have to first uninstall it in interactive mode. Press `U` (uninstall)
followed by `F2` (save).

To restore the partition data via the command line interface use the
following command (adjust disk number and file accordingly):

    part -d 1 -l a:\disk1.mbr

You may also restore the partition layout in interactive mode. Press
`L` to load the data, enter the file name and press `F2` to save
the changes to disk.


## Building
While the source may compile with other tools, the Makefile is currently
targeted at the following tool chain:

 * Open Watcom 1.9
 * Netwide Assembler 2.15

To build `part.exe`, simply invoke `wmake` in the root directory of the
repository if you downloaded the source repository. If you downloaded the
binary release you first have to unpack the `sources.zip` file.

NASM is expected to be found via PATH. If that is not the case
you have to adjust the makefile.

To build a ZIP archive of the binaries and source run `wmake ranish.zip`.
Info-ZIP is used, so it should be in the PATH.
