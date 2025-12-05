#include "ns3/defiance-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/test.h"

#include <sstream>
#include <string>

using namespace ns3;

/**
 * @ingroup defiance-tests
 * Base test case class that unifies setup functionality for different tests.
 * This is not a full test case yet, actual tests need to inherit from it.
 */
class SimpleChannelInterfaceBaseTestCase : public TestCase
{
  public:
    SimpleChannelInterfaceBaseTestCase(std::string name);
    ~SimpleChannelInterfaceBaseTestCase() override;

  protected:
    void DoSetup() override;
    void DoRun() override;
    virtual void Simulate() = 0;

    NodeContainer m_nodes;
    NetDeviceContainer m_devices;
    Ipv4InterfaceContainer m_interfaces;

  private:
    NetDeviceContainer GetDevices(NodeContainer nc)
    {
        // Create a point-to-point helper
        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
        p2p.SetChannelAttribute("Delay", StringValue("2ms"));

        // Create devices and install them on nodes
        return p2p.Install(nc);
    }

    Ipv4InterfaceContainer GetInterfaces(NetDeviceContainer ndc)
    {
        InternetStackHelper internet;
        internet.Install(m_nodes);

        Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");
        return address.Assign(ndc);
    }
};

SimpleChannelInterfaceBaseTestCase::SimpleChannelInterfaceBaseTestCase(std::string name)
    : TestCase(name)
{
    Ipv4AddressGenerator::TestMode();
}

SimpleChannelInterfaceBaseTestCase::~SimpleChannelInterfaceBaseTestCase()
{
}

void
SimpleChannelInterfaceBaseTestCase::DoSetup()
{
    m_nodes = NodeContainer{2};
    m_devices = GetDevices(m_nodes);
    m_interfaces = GetInterfaces(m_devices);
}

void
SimpleChannelInterfaceBaseTestCase::DoRun()
{
    Simulate();
}

/**
 * @ingroup defiance-tests
 * Test to check that the connection procedure between SimpleChannelInterfaces works as expected.
 */
class SimpleChannelInterfaceConnectTestCase : public SimpleChannelInterfaceBaseTestCase
{
  public:
    SimpleChannelInterfaceConnectTestCase();

  private:
    void Simulate() override;
};

SimpleChannelInterfaceConnectTestCase::SimpleChannelInterfaceConnectTestCase()
    : SimpleChannelInterfaceBaseTestCase(
          "Check the connection procedure of the SimpleChannelInterface")
{
}

void
SimpleChannelInterfaceConnectTestCase::Simulate()
{
    // Create the channel interfaces that we will use to test our connection routine
    auto simpleChannelInterfaceA = CreateObject<SimpleChannelInterface>();
    auto simpleChannelInterfaceB = CreateObject<SimpleChannelInterface>();
    auto simpleChannelInterfaceC = CreateObject<SimpleChannelInterface>();

    auto protocol = UdpSocketFactory::GetTypeId();
    auto socketChannelInterface =
        CreateObject<SocketChannelInterface>(m_nodes.Get(0), m_interfaces.GetAddress(1), protocol);

    // Connecting two not yet connected interfaces (A -> B); both not connected before
    auto connectionStatus = simpleChannelInterfaceA->Connect(simpleChannelInterfaceB);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          CONNECTED,
                          "Connecting two not yet connected interfaces (A -> B): Connection Status "
                          "overall = Connected");
    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceA->GetConnectionStatus(),
        CONNECTED,
        "Connecting two not yet connected interfaces (A -> B): Connection Status A = Connected");
    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceB->GetConnectionStatus(),
        CONNECTED,
        "Connecting two not yet connected interfaces (A -> B): Connection Status B = Connected");

    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceA->GetCommunicationPartner(),
                          simpleChannelInterfaceB,
                          "Connecting two not yet connected interfaces (A -> B) - CommPartner of A "
                          "set correctly to B");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceB->GetCommunicationPartner(),
                          simpleChannelInterfaceA,
                          "Connecting two not yet connected interfaces (A -> B) - CommPartner of B "
                          "set correctly to A");

    // Connecting (A -> C) - A was already connected to B before; C was disconnected
    connectionStatus = simpleChannelInterfaceA->Connect(simpleChannelInterfaceC);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          CONNECTED,
                          "Connecting (A -> C) - A was connected to B and C was disconnected:  "
                          "Connection Status overall = Connected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceA->GetConnectionStatus(),
                          CONNECTED,
                          "Connecting (A -> C) - A was connected to B and C was disconnected: "
                          "Connection Status A = Connected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetConnectionStatus(),
                          DISCONNECTED,
                          "Connecting (A -> C) - A was connected to B and C was disconnected: "
                          "Connection Status C = Disconnected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceA->GetCommunicationPartner(),
                          simpleChannelInterfaceB,
                          "Connecting (A -> C) - A was connected to B and C was disconnected: "
                          "CommPartner of A set correctly to B");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetCommunicationPartner(),
                          nullptr,
                          "Connecting (A -> C) - A was connected to B and C was disconnected:  - "
                          "CommPartner of C = nullptr");

    // Connecting (C -> A) - A was already connected before to B; C was disconnected
    connectionStatus = simpleChannelInterfaceC->Connect(simpleChannelInterfaceA);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          DISCONNECTED,
                          "Connecting (C -> A) - A was connected to B and C was disconnected: "
                          "Connection Status overall = Disconnected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceA->GetConnectionStatus(),
                          CONNECTED,
                          "Connecting (C -> A) - A was connected to B and C was disconnected: "
                          "Connection Status A = ");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetConnectionStatus(),
                          DISCONNECTED,
                          "Connecting (C -> A) - A was connected to B and C was disconnected: "
                          "Connection Status C = Disconnected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceA->GetCommunicationPartner(),
                          simpleChannelInterfaceB,
                          "Connecting (C -> A) - A was connected to B and C was disconnected: "
                          "CommPartner of A set correctly to B");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetCommunicationPartner(),
                          nullptr,
                          "Connecting (C -> A) - A was connected to B and C was disconnected: "
                          "CommPartner of C set correctly to nullptr");

    // Connecting two already connected interfaces (B -> A); both were connected with each other
    connectionStatus = simpleChannelInterfaceB->Connect(simpleChannelInterfaceA);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          CONNECTED,
                          "Connecting two already connected interfaces (B -> A) - Connection "
                          "Status overall = Connected");
    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceA->GetConnectionStatus(),
        CONNECTED,
        "Connecting two already connected interfaces (B -> A) - Connection Status A = Connected");
    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceB->GetConnectionStatus(),
        CONNECTED,
        "Connecting two already connected interfaces (B -> A) - Connection Status B = Connected");

    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceA->GetCommunicationPartner(),
                          simpleChannelInterfaceB,
                          "Connecting two already connected interfaces (B -> A) - CommPartner of A "
                          "set correctly to B");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceB->GetCommunicationPartner(),
                          simpleChannelInterfaceA,
                          "Connecting two already connected interfaces (B -> A) - CommPartner of B "
                          "set correctly to A");

    // Connecting a simple interface with itself (A -> A); A was connected before to B
    connectionStatus = simpleChannelInterfaceA->Connect(simpleChannelInterfaceA);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          CONNECTED,
                          "Connecting two already connected interfaces (A -> A) - Connection "
                          "Status overall = Connected");
    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceA->GetConnectionStatus(),
        CONNECTED,
        "Connecting a simple interface with itself (A -> A) - Connection Status A = Connected");

    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceA->GetCommunicationPartner(),
        simpleChannelInterfaceB,
        "Connecting a simple interface with itself (A -> A) - CommPartner of A set correctly to B");

    // Connecting a simple interface with itself (C -> C); C was not connected before
    connectionStatus = simpleChannelInterfaceC->Connect(simpleChannelInterfaceC);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          DISCONNECTED,
                          "Connecting a simple interface with itself (C -> C) - Connection Status "
                          "overall = Disconnected");
    NS_TEST_ASSERT_MSG_EQ(
        simpleChannelInterfaceC->GetConnectionStatus(),
        DISCONNECTED,
        "Connecting a simple interface with itself (C -> C) - Connection Status C = Disconnected");

    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetCommunicationPartner(),
                          nullptr,
                          "Connecting a simple interface with itself (C -> C) - CommPartner of C "
                          "set correctly to nullptr");

    // Connecting a simple interface with a socket interface (C -> Socket); both were not connected
    // before
    connectionStatus = simpleChannelInterfaceC->Connect(socketChannelInterface);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          DISCONNECTED,
                          "Connecting a simple interface with a socket interface (C -> Socket) - "
                          "Connection Status overall = Disconnected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetConnectionStatus(),
                          DISCONNECTED,
                          "Connecting a simple interface with a socket interface (C -> Socket) - "
                          "Connection Status C = Disconnected");
    connectionStatus = socketChannelInterface->Connect(simpleChannelInterfaceC);
    NS_TEST_ASSERT_MSG_EQ(connectionStatus,
                          DISCONNECTED,
                          "Connecting a simple interface with a socket interface (C -> Socket) - "
                          "Connection Status overall = Disconnected");
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->GetConnectionStatus(),
                          DISCONNECTED,
                          "Connecting a simple interface with a socket interface (C -> Socket) - "
                          "Connection Status C = Disconnected");
}

/**
 * @ingroup defiance-tests
 * Test to check that the connection procedure between SimpleChannelInterfaces works as expected.
 */
class SimpleChannelInterfaceReceiveTestCase : public SimpleChannelInterfaceBaseTestCase
{
  public:
    SimpleChannelInterfaceReceiveTestCase();
    ~SimpleChannelInterfaceReceiveTestCase() override;

  private:
    void Simulate() override;
    void RecvCallback(std::string& string, Ptr<OpenGymDictContainer> msg);
};

SimpleChannelInterfaceReceiveTestCase::SimpleChannelInterfaceReceiveTestCase()
    : SimpleChannelInterfaceBaseTestCase(
          "Check that messages are correctly sent and received between simple channel interfaces")
{
}

SimpleChannelInterfaceReceiveTestCase::~SimpleChannelInterfaceReceiveTestCase()
{
}

void
SimpleChannelInterfaceReceiveTestCase::RecvCallback(std::string& string,
                                                    Ptr<OpenGymDictContainer> msg)
{
    std::stringstream ss;
    msg->Print(ss);
    string = ss.str();
}

void
SimpleChannelInterfaceReceiveTestCase::Simulate()
{
    Ptr<OpenGymDictContainer> msg1 = CreateObject<OpenGymDictContainer>();
    Ptr<OpenGymBoxContainer<float>> box1 = CreateObject<OpenGymBoxContainer<float>>();
    box1->AddValue(1.0);
    box1->AddValue(2.0);
    msg1->Add("box", box1);

    Ptr<OpenGymDictContainer> msg2 = CreateObject<OpenGymDictContainer>();
    Ptr<OpenGymBoxContainer<float>> box2 = CreateObject<OpenGymBoxContainer<float>>();
    box2->AddValue(3.0);
    box2->AddValue(4.0);
    msg2->Add("box", box2);

    Ptr<OpenGymDictContainer> msg3 = CreateObject<OpenGymDictContainer>(); // Empty message

    std::string defaultA = "defaultA";
    std::string defaultB = "defaultB";

    const std::string& resultAtA = defaultA;
    const std::string& resultAtB = defaultB;

    auto simpleChannelInterfaceA = CreateObject<SimpleChannelInterface>();
    simpleChannelInterfaceA->AddRecvCallback(
        MakeCallback(&SimpleChannelInterfaceReceiveTestCase::RecvCallback, this, resultAtA));

    auto simpleChannelInterfaceB = CreateObject<SimpleChannelInterface>();
    simpleChannelInterfaceB->AddRecvCallback(
        MakeCallback(&SimpleChannelInterfaceReceiveTestCase::RecvCallback, this, resultAtB));
    simpleChannelInterfaceB->SetPropagationDelay(Seconds(2));

    auto simpleChannelInterfaceC = CreateObject<SimpleChannelInterface>();

    Simulator::Schedule(Seconds(1),
                        &SimpleChannelInterface::Connect,
                        simpleChannelInterfaceA,
                        simpleChannelInterfaceB);
    Simulator::Schedule(Seconds(1),
                        &SimpleChannelInterface::Connect,
                        simpleChannelInterfaceB,
                        simpleChannelInterfaceA);
    // only C is not connected to any other interface now

    // test that sending from C gives return value -1
    NS_TEST_ASSERT_MSG_EQ(simpleChannelInterfaceC->Send(msg1),
                          -1,
                          "Sending from not connected interface returns -1");

    // TODO: Compare that the received message is correct

    Simulator::Stop(Seconds(10));
    Simulator::Run();
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for SimpleChannelInterface
 */
class SimpleChannelInterfaceTestSuite : public TestSuite
{
  public:
    SimpleChannelInterfaceTestSuite();
};

SimpleChannelInterfaceTestSuite::SimpleChannelInterfaceTestSuite()
    : TestSuite("defiance-simple-channel-interface", Type::UNIT)
{
    AddTestCase(new SimpleChannelInterfaceConnectTestCase, Duration::QUICK);
    AddTestCase(new SimpleChannelInterfaceReceiveTestCase, Duration::QUICK);
}

static SimpleChannelInterfaceTestSuite
    sSimpleChannelInterfaceTestSuite; //!< Static variable for test initialization
