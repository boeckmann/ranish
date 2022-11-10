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


## Limitations
As of now Ranish is not capable of accessing disks via LBA. Therefore the
maximum supported disk size is 8GB (the disk itself may be larger but only the
first 8GB can be accessed).

## Minimum System Requirements
These are the minimum requirements the trunk version of Ranish is _confirmed_
to run on. It may or may not run on less capable hardware.

 * IBM-AT compatible PC
 * minimum 256K RAM
 * IDE compatible controller supporting INT 13h BIOS functions 00h-0Ah
 * MS-DOS 3.3 compatible operating system
 * Color- or monochrome graphics adapter

## Building
While the source may compile with other tools, the Makefile is currently
targeted at the following tool chain:

 * Open Watcom 1.9
 * Netwide Assembler 2.15

To build `part.exe`, simply invoke `wmake` in the root directory of the
repository.
