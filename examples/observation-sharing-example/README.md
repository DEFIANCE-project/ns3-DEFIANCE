# DEFIANCE observation sharing example

This example shows how agent applications can forward observations they receive from an observation application to another agent application.
Again, no RL as used. Mobility data serves as observations.

## Running this scenario

This example can be executed with the command `ns3 run defiance-observation-sharing-example`.

For more detailed logging, enable the `AgentApplication` logging component, i.e. use the command `NS_LOG="AgentApplication" ns3 run defiance-observation-sharing-example`.

## Scenario overview

The scenario consists of an observation application and two agent applications that are installed on distinct nodes and connected via simple channel interfaces. The observation application observes when its node changes its position. These observations are sent to the first agent application. This first agent application forwards them to the second agent application. In the end, the messages that each agent application received are logged.
