#!/usr/bin/env python3

import argparse

import matplotlib.pyplot as plt
import pandas as pd

# Embed fonts in the output file
plt.rcParams["pdf.fonttype"] = 42


def plot_reward(progress_dir: str, subdir: str, label: str, plotted_iterations: int) -> None:
    progress_file = f"{progress_dir}/{subdir}/result.json"
    history = pd.read_json(progress_file, lines=True)["episode_reward_mean"].head(plotted_iterations)
    plt.plot(history.index, history, label=label)


parser = argparse.ArgumentParser(description="Visualize training curves of handover example runs.")
parser.add_argument(
    "-p", "--resultDir", default="./results", help="The directory where the result.json files are stored"
)
parser.add_argument(
    "-d",
    "--diagramFile",
    default="handover_example_training_curve.pdf",
    help="The path and filename where the plot shall be stored",
)
parser.add_argument("-i", "--plottedIterations", default="50", type=int, help="Number of iterations to plot")
args = parser.parse_args()

plot_reward(args.resultDir, "a3", "A3RsrpHandoverAlgorithm", args.plottedIterations)
plot_reward(args.resultDir, "0ms-agent", "RL Agent (0 s delay)", args.plottedIterations)
plot_reward(args.resultDir, "500ms-agent", "RL Agent (500 ms delay)", args.plottedIterations)
plot_reward(args.resultDir, "5000ms-agent", "RL Agent (5 s delay)", args.plottedIterations)

plt.xlabel("Iteration")
plt.ylabel("Mean reward per iteration")
plt.legend()
plt.grid(visible=True)
plt.savefig(args.diagramFile, bbox_inches="tight")
plt.show()
plt.close()
