# vm-measure
A camkes app for the measurement of a linux instance by a deeper component.

This is to be used with my kernel-module-workstation, which prepares the environment for kernel module compilation

## What does this do?
This app demonstrates the detection at runtime of an unexpected Linux kernel module by a seL4 CAmkES component. 
Upon detection, the Linux instance is killed and reset, but the seL4 instance persists.

## Why is this important?
A camkes component is considered to be *deeper* in the system than the Linux instance. 
By having this deeper component measure the Linux instance, we constrain the attacker.
By measuring in this way, we force the attacker to attack seL4 itself, which has certain guarantees.
See Paul Rowe for more on this constraint

## What attacks does this defend against?
- The addition of any unknown kernel module
- The impersonation of the measurement kernel module

## See the Wiki
The wiki has detailed information on this application.


