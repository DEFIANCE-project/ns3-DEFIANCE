#include "ns3/ai-module.h"
#include "ns3/core-module.h"
#include "ns3/defiance-module.h"
#include "ns3/mobility-module.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(AgentApplication);
NS_OBJECT_ENSURE_REGISTERED(ObservationApplication);

NS_LOG_COMPONENT_DEFINE("ObservationSharing");

template <typename T>
Ptr<OpenGymBoxContainer<T>>
CreateBox(uint32_t shape)
{
    std::vector<uint32_t> vShape = {shape};
    return CreateObject<OpenGymBoxContainer<T>>(vShape);
}

class TestObservationApplication : public ObservationApplication
{
  private:
    Ptr<OpenGymDictContainer> CreateDictContainer(std::vector<float> observation)
    {
        auto box = CreateBox<float>(3);
        for (auto val : observation)
        {
            box->AddValue(val);
        }

        return MakeDictContainer("floatObs", box);
    }

  public:
    TestObservationApplication()
        : ObservationApplication() {};

    ~TestObservationApplication() override = default;

    void RegisterCallbacks() override
    {
        Config::ConnectWithoutContext("/NodeList/*/$ns3::MobilityModel/CourseChange",
                                      MakeCallback(&TestObservationApplication::Observe, this));
    }

    void Observe(Ptr<const MobilityModel> observation)
    {
        // NS_LOG_INFO("Received observation");
        std::vector<float> obs;
        auto obsX = observation->GetPosition().x;
        auto obsY = observation->GetPosition().y;
        auto obsZ = observation->GetPosition().z;
        obs.push_back(obsX);
        obs.push_back(obsY);
        obs.push_back(obsZ);
        // NS_LOG_INFO("Position: " << obsX << ", " << obsY << ", " << obsZ);
        auto test = CreateDictContainer(obs);
        Send(test, 0);
    }
};

class ObservationSharingTestAgent : public AgentApplication
{
  public:
    ObservationSharingTestAgent()
        // register custom data structure for agent messages here
        : AgentApplication(),
          m_agentDataStruct(10) {};

    ~ObservationSharingTestAgent() override = default;

    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::ObservationSharingTestAgent")
                                .SetParent<AgentApplication>()
                                .SetGroupName("defiance")
                                .AddConstructor<ObservationSharingTestAgent>();
        return tid;
    }

    void OnRecvObs(uint id) override
    {
        NS_LOG_INFO("Received Observation from " << id);
        auto info = m_obsDataStruct.AggregateNewest(id);
        //  send to all other agents
        auto newest = m_obsDataStruct.GetNewestByID(id);
        auto dictContainer = MakeDictContainer(
            "floatObs",
            newest->data->Get("floatObs")->GetObject<OpenGymBoxContainer<float>>());
        SendToAgent(dictContainer, 1, 0);
    }

    void OnRecvFromAgent(uint id, Ptr<OpenGymDictContainer> payload) override
    {
        m_agentDataStruct.Push(payload, id);
        NS_LOG_FUNCTION(this << "Received message from agent " << id);
        auto latestMessage = m_agentDataStruct.AggregateNewest(id);
        auto message = latestMessage["floatObs"].GetAvg();
        NS_LOG_INFO("Message: " << message);
    }

    void OnRecvReward(uint id) override
    {
        NS_LOG_INFO("Received Reward from " << id);
    }

    void PrintLatestMessages(uint id = 0)
    {
        std::cout << "Observations:" << std::endl;
        m_obsDataStruct.Print(std::cout);
        m_obsDataStruct.PrintHistory(std::cout, 0, ns3_ai_gym::Box);
        std::cout << "Agent messages:" << std::endl;
        m_agentDataStruct.Print(std::cout);
        m_agentDataStruct.PrintHistory(std::cout, id, ns3_ai_gym::Box);
    }

  protected:
    HistoryContainer m_agentDataStruct;

  private:
    Ptr<OpenGymSpace> GetObservationSpace() override
    {
        return {};
    }

    Ptr<OpenGymSpace> GetActionSpace() override
    {
        return {};
    }
};

NS_OBJECT_ENSURE_REGISTERED(ObservationSharingTestAgent);
} // namespace ns3

// run this example with 'ns3 run defiance-observation-sharing'

/**
 * @brief Main function for the observation sharing example.
 * This function creates an observation application and two agents. The observation application
 * sends 10 observations to agent0 who forwards them to agent1. Therefore agent0 receives 10
 * observations and no agent messages, while agent1 receives 10 agent messages and no observations.
 */
int
main(int argc, char* argv[])
{
    using namespace ns3;
    // enable this for more detailed logging
    // LogComponentEnable("AgentApplication", LOG_LEVEL_FUNCTION);
    LogComponentEnable("ObservationSharing", LOG_LEVEL_FUNCTION);

    auto observationApp0 = CreateObject<TestObservationApplication>();
    auto agent0 =
        CreateObjectWithAttributes<ObservationSharingTestAgent>("MaxObservationHistoryLength",
                                                                UintegerValue(10),
                                                                "MaxRewardHistoryLength",
                                                                UintegerValue(10));
    auto agent1 =
        CreateObjectWithAttributes<ObservationSharingTestAgent>("MaxObservationHistoryLength",
                                                                UintegerValue(10),
                                                                "MaxRewardHistoryLength",
                                                                UintegerValue(10));

    NodeContainer nodes{3};

    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Mode", StringValue("Time"));
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Time", StringValue("2s"));
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Speed",
                       StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Bounds", StringValue("0|200|0|200"));

    uint32_t seed = 0;
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

    // create interfaces
    auto channelInterfaceO_A = CreateObject<SimpleChannelInterface>();
    auto channelInterfaceA_O = CreateObject<SimpleChannelInterface>();
    auto channelInterface0_1 = CreateObject<SimpleChannelInterface>();
    auto channelInterface1_0 = CreateObject<SimpleChannelInterface>();

    channelInterfaceO_A->Connect(channelInterfaceA_O);
    channelInterface0_1->Connect(channelInterface1_0);

    // add interfaces
    observationApp0->AddAgentInterface(0, channelInterfaceO_A);
    observationApp0->Setup();
    auto interfaceId0 = agent0->AddObservationInterface(0, channelInterfaceA_O);
    NS_LOG_INFO("Interface ID: " << interfaceId0);

    agent0->AddAgentInterface(1, channelInterface0_1);
    agent0->SetId(RlApplicationId{AGENT, 0});
    agent0->Setup();
    agent1->AddAgentInterface(0, channelInterface1_0);
    agent1->SetId(RlApplicationId{AGENT, 1});
    agent1->Setup();

    // add applications to nodes
    nodes.Get(0)->AddApplication(observationApp0);
    nodes.Get(1)->AddApplication(agent0);
    nodes.Get(2)->AddApplication(agent1);

    observationApp0->SetStartTime(Seconds(0));
    observationApp0->SetStopTime(Seconds(10));
    agent0->SetStartTime(Seconds(0));
    agent0->SetStopTime(Seconds(10));
    agent1->SetStartTime(Seconds(0));
    agent1->SetStopTime(Seconds(10));

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Simulation ended.");

    NS_LOG_INFO("Agent 0:");
    agent0->PrintLatestMessages(1);
    NS_LOG_INFO("Agent 1:");
    agent1->PrintLatestMessages(0);

    return 0;
}
