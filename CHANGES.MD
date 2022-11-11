# Change Log

## Version 2.90.00
### Fixes
 * Do not overwrite the last 16 bytes of IPL code with old signature.

### Changes
 * Use checksum to verify ADV boot sector code integrity.
 * Decrease bad block tracking buffer to 128 entries to reduce memory consumption.
 * Change save menu entry to make it clear that ADV data is saved in advanced mode and NOT the MBR itself.
 
 * This is the first version which completely builds with open source tools.
 * Ported to Open Watcom C and Netwide Assembler.
 * Brought the source in a more readable shape.

## Version 2.37.99
 * Rebuilt the binaries of the original source with Borland C++ 3.1 and TASM 5.1.

## Version 2.37.11
 * This is the unmodified source of Mikhail Ranish's last public domain version of the software excluding the freeware binaries.
