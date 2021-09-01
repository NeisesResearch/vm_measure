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
