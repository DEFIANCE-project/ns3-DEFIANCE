#include "ns3/callback.h"
#include "ns3/defiance-module.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup defiance-tests
 * Test to check if data can be sent to a single interface
 */
class DataCollectorAppOneInterfaceTestCase : public RlAppBaseTestCase
{
  public:
    DataCollectorAppOneInterfaceTestCase();
    virtual ~DataCollectorAppOneInterfaceTestCase();

  private:
    void Simulate() override;
};

DataCollectorAppOneInterfaceTestCase::DataCollectorAppOneInterfaceTestCase()
    : RlAppBaseTestCase("Check if data can be sent to a single interface")
{
}

DataCollectorAppOneInterfaceTestCase::~DataCollectorAppOneInterfaceTestCase()
{
}

void
DataCollectorAppOneInterfaceTestCase::Simulate()
{
    // Create DataCollectorApp without helper
    Ptr<TestDataCollectorApp> dataCollectorApp = CreateObject<TestDataCollectorApp>();
    for (uint i = 0; i < m_sendingInterfaces.size(); ++i)
    {
        uint appId = (i == 1) ? 2 : 5;
        dataCollectorApp->AddInterface({AGENT, appId}, m_sendingInterfaces[i]);
    }
    dataCollectorApp->Setup();
    // Install DataCollectorApp on Node
    m_node->AddApplication(dataCollectorApp);

    dataCollectorApp->SetStartTime(Seconds(0.0));
    dataCollectorApp->SetStopTime(Seconds(10.0));

    // Send data
    Simulator::Schedule(Seconds(5),
                        &TestDataCollectorApp::ExecuteCallback,
                        dataCollectorApp,
                        42.0,
                        5,
                        1);

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(0.0, m_receivedData[0], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(0.0, m_receivedData[1], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[2], "received data equals sent data");
}

/**
 * @ingroup defiance-tests
 * Test to check if data can be sent to all interfaces
 */
class DataCollectorAppAllInterfacesTestCase : public RlAppBaseTestCase
{
  public:
    DataCollectorAppAllInterfacesTestCase();
    virtual ~DataCollectorAppAllInterfacesTestCase();

  private:
    void Simulate() override;
};

DataCollectorAppAllInterfacesTestCase::DataCollectorAppAllInterfacesTestCase()
    : RlAppBaseTestCase("Check if data can be sent to all interfaces")
{
}

DataCollectorAppAllInterfacesTestCase::~DataCollectorAppAllInterfacesTestCase()
{
}

void
DataCollectorAppAllInterfacesTestCase::Simulate()
{
    // Create DataCollectorApp without helper
    Ptr<TestDataCollectorApp> dataCollectorApp = CreateObject<TestDataCollectorApp>();
    for (uint i = 0; i < m_sendingInterfaces.size(); ++i)
    {
        uint appId = (i == 1) ? 2 : 5;
        dataCollectorApp->AddInterface({AGENT, appId}, m_sendingInterfaces[i]);
    }
    dataCollectorApp->Setup();
    // Install DataCollectorApp on Node
    m_node->AddApplication(dataCollectorApp);

    dataCollectorApp->SetStartTime(Seconds(0.0));
    dataCollectorApp->SetStopTime(Seconds(10.0));

    // Send data
    Simulator::Schedule(Seconds(5),
                        &TestDataCollectorApp::ExecuteCallback,
                        dataCollectorApp,
                        42.0,
                        -1,
                        -1);

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[0], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[1], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[2], "received data equals sent data");
}

/**
 * @ingroup defiance-tests
 * Test to check if data can be sent to a custom number of interfaces
 */

class DataCollectorAppOneAppTestCase : public RlAppBaseTestCase
{
  public:
    DataCollectorAppOneAppTestCase();
    virtual ~DataCollectorAppOneAppTestCase();

  private:
    void Simulate() override;
};

DataCollectorAppOneAppTestCase::DataCollectorAppOneAppTestCase()
    : RlAppBaseTestCase("Check if data can be sent to a custom number of interfaces")
{
}

DataCollectorAppOneAppTestCase::~DataCollectorAppOneAppTestCase()
{
}

void
DataCollectorAppOneAppTestCase::Simulate()
{
    // Create DataCollectorApp without helper
    Ptr<TestDataCollectorApp> dataCollectorApp = CreateObject<TestDataCollectorApp>();
    for (uint i = 0; i < m_sendingInterfaces.size(); ++i)
    {
        uint appId = (i == 1) ? 2 : 5;
        dataCollectorApp->AddInterface({AGENT, appId}, m_sendingInterfaces[i]);
    }
    dataCollectorApp->Setup();
    // Install DataCollectorApp on Node
    m_node->AddApplication(dataCollectorApp);

    dataCollectorApp->SetStartTime(Seconds(0.0));
    dataCollectorApp->SetStopTime(Seconds(10.0));

    // Send data
    Simulator::Schedule(Seconds(5),
                        &TestDataCollectorApp::ExecuteCallback,
                        dataCollectorApp,
                        42.0,
                        5,
                        -1);

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[0], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(0.0, m_receivedData[1], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[2], "received data equals sent data");
}

/**
 * @ingroup defiance-tests
 * Test to check if interfaces can be deleted and added after setup
 */

class DataCollectorAppDynamicTestCase : public RlAppBaseTestCase
{
  public:
    DataCollectorAppDynamicTestCase();
    virtual ~DataCollectorAppDynamicTestCase();

  private:
    void Simulate() override;
};

DataCollectorAppDynamicTestCase::DataCollectorAppDynamicTestCase()
    : RlAppBaseTestCase("Check if interfaces can be deleted and added after setup")
{
}

DataCollectorAppDynamicTestCase::~DataCollectorAppDynamicTestCase()
{
}

void
DataCollectorAppDynamicTestCase::Simulate()
{
    // Create DataCollectorApp without helper
    Ptr<TestDataCollectorApp> dataCollectorApp = CreateObject<TestDataCollectorApp>();
    uint interfaceId1 = dataCollectorApp->AddInterface({AGENT, 72}, m_sendingInterfaces[0]);
    NS_TEST_ASSERT_MSG_EQ(0, interfaceId1, "InterfaceID 0 is set correctly.");

    dataCollectorApp->Setup();

    // Install DataCollectorApp on Node
    m_node->AddApplication(dataCollectorApp);

    dataCollectorApp->SetStartTime(Seconds(0.0));
    dataCollectorApp->SetStopTime(Seconds(20.0));

    // Add interface after Setup
    uint interfaceId2 = dataCollectorApp->AddInterface({AGENT, 72}, m_sendingInterfaces[1]);
    NS_TEST_ASSERT_MSG_EQ(1, interfaceId2, "InterfaceID 1 is set correctly.");

    dataCollectorApp->DeleteAgentInterface(72, interfaceId1);

    // Send data
    Simulator::Schedule(Seconds(5),
                        &TestDataCollectorApp::ExecuteCallback,
                        dataCollectorApp,
                        42.0,
                        72,
                        -1);

    Simulator::Schedule(Seconds(6),
                        [&] { dataCollectorApp->DeleteAgentInterface(72, interfaceId2); });
    Simulator::Schedule(Seconds(7),
                        &TestDataCollectorApp::ExecuteCallback,
                        dataCollectorApp,
                        4.0,
                        72,
                        -1);
    Simulator::Schedule(Seconds(10), [&] {
        uint interfaceId3 = dataCollectorApp->AddInterface({AGENT, 3}, m_sendingInterfaces[2]);
        NS_TEST_ASSERT_MSG_EQ(0, interfaceId3, "InterfaceID 3 is set correctly.");
        Simulator::Schedule(Seconds(5),
                            &TestDataCollectorApp::ExecuteCallback,
                            dataCollectorApp,
                            48.0,
                            3,
                            interfaceId3);
    });

    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(0.0, m_receivedData[0], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(42.0, m_receivedData[1], "received data equals sent data");
    NS_TEST_ASSERT_MSG_EQ(48.0, m_receivedData[2], "received data equals sent data");
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for DataCollectorApp
 */
class DataCollectorAppTestSuite : public TestSuite
{
  public:
    DataCollectorAppTestSuite();

  protected:
    void DoTeardown() override;
};

DataCollectorAppTestSuite::DataCollectorAppTestSuite()
    : TestSuite("defiance-data-collector-application", Type::UNIT)
{
    AddTestCase(new DataCollectorAppOneInterfaceTestCase, Duration::QUICK);
    AddTestCase(new DataCollectorAppAllInterfacesTestCase, Duration::QUICK);
    AddTestCase(new DataCollectorAppOneAppTestCase, Duration::QUICK);
    AddTestCase(new DataCollectorAppDynamicTestCase, Duration::QUICK);
}

void
DataCollectorAppTestSuite::DoTeardown()
{
    Simulator::Destroy();
}

static DataCollectorAppTestSuite
    sDataCollectorAppTestSuite; //!< Static variable for test initialization
