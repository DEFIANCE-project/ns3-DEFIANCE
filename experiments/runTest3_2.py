import json
import logging
import os
from typing import Any

import gymnasium as gym
import ns3ai_gym_env  # noqa: F401  # import to register env
import pandas as pd
from gymnasium.wrappers import TimeLimit
from ray.tune.registry import register_env

NS3_HOME = os.getenv("NS3_HOME") or "/code/ns3"
logger = logging.getLogger(__name__)


def env_creator(_: str) -> gym.Env:
    return env


cpp_settings = {
    "topology": "multi_bs",
}

limits = (500, 1000, 1500, 2000)

if __name__ == "__main__":
    global_info: dict[str, list[list[Any]]] = {
        "action": [],
        "sinrs": [],
        "distances": [],
    }

    env = TimeLimit(
        gym.make(
            "ns3ai_gym_env/Ns3-v0",
            targetName="defiance-lte-learning",
            ns3Path=NS3_HOME,
            ns3Settings=cpp_settings,
        ),
        max_episode_steps=100,
    )
    register_env("defiance", env_creator)

    try:
        obs, info = env.reset()
        logger.info("Initial Observations after Env was reset %s", obs)
        logger.info("Initial Info after Env was reset %s", info)

        reward = 0
        done = False

        for steps in range(2500):
            if steps % 100 == 0:
                logger.info("Progress %s", steps)
            if steps <= limits[0]:
                action = [0, 0]
            elif steps <= limits[1]:
                action = [10, 0]
            elif steps <= limits[2]:
                action = [15, 10]
            elif steps <= limits[3]:
                action = [20, 10]
            else:
                action = [20, 20]

            global_info["action"].append(action)

            obs, reward, done, _, info = env.step(action)
            info = json.loads(info["info"])
            global_info["sinrs"].append(info["sinrs"])
            global_info["distances"].append(info["distances"])

    except Exception:
        logger.exception("Exception occurred")

    finally:
        logger.info("Finally exiting...")
        env.close()

    pd.DataFrame(global_info).to_csv(NS3_HOME + "/contrib/defiance/experiments/results/test3_2.csv")
