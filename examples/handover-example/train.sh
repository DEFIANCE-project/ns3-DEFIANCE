#!/usr/bin/env sh

# Usual handover algorithms for comparison
run-agent train -n defiance-handover-example -c parallel=124 stepTime=1000 simTime=150 handoverAlgorithm=a2a4 -i 300
run-agent train -n defiance-handover-example -c parallel=124 stepTime=1000 simTime=150 handoverAlgorithm=a3 -i 300

# RL handover algorithm
## No transmission delay
run-agent train -n defiance-handover-example -c parallel=124 stepTime=1000 simTime=150 handoverAlgorithm=agent -i 300

## Transmission delay 500 ms
run-agent train -n defiance-handover-example -c parallel=124 stepTime=1000 simTime=150 handoverAlgorithm=agent delay=500 -i 300

## Transmission delay 5000 ms
run-agent train -n defiance-handover-example -c parallel=124 stepTime=1000 simTime=150 handoverAlgorithm=agent delay=5000 -i 300
