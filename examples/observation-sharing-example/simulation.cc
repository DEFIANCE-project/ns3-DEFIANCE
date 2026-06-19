#include "observation-sharing-agent-app.h"
#include "position-observation-app.h"

#include "ns3/defiance-module.h"
#include "ns3/mobility-module.h"

#include <cstdint>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ObservationSharingExample");

// Run this example with 'ns3 run defiance-observation-sharing-example'

/**
 * @brief Main function of the observation sharing example.
 * This function creates an observation application and two agent applications. The observation
 * application sends its observations to agent0 which forwards them to agent1. Therefore, agent0
 * receives observations and no agent messages, while agent1 receives agent messages but no
 * observations.
 */
int
main(int argc, char* argv[])
{
    LogComponentEnable("ObservationSharingExample", LOG_LEVEL_INFO);

    uint32_t seed = 1;
    uint32_t runId = 0;
    uint32_t parallel = 0;
    std::string trialName = "";

    CommandLine cmd(__FILE__);
    cmd.AddValue("seed", "Seed for random number generator", seed);
    cmd.AddValue("runId", "Run ID. Is increased for every reset of the environment", runId);
    cmd.AddValue("parallel",
                 "Parallel ID. When running multiple environments in parallel, this is the index.",
                 parallel);
    cmd.AddValue("trial_name", "name of the trial", trialName);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(seed + parallel);
    RngSeedManager::SetRun(runId);
    Ns3AiMsgInterface::Get()->SetTrialName(trialName);

    // Create RL apps
    auto observationApp = CreateObject<PositionObservationApp>();
    auto agent0 = CreateObject<ObservationSharingAgentApp>();
    auto agent1 = CreateObject<ObservationSharingAgentApp>();

    // Create nodes and add applications to them
    NodeContainer nodes{3};
    nodes.Get(0)->AddApplication(observationApp);
    nodes.Get(1)->AddApplication(agent0);
    nodes.Get(2)->AddApplication(agent1);

    // Install mobility model on nodes
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                  "X",
                                  StringValue("100.0"),
                                  "Y",
                                  StringValue("100.0"),
                                  "Rho",
                                  StringValue("ns3::UniformRandomVariable[Min=0|Max=30]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue("Time"),
                              "Time",
                              StringValue("2s"),
                              "Speed",
                              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds",
                              StringValue("0|200|0|200"));
    mobility.InstallAll();

    // Create and connect channel interfaces
    auto channelInterfaceO_A = CreateObject<SimpleChannelInterface>();
    auto channelInterfaceA_O = CreateObject<SimpleChannelInterface>();
    auto channelInterface0_1 = CreateObject<SimpleChannelInterface>();
    auto channelInterface1_0 = CreateObject<SimpleChannelInterface>();

    channelInterfaceO_A->Connect(channelInterfaceA_O);
    channelInterface0_1->Connect(channelInterface1_0);

    // Add channel interfaces to apps
    observationApp->AddAgentInterface(0, channelInterfaceO_A);
    observationApp->SetId(RlApplicationId{OBSERVATION, 0});
    observationApp->Setup();
    agent0->AddObservationInterface(0, channelInterfaceA_O);

    agent0->AddAgentInterface(1, channelInterface0_1);
    agent0->SetId(RlApplicationId{AGENT, 0});
    agent0->Setup();
    agent1->AddAgentInterface(0, channelInterface1_0);
    agent1->SetId(RlApplicationId{AGENT, 1});
    agent1->Setup();

    // Run the simulation
    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Simulation ended.");

    // Log received messages
    NS_LOG_INFO("Agent 0 received the following messages:");
    agent0->PrintLatestMessages(1);
    NS_LOG_INFO("Agent 1 received the following messages:");
    agent1->PrintLatestMessages(0);

    return 0;
}
