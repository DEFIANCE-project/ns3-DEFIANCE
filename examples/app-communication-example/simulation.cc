#include "mobility-agent-app.h"
#include "mobility-observation-app.h"
#include "position-action-app.h"
#include "position-reward-app.h"

#include "ns3/base-test.h"
#include "ns3/communication-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/reward-application.h"
#include "ns3/rl-application-helper.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AppCommunicationExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("AppCommunicationExample", LOG_LEVEL_INFO);
    LogComponentEnable("MobilityAgentApp", LOG_LEVEL_INFO);

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

    // Set up nodes with mobility model
    NodeContainer observationNodes;
    observationNodes.Create(2);
    auto agentNode = CreateObject<Node>();
    NodeContainer rewardNodes;
    rewardNodes.Create(1);
    NodeContainer actionNodes;
    actionNodes.Create(1);

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
                              StringValue("ns3::UniformRandomVariable[Min=0.5|Max=2]"),
                              "Bounds",
                              StringValue("0|200|0|200"));
    mobility.InstallAll();

    // Create and install observation apps
    RlApplicationHelper helper(TypeId::LookupByName("ns3::MobilityObservationApp"));
    helper.SetAttribute("StartTime", TimeValue(Seconds(0)));
    helper.SetAttribute("StopTime", TimeValue(Seconds(10)));
    RlApplicationContainer observationApps = helper.Install(observationNodes);

    // Create and install reward app
    helper.SetTypeId("ns3::PositionRewardApp");
    RlApplicationContainer rewardApps = helper.Install(rewardNodes);

    // Create and install action app
    helper.SetTypeId("ns3::PositionActionApp");
    RlApplicationContainer actionApps = helper.Install(actionNodes);

    // Create and install agent app
    auto agentApp =
        CreateObjectWithAttributes<MobilityAgentApp>("MaxRewardHistoryLength", UintegerValue(5));
    agentNode->AddApplication(agentApp);

    // Create a CommunicationHelper and use it to assign IDs to the apps
    CommunicationHelper commHelper = CommunicationHelper();
    commHelper.SetObservationApps(observationApps);
    commHelper.SetAgentApps(RlApplicationContainer(agentApp));
    commHelper.SetRewardApps(rewardApps);
    commHelper.SetActionApps(actionApps);
    commHelper.SetIds();

    // Install the internet on the first observation node and the agent node
    NodeContainer internetNodes{observationNodes.Get(0), agentNode};
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    auto internetDevices = p2p.Install(internetNodes);
    InternetStackHelper internet;
    internet.Install(internetNodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(internetDevices);

    commHelper.AddCommunication(
        {// Connect the first observation app and the agent app via a socket channel interface
         {observationApps.GetId(0),
          agentApp->GetId(),
          SocketCommunicationAttributes{interfaces.GetAddress(0), interfaces.GetAddress(1)}},
         // Connect the second observation app and the agent app via a simple channel interface
         {observationApps.GetId(1), agentApp->GetId(), {}},
         // Connect the reward app and the agent app via a simple channel interface
         {rewardApps.GetId(0), agentApp->GetId(), {}},
         // Connect the agent app and the action app via a simple channel interface
         {agentApp->GetId(), actionApps.GetId(0), {}}});

    commHelper.Configure();
    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Simulation ended");

    return 0;
}
