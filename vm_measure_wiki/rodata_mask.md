# Measurement Confusion
Some of the kernel module read-only data is dynamic. 
In a certain kernel module, bytes 4176-4874 out of 8192 must be ignored for the hash digest to be consistent. 
The purpose of these 698 bytes is not clear, but there could be certain addresses that have changed. 
See maskedBytes.png for an example of the dynamic section. 

