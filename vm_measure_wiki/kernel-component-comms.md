
### This is a pickle.

The dataport method, given in an example,
    exposes the dataport to user space.
    I don't really want to do that.
    but I don't think it's a critical problem.

The other problem is that I can't "open" the pci device file in kernel space
    I don't have that userspace library
    so I have to find some other way to interface with the device

I found there are special modules that get loaded by camkes/vm-arm
    see: src/cross_vm_connections.c
    there can be found a handle to the dataports,
    but I don't think that executes with kernel privileges
        try printk :shrug:

Ideally I want to make the dataports visible only to the kernel module.


### solution

The pci device is still accessible in the kernel module in which it was registered.

I was able to read the dataport successfully by accessing the address through the pci device.


### todo
- [ ] implement writing to the dataport
- [ ] disable access to /dev/uio0
    
