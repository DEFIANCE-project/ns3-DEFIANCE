# DEFIANCE agent communication example

This example shows how agent applications can exchange messages.
To do so, fixed float values are sent so that neither RL nor *ns3-ai* are used in this example.

## Running this scenario

This example can be executed with the command `ns3 run defiance-agent-communication-example`.

## Scenario overview

The scenario consists of two agent applications that are installed on distinct nodes and connected via
a pair of simple channel interfaces. If an agent app receives a message of float values, it logs it and
stores the message.
