# Change Log

## Version 2.90.10 alpha
### Enhancements
* Re-enable smallest file system size calculation for FAT12/16. Because it may take a long time it has to be triggered by pressing F7.

## Version 2.90.10 alpha
### Enhancements
 * Create Volume ID file when formatting FAT-12/16 file system.
 * Re-enable smallest file system size calculation for FAT32. Because it may take a long time it has to be triggered by pressing F7.

### Changes
 * Make quick format the default format option. Use /verify switch to do a bad sector scan.
 * Do not create or Volume ID file or make sure it does not exist if volume label is "NO NAME".


## Version 2.90.09 alpha
### Bug fixes
 * Fixed a bug in FAT32 format routine introduced in 2.90.08 resulting from reducing the reserved sectors from 33 count to 32.
 * Fix crash when showing FAT file system information for uninitialized file systems.
 
### Enhancements
 * Create Volume ID file when formatting FAT-32 file systems
 * Inform users that all nested partitions are deleted when deleting extended partitions.


## Version 2.90.08 alpha
### Bug fixes
 * Fix cluster size calculation in FAT format routines.
 * Fix calculation of FAT table size in format routines.
 * Fix sector number overflow in FAT format routines when crossing 2TiB barrier.
 * Prevent user from formatting partitions with wrong FAT type leading to non-conformant file systems.

### Changes
 * Remove display of minimum and maximum size in FAT file system screen because it can take a long time to calculate and nobody knows what this really means (it means that one can change the size of the file system and partition entry without having to change the file system layout, i.e. the size of the FAT tables or performing defragmentation).


## Version 2.90.07 alpha

### Enhancements
 * Allow hiding and unhiding of partition type 0F.


## Version 2.90.06 alpha
### Bug fixes
 * When formatting FAT-16 or FAT-32 partitions store absolute value for hidden sectors if partition type is LBA or partition is contained in an Extended LBA partition. In other cases relative sector count is stored to remain compatibility with MS-DOS / Windows format utility.

### Enhancements
 * Add the ability to erase partition data of any partition type via menu item. Previously there was no possibility eg. to zero out the boot sector of FAT partitions without changing the partition type to unused.
 * Allow hiding and unhiding of partition types 0C and 0E.

### Changes
 * Be more relaxed on LBA<->CHS mismatch. If head and sector number match the ones calculated from LBA number and the cylinder number matches the _wrapped-around one_ calculated from LBA then output a warning instead of considering the CHS value to be garbage.


## Version 2.90.05 alpha
### Enhancements
 * Allow using a custom CHS geometry given as command line argument `-g cyls,heads,sects`. If cyls=0, it gets calculated from disk size, number of heads and sectors per track.

### Changes
 * Only use standard CHS geometry in LBA mode if BIOS provided geometry is obviously wrong.


## Version 2.90.04 alpha
### Enhancements
 * Implement virtual CHS geometry in accordance to BIOS assisted LBA instead of relying on info given by BIOS INT13,2 when in LBA mode.
 * Add partition type 0xEE, the GPT guard partition

### Fixes
 * Fix value wrap-around if disk is larger than 2TiB. Set instead to 2TiB.
 * Fix sprintf format specifier error


## Version 2.90.03 alpha
### Enhancements
 * Implement LBA disk access - part.exe for now
 * Support drives >8GB - part.exe for now
 * Allow editing type 0x0F Extended Partitions (LBA)

### Fixes
 * disk_verify could lead to memory corruption on very old BIOS (<1985)

### Changes
 * Do not use INT13,5 for partition formatting any more. It has no LBA equivalent.


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
