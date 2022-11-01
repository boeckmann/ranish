# Ranish Partition Manager

This is a fork of Ranish Partition Manager 2.37.11, developed until 1998 by
Mikhail Ranish. Version 2.37.11 was the last version of which the source code
is distributed under public domain. There are more capable binary versions of
RANISH, but these are not public domain.

The public domain statement for the source code of RANISH is given in the
original README.TXT file. I have put the source code in this repository under
Creative Commons CC-0 1.0 license.

## Binaries
The original binaries are not included in the repository. They are freeware
but not public domain. To provide binaries I rebuilt the original sources with
Borland C++ 3.1 and Borland TASM and released it as 2.37.99 with updated
contact information and URL references to this repository on codeberg. Beside
that it should hopefully be bug for bug compatible with the original version.

## Limitations
As of now RANISH can only handle partitions below the 8.4GB limit, because it
uses BIOS INT 13 function 2 for disk access.

## Port to Open Watcom C and NASM
I am in the process of porting RANISH to the Open Watcom C Compiler and
Netwide Assembler (NASM). This process is performed in the watcom branch.
