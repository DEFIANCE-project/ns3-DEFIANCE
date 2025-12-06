#include "ns3/callback.h"
#include "ns3/defiance-module.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup defiance-tests
 *
 */
class ActionAppTestCase : public TestCase
{
  public:
    ActionAppTestCase();
    virtual ~ActionAppTestCase();

  protected:
    void DoRun() override;
};

ActionAppTestCase::ActionAppTestCase()
    : TestCase("Check if actions can be sent to the ActionApplication")
{
}

ActionAppTestCase::~ActionAppTestCase()
{
}

void
ActionAppTestCase::DoRun()
{
    auto actionAppNode = CreateObject<Node>();
    auto actionApp = CreateObject<TestActionApp>();
    actionAppNode->AddApplication(actionApp);

    auto sendingChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto sendingChannelInterface2 = CreateObject<SimpleChannelInterface>();

    auto receivingChannelInterface1 = CreateObject<SimpleChannelInterface>();
    auto receivingChannelInterface2 = CreateObject<SimpleChannelInterface>();

    sendingChannelInterface1->Connect(receivingChannelInterface1);
    sendingChannelInterface2->Connect(receivingChannelInterface2);

    Simulator::ScheduleDestroy(&SimpleChannelInterface::Disconnect, sendingChannelInterface1);
    Simulator::ScheduleDestroy(&SimpleChannelInterface::Disconnect, sendingChannelInterface2);

    actionApp->AddAgentInterface(43, receivingChannelInterface1);
    actionApp->AddAgentInterface(7, receivingChannelInterface2);

    actionApp->SetStartTime(Seconds(0.0));
    actionApp->SetStopTime(Seconds(10.0));

    // Send action
    Simulator::Schedule(Seconds(5),
                        &SimpleChannelInterface::Send,
                        sendingChannelInterface1,
                        MakeDictBoxContainer<float>(1, "floatAct", 42.0));
    Simulator::Schedule(Seconds(5),
                        &SimpleChannelInterface::Send,
                        sendingChannelInterface2,
                        MakeDictBoxContainer<float>(1, "floatAct", 43.0));
    Simulator::Schedule(Seconds(5),
                        &SimpleChannelInterface::Send,
                        sendingChannelInterface2,
                        MakeDictBoxContainer<float>(1, "floatAct", 44.0));

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(42.0,
                          actionApp->GetAction()[43][0],
                          "received action equals sent action");
    NS_TEST_ASSERT_MSG_EQ(43.0, actionApp->GetAction()[7][0], "received action equals sent action");
    NS_TEST_ASSERT_MSG_EQ(2,
                          actionApp->GetAction()[7].size(),
                          "number of received actions equals number of sent actions");
    NS_TEST_ASSERT_MSG_EQ(44.0, actionApp->GetAction()[7][1], "received action equals sent action");
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for ActionApp
 */
class ActionAppTestSuite : public TestSuite
{
  public:
    ActionAppTestSuite();
};

ActionAppTestSuite::ActionAppTestSuite()
    : TestSuite("defiance-action-application", Type::UNIT)
{
    AddTestCase(new ActionAppTestCase, Duration::QUICK);
}

static ActionAppTestSuite sActionAppTestSuite; //!< Static variable for test initialization
