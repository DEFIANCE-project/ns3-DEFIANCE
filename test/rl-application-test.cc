#include "ns3/callback.h"
#include "ns3/defiance-module.h"
#include "ns3/rl-application.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup defiance-tests
 * Child class of RlApplication that unifies functionality to create
 * the appropriate DictContainer and functionality to allow calling
 * the callback object
 */
class CustomRlApp : public RlApplication
{
  public:
    CustomRlApp();
    virtual ~CustomRlApp();
    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();

    void SendData(Ptr<OpenGymDictContainer> data,
                  const std::vector<Ptr<ChannelInterface>>& interfaces);
};

TypeId
CustomRlApp::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CustomRlApp").SetParent<RlApplication>().SetGroupName("defiance");
    return tid;
}

CustomRlApp::CustomRlApp()
{
}

CustomRlApp::~CustomRlApp()
{
}

void
CustomRlApp::SendData(Ptr<OpenGymDictContainer> data,
                      const std::vector<Ptr<ChannelInterface>>& interfaces)
{
    Send(data, interfaces);
}

/**
 * @ingroup defiance-tests
 * Test to check if data can be sent to a custom number of interfaces
 */

class RlAppSendTestCase : public RlAppBaseTestCase
{
  public:
    RlAppSendTestCase();
    virtual ~RlAppSendTestCase();

  private:
    void Simulate() override;
};

RlAppSendTestCase::RlAppSendTestCase()
    : RlAppBaseTestCase("Check if data can be sent to a custom number of interfaces")
{
}

RlAppSendTestCase::~RlAppSendTestCase()
{
}

void
RlAppSendTestCase::Simulate()
{
    // Create RlApplication without helper
    auto rlApp = CreateObject<CustomRlApp>();

    // Install RlApplication on Node
    m_node->AddApplication(rlApp);

    rlApp->SetStartTime(Seconds(0.0));
    rlApp->SetStopTime(Seconds(10.0));

    // Send data
    Simulator::Schedule(Seconds(5),
                        &CustomRlApp::SendData,
                        rlApp,
                        MakeDictBoxContainer<float>(1, "floatData", 42.0),
                        m_sendingInterfaces);

    for (auto& interface : m_sendingInterfaces)
    {
        Simulator::ScheduleDestroy(&ChannelInterface::Disconnect, interface);
    }

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[0], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[1], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[2], "received data equals sent data");
}

/**
 * @ingroup defiance-tests
 * Test to check if observation is impossible to send after stopping the application
 */

class RlAppStopTestCase : public RlAppBaseTestCase
{
  public:
    RlAppStopTestCase();
    virtual ~RlAppStopTestCase();

  private:
    void Simulate() override;
};

RlAppStopTestCase::RlAppStopTestCase()
    : RlAppBaseTestCase("Check if observation cannot be sent if app is stopped")
{
}

RlAppStopTestCase::~RlAppStopTestCase()
{
}

void
RlAppStopTestCase::Simulate()
{
    // Create RlApplication without helper
    auto rlApp = CreateObject<CustomRlApp>();

    // Install RlApplication on Node
    m_node->AddApplication(rlApp);

    rlApp->SetStartTime(Seconds(0.0));
    rlApp->SetStopTime(Seconds(10.0));

    // Send data
    std::vector<Ptr<ChannelInterface>> interface0{m_sendingInterfaces[0]};
    Simulator::Schedule(Seconds(5),
                        &CustomRlApp::SendData,
                        rlApp,
                        MakeDictBoxContainer<float>(1, "floatData", 42.0),
                        interface0);
    std::vector<Ptr<ChannelInterface>> interface2{m_sendingInterfaces[2]};
    Simulator::Schedule(Seconds(12),
                        &CustomRlApp::SendData,
                        rlApp,
                        MakeDictBoxContainer<float>(1, "floatData", 43.0),
                        interface2);

    for (auto& interface : m_sendingInterfaces)
    {
        Simulator::ScheduleDestroy(&ChannelInterface::Disconnect, interface);
    }

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[0], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(0.0, m_receivedData[2], "received data equals sent data");
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for RlApp
 */
class RlAppTestSuite : public TestSuite
{
  public:
    RlAppTestSuite();

  protected:
    void DoTeardown() override;
};

RlAppTestSuite::RlAppTestSuite()
    : TestSuite("defiance-rl-application", Type::UNIT)
{
    AddTestCase(new RlAppSendTestCase, Duration::QUICK);
    AddTestCase(new RlAppStopTestCase, Duration::QUICK);
}

void
RlAppTestSuite::DoTeardown()
{
    Simulator::Destroy();
}

static RlAppTestSuite sRlAppTestSuite; //!< Static variable for test initialization
