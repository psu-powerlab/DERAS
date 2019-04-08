# DERAS
Distributed Energy Resource Aggregation System

If you are new to github, then you should read the following link to get an understanding of the workflow required.

https://github.com/emesene/emesene/wiki/GitHowTo#what-should-i-do-if-im-in-a-bad-situation

## Dependancies
### Linux Environment
``` console
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install tmux git build-essential gcc g++ cmake make xsltproc scons doxygen graphviz libgtk2.0-dev libssl-dev libxml2-dev libcap-dev
mkdir ~/src ~/dev ~/dev/LOGS
```

### OpenVPN_Bridged
OpenVPN setup with a tap interface on Ubuntu 16.04

#### Initial Setup
Follow steps 1-6 in the following link:

https://www.digitalocean.com/community/tutorials/how-to-set-up-an-openvpn-server-on-ubuntu-16-04

This specific tutorial shows how to setup tls-authentication, but we will ignore those settings within the server config for now to reduce complexity.

#### Bridged : Server Config

To create a bridged vpn connection, you must create a virtual bridge within the server to pass packets between interfaces.

``` bash
sudo apt-get install bridge-utils
```

Now we need to copy the files created in steps 1-6 to the /etc/openvpn directory.

``` bash
cd ~/openvpn-ca/keys
sudo cp ca.crt server.crt server.key ta.key dh2048.pem /etc/openvpn
```

Next, copy the sample openvpn server.conf file into the same directory

``` bash
gunzip -c /usr/share/doc/openvpn/examples/sample-config-files/server.conf.gz | sudo tee /etc/openvpn/server.conf
```

The last step is to modify the server.conf file to create an ethernet tunnel.

``` base
sudo nano /etc/openvpn/server.conf
```
Disable "dev tun" by prefacing with a ";" and enable "dev tap0" by removing the ";".

```
# "dev tun" will create a routed IP tunnel,
# "dev tap" will create an ethernet tunnel.
# Use "dev tap0" if you are ethernet bridging
# and have precreated a tap0 virtual interface
# and bridged it with your ethernet interface.
# If you want to control access policies
# over the VPN, you must create firewall
# rules for the the TUN/TAP interface.
# On non-Windows systems, you can give
# an explicit unit number, such as tun0.
# On Windows, use "dev-node" for this.
# On most systems, the VPN will not function
# unless you partially or fully disable
# the firewall for the TUN/TAP interface.
dev tap0
;dev tun
```

Disable server by prefacing with ";".

```
# Configure server mode and supply a VPN subnet
# for OpenVPN to draw client addresses from.
# The server will take 10.8.0.1 for itself,
# the rest will be made available to clients.
# Each client will be able to reach the server
# on 10.8.0.1. Comment this line out if you are
# ethernet bridging. See the man page for more info.
;server 10.8.0.0 255.255.255.0
```

Enable server-bridge by removing ";" and set subnet.

**Keep the subnet ip and netmask as they will be used later**


```
# Configure server mode for ethernet bridging.
# You must first use your OS's bridging capability
# to bridge the TAP interface with the ethernet
# NIC interface.  Then you must manually set the
# IP/netmask on the bridge interface, here we
# assume 10.8.0.4/255.255.255.0.  Finally we
# must set aside an IP range in this subnet
# (start=10.8.0.50 end=10.8.0.100) to allocate
# to connecting clients.  Leave this line commented
# out unless you are ethernet bridging.
server-bridge 10.8.0.4 255.255.255.0 10.8.0.50 10.8.0.100
```

For our purposes we will configure all clients to redirect all traffic through the VPN by enabling push by removing ";".

```
# If enabled, this directive will configure
# all clients to redirect their default
# network gateway through the VPN, causing
# all IP traffic such as web browsing and
# and DNS lookups to go through the VPN
# (The OpenVPN server machine may need to NAT
# or bridge the TUN/TAP interface to the internet
# in order for this to work properly).
push "redirect-gateway def1 bypass-dhcp"
```

Finally we will enable dublicate certificate/key by enabling duplicate-cn by removing ";".

```
# Uncomment this directive if multiple clients
# might connect with the same certificate/key
# files or common names.  This is recommended
# only for testing purposes.  For production use,
# each client should have its own certificate/key
# pair.
#
# IF YOU HAVE NOT GENERATED INDIVIDUAL
# CERTIFICATE/KEY PAIRS FOR EACH CLIENT,
# EACH HAVING ITS OWN UNIQUE "COMMON NAME",
# UNCOMMENT THIS LINE OUT.
duplicate-cn
```

#### Setup Bridge Start/Stop

``` base
sudo cp /usr/share/doc/openvpn/examples/sample-scripts/bridge-start /bin
sudo chmod 755 /bin/bridge-start
sudo nano /bin/bridge-start
```

We can ignot the physical ethernet interface to be bridged since we only want clients to communicate with the server by commenting out "eth" with a "#". Next use the subnet IP and Netmask from the server config and set the "eth_" variables.


``` bash
# Define physical ethernet interface to be bridged
# with TAP interface(s) above.
# eth="eth0"
eth_ip="10.8.0.4"
eth_netmask="255.255.255.0"
eth_broadcast="10.8.0.255"
```

Then disable the "brctl addif" because we don't need the clients to speak to anyone but the server
``` bash
brctl addbr $br
# brctl addif $br $eth
```

Now copy bridge-start to /etc/init.d so it will automatically be ran at startup.

``` base
sudo cp /bin/bridge-start /etc/inid.d
```

The bridge-stop script just needs to be copied to the bin folder.


``` base
sudo cp /usr/share/doc/openvpn/examples/sample-scripts/bridge-stop /bin
sudo chmod 755 /bin/bridge-stopifco
```

#### Server Network / Client Config

Now continue the previous tutorial from step 8-10.

**NOTE: Ignore step 10 below the following line:**  

> Mirror the cipher and auth settings that we set in the /etc/openvpn/server.conf file: 

#### Start/Stop OpenVPN
If you need to stop/start the VPN connection you need to stop the vpn service before stoping the bridge and then start the bridge before you start the service

```
sudo systemctl stop openvpn@service
sudo bridge-stop.sh
sudo bridge-start.sh
sudo systemctl start openvpn@service
```

### AllJoyn
First clone the AllJoyn repository into your /src folder. The run the following "scons" command that corresponds to your processor. 
``` console
cd ~/src
git clone https://github.com/alljoyn/core-alljoyn
cd core-alljoyn
```

#### x86 and x86_64 processors
``` console
scons BINDINGS=cpp WS=off DOCS=html
```

#### Test AllJoyn Installation
Note: the CPU variable is dependant on your system. 
``` console
export CPU=x86_64
export AJ_LIB=~/src/core-alljoyn/build/linux/$CPU/debug/dist/cpp/lib
export LD_LIBRARY_PATH=$AJ_LIB:$LD_LIBRARY_PATH

cd $AJ_LIB
cd ../bin/samples
./AboutService
```
If the BusAttachment/AboutObj succeed, then the installation should be fine.

## Install
``` console
cd ~/dev
git clone https://github.com/psu-powerlab/DERAS
```

### Setup
1. Open /DERAS/tools/build-run.sh
2. Modify "CPU" to reflect the system you are working on
3. Ensure "AJ_ROOT" path is correct
4. The program arguments can be modified to change config file path and enable the auto operator.

### Program Arguments
#### Config File (required)
The config file holds the initialization variables for the DERAS program.

-c ../data/config.ini

### Run
Open a terminal an start the vpn client.
``` console
sudo openvpn --config /etc/openvpn/<client-name>.ovpn
```

Next open another therminal and start the dcs.
``` console
cd ~/dev/DCS/tools
./build-run.sh
```

## Use

