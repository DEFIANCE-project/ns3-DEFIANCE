#include "ns3/defiance-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/rl-application-helper.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup defiance-tests
 *
 * Test to check if communication relationships between all app types can be established via the
 * CommunicationHelper
 */
class CommunicationTestCase : public TestCase
{
  public:
    CommunicationTestCase();
    virtual ~CommunicationTestCase();

  protected:
    void DoRun() override;
};

CommunicationTestCase::CommunicationTestCase()
    : TestCase("Check if communication relationships between all app types can be established via "
               "the CommunicationHelper.")
{
}

CommunicationTestCase::~CommunicationTestCase()
{
}

void
CommunicationTestCase::DoRun()
{
    NodeContainer nodes;
    nodes.Create(5);

    RlApplicationHelper helper(TypeId::LookupByName("ns3::TestObservationApp"));
    helper.SetAttribute("StartTime", TimeValue(Seconds(0)));
    helper.SetAttribute("StopTime", TimeValue(Seconds(20)));
    RlApplicationContainer observationApps = helper.Install(nodes.Get(0));
    helper.SetTypeId(TypeId::LookupByName("ns3::TestRewardApp"));
    RlApplicationContainer rewardApps = helper.Install(nodes.Get(1));
    helper.SetTypeId(TypeId::LookupByName("ns3::TestAgentApp"));
    RlApplicationContainer agentApps = helper.Install({nodes.Get(2), nodes.Get(3)});
    helper.SetTypeId(TypeId::LookupByName("ns3::TestActionApp"));
    RlApplicationContainer actionApps = helper.Install({nodes.Get(4)});

    CommunicationHelper commHelper = CommunicationHelper();

    commHelper.SetObservationApps(observationApps);
    commHelper.SetRewardApps(rewardApps);
    commHelper.SetAgentApps(agentApps);
    commHelper.SetActionApps(actionApps);
    commHelper.SetIds();

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    auto observationDevices = p2p.Install({nodes.Get(0), nodes.Get(2)});
    auto rewardDevices = p2p.Install({nodes.Get(1), nodes.Get(2)});
    auto agentDevices = p2p.Install({nodes.Get(3), nodes.Get(2)});
    auto actionDevices = p2p.Install({nodes.Get(4), nodes.Get(2)});
    InternetStackHelper internet;
    internet.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer observationInterfaces = address.Assign(observationDevices);
    Ipv4InterfaceContainer rewardInterfaces = address.Assign(rewardDevices);
    Ipv4InterfaceContainer agentInterfaces = address.Assign(agentDevices);
    Ipv4InterfaceContainer actionInterfaces = address.Assign(actionDevices);
    commHelper.Configure();

    commHelper.AddCommunication(
        {{observationApps.GetId(0),
          agentApps.GetId(0),
          SocketCommunicationAttributes{observationInterfaces.GetAddress(0),
                                        observationInterfaces.GetAddress(1)}},
         {observationApps.GetId(0), agentApps.GetId(1), {}},
         {rewardApps.GetId(0),
          agentApps.GetId(0),
          SocketCommunicationAttributes{rewardInterfaces.GetAddress(0),
                                        rewardInterfaces.GetAddress(1)}},
         {rewardApps.GetId(0), agentApps.GetId(1), {}},
         {agentApps.GetId(0),
          actionApps.GetId(0),
          SocketCommunicationAttributes{actionInterfaces.GetAddress(1),
                                        actionApps.Get(0)->GetDefaultAddress()}},
         {agentApps.GetId(0),
          agentApps.GetId(1),
          SocketCommunicationAttributes{agentInterfaces.GetAddress(1),
                                        agentInterfaces.GetAddress(0)}}}

    );

    commHelper.Configure();

    Simulator::Schedule(Seconds(3),
                        &TestObservationApp::ExecuteCallback,
                        DynamicCast<TestObservationApp>(observationApps.Get(0)),
                        47.0,
                        -1,
                        -1);
    Simulator::Schedule(Seconds(3.1),
                        &TestRewardApp::ExecuteCallback,
                        DynamicCast<TestRewardApp>(rewardApps.Get(0)),
                        48.0,
                        agentApps.GetId(0).applicationId, // send over SocketChannelInterface
                        -1);

    Simulator::Schedule(Seconds(3.1),
                        &TestRewardApp::ExecuteCallback,
                        DynamicCast<TestRewardApp>(rewardApps.Get(0)),
                        49.0,
                        agentApps.GetId(1).applicationId, // send over SimpleChannelInterface
                        -1);
    Simulator::Schedule(Seconds(3.2), // use default address
                        [&] {
                            DynamicCast<TestAgentApp>(agentApps.Get(0))
                                ->SendAction(MakeDictBoxContainer<float>(1, "floatAct", 7.5));
                        });
    Simulator::Schedule(Seconds(3.2), [&] {
        DynamicCast<TestAgentApp>(agentApps.Get(1))
            ->SendToAgent(MakeDictBoxContainer<float>(1, "floatMessage", 7));
    });

    Simulator::Schedule(Seconds(4), [&] {
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(0))->GetObservation()[0][0],
                              47.0,
                              "received observation equals sent observation");
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(1))->GetObservation()[0][0],
                              47.0,
                              "received observation equals sent observation");
    });
    Simulator::Schedule(Seconds(4.1), [&] {
        // SocketChannelInterface connection
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(0))
                                  ->GetReward()[rewardApps.GetId(0).applicationId][0],
                              48.0,
                              "received reward equals sent reward");
        // SimpleChannelInterface Connection
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(1))
                                  ->GetReward()[rewardApps.GetId(0).applicationId][0],
                              49.0,
                              "received reward equals sent reward");
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestActionApp>(actionApps.Get(0))
                                  ->GetAction()[agentApps.GetId(0).applicationId][0],
                              7.5,
                              "received action equals sent action");

        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(0))
                                  ->GetMessage()[agentApps.GetId(1).applicationId][0],
                              7,
                              "received action equals sent action");
    });

    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
}

/**
 * @ingroup DEFIANCE-tests
 * Test to check if communication connections can be deleted and added after setup
 */

class DynamicInterfacesTestCase : public TestCase
{
  public:
    DynamicInterfacesTestCase();
    virtual ~DynamicInterfacesTestCase();

  private:
    void DoRun() override;
};

DynamicInterfacesTestCase::DynamicInterfacesTestCase()
    : TestCase("Check if communication connections can be deleted and added after setup")
{
}

DynamicInterfacesTestCase::~DynamicInterfacesTestCase()
{
}

void
DynamicInterfacesTestCase::DoRun()
{
    NodeContainer nodes;
    nodes.Create(3);

    RlApplicationHelper helper(TypeId::LookupByName("ns3::TestObservationApp"));
    helper.SetAttribute("StartTime", TimeValue(Seconds(0)));
    helper.SetAttribute("StopTime", TimeValue(Seconds(20)));
    RlApplicationContainer observationApps = helper.Install(nodes.Get(0));
    helper.SetTypeId(TypeId::LookupByName("ns3::TestAgentApp"));
    RlApplicationContainer agentApps = helper.Install({nodes.Get(1), nodes.Get(2)});

    CommunicationHelper commHelper = CommunicationHelper();

    commHelper.SetObservationApps(observationApps);
    commHelper.SetAgentApps(agentApps);
    commHelper.SetIds();

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    auto devices = p2p.Install({nodes.Get(0), nodes.Get(1)});
    InternetStackHelper internet;
    internet.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    commHelper.AddCommunication(
        {{observationApps.GetId(0),
          agentApps.GetId(0),
          SocketCommunicationAttributes{interfaces.GetAddress(0), interfaces.GetAddress(1)}},
         {observationApps.GetId(0), agentApps.GetId(1), {}}});
    commHelper.Configure();

    Simulator::Schedule(Seconds(3),
                        &TestObservationApp::ExecuteCallback,
                        DynamicCast<TestObservationApp>(observationApps.Get(0)),
                        47.0,
                        -1,
                        -1);
    Simulator::Schedule(Seconds(4), [&] {
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(0))->GetObservation()[0][0],
                              47.0,
                              "received observation equals sent observation");
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(1))->GetObservation()[0][0],
                              47.0,
                              "received observation equals sent observation");
    });

    Simulator::Schedule(Seconds(5), [&] {
        commHelper.AddCommunication({{observationApps.GetId(0), agentApps.GetId(0), {}}});
    });

    Simulator::Schedule(Seconds(6),
                        &TestObservationApp::ExecuteCallback,
                        DynamicCast<TestObservationApp>(observationApps.Get(0)),
                        99.0,
                        agentApps.GetId(0).applicationId,
                        1);
    Simulator::Schedule(Seconds(7), [&] {
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(0))->GetObservation()[0][1],
                              99.0,
                              "received observation equals sent observation");
    });

    Simulator::Schedule(Seconds(10), [&] {
        commHelper.DeleteCommunication(observationApps.GetId(0), agentApps.GetId(1), 0, 0);
    });

    Simulator::Schedule(Seconds(11),
                        &TestObservationApp::ExecuteCallback,
                        DynamicCast<TestObservationApp>(observationApps.Get(0)),
                        65.0,
                        1,
                        -1);
    Simulator::Schedule(Seconds(12), [&] {
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(1))->GetObservation()[0][0],
                              47.0,
                              "received observation equals sent observation");
    });

    Simulator::Schedule(Seconds(14), [&] {
        commHelper.AddCommunication({{observationApps.GetId(0), agentApps.GetId(1), {}}});
    });

    Simulator::Schedule(Seconds(15),
                        &TestObservationApp::ExecuteCallback,
                        DynamicCast<TestObservationApp>(observationApps.Get(0)),
                        23.0,
                        agentApps.GetId(1).applicationId,
                        0);
    Simulator::Schedule(Seconds(16), [&] {
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(1))->GetObservation()[0][1],
                              23.0,
                              "received observation equals sent observation");
    });

    Simulator::Schedule(Seconds(17), [&] {
        commHelper.DeleteCommunication(observationApps.GetId(0), agentApps.GetId(0));
        commHelper.DeleteCommunication(observationApps.GetId(0), agentApps.GetId(0), 1, 1);
    });

    Simulator::Schedule(Seconds(18),
                        &TestObservationApp::ExecuteCallback,
                        DynamicCast<TestObservationApp>(observationApps.Get(0)),
                        77.0,
                        -1,
                        -1);
    Simulator::Schedule(Seconds(19), [&] {
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(0))->GetObservation()[0][1],
                              99.0,
                              "received observation equals sent observation");
        NS_TEST_ASSERT_MSG_EQ(DynamicCast<TestAgentApp>(agentApps.Get(1))->GetObservation()[0][2],
                              77.0,
                              "received observation equals sent observation");
    });

    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for Communication Helper
 */
class CommunicationTestSuite : public TestSuite
{
  public:
    CommunicationTestSuite();

  protected:
    void DoTeardown() override;
};

CommunicationTestSuite::CommunicationTestSuite()
    : TestSuite("defiance-communication", Type::UNIT)
{
    AddTestCase(new CommunicationTestCase, Duration::QUICK);
    AddTestCase(new DynamicInterfacesTestCase, Duration::QUICK);
}

void
CommunicationTestSuite::DoTeardown()
{
    Simulator::Destroy();
}

static CommunicationTestSuite sCommunicationTestSuite; //!< Static variable for test initialization
