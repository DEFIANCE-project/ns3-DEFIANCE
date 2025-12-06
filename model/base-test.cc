#include "base-test.h"

#include "simple-channel-interface.h"

namespace ns3
{

Ptr<OpenGymDictContainer>
MakeDictContainer(std::string key, Ptr<OpenGymDataContainer> data)
{
    auto dictContainer = CreateObject<OpenGymDictContainer>();
    dictContainer->Add(key, data);
    return dictContainer;
}

Callback<void, Ptr<OpenGymDataContainer>> noopCallback =
    MakeCallback(*[](Ptr<OpenGymDataContainer>) {});

RlAppBaseTestCase::RlAppBaseTestCase(std::string name)
    : TestCase(name)
{
    m_receivedData = {0.0, 0.0, 0.0};
}

RlAppBaseTestCase::~RlAppBaseTestCase()
{
}

void
RlAppBaseTestCase::DoTeardown()
{
    Simulator::Destroy();
}

void
RlAppBaseTestCase::ReceiveData(int id, Ptr<OpenGymDictContainer> data)
{
    auto boxContainer = data->Get("floatData")->GetObject<OpenGymBoxContainer<float>>();
    m_receivedData[id] = boxContainer->GetValue(0);
}

void
RlAppBaseTestCase::CreateInterfaces()
{
    auto sendingChannelInterface = CreateObject<SimpleChannelInterface>();
    auto receivingChannelInterface = CreateObject<SimpleChannelInterface>();
    m_sendingInterfaces.emplace_back(sendingChannelInterface);
    m_receivingInterfaces.emplace_back(receivingChannelInterface);
}

void
RlAppBaseTestCase::ScenarioSetup()
{
    // Create Nodes
    m_node = CreateObject<Node>();
    auto receivingNode = CreateObject<Node>();
    NodeContainer nodes;
    nodes.Add(m_node);
    nodes.Add(receivingNode);

    for (int i = 0; i < 3; i++)
    {
        CreateInterfaces();

        // Register Callback which receives data
        m_receivingInterfaces[i]->AddRecvCallback(
            MakeCallback(&RlAppBaseTestCase::ReceiveData, this, i));

        // Connect ChannelInterfaces
        (DynamicCast<SimpleChannelInterface>(m_sendingInterfaces[i]))
            ->Connect(DynamicCast<SimpleChannelInterface>(m_receivingInterfaces[i]));
    }
}

void
RlAppBaseTestCase::DoRun()
{
    ScenarioSetup();
    Simulate();
}

NS_OBJECT_ENSURE_REGISTERED(TestDataCollectorApp);

TypeId
TestDataCollectorApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestDataCollectorApp")
                            .SetParent<DataCollectorApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestDataCollectorApp>();
    return tid;
}

TestDataCollectorApp::TestDataCollectorApp()
{
}

TestDataCollectorApp::~TestDataCollectorApp()
{
}

void
TestDataCollectorApp::ExecuteCallback(float arg, int arg2, int arg3)
{
    m_callback(arg, arg2, arg3);
}

void
TestDataCollectorApp::CollectData(float data, int appId, int interfaceIndex)
{
    if (appId < 0)
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", data));
    }
    else if (interfaceIndex < 0)
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", data), appId);
    }
    else
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", data), appId, interfaceIndex);
    }
}

void
TestDataCollectorApp::RegisterCallbacks()
{
    m_callback = MakeCallback(&TestDataCollectorApp::CollectData, this);
}

NS_OBJECT_ENSURE_REGISTERED(TestObservationApp);

TypeId
TestObservationApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestObservationApp")
                            .SetParent<ObservationApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestObservationApp>();
    return tid;
}

TestObservationApp::TestObservationApp()
{
}

TestObservationApp::~TestObservationApp()
{
}

void
TestObservationApp::ExecuteCallback(float arg, int arg2, int arg3)
{
    m_callback(arg, arg2, arg3);
}

void
TestObservationApp::Observe(float observation, int appId, int interfaceIndex)
{
    if (appId < 0)
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", observation));
    }
    else if (interfaceIndex < 0)
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", observation), appId);
    }
    else
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", observation), appId, interfaceIndex);
    }
}

void
TestObservationApp::RegisterCallbacks()
{
    m_callback = MakeCallback(&TestObservationApp::Observe, this);
}

NS_OBJECT_ENSURE_REGISTERED(TestRewardApp);

TypeId
TestRewardApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestRewardApp")
                            .SetParent<RewardApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestRewardApp>();
    return tid;
}

TestRewardApp::TestRewardApp()
{
}

TestRewardApp::~TestRewardApp()
{
}

void
TestRewardApp::ExecuteCallback(float arg, int arg2, int arg3)
{
    m_callback(arg, arg2, arg3);
}

void
TestRewardApp::Reward(float observation, int appId, int interfaceIndex)
{
    if (appId < 0)
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", observation));
    }
    else if (interfaceIndex < 0)
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", observation), appId);
    }
    else
    {
        Send(MakeDictBoxContainer<float>(1, "floatData", observation), appId, interfaceIndex);
    }
}

void
TestRewardApp::RegisterCallbacks()
{
    m_callback = MakeCallback(&TestRewardApp::Reward, this);
}

NS_OBJECT_ENSURE_REGISTERED(TestAgentApp);

TypeId
TestAgentApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestAgentApp")
                            .SetParent<AgentApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestAgentApp>();
    return tid;
}

std::map<uint, std::vector<float>>
TestAgentApp::GetObservation() const
{
    return m_observation;
}

std::map<uint, std::vector<float>>
TestAgentApp::GetReward() const
{
    return m_reward;
}

std::map<uint, std::vector<float>>
TestAgentApp::GetMessage() const
{
    return m_agentMessages;
}

void
TestAgentApp::OnRecvObs(uint remoteAppId)
{
    auto value = m_obsDataStruct.AggregateNewest(remoteAppId, 1)["floatData"].GetAvg();
    m_observation[remoteAppId].emplace_back(value);
}

void
TestAgentApp::OnRecvReward(uint remoteAppId)
{
    m_reward[remoteAppId].emplace_back(
        m_rewardDataStruct.AggregateNewest(remoteAppId, 1)["floatData"].GetAvg());
}

void
TestAgentApp::OnRecvFromAgent(uint remoteAppId, Ptr<OpenGymDictContainer> payload)
{
    auto boxContainer = DynamicCast<OpenGymBoxContainer<float>>(payload->Get("floatMessage"));
    m_agentMessages[remoteAppId].emplace_back(boxContainer->GetValue(0));
}

Ptr<OpenGymSpace>
TestAgentApp::GetObservationSpace()
{
    return {};
};

Ptr<OpenGymSpace>
TestAgentApp::GetActionSpace()
{
    return {};
};

NS_OBJECT_ENSURE_REGISTERED(TestActionApp);

TestActionApp::TestActionApp()
{
}

TestActionApp::~TestActionApp()
{
}

TypeId
TestActionApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestActionApp")
                            .SetParent<ActionApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestActionApp>();
    return tid;
}

void
TestActionApp::ExecuteAction(uint remoteAppId, Ptr<OpenGymDictContainer> action)
{
    auto value = DynamicCast<OpenGymBoxContainer<float>>(action->Get("floatAct"))->GetValue(0);
    m_action[remoteAppId].emplace_back(value);
}

std::map<uint, std::vector<float>>
TestActionApp::GetAction() const
{
    return m_action;
}

} // namespace ns3
