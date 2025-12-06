#include "simple-channel-interface.h"

#include "channel-interface.h"

#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleChannelInterface");

TypeId
SimpleChannelInterface::GetTypeId()
{
    static TypeId tid =
        TypeId("SimpleChannelInterface").SetParent<ChannelInterface>().SetGroupName("defiance");
    return tid;
}

SimpleChannelInterface::SimpleChannelInterface()
    : m_communicationPartner(nullptr),
      m_propagationDelay(Time())
{
    NS_LOG_FUNCTION(this);
}

SimpleChannelInterface::~SimpleChannelInterface()
{
    NS_LOG_FUNCTION(this);
}

int
SimpleChannelInterface::Send(const Ptr<OpenGymDictContainer> data)
{
    NS_LOG_FUNCTION(this << m_communicationPartner << data);
    if (!m_communicationPartner)
    {
        NS_LOG_INFO("Send request but no communication partner is connected");
        return -1;
    }

    // TODO: Because we are dealing with pointers the data could technically be manipulated before
    // it is received - should this be dealt with?

    // this will lead to the data being received by the communication partner after the propagation
    // delay
    size_t size = data->GetDataContainerPbMsg().ByteSizeLong();
    NS_LOG_INFO("Data will be received at: " << Simulator::Now() + m_propagationDelay
                                             << " - Size of the data in Bytes: " << size);
    Simulator::Schedule(m_propagationDelay, [this, data]() {
        NS_LOG_INFO(m_communicationPartner << " Received data: " << data);
        m_communicationPartner->GetReceiveCallbacks()(data);
    });

    return size;
}

ConnectionStatus
SimpleChannelInterface::Connect(Ptr<ChannelInterface> otherInterface)
{
    NS_LOG_FUNCTION(this << otherInterface);
    try
    {
        auto simpleInterface = DynamicCast<SimpleChannelInterface>(otherInterface);
        if (!simpleInterface)
        {
            throw std::bad_cast(); // Checks that we are actually connecting two
                                   // SimpleChannelInterfaces with one another
        }

        // in case this interface already has a connection - do not connect and keep the old
        // connection
        if (GetConnectionStatus() == CONNECTED)
        {
            NS_LOG_INFO("Connection failed - this interface is already connected. The old "
                        "connection is kept.");
            return CONNECTED;
        }

        // in case the other interface is already connected to a different interface than this - Do
        // not allow to connect, the other interface will keep its connection
        if (simpleInterface->GetConnectionStatus() == CONNECTED &&
            simpleInterface->GetCommunicationPartner() != this)
        {
            NS_LOG_INFO("Connection failed - the other interface is already connected to another "
                        "interface. This interface will stay disconnected.");
            return DISCONNECTED;
        }

        // do not allow to connect to oneself
        if (simpleInterface == this)
        {
            NS_LOG_INFO("Connection failed - connecting to oneself is not allowed.");
            return DISCONNECTED;
        }

        // otherwise connect both interfaces to each other
        SetCommunicationPartner(simpleInterface);
        SetConnectionStatus(CONNECTED);

        simpleInterface->SetCommunicationPartner(this);
        simpleInterface->SetConnectionStatus(CONNECTED);

        NS_LOG_INFO("Connecting successful - both interfaces are now connected to each other.");
        return CONNECTED;
    }
    catch (std::bad_cast& e)
    {
        NS_LOG_ERROR("CommPartner is not of type SimpleChannelInterface - new connection attempt "
                     "failed. The own connection status now is: "
                     << GetConnectionStatus());
        return GetConnectionStatus();
    }
}

void
SimpleChannelInterface::PartialDisconnect()
{
    NS_LOG_FUNCTION(this);
    if (GetConnectionStatus() == DISCONNECTED)
    {
        NS_LOG_INFO("Partial disconnect failed - this interface is already disconnected.");
        return;
    }
    m_communicationPartner = nullptr;
    SetConnectionStatus(DISCONNECTED);
}

void
SimpleChannelInterface::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Disconnect();
    ChannelInterface::DoDispose();
}

// TODO: This should never lead to problems but the method itself has not been tested completely
void
SimpleChannelInterface::Disconnect()
{
    NS_LOG_FUNCTION(this);
    auto commPartnerBeforeDisconnect = m_communicationPartner;
    PartialDisconnect();
    if (commPartnerBeforeDisconnect)
    {
        commPartnerBeforeDisconnect->PartialDisconnect();
    }
}

void
SimpleChannelInterface::SetPropagationDelay(Time delay)
{
    NS_LOG_FUNCTION(this << delay);
    m_propagationDelay = delay;
}

Time
SimpleChannelInterface::GetPropagationDelay() const
{
    NS_LOG_FUNCTION(this);
    return m_propagationDelay;
}

void
SimpleChannelInterface::SetCommunicationPartner(Ptr<SimpleChannelInterface> communicationPartner)
{
    NS_LOG_FUNCTION(this << communicationPartner);
    m_communicationPartner = communicationPartner;
}

Ptr<SimpleChannelInterface>
SimpleChannelInterface::GetCommunicationPartner() const
{
    NS_LOG_FUNCTION(this);
    return m_communicationPartner;
}
