#!/usr/bin/env python

import argparse

import matplotlib.pyplot as plt
import pandas as pd

# Embed fonts in the output file
plt.rcParams["pdf.fonttype"] = 42

parser = argparse.ArgumentParser(description="Visualize training curves of addition example runs.")
parser.add_argument(
    "-r", "--resultFile", default="./results/result.json", help="The relative path of the file result.json"
)
parser.add_argument(
    "-d",
    "--diagramFile",
    default="addition_example_training_curve.pdf",
    help="The path and filename where the plot shall be stored",
)
args = parser.parse_args()

number_of_plotted_iterations = 100
# The optimal value is 399 since the agent makes 399 steps, the maximum reward per step is 1 and the initial reward is
# (mostly) 0.
optimal_value = 399

history = pd.read_json(args.resultFile, lines=True)["policy_reward_mean"].head(number_of_plotted_iterations)
# Remove empty entries
cleaned_history = history[history.apply(lambda x: bool(x))]

for agent_id in range(5):
    agent_values = [v.get(f"agent_{agent_id}") for v in cleaned_history.to_numpy()]
    plt.plot(cleaned_history.index, agent_values, label=f"Agent {agent_id}")
plt.plot(history.index, [optimal_value] * number_of_plotted_iterations, linestyle="--", label="Optimum")
plt.xlabel("Iteration")
plt.ylabel("Mean reward per iteration")
plt.legend()
plt.grid(visible=True)
plt.savefig(args.diagramFile, bbox_inches="tight")
plt.show()
plt.close()
