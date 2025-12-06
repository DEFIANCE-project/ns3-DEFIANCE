#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/defiance-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-module.h"
#include "ns3/test.h"

#include <vector>

namespace ns3
{

/**
 * @ingroup defiance-tests
 *
 * Test to check if SocketChannelInterface can be used for communication
 */
class SocketChannelInterfaceTestCase : public TestCase
{
  public:
    SocketChannelInterfaceTestCase();
    virtual ~SocketChannelInterfaceTestCase();
    void ProcessMessage(uint interfaceId, Ptr<OpenGymDictContainer> msg);
    void ExpectNotReceived(uint interfaceId, float previous);
    void ExpectReceived(uint interfaceId, float expected);

  protected:
    void DoRun() override;

  private:
    std::map<uint, float> m_received;
};

SocketChannelInterfaceTestCase::SocketChannelInterfaceTestCase()
    : TestCase("check if SocketChannelInterface can be used for communication")
{
}

SocketChannelInterfaceTestCase::~SocketChannelInterfaceTestCase()
{
}

void
SocketChannelInterfaceTestCase::ProcessMessage(uint interfaceId, Ptr<OpenGymDictContainer> msg)
{
    m_received[interfaceId] =
        DynamicCast<OpenGymBoxContainer<float>>(msg->Get("box"))->GetData()[0];
}

void
SocketChannelInterfaceTestCase::ExpectNotReceived(uint interfaceId, float previous)
{
    // if previous == -1.0, expect that never a message was received on this interface
    if (previous == -1.0)
    {
        NS_ASSERT_MSG(m_received.find(interfaceId) == m_received.end(),
                      "no message on this interface received");
    }
    else
    {
        NS_TEST_ASSERT_MSG_EQ(m_received[interfaceId],
                              previous,
                              "last message recveived on this interface equals previous message");
    }
}

void
SocketChannelInterfaceTestCase::ExpectReceived(uint interfaceId, float expected)
{
    NS_TEST_ASSERT_MSG_EQ(m_received[interfaceId],
                          expected,
                          "expected message value " << expected << ", got "
                                                    << m_received[interfaceId]);
}

void
SocketChannelInterfaceTestCase::DoRun()
{
    NodeContainer nodes;
    nodes.Create(2);

    // Create a point-to-point helper
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // Create devices and install them on nodes
    NetDeviceContainer devices;
    devices.Add(p2p.Install(nodes.Get(0), nodes.Get(1)));

    // Assign IP addresses
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    auto udpProtocol = UdpSocketFactory::GetTypeId();
    auto tcpProtocol = TcpSocketFactory::GetTypeId();

    auto interfaceSimple0 = CreateObject<SimpleChannelInterface>();
    auto interfaceSimple1 = CreateObject<SimpleChannelInterface>();

    auto interfaceUdp0_1A =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), udpProtocol);
    auto interfaceUdp0_1B =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), udpProtocol);
    auto interfaceUdp1_0 =
        CreateObject<SocketChannelInterface>(nodes.Get(1), interfaces.GetAddress(1), udpProtocol);

    auto interfaceTcp0_1A =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), tcpProtocol);
    auto interfaceTcp0_1B =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), tcpProtocol);
    auto interfaceTcp1_0 =
        CreateObject<SocketChannelInterface>(nodes.Get(1), interfaces.GetAddress(1), tcpProtocol);

    // add the receive callback to the channel interfaces
    interfaceSimple0->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 0));
    interfaceSimple1->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 1));
    interfaceUdp0_1A->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 2));
    interfaceUdp0_1B->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 3));
    interfaceUdp1_0->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 4));
    interfaceTcp0_1A->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 5));
    interfaceTcp0_1B->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 6));
    interfaceTcp1_0->AddRecvCallback(
        MakeCallback(&SocketChannelInterfaceTestCase::ProcessMessage, this, 7));

    Simulator::Schedule(Seconds(0.1),
                        &SimpleChannelInterface::Connect,
                        interfaceSimple0,
                        interfaceSimple1);

    Simulator::Schedule(Seconds(0.1),
                        &SocketChannelInterface::Connect,
                        interfaceUdp0_1A,
                        interfaceUdp1_0);

    Simulator::Schedule(Seconds(0.1),
                        &SocketChannelInterface::Connect,
                        interfaceTcp0_1A,
                        interfaceTcp1_0);

    // Add a simple propagation delay
    Simulator::Schedule(Seconds(0.3), [&] {
        interfaceSimple0->SetPropagationDelay(Seconds(0));
        interfaceSimple1->SetPropagationDelay(Seconds(0));
    });

    Simulator::Schedule(Seconds(0.5),
                        &SimpleChannelInterface::Send,
                        interfaceSimple0,
                        MakeDictBoxContainer<float>(1, "box", 1));
    Simulator::Schedule(Seconds(0.51), [&] { ExpectReceived(1, 1); });
    Simulator::Schedule(Seconds(0.6),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1A,
                        MakeDictBoxContainer<float>(1, "box", 4));
    Simulator::Schedule(Seconds(0.61), [&] { ExpectReceived(4, 4); });
    Simulator::Schedule(Seconds(0.7),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1A,
                        MakeDictBoxContainer<float>(1, "box", 7));
    Simulator::Schedule(Seconds(0.71), [&] { ExpectReceived(7, 7); });

    // Add a simple propagation delay
    Simulator::Schedule(Seconds(0.8), [&] { interfaceSimple0->SetPropagationDelay(Seconds(10)); });

    Simulator::Schedule(Seconds(0.9),
                        &SimpleChannelInterface::Send,
                        interfaceSimple0,
                        MakeDictBoxContainer<float>(1, "box", 42));
    // Check that message was not received before delay
    Simulator::Schedule(Seconds(10.89), [&] { ExpectNotReceived(1, 1); });
    // Check that message was received after delay
    Simulator::Schedule(Seconds(10.91), [&] { ExpectReceived(1, 42); });

    // sending in the other direction
    Simulator::Schedule(Seconds(11),
                        &SimpleChannelInterface::Send,
                        interfaceSimple1,
                        MakeDictBoxContainer<float>(1, "box", 0));
    Simulator::Schedule(Seconds(11.1), [&] { ExpectReceived(0, 0); });

    // If we try to reconnect to already connected interfaces this will fail
    Simulator::Schedule(Seconds(11.1),
                        &SocketChannelInterface::Connect,
                        interfaceUdp0_1B,
                        interfaceUdp1_0);
    // if we were connected with interfaceUdp0_1B the value 3 would be expected
    Simulator::Schedule(Seconds(11.2),
                        &SocketChannelInterface::Send,
                        interfaceUdp1_0,
                        MakeDictBoxContainer<float>(1, "box", 2));
    Simulator::Schedule(Seconds(11.3), [&] { ExpectNotReceived(3, -1); });
    Simulator::Schedule(Seconds(11.3), [&] { ExpectReceived(2, 2); });

    // These interfaces are already connected: Therefore, no new connection will be established.
    // Still, such a call should not lead to a crash or unexpected behaviour.
    Simulator::Schedule(Seconds(11.3),
                        &SocketChannelInterface::Connect,
                        interfaceTcp1_0,
                        interfaceTcp0_1A);
    // if we were connected with interfaceTcp0_1B the value 6 would be expected
    Simulator::Schedule(Seconds(11.4),
                        &SocketChannelInterface::Send,
                        interfaceTcp1_0,
                        MakeDictBoxContainer<float>(1, "box", 5));
    Simulator::Schedule(Seconds(11.5), [&] { ExpectNotReceived(6, -1); });
    Simulator::Schedule(Seconds(11.5), [&] { ExpectReceived(5, 5); });

    // should not be received because the connection is not established
    Simulator::Schedule(Seconds(11.6),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1B,
                        MakeDictBoxContainer<float>(1, "box", -2.0));
    Simulator::Schedule(Seconds(11.65), [&] { ExpectNotReceived(4, 4); });
    // should not be received because the connection is not established
    Simulator::Schedule(Seconds(11.7),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1B,
                        MakeDictBoxContainer<float>(1, "box", -2.0));
    Simulator::Schedule(Seconds(11.75), [&] { ExpectNotReceived(7, 7); });

    // but if we closed the old connection before and try again to connect this will work
    Simulator::Schedule(Seconds(11.8), &SocketChannelInterface::Disconnect, interfaceTcp1_0);

    // this should not be received
    Simulator::Schedule(Seconds(11.9),
                        &SocketChannelInterface::Send,
                        interfaceTcp1_0,
                        MakeDictBoxContainer<float>(1, "box", -2.0));
    Simulator::Schedule(Seconds(11.95), [&] { ExpectNotReceived(5, 5); });

    // now we do the new connect
    Simulator::Schedule(Seconds(12),
                        &SocketChannelInterface::Connect,
                        interfaceTcp0_1B,
                        interfaceTcp1_0);

    // should now be received (used to be connected with interfaceTcp0_1A which would expect 5; but
    // is now connected with interfaceTcp0_1B)
    Simulator::Schedule(Seconds(12.1),
                        &SocketChannelInterface::Send,
                        interfaceTcp1_0,
                        MakeDictBoxContainer<float>(1, "box", 6));
    Simulator::Schedule(Seconds(12.2), [&] { ExpectNotReceived(5, 5); });
    Simulator::Schedule(Seconds(12.2), [&] { ExpectReceived(6, 6); });

    Simulator::ScheduleDestroy(&SimpleChannelInterface::Dispose, interfaceSimple0);
    Simulator::ScheduleDestroy(&SimpleChannelInterface::Dispose, interfaceSimple1);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceUdp0_1A);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceTcp0_1A);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceUdp1_0);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceTcp1_0);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceUdp0_1B);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceTcp0_1B);

    // Run the simulation
    Simulator::Stop(Seconds(15));
    Simulator::Run();
    Simulator::Destroy();
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for SocketChannelInterface
 */
class SocketChannelInterfaceTestSuite : public TestSuite
{
  public:
    SocketChannelInterfaceTestSuite();

  protected:
    void DoTeardown() override;
};

SocketChannelInterfaceTestSuite::SocketChannelInterfaceTestSuite()
    : TestSuite("defiance-socket-channel-interface", Type::UNIT)
{
    AddTestCase(new SocketChannelInterfaceTestCase, Duration::QUICK);
}

void
SocketChannelInterfaceTestSuite::DoTeardown()
{
    Simulator::Destroy();
}

static SocketChannelInterfaceTestSuite
    sSocketChannelInterfaceTestSuite; //!< Static variable for test initialization
} // namespace ns3
