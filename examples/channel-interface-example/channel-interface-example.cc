#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/simple-channel-interface.h"
#include "ns3/socket-channel-interface.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ChannelInterfaceExample");

// Helper method to create a simple message easily.
Ptr<OpenGymDictContainer>
CreateTestMessage(float value)
{
    Ptr<OpenGymDictContainer> msg = CreateObject<OpenGymDictContainer>();
    Ptr<OpenGymBoxContainer<float>> box = CreateObject<OpenGymBoxContainer<float>>();
    box->AddValue(value);
    msg->Add("box", box);
    return msg;
}

int
main(int argc, char* argv[])
{
    LogComponentEnable("ChannelInterfaceExample", LOG_LEVEL_INFO);
    // Create nodes
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

    Ptr<SocketChannelInterface> interfaceUdp0_1A =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), udpProtocol);
    Ptr<SocketChannelInterface> interfaceUdp0_1B =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), udpProtocol);
    Ptr<SocketChannelInterface> interfaceUdp1_0 =
        CreateObject<SocketChannelInterface>(nodes.Get(1), interfaces.GetAddress(1), udpProtocol);

    Ptr<SocketChannelInterface> interfaceTcp0_1A =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), tcpProtocol);
    Ptr<SocketChannelInterface> interfaceTcp0_1B =
        CreateObject<SocketChannelInterface>(nodes.Get(0), interfaces.GetAddress(0), tcpProtocol);
    Ptr<SocketChannelInterface> interfaceTcp1_0 =
        CreateObject<SocketChannelInterface>(nodes.Get(1), interfaces.GetAddress(1), tcpProtocol);

    // add the receive callback to the channel interfaces
    auto recvCallback = Callback<void, Ptr<OpenGymDictContainer>>(
        [](Ptr<OpenGymDictContainer> msg) { NS_LOG_INFO(msg->Get("box")); });

    interfaceSimple0->AddRecvCallback(recvCallback);
    interfaceSimple1->AddRecvCallback(recvCallback);
    interfaceUdp0_1A->AddRecvCallback(recvCallback);
    interfaceUdp0_1B->AddRecvCallback(recvCallback);
    interfaceUdp1_0->AddRecvCallback(recvCallback);
    interfaceTcp0_1A->AddRecvCallback(recvCallback);
    interfaceTcp0_1B->AddRecvCallback(recvCallback);
    interfaceTcp1_0->AddRecvCallback(recvCallback);

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

    // These interfaces are already connected: Therfore no new connection will be established.
    // Still, such a call should not lead to a crash or unexpected behaviour.
    Simulator::Schedule(Seconds(0.1),
                        &SocketChannelInterface::Connect,
                        interfaceTcp1_0,
                        interfaceTcp0_1A);

    // Add a simple propagation delay
    interfaceSimple0->SetPropagationDelay(Seconds(0));
    interfaceSimple1->SetPropagationDelay(Seconds(0));

    Simulator::Schedule(Seconds(0.5),
                        &SimpleChannelInterface::Send,
                        interfaceSimple0,
                        CreateTestMessage(0));
    Simulator::Schedule(Seconds(0.6),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1A,
                        CreateTestMessage(1));
    Simulator::Schedule(Seconds(0.7),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1A,
                        CreateTestMessage(2));

    // sending in the other direction

    Simulator::Schedule(Seconds(0.8),
                        &SimpleChannelInterface::Send,
                        interfaceSimple1,
                        CreateTestMessage(3));
    Simulator::Schedule(Seconds(0.9),
                        &SocketChannelInterface::Send,
                        interfaceUdp1_0,
                        CreateTestMessage(4));
    Simulator::Schedule(Seconds(1.0),
                        &SocketChannelInterface::Send,
                        interfaceTcp1_0,
                        CreateTestMessage(5));

    // If we try to reconnect to already connected interfaces this will fail
    Simulator::Schedule(Seconds(2),
                        &SocketChannelInterface::Connect,
                        interfaceUdp0_1B,
                        interfaceUdp1_0);
    Simulator::Schedule(Seconds(2),
                        &SocketChannelInterface::Connect,
                        interfaceTcp0_1B,
                        interfaceTcp1_0);

    Simulator::Schedule(Seconds(2.5),
                        &SimpleChannelInterface::Send,
                        interfaceSimple0,
                        CreateTestMessage(6));
    // should not be received because the connection is not established
    Simulator::Schedule(Seconds(2.6),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1B,
                        CreateTestMessage(7));
    // should not be received because the connection is not established
    Simulator::Schedule(Seconds(2.7),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1B,
                        CreateTestMessage(8));

    Simulator::Schedule(Seconds(2.8),
                        &SimpleChannelInterface::Send,
                        interfaceSimple1,
                        CreateTestMessage(9));
    Simulator::Schedule(Seconds(2.9),
                        &SocketChannelInterface::Send,
                        interfaceUdp1_0,
                        CreateTestMessage(10));
    Simulator::Schedule(Seconds(3.0),
                        &SocketChannelInterface::Send,
                        interfaceTcp1_0,
                        CreateTestMessage(11));

    Simulator::Schedule(Seconds(3.5),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1A,
                        CreateTestMessage(12));
    Simulator::Schedule(Seconds(3.6),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1A,
                        CreateTestMessage(13));

    // but if we closed the old connection before and try again to connect this will work
    Simulator::Schedule(Seconds(4), &SocketChannelInterface::Disconnect, interfaceUdp1_0);
    Simulator::Schedule(Seconds(4), &SocketChannelInterface::Disconnect, interfaceTcp1_0);

    // this should not be received
    Simulator::Schedule(Seconds(4.1),
                        &SocketChannelInterface::Send,
                        interfaceUdp1_0,
                        CreateTestMessage(14));
    Simulator::Schedule(Seconds(4.1),
                        &SocketChannelInterface::Send,
                        interfaceTcp1_0,
                        CreateTestMessage(15));
    Simulator::Schedule(Seconds(4.8),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1A,
                        CreateTestMessage(16));
    Simulator::Schedule(Seconds(4.8),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1A,
                        CreateTestMessage(17));

    // now we do the new connect
    Simulator::Schedule(Seconds(4.5),
                        &SocketChannelInterface::Connect,
                        interfaceUdp0_1B,
                        interfaceUdp1_0);
    Simulator::Schedule(Seconds(4.5),
                        &SocketChannelInterface::Connect,
                        interfaceTcp0_1B,
                        interfaceTcp1_0);

    // should now be received
    Simulator::Schedule(Seconds(5),
                        &SocketChannelInterface::Send,
                        interfaceUdp0_1B,
                        CreateTestMessage(18));
    // should now be received
    Simulator::Schedule(Seconds(5),
                        &SocketChannelInterface::Send,
                        interfaceTcp0_1B,
                        CreateTestMessage(19));
    Simulator::Schedule(Seconds(6), &ChannelInterface::Disconnect, interfaceUdp1_0);
    Simulator::Schedule(Seconds(6), &ChannelInterface::Disconnect, interfaceSimple0);
    Simulator::Schedule(Seconds(7), &ChannelInterface::Connect, interfaceUdp1_0, interfaceSimple0);
    Simulator::Schedule(Seconds(7), &ChannelInterface::Connect, interfaceSimple0, interfaceUdp1_0);

    Simulator::ScheduleDestroy(&SimpleChannelInterface::Dispose, interfaceSimple0);
    Simulator::ScheduleDestroy(&SimpleChannelInterface::Dispose, interfaceSimple1);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceUdp0_1A);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceTcp0_1A);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceUdp1_0);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceTcp1_0);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceUdp0_1B);
    Simulator::ScheduleDestroy(&SocketChannelInterface::Dispose, interfaceTcp0_1B);

    // Run the simulation
    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
