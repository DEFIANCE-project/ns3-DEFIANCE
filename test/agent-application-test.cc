#include "ns3/callback.h"
#include "ns3/defiance-module.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup defiance-tests
 *
 * Test to check if AgentApplication can communicate with registered interfaces of all app types
 */
class AgentAppTestCase : public TestCase
{
  public:
    AgentAppTestCase();
    virtual ~AgentAppTestCase();

  protected:
    void DoRun() override;
    void DoTeardown() override;
    void ReceiveAction(int id, Ptr<OpenGymDictContainer> data);
    void ReceiveMessage(Ptr<OpenGymDictContainer> data);

  private:
    std::map<uint, float> m_actions;
    float m_agentMessage;
};

AgentAppTestCase::AgentAppTestCase()
    : TestCase(
          "Check if AgentApplication can communicate with registered interfaces of all app types.")
{
}

AgentAppTestCase::~AgentAppTestCase()
{
}

void
AgentAppTestCase::DoTeardown()
{
    Simulator::Destroy();
}

void
AgentAppTestCase::ReceiveAction(int id, Ptr<OpenGymDictContainer> data)
{
    auto boxContainer = DynamicCast<OpenGymBoxContainer<float>>(data->Get("floatAct"));
    m_actions[id] = boxContainer->GetValue(0);
}

void
AgentAppTestCase::ReceiveMessage(Ptr<OpenGymDictContainer> data)
{
    auto boxContainer = DynamicCast<OpenGymBoxContainer<float>>(data->Get("floatMessage"));
    m_agentMessage = boxContainer->GetValue(0);
}

void
AgentAppTestCase::DoRun()
{
    auto agentAppNode = CreateObject<Node>();
    auto agentApp = CreateObject<TestAgentApp>();
    agentAppNode->AddApplication(agentApp);
    agentApp->SetId(RlApplicationId{AGENT, 1});
    agentApp->Setup();

    // Create ObservationInterfaces
    auto sendingObservationChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto sendingObservationChannelInterface2 = CreateObject<SimpleChannelInterface>();
    auto receivingObservationChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto receivingObservationChannelInterface2 = CreateObject<SimpleChannelInterface>();

    // Create RewardInterfaces
    auto sendingRewardChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto sendingRewardChannelInterface2 = CreateObject<SimpleChannelInterface>();
    auto receivingRewardChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto receivingRewardChannelInterface2 = CreateObject<SimpleChannelInterface>();

    // Create AgentInterfaces
    auto sendingAgentChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto sendingAgentChannelInterface2 = CreateObject<SimpleChannelInterface>();
    auto sendingAgentChannelInterface3 = CreateObject<SimpleChannelInterface>();
    auto receivingAgentChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto receivingAgentChannelInterface2 = CreateObject<SimpleChannelInterface>();
    auto receivingAgentChannelInterface3 = CreateObject<SimpleChannelInterface>();

    // Create ActionInterfaces
    auto sendingActionChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto sendingActionChannelInterface2 = CreateObject<SimpleChannelInterface>();
    auto receivingActionChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto receivingActionChannelInterface2 = CreateObject<SimpleChannelInterface>();

    // Connect and register ObservationInterfaces
    sendingObservationChannelInterface1->Connect(receivingObservationChannelInterface1);
    sendingObservationChannelInterface2->Connect(receivingObservationChannelInterface2);
    agentApp->AddObservationInterface(3, receivingObservationChannelInterface1);
    agentApp->AddObservationInterface(500, receivingObservationChannelInterface2);

    // Connect and register RewardInterfaces
    sendingRewardChannelInterface1->Connect(receivingRewardChannelInterface1);
    sendingRewardChannelInterface2->Connect(receivingRewardChannelInterface2);
    agentApp->AddRewardInterface(73, receivingRewardChannelInterface1);
    agentApp->AddRewardInterface(48, receivingRewardChannelInterface2);

    // Connect and register AgentInterfaces
    sendingAgentChannelInterface1->Connect(receivingAgentChannelInterface1);
    sendingAgentChannelInterface2->Connect(receivingAgentChannelInterface2);
    sendingAgentChannelInterface3->Connect(receivingAgentChannelInterface3);
    agentApp->AddAgentInterface(85, receivingAgentChannelInterface1);
    agentApp->AddAgentInterface(3, receivingAgentChannelInterface2);
    agentApp->AddAgentInterface(5, sendingAgentChannelInterface3);
    receivingAgentChannelInterface3->AddRecvCallback(
        MakeCallback(&AgentAppTestCase::ReceiveMessage, this));

    // Connect and register ActionInterfaces
    sendingActionChannelInterface1->Connect(receivingActionChannelInterface1);
    sendingActionChannelInterface2->Connect(receivingActionChannelInterface2);
    agentApp->AddActionInterface(83, sendingActionChannelInterface1);
    agentApp->AddActionInterface(45, sendingActionChannelInterface2);
    receivingActionChannelInterface1->AddRecvCallback(
        MakeCallback(&AgentAppTestCase::ReceiveAction, this, 83));
    receivingActionChannelInterface2->AddRecvCallback(
        MakeCallback(&AgentAppTestCase::ReceiveAction, this, 45));

    agentApp->SetStartTime(Seconds(0.0));
    agentApp->SetStopTime(Seconds(20.0));

    // Send observations to AgentApplication
    Simulator::Schedule(Seconds(5),
                        &SimpleChannelInterface::Send,
                        sendingObservationChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatData", 42.0));
    Simulator::Schedule(Seconds(6),
                        &SimpleChannelInterface::Send,
                        sendingObservationChannelInterface2,
                        MakeDictBoxContainer<float>(1, "floatData", 43.0));
    Simulator::Schedule(Seconds(7),
                        &SimpleChannelInterface::Send,
                        sendingObservationChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatData", 44.0));

    // Send rewards to AgentApplication
    Simulator::Schedule(Seconds(5),
                        &SimpleChannelInterface::Send,
                        sendingRewardChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatData", 45.0));
    Simulator::Schedule(Seconds(6),
                        &SimpleChannelInterface::Send,
                        sendingRewardChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatData", 46.0));
    Simulator::Schedule(Seconds(7),
                        &SimpleChannelInterface::Send,
                        sendingRewardChannelInterface2,
                        MakeDictBoxContainer<float>(1, "floatData", 47.0));

    // Send agent messages to AgentApplication
    Simulator::Schedule(Seconds(5),
                        &SimpleChannelInterface::Send,
                        sendingAgentChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatMessage", 48.0));
    Simulator::Schedule(Seconds(6),
                        &SimpleChannelInterface::Send,
                        sendingAgentChannelInterface2,
                        MakeDictBoxContainer<float>(1, "floatMessage", 49.0));
    // Send agent message from AgentApplication to all connected interfaces
    Simulator::Schedule(Seconds(6), [&] {
        agentApp->SendToAgent(MakeDictBoxContainer<float>(1, "floatMessage", 50.0));
    });
    // Send agent message to AgentApplication
    Simulator::Schedule(Seconds(7),
                        &SimpleChannelInterface::Send,
                        sendingAgentChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatMessage", 51.0));
    // Send agent message from AgentApplication to one specific interface
    Simulator::Schedule(Seconds(8), [&] {
        agentApp->SendToAgent(MakeDictBoxContainer<float>(1, "floatMessage", 55.0), 5, 0);
    });
    // Send agent message from AgentApplication to all interfaces of one App
    Simulator::Schedule(Seconds(10), [&] {
        agentApp->SendToAgent(MakeDictBoxContainer<float>(1, "floatMessage", 56.0), 5);
    });

    // Send actions from AgentApplication
    Simulator::Schedule(Seconds(5), [&] {
        agentApp->SendAction(MakeDictBoxContainer<float>(1, "floatAct", 52.0), 83);
    });
    Simulator::Schedule(Seconds(6), [&] {
        agentApp->SendAction(MakeDictBoxContainer<float>(1, "floatAct", 53.0), 45);
    });
    Simulator::Schedule(Seconds(7), [&] {
        agentApp->SendAction(MakeDictBoxContainer<float>(1, "floatAct", 54.0), 83);
    });

    Simulator::Schedule(Seconds(6), [&] {
        NS_TEST_ASSERT_MSG_EQ(42.0,
                              agentApp->GetObservation()[3][0],
                              "observation received at AgentApplication equals sent observation");
        NS_TEST_ASSERT_MSG_EQ(45.0,
                              agentApp->GetReward()[73][0],
                              "reward received at AgentApplication equals sent reward");
        NS_TEST_ASSERT_MSG_EQ(48.0,
                              agentApp->GetMessage()[85][0],
                              "message received at AgentApplication equals sent agent message");
        NS_TEST_ASSERT_MSG_EQ(52.0,
                              m_actions[83],
                              "action received at AgentApplication equals sent action");
    });

    Simulator::Schedule(Seconds(7), [&] {
        NS_TEST_ASSERT_MSG_EQ(43.0,
                              agentApp->GetObservation()[500][0],
                              "observation received at AgentApplication equals sent observation");
        NS_TEST_ASSERT_MSG_EQ(46.0,
                              agentApp->GetReward()[73][1],
                              "reward received at AgentApplication equals sent reward");
        NS_TEST_ASSERT_MSG_EQ(49.0,
                              agentApp->GetMessage()[3][0],
                              "received agent message equals sent agent message");
        NS_TEST_ASSERT_MSG_EQ(50.0,
                              m_agentMessage,
                              "received agent message equals message sent from AgentApplication");
        NS_TEST_ASSERT_MSG_EQ(53.0,
                              m_actions[45],
                              "action received at AgentApplication equals sent action");
    });

    Simulator::Schedule(Seconds(8), [&] {
        NS_TEST_ASSERT_MSG_EQ(44.0,
                              agentApp->GetObservation()[3][1],
                              "observation received at AgentApplication equals sent observation");
        NS_TEST_ASSERT_MSG_EQ(47.0,
                              agentApp->GetReward()[48][0],
                              "reward received at AgentApplication equals sent reward");
        NS_TEST_ASSERT_MSG_EQ(51.0,
                              agentApp->GetMessage()[85][1],
                              "message received at AgentApplication equals sent agent message");
        NS_TEST_ASSERT_MSG_EQ(54.0,
                              m_actions[83],
                              "action received at AgentApplication equals sent action");
    });

    Simulator::Schedule(Seconds(9), [&] {
        NS_TEST_ASSERT_MSG_EQ(55.0,
                              m_agentMessage,
                              "received agent message equals message sent from AgentApplication");
    });

    Simulator::Schedule(Seconds(11), [&] {
        NS_TEST_ASSERT_MSG_EQ(56.0,
                              m_agentMessage,
                              "received agent message equals message sent from AgentApplication");
    });

    Simulator::Run();
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for AgentApp
 */
class AgentAppTestSuite : public TestSuite
{
  public:
    AgentAppTestSuite();
};

AgentAppTestSuite::AgentAppTestSuite()
    : TestSuite("defiance-agent-application", Type::UNIT)
{
    AddTestCase(new AgentAppTestCase, Duration::QUICK);
}

static AgentAppTestSuite sAgentAppTestSuite; //!< Static variable for test initialization
