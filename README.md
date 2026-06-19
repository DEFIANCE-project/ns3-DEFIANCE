# DEFIANCE project: How to Simulate MARL Deployments in Realistic Network Scenarios

Our project mainly builds upon the network simulator *ns-3* and *ns3-ai* for ML integration.

We provide a framework for ML and RL research using *ns-3*.
You can find our [design documentation and user documentation here](https://DEFIANCE-project.github.io).

For a practical example of how our framework is used, see [this Medium article](https://medium.com/@oliver.zimmermann/reinforcement-learning-in-ns3-part-1-698b9c30c0cd). The blog is divided into two parts and demonstrates building a balancing inverted pendulum in a network scenario using our framework.

## Setup the development environment

### Installation with setup helper

See <https://github.com/DEFIANCE-project/bake-defiance> for easy instructions.
Despite the name, it doesn't use bake anymore.

### Docker

If you just want a docker container ready to build ns3-defiance, pull <ghcr.io/defiance-project/bake-defiance:full-latest>.

For development, we supply a `Dockerfile` here, in which you can build your custom changes to ns3-defiance. It builds upon the aforementioned docker image.
Then, you can build a docker image containing your local changes with a simple `docker build .`.
The *ns-3* root directory is at `$NS3_HOME`; the default working directory.
By default, *ns-3* and *ns3-defiance* are built directly. To skip the build, add `--build-arg BUILD_NS3=False`.

### Manual installation

Requirements: Depending on your use-case, different dependencies are needed. For a complete list of all possible
development dependencies, refer
to [our devcontainer Dockerfile](https://github.com/DEFIANCE-project/bake-defiance/blob/main/.devcontainer/Dockerfile#L9)

1. Clone *ns-3* `git clone https://gitlab.com/nsnam/ns-3-dev.git -b ns-3.XX`
1. Some of our code needs that the environment variable `NS3_HOME` is set. Set it with `export NS3_HOME=$(pwd)/ns-3.XX`
1. Clone *ns3-ai* and *ns3-defiance* into `ns-3-dev/contrib`:

    ```shell-c
   cd ns-3-dev/
   git clone https://github.com/DEFIANCE-project/ns3-ai contrib/ai
   git clone https://github.com/DEFIANCE-project/ns3-defiance contrib/defiance
    ```

1. Install the python dependency of defiance with poetry: `poetry -C contrib/defiance install --without local` and activate the venv.
1. Make sure, you have all other dependencies. Running `./ns3 configure --enable-python --enable-examples --enable-tests`
   should succeed.
1. Then, compile *ns3-ai* to generate the message types with protobuf: `./ns3 build ai`
1. Install the python packages of *ns3-ai* with `poetry -C contrib/defiance install --with local`
1. Compile everything with `./ns3 build`
1. You are now able to start the training of our example scenario, such as `defiance-balance2`
   with `run-agent train -n defiance-handover`. See `run-agent --help` for more info.

### Development tools

This repo comes with additional developer tools, which may be installed with `poetry install --with dev`.

- We format and lint our python code with `ruff`. Simply run `ruff check` to lint and `ruff format` to format.
- We enforce type-checking on our code with `mypy`. Simply run it!
- Optional jupyter notebook support can be installed with `poetry`. Simply run `poetry install --with ipynb`!

In order to test *ns-3*, it needs to be configured correctly. Refer to <https://github.com/DEFIANCE-project/bake-defiance>
for a complete command suggestion.

The *ns-3* testsuites in the `test` directory can be run with `./test.py -s <test-suite>`,
e.g. `./test.py -s defiance-agent-application` for the `defiance-agent-application` testsuite
in `/test/agent-application-test.cc`. For
further information refer to <https://www.nsnam.org/docs/manual/html/how-to-write-tests.html>.

The special *ns3-ai* tests need to be executed with `pytest contrib/defiance`.

## Frequent problems

When you run a *ns-3* simulation which uses *ns3-ai*, a segmentation fault occurs. A corresponding python agent is required
to run the simulation. For this, you can use the `run-agent` cli program, i.e. `run-agent train` for training with ray
and `run-agent debug` for debugging and `run-agent random` for a random agent. Check out `run-agent -h` for help.
