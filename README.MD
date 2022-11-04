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

## Minimum System Requirements
These are the minimum requirements the trunk version of Ranish is _confirmed_ to run on. It may or may not run on less capable hardware.

 * IBM-AT compatible PC
 * minimum 256K RAM
 * IDE compatible controller supporting INT10 BIOS functions
 * MS-DOS 3.3 compatible operating system
 * Color- or monochrome graphics adapter

## Building
While the source may compile with other tools, the Makefile is currently
targeted at the following tool chain:

 * Open Watcom 11.9
 * Netwide Assembler 2.15
 * Borland TASM 5.1 (MANAGER.ASM still has to be ported to NASM)

To build `part.exe`, simply invoke `wmake` in the root directory of the
repository.
