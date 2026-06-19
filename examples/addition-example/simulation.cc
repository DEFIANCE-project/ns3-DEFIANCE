#include "addition-agent-app.h"
#include "no-op-action-app.h"
#include "node-id-reward-app.h"
#include "random-observation-app.h"

#include "ns3/communication-helper.h"
#include "ns3/node-container.h"
#include "ns3/rl-application-helper.h"

using namespace ns3;

int
main(int argc, char* argv[])
{
    uint32_t seed = 1;
    uint32_t runId = 1;
    uint32_t parallel = 0;
    std::string trialName = "single_trial";

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

    auto rewardNodes = NodeContainer(5);
    auto agentNodes = NodeContainer(5);
    auto actionNodes = NodeContainer(5);
    auto observationNodes = NodeContainer(5);

    RlApplicationHelper rlAppHelper(AdditionAgentApp::GetTypeId());
    auto agentApps = rlAppHelper.Install(agentNodes);
    rlAppHelper = RlApplicationHelper(NoOpActionApp::GetTypeId());
    auto actionApps = rlAppHelper.Install(actionNodes);
    rlAppHelper = RlApplicationHelper(NodeListRewardApp::GetTypeId());
    auto rewardApps = rlAppHelper.Install(rewardNodes);
    rlAppHelper = RlApplicationHelper(RandomObservationApp::GetTypeId());
    auto observationApps = rlAppHelper.Install(observationNodes);

    CommunicationHelper commHelper = CommunicationHelper();

    commHelper.SetObservationApps(observationApps);
    commHelper.SetAgentApps(agentApps);
    commHelper.SetRewardApps(rewardApps);
    commHelper.SetActionApps(actionApps);
    commHelper.SetIds();

    std::vector<CommunicationPair> adjacency = {};
    for (uint i = 0; i < agentNodes.GetN(); i++)
    {
        CommunicationPair observationCommPair = {observationApps.GetId(i), agentApps.GetId(i), {}};
        CommunicationPair rewardCommPair = {rewardApps.GetId(i), agentApps.GetId(i), {}};
        CommunicationPair actionCommPair = {actionApps.GetId(i), agentApps.GetId(i), {}};
        adjacency.emplace_back(observationCommPair);
        adjacency.emplace_back(rewardCommPair);
        adjacency.emplace_back(actionCommPair);
    }
    commHelper.AddCommunication(adjacency);
    commHelper.Configure();

    for (auto it = agentApps.Begin(); it != agentApps.End(); it++)
    {
        Simulator::Schedule(Seconds(400),
                            MakeCallback(&AgentApplication::RequestTruncation,
                                         PeekPointer((*it)->GetObject<AgentApplication>())));
    }
    Simulator::Run();

    return 0;
}
