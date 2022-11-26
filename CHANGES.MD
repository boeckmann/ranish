# Change Log

## Version 2.90.03 alpha
### Enhancements
 * part.exe: Implement logical block access (LBA)
 * part.exe: Support drives >8GB
 * part.exe: Allow editing type 0x0F Extended Partitions (LBA)

### Fixes
 * verify could lead to memory corruption on very old BIOS (<1985)

## Version 2.90.02 alpha
### Fixes
 * Fix freeze / crash when help is opened a second time, caused by a memory invalidation.
 * Fix a bug that causes the scroll bar of help window to stay at the topmost position.

## Version 2.90.01 alpha
### Fixes
 * prevent IPL check sum corruption by advanced boot loader

## Version 2.90.00 alpha
### Fixes
 * Do not overwrite the last 16 bytes of IPL code with old signature.

### Enhancements
 * Use checksum to verify ADV boot sector code integrity.
 * Decrease bad block tracking buffer to 128 entries to reduce memory consumption.
 * Clarify messages and menu entries what is saved / loaded.

### Other Changes
 * This is the first version which completely builds with open source tools.
 * Ported to Open Watcom C and Netwide Assembler.
 * Brought the source in a more readable shape.

## Version 2.37.99
 * Rebuilt the binaries of the original source with Borland C++ 3.1 and TASM 5.1.

## Version 2.37.11
 * This is the unmodified source of Mikhail Ranish's last public domain version of the software excluding the freeware binaries.
