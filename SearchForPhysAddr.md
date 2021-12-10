
These are the steps we've taken to determine the physical address in memory of the list-head struct which points to the list of loaded (kernel) modules.

1. Compile the linux kernel as part of the build environment
2. Use dwarf2json on the resultant System.map and vmlinux files to generate a profile for this system-to-be-virtualized.
3. Search the output of dwarf2json for an entry named simply "modules." This entry has an item called "Address" which is the virtual address of the list-head struct.
4. Copy this Virtual Address into the source of a linux kernel module built for the explicit purpose of calling the kernel-space function "virt-to-phys" in order to convert that virtual address to a physical address.
5. Celebrate.
