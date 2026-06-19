# DEFIANCE app communication example

This example showcases the typical communication between the different types of RL applications within the ns-3 simulation. To do so, the reinforcement learning itself is completely omitted here, which means that *ns3-ai*'s functionality is not used in this example.

## Running this scenario

As this example is exported under the name `defiance-app-communication-example`, it can be executed with the command `ns3 run defiance-app-communication-example`.
To enable further logging, use the logging components `PositionObservationApp`, `PositionRewardApp`, and `PositionActionApp`.

## Scenario overview

The scenario of this example includes two observation applications, one reward application, one agent application, and one action application each of which is installed on a different node.
These nodes move according to the `RandomWalk2dMobilityModel` and their mobility data is used as mock data to send it between the RL applications.

The first observation application is connected to the agent application via a socket channel interface.
All other connections between RL applications use simple channel interfaces.

## App overview

### Observation app

The observation app observes the position (x and y coordinates) and velocity (length of the velocity vector) of the respective node whenever the underlying mobility model changes its course.

### Reward app

The reward app provides the distance to the origin of coordinates whenever the underlying mobility model changes its course.

### Agent app

The agent app outputs statistics about received observations and rewards.
Additionally, it sends the last positions of both observation apps to the action app whenever it receives an update from one of the observation apps.

### Action app

The action app calculates the midpoint between the two observation applications and sets the position of its own node to this point.
