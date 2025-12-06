#include "socket-channel-interface.h"

#include "channel-interface.h"

#include "ns3/node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SocketChannelInterface");

TypeId
SocketChannelInterface::GetTypeId()
{
    static TypeId tid =
        TypeId("SocketChannelInterface").SetParent<ChannelInterface>().SetGroupName("defiance");
    return tid;
}

SocketChannelInterface::SocketChannelInterface(Ptr<Node> localNode,
                                               Ipv4Address localAddress,
                                               TypeId transmissionProtocol)
{
    NS_LOG_FUNCTION(this);
    // Create a socket with the provided protocol
    m_localSocket = Socket::CreateSocket(localNode, transmissionProtocol);

    // bind the socket at the specified address and allocates a free port
    m_localSocket->Bind(InetSocketAddress(localAddress, 0));

    // saves the allocated address (ip+port) in the variable m_localAddress so that the socket can
    // be bound again later to this original address
    m_localSocket->GetSockName(m_localAddress);

    m_localSocket->Listen();
    m_localSocket->SetRecvCallback(MakeCallback(&SocketChannelInterface::Receive, this));

    // Set the accept callback to always accept incoming connections (because at this point we have
    // already decided if a connection is allowed or not) Also, stores the forked socket in the
    // m_remoteSocket variable and sets the callbacks for it to receive data
    m_localSocket->SetAcceptCallback(
        Callback<bool, Ptr<Socket>, const Address&>(
            [](Ptr<Socket> socket, const Address& address) { return true; }),
        MakeCallback(&SocketChannelInterface::AddConnection, this));

    // Set the close callbacks which remove the remote socket when the connection is closed
    m_localSocket->SetCloseCallbacks(MakeCallback(&SocketChannelInterface::RemoveConnection, this),
                                     MakeCallback(&SocketChannelInterface::RemoveConnection, this));
}

SocketChannelInterface::~SocketChannelInterface()
{
    NS_LOG_FUNCTION(this);
}

void
SocketChannelInterface::DoDispose()
{
    NS_LOG_FUNCTION(this);
    if (m_communicationPartner)
    {
        m_communicationPartner = nullptr;
        m_localSocket = nullptr;
        m_remoteSocket = nullptr;
    }
    ChannelInterface::DoDispose();
}

int
SocketChannelInterface::Send(const Ptr<OpenGymDictContainer> data)
{
    NS_LOG_FUNCTION(this << data);
    if (!m_communicationPartner)
    {
        NS_LOG_INFO("Send request but no communication partner is connected");
    }
    else
    {
        NS_LOG_INFO(
            "Sending data from the local address "
            << InetSocketAddress::ConvertFrom(m_localAddress).GetIpv4() << ":"
            << InetSocketAddress::ConvertFrom(m_localAddress).GetPort() << " to the remote address "
            << InetSocketAddress::ConvertFrom(m_communicationPartner->m_localAddress).GetIpv4()
            << ":"
            << InetSocketAddress::ConvertFrom(m_communicationPartner->m_localAddress).GetPort());
    }
    Ptr<Packet> packet = Serialize(data);
    return SendRaw(packet);
}

int
SocketChannelInterface::SendRaw(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    // the following logic will only ever send one packet of data to the communication partner

    if (m_remoteSocket) // when the interface itself acts as server (was connected to)
    {
        m_remoteSocket->Send(packet);
    }

    return m_localSocket->Send(packet); // when the interface itself acts as client (connection
                                        // initiator) or in case of UDP (connectionless)
}

void
SocketChannelInterface::Receive(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    // Receive all packets that are at the queue of the socket and process them with the registered
    // callbacks

    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        NS_LOG_INFO("Received packet from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << ":"
                                            << InetSocketAddress::ConvertFrom(from).GetPort());
        NS_LOG_INFO("Received packet size: " << packet->GetSize());

        Ptr<OpenGymDictContainer> data = Deserialize(packet);

        NS_LOG_INFO("Received data: " << data);

        // executes all the registered callbacks with the received data
        GetReceiveCallbacks()(data);
    }
}

Ptr<Packet>
SocketChannelInterface::Serialize(const Ptr<OpenGymDictContainer> data)
{
    NS_LOG_FUNCTION(this << data);

    size_t size = data->GetDataContainerPbMsg().ByteSizeLong();
    auto buffer = new u_int8_t[size];

    data->GetDataContainerPbMsg().SerializeToArray(buffer, size);
    auto packet = Create<Packet>(buffer, size);
    return packet;
}

Ptr<OpenGymDictContainer>
SocketChannelInterface::Deserialize(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    auto buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());

    ns3_ai_gym::DataContainer dataContainerPbMsg;
    dataContainerPbMsg.ParseFromArray(buffer, packet->GetSize());

    Ptr<OpenGymDataContainer> gymDataContainer =
        OpenGymDataContainer::CreateFromDataContainerPbMsg(dataContainerPbMsg);
    auto gymDict = gymDataContainer->GetObject<OpenGymDictContainer>();

    return gymDict;
}

// TODO: Test more
ConnectionStatus
SocketChannelInterface::Connect(Ptr<ChannelInterface> otherInterface)
{
    //  Previous state: A is connected to B; C and D are not connected
    //  We list all of the scenarios that could take place
    //  1. A connects to C -> not allowed: A stays connected to B, C stays disconnected
    //  2. C connects to A -> not allowed: A stays connected to B, C stays disconnected
    //  3. A connects to B -> not allowed: A and B stay connected
    //  4. A disconnects from B -> allowed: A and B are disconnected (BOTH)
    //  5. C connects to D -> allowed: C and D are connected
    //  6. C connects to C -> not allowed: C stays disconnected
    //  7. UDP tries to connect to TCP and vice versa -> not allowed: an assertion is thrown

    // to fix the bug in ns3 that occurs when two tcp connections are bidirectional and scheduled at
    // the exact same time we also introduce the CONNECTING status and only allow to connect in the
    // DISCONNECTED status

    NS_LOG_FUNCTION(this << otherInterface);

    // do not connect if this interface itself already has a connection
    if (GetConnectionStatus() == CONNECTED)
    {
        NS_LOG_INFO(
            "Connection failed - this interface is already connected. The old connection is "
            "kept.");
        return CONNECTED;
    }

    // do not connect to yourself
    if (otherInterface == this)
    {
        NS_LOG_INFO("Connection failed - connecting to oneself is not allowed. This interface will "
                    "stay disconnected.");
        return DISCONNECTED;
    }

    // includes a check that only two channel interfaces based on sockets can be connected
    try
    {
        if (!dynamic_cast<SocketChannelInterface*>(GetPointer(otherInterface)))
        {
            throw std::bad_cast(); // Checks that we are actually connecting two
                                   // SimpleChannelInterfaces with one another
        }
        auto otherSocketInterface = DynamicCast<SocketChannelInterface>(otherInterface);

        // ensure that the transmission protocol of both ends of the channel is the same
        auto mySocketType = m_localSocket->GetInstanceTypeId();
        auto otherSocketType = otherSocketInterface->m_localSocket->GetInstanceTypeId();
        NS_ASSERT_MSG(mySocketType == otherSocketType,
                      "The communication partner is incompatible.");

        // do not connect if the other interface is already connected to a different interface than
        // this
        if (otherSocketInterface->GetConnectionStatus() != DISCONNECTED)
        {
            NS_LOG_INFO("Connection failed - the other interface is not free for a connection. "
                        "This interface will stay disconnected.");
            return DISCONNECTED;
        }

        // we got here - therefore we are allowed to connect (connecting might still fail though)
        bool didConnectWork = false; // keeps track of connection success

        if (mySocketType == TcpSocketBase::GetTypeId())
        {
            NS_LOG_INFO("Connecting via TCP");
            didConnectWork = DoConnect(otherSocketInterface);
            if (!didConnectWork)
            {
                NS_LOG_INFO("TCP connection failed");
                SetConnectionStatus(DISCONNECTED);
                return DISCONNECTED;
            }
        }
        else
        {
            NS_LOG_INFO("Using UDP (connect)");
            auto didConnectWorkA = DoConnect(otherSocketInterface);
            auto didConnectWorkB = otherSocketInterface->DoConnect(this);
            didConnectWork = didConnectWorkA && didConnectWorkB;
            if (!didConnectWork)
            {
                NS_LOG_INFO("UDP connection failed");
                this->PartialDisconnect();
                otherSocketInterface->PartialDisconnect();
                return DISCONNECTED;
            }
        }
        // we got here - therefore the connection was successful and we can set connection status
        // and comm partner
        m_communicationPartner = otherSocketInterface;
        SetConnectionStatus(CONNECTED);
        otherSocketInterface->m_communicationPartner = this;
        otherSocketInterface->SetConnectionStatus(CONNECTED);
        return CONNECTED;
    }
    catch (std::bad_cast& e)
    {
        NS_LOG_ERROR("CommPartner is not of type SocketChannelInterface - new connection attempt "
                     "failed. The old connection status - "
                     << GetConnectionStatus() << " - is kept.");
        return GetConnectionStatus();
    }
}

// TODO: Test in an example that a connection attempt that is not possible (e.g. missing routing)
// actually leads to the desired behaviour (still disconnected after attempt)
bool
SocketChannelInterface::DoConnect(Ptr<SocketChannelInterface> otherInterface)
{
    NS_LOG_FUNCTION(this << otherInterface);
    SetConnectionStatus(CONNECTING);
    bool didConnectWork = m_localSocket->Connect(otherInterface->m_localAddress) + 1;
    NS_LOG_INFO("Connection attempt was " << (didConnectWork ? "successful" : "unsuccessful"));
    return didConnectWork;
}

// TODO: Test this a bit more
void
SocketChannelInterface::PartialDisconnect()
{
    NS_LOG_FUNCTION(this);
    if (GetConnectionStatus() != CONNECTED)
    {
        NS_LOG_INFO("Partial disconnect failed - this interface is not connected.");
        return;
    }

    // reset the socket to a fresh state
    auto node = m_localSocket->GetNode();
    auto protocol = (m_localSocket->GetInstanceTypeId() == TcpSocketBase::GetTypeId())
                        ? TcpSocketFactory::GetTypeId()
                        : UdpSocketFactory::GetTypeId();
    m_localSocket = Socket::CreateSocket(node, protocol);

    m_localSocket->Bind(m_localAddress);
    m_localSocket->Listen();

    m_communicationPartner = nullptr;
    m_remoteSocket = nullptr;

    SetConnectionStatus(DISCONNECTED);
}

// TODO: Test this a bit more
void
SocketChannelInterface::Disconnect()
{
    NS_LOG_FUNCTION(this);
    Ptr<SocketChannelInterface> commPartnerBeforeDisconnect = m_communicationPartner;
    PartialDisconnect();
    if (commPartnerBeforeDisconnect)
    {
        commPartnerBeforeDisconnect->PartialDisconnect();
    }
}

// has a reference to the socket that is forked after a connection is established and the address of
// the connection originator
void
SocketChannelInterface::AddConnection(Ptr<Socket> socket, const Address& address)
{
    NS_LOG_FUNCTION(this << socket << address);
    socket->SetRecvCallback(MakeCallback(&SocketChannelInterface::Receive, this));
    m_remoteSocket = socket;
}

// has a reference to the socket that is closed
void
SocketChannelInterface::RemoveConnection(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    m_remoteSocket = nullptr;
    SetConnectionStatus(DISCONNECTED);
}
