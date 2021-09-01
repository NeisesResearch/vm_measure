# One internet, please

## Setting up a DCHP server for automatic assignment of in-vm ip addresses

First download a DHCP server via `sudo apt-get install isc-dhcp-server`

Write /etc/default/isc-dhcp-server:
```
DHCPDv4_CONF=/etc/dhcp/dhcpd.conf
INTERFACESv4="enp3s0"
```

Write /etc/dhcp/dhcp.conf
```
subnet 192.168.1.0 netmask 255.255.255.0
{
    authoritative;
    range dynamic-bootp 192.168.1.1 192.168.1.254;
    default-lease-time 3600;
    max-lease-time 3600;
    option subnet-mask 255.255.255.0;
    option routers 192.168.0.1;
    option domain-name "dmitri.net";
    option domain-name-servers bing1.dmitri.net;
}
```

Start the DHCP service with `sudo systemctl resteart isc-dhcp-server.service`

Verify the DHCP server is active with `sudo nmap --script broadcast-dhcp-discover`

## Setting up the Tap Device

Get tunctl with `sudo apt-get install uml-utilities`

Run just the command `tunctl` to create a tap device `tap0`

`sudo mkdir /dev/net && sudo mknod /dev/net/tun c 10 200`
This creates a special character file necessary for the tap device configuration step.

Start the Docker instance with --privileged flag: `docker run --privileged...`
This is so we can create a network interface within the Docker Container.

In Docker container, `sudo ip tuntap add mode tap tap0` 
This creates a network interface "tap0" which corresponds to the /dev/net/tun we created earlier.

## Set up a network bridge within the Docker Container

Get the app
`sudo apt-get install bridge-utils`

Add a bridge and configure it to interface with tap0 and eth0 (the internet receiving device)
`sudo brctl addbr virbr0`
`sudo brctl addif virbr0 tap0 &&
 sudo brctl addif virbr0 eth0`

Enable all devices concerned
 `sudo ip link set tap0 up`
 `sudo ip link set virbr0 up`
 `sudo ip link set eth0 up`

Verify the bridge was setup correctly
 `sudo brctl show`

