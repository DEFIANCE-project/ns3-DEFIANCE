#include "ns3/base-test.h"
#include "ns3/communication-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/reward-application.h"
#include "ns3/rl-application-helper.h"

#include <iostream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ApplicationCommunicationExample");

/**
 * @ingroup defiance
 * Child class of ObservationApp that unifies functionality to create
 * the appropriate DictContainer and functionality to allow calling
 * the callback object
 */
class TestObservation : public ObservationApplication
{
  public:
    TestObservation() {};
    ~TestObservation() override {};
    static TypeId GetTypeId();
    Ptr<OpenGymDictContainer> CreateDictContainer(std::vector<float> observation);
    void Observe(Ptr<const MobilityModel> observation);
    void RegisterCallbacks() override;
};

NS_OBJECT_ENSURE_REGISTERED(TestObservation);

TypeId
TestObservation::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestObservation")
                            .SetParent<ObservationApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestObservation>();
    return tid;
}

Ptr<OpenGymDictContainer>
TestObservation::CreateDictContainer(std::vector<float> observation)
{
    auto box = MakeBoxContainer<float>(3);
    for (auto val : observation)
    {
        box->AddValue(val);
    }

    auto dictContainer = CreateObject<OpenGymDictContainer>();
    dictContainer->Add("floatObs", box);
    return dictContainer;
}

void
TestObservation::Observe(Ptr<const MobilityModel> observation)
{
    std::vector<float> obs;
    obs.push_back(observation->GetPosition().x);
    obs.push_back(observation->GetPosition().y);
    obs.push_back(observation->GetPosition().z);
    Send(CreateDictContainer(obs), 0);
}

void
TestObservation::RegisterCallbacks()
{
    Config::ConnectWithoutContext("/NodeList/*/$ns3::MobilityModel/CourseChange",
                                  MakeCallback(&TestObservation::Observe, this));
}

/**
 * @ingroup defiance
 * Child class of RewardApp that unifies functionality to create the appropriate DictContainer and
 * functionality to allow calling the callback object
 */
class CallbackRewardApp : public RewardApplication
{
  public:
    CallbackRewardApp() {};
    ~CallbackRewardApp() override {};
    static TypeId GetTypeId();
    Ptr<OpenGymDictContainer> CreateDictContainer(std::vector<float> reward);
    void Reward(Ptr<const MobilityModel> observation);
    void RegisterCallbacks() override;
};

NS_OBJECT_ENSURE_REGISTERED(CallbackRewardApp);

TypeId
CallbackRewardApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CallbackRewardApp")
                            .SetParent<RewardApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<CallbackRewardApp>();
    return tid;
}

Ptr<OpenGymDictContainer>
CallbackRewardApp::CreateDictContainer(std::vector<float> reward)
{
    auto box = MakeBoxContainer<float>(3);
    for (auto val : reward)
    {
        box->AddValue(val);
    }

    auto dictContainer = CreateObject<OpenGymDictContainer>();
    dictContainer->Add("floatReward", box);
    return dictContainer;
}

void
CallbackRewardApp::Reward(Ptr<const MobilityModel> observation)
{
    std::vector<float> obs;
    obs.push_back(observation->GetPosition().x);
    obs.push_back(observation->GetPosition().y);
    obs.push_back(observation->GetPosition().z);
    // reward gets calculated by a function over the length of the vector
    float length = std::sqrt(std::pow(obs[0], 2) + std::pow(obs[1], 2) + std::pow(obs[2], 2));
    std::vector<float> reward = {length};
    Send(CreateDictContainer(reward), 0);
}

void
CallbackRewardApp::RegisterCallbacks()
{
    Config::ConnectWithoutContext("/NodeList/*/$ns3::MobilityModel/CourseChange",
                                  MakeCallback(&CallbackRewardApp::Reward, this));
}

class TestAgent : public AgentApplication
{
  public:
    TestAgent() {};

    ~TestAgent() override = default;
    static TypeId GetTypeId();

    void OnRecvObs(uint remoteAppId) override
    {
        NS_LOG_INFO("received Observation from " << remoteAppId);
        NS_LOG_INFO(
            "avg: " << m_obsDataStruct.AggregateNewest(remoteAppId, 10)["floatObs"].GetAvg());
        NS_LOG_INFO(
            "min: " << m_obsDataStruct.AggregateNewest(remoteAppId, 10)["floatObs"].GetMin());
        NS_LOG_INFO(
            "max: " << m_obsDataStruct.AggregateNewest(remoteAppId, 10)["floatObs"].GetMax());
    }

    void OnRecvReward(uint remoteAppId) override
    {
        NS_LOG_INFO("received reward from reward interface " << remoteAppId);
        NS_LOG_INFO(
            "avg: " << m_rewardDataStruct.AggregateNewest(remoteAppId)["floatReward"].GetAvg());
    }

    Ptr<OpenGymSpace> GetObservationSpace() override;
    Ptr<OpenGymSpace> GetActionSpace() override;
};

TypeId
TestAgent::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestAgent")
                            .SetParent<AgentApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestAgent>();
    return tid;
}

Ptr<OpenGymSpace>
TestAgent::GetObservationSpace()
{
    return {};
}

Ptr<OpenGymSpace>
TestAgent::GetActionSpace()
{
    return {};
}

int
main(int argc, char* argv[])
{
    LogComponentEnable("ApplicationCommunicationExample", LOG_LEVEL_INFO);

    // Set up nodes with mobility model
    NodeContainer observationNodes;
    observationNodes.Create(2);
    auto agentNode = CreateObject<Node>();
    NodeContainer rewardNodes;
    rewardNodes.Create(1);

    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Mode", StringValue("Time"));
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Time", StringValue("2s"));
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Speed",
                       StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Bounds", StringValue("0|200|0|200"));

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

    // Create and install observation apps
    RlApplicationHelper helper(TypeId::LookupByName("ns3::TestObservation"));
    helper.SetAttribute("StartTime", TimeValue(Seconds(0)));
    helper.SetAttribute("StopTime", TimeValue(Seconds(10)));
    RlApplicationContainer observationApps = helper.Install(observationNodes);
    helper.SetTypeId("ns3::CallbackRewardApp");
    RlApplicationContainer rewardApps = helper.Install(rewardNodes);

    // Create and install agent app
    auto agentApp = CreateObjectWithAttributes<TestAgent>("MaxObservationHistoryLength",
                                                          UintegerValue(10),
                                                          "MaxRewardHistoryLength",
                                                          UintegerValue(5));
    agentNode->AddApplication(agentApp);

    CommunicationHelper commHelper = CommunicationHelper();

    commHelper.SetObservationApps(observationApps);
    commHelper.SetAgentApps(RlApplicationContainer(agentApp));
    commHelper.SetRewardApps(rewardApps);
    commHelper.SetIds();

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
        {{observationApps.GetId(0),
          agentApp->GetId(),
          SocketCommunicationAttributes{interfaces.GetAddress(0), interfaces.GetAddress(1)}},
         {observationApps.GetId(1), agentApp->GetId(), {}},
         {rewardApps.GetId(0), agentApp->GetId(), {}}});

    commHelper.Configure();
    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Simulation ended");

    return 0;
}
