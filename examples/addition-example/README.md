# DEFIANCE addition example

This example showcases the full DEFIANCE framework within a silly environment: add a constant number to a random observation.

## Running this scenario

You can use the `run-agent` tool to interact with this environment.
*ns-3* exports this environment as the `defiance-addition-example` target.
For a training session, you can start `run-agent train -n defiance-addition-example`.
`agent.py` showcases custom environment interaction using the `MultiAgentEnv`-API.

## App overview

### Observation app

The observation app generates a random number.

### Reward app

The reward app provides the constant number, which is called "reward-hint".

### Agent app

The agent app calculates the actual reward from the last observation, the last action and the reward-hint.
The observation is directly forwarded from the observation app.

### Action app

In this scenario, an action app is not needed.
To showcase a fully configured DEFIANCE scenario, the action is implemented as a no-op, which receives the action of the agent and discards it.
It is also possible to remove the action app in this case altogether.

## Reproducibility

The results were obained by the training commands in `train.sh`. The `results` folder contains the `result.json` and the best checkpoint from training for the training command. A diagram showing the training curves of all agents can be created with `plot_training_curves.py`.
