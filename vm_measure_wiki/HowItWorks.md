This app facilitates the measurement of a virtualized Linux instance by a deeper-level CAmkES component.


This architecture can detect unexpected kernel modules at runtime.

    The control flow goes like this:
    1. The CAmkES component measures the Linux binary.
    2. The Linux instance boots.
    3. The CAmkES component requests a measurement from the Linux kernel "measurement" module.
    4. The measurement module collects information about the other loaded kernel modules.
    5. The measurement module reports this information to the CAmkES component.
    6. The CAmkES component analyzes the measurement.

This architecture is trustworthy. 

    We consider the attacker to have network-level access to the Linux instance.
        
    We consider that an owned Linux instance cannot deceive the measurement architecture.

        The measurement module cannot be impersonated because...?
            this module is loaded into the kernel at boot-time.
                (it pre-dates any impersonator)
            this module locks memory as soon as a measurment is requested
                (it will not share its resources with an impersonator)
            this module will report its removal from the kernel to the CAmkES component.
                (it will not go silently)


