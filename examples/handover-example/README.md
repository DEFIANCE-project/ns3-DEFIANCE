# RL Handover training in *ns-3*

This example demonstrates an example of how to train RL agents in *ns-3* to learn handover decisions.

## Scenario

Four eNBs are placed in a square of 1000m x 1000m. Three UEs follow a RandomWaypointMobilityModel within the square, meaning they walk from a randomly sampled point to another. This happens in a straight line and with constant speed.

Automatic handovers, e.g., via the A2A4RsrqHandoverAlgorithm or the A3RsrpHandoverAlgorithm are disabled. Instead, one UE (UE0) is controlled by an agent.

## Deployment of RL applications

The RL applications introduced by the DEFIANCE framework are deployed at different locations in the scenario.
- Observations of RSRQ/RSRP values are done at the BS, so it comes natural to install the observation application there.
- The same logic applies for the action application. The BS receives the action by the agent and potentially initiates a handover for UE0 if it is currently connected to the cell.
- Ultimately we are optimizing for throughput here, thus the reward application is deployed at UE0, since we can measure the arriving data traffic there.
- The agent application is located at the remote host and communicates with the Python side for training and inference. It is also possible to deploy it at BS or a another node, but in this case we opt for straightforward communication channels.

## Usage

### Training

If inside a devcontainer, follow these steps:

1. Configure the project to enable examples and python: \
```ns3 configure --enable-examples --enable-tests --enable-python``` \
Add the -d flag depending on whether you want to run it in debug or the faster release mode.

2. Build the files necessary: \
```ns3 build defiance-handover-example```

3. Train:\
```run-agent train -n defiance-handover-example``` \
Look at handover-scenario.cc for more parameters like number of training iterations. Add those parameters with ```--ns3-settings key=value key2=value2```

### Reproducibility

The results were obained by the training commands in `train.sh`. The `results` folder contains the `result.json` and the best checkpoint from training for each command. A diagram showing the training curves can be created with `plot_training_curves.py`.
