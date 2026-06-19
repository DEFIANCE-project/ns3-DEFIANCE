# DEFIANCE channel interface example

Here, we show the basic functionality of channel interfaces.
For typical use cases of our framework, it is not necessary to deal with these low-level aspects.
Instead, you can usually just use the CommunicationHelper to connect your RL applications.
But if you want to set up connections yourself, this example shows you how to create, connect, and use simple and socket channel interfaces.

## Running this scenario

This example can be executed with the command `ns3 run defiance-channel-interface-example`.
For further logging, use the logging components `SimpleChannelInterface` and `SocketChannelInterface`.
