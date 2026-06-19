#!/usr/bin/env python3

import argparse

import matplotlib.pyplot as plt
import pandas as pd
from defiance import NS3_HOME


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Plot throughput of UE 0 over time")
    parser.add_argument("-s", "--seed", type=int, default=1, help="Seed of the simulation")
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    data = pd.read_csv(
        NS3_HOME + "/contrib/defiance/examples/handover/handover-stats_" + str(args.seed) + ".csv",
        names=["Time", "Throughput"],
    )

    data["Throughput (1e6)"] = data["Throughput"] / 1000000
    data.plot(x="Time", y="Throughput (1e6)", title="Throughput of UE 0")

    plt.xlabel("Time (s)")
    plt.ylabel("Throughput (Mbit/s)")
    plt.savefig(NS3_HOME + "/contrib/defiance/examples/handover/handover-throughput_" + str(args.seed) + ".png")


if __name__ == "__main__":
    main()
