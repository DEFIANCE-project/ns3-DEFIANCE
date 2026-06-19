#include "rl-application.h"

#include <string>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("RlApplication");

std::string
RlApplicationId::ToString() const
{
    return ApplicationTypeToString(applicationType) + "_" + std::to_string(applicationId);
}

std::string
ApplicationTypeToString(ApplicationType type)
{
    switch (type)
    {
    case AGENT:
        return "agent";
    default:
        return "no_agent";
    }
}

TypeId
RlApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::RlApplication").SetParent<Application>().SetGroupName("defiance");
    return tid;
}

RlApplication::RlApplication()
    : m_running(false)
{
}

RlApplication::~RlApplication()
{
}

void
RlApplication::StartApplication()
{
    NS_LOG_FUNCTION(this << m_running);
    m_running = true;
}

void
RlApplication::StopApplication()
{
    NS_LOG_FUNCTION(this << m_running);
    m_running = false;
}

bool
RlApplication::IsRunning()
{
    NS_LOG_FUNCTION(this);
    return m_running;
}

void
RlApplication::SetId(RlApplicationId id)
{
    NS_LOG_FUNCTION(this << id.ToString());
    m_id = id;
}

RlApplicationId
RlApplication::GetId()
{
    NS_LOG_FUNCTION(this);
    return m_id;
}

void
RlApplication::SetDefaultAddress(Ipv4Address address)
{
    NS_LOG_FUNCTION(this);
    m_defaultAddress = address;
}

Ipv4Address
RlApplication::GetDefaultAddress()
{
    NS_LOG_FUNCTION(this);
    if (m_defaultAddress == GetNode()->GetObject<Ipv4>()->GetAddress(0, 0).GetAddress())
    { // if only loopback was available at setup check for better options
      // without overriding non-loopback-address
        if (GetNode()->GetObject<Ipv4>()->GetNInterfaces() >= 2)
        {
            return GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetAddress();
        }
    }
    return m_defaultAddress;
}

void
RlApplication::Setup()
{
    NS_LOG_FUNCTION(this);
    if (!GetNode()->GetObject<Ipv4>())
    {
        return;
    }
    // first interface should be loopback -> skip it if you can
    if (GetNode()->GetObject<Ipv4>()->GetNInterfaces() >= 2)
    {
        SetDefaultAddress(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetAddress());
    }
    else
    {
        SetDefaultAddress(GetNode()->GetObject<Ipv4>()->GetAddress(0, 0).GetAddress());
    }
}

uint
RlApplication::AddObservationInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_ABORT_MSG("This RL app does not support observation communication.");
};

uint
RlApplication::AddRewardInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_ABORT_MSG("This RL app does not support reward communication.");
};

uint
RlApplication::AddActionInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_ABORT_MSG("This RL app does not support action communication.");
};

uint
RlApplication::AddAgentInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_ABORT_MSG("This RL app does not support agent communication.");
}

uint
RlApplication::AddInterface(RlApplicationId applicationId, Ptr<ChannelInterface> interface)
{
    NS_LOG_FUNCTION(this << applicationId.ToString() << interface);
    auto appId = applicationId.applicationId;
    switch (applicationId.applicationType)
    {
    case OBSERVATION:
        return AddObservationInterface(appId, interface);
    case REWARD:
        return AddRewardInterface(appId, interface);
    case AGENT:
        return AddAgentInterface(appId, interface);
    case ACTION:
        return AddActionInterface(appId, interface);
    default:
        NS_ABORT_MSG("Unknown application type " << applicationId.applicationType);
    }
}

void
RlApplication::DeleteObservationInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_ABORT_MSG("This RL app does not support observation communication.");
};

void
RlApplication::DeleteRewardInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_ABORT_MSG("This RL app does not support reward communication.");
};

void
RlApplication::DeleteActionInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_ABORT_MSG("This RL app does not support action communication.");
};

void
RlApplication::DeleteAgentInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_ABORT_MSG("This RL app does not support agent communication.");
}

void
RlApplication::DeleteInterface(RlApplicationId applicationId, uint interfaceId)
{
    NS_LOG_FUNCTION(this << applicationId.ToString() << interfaceId);
    auto appId = applicationId.applicationId;
    switch (applicationId.applicationType)
    {
    case OBSERVATION:
        return DeleteObservationInterface(appId, interfaceId);
    case REWARD:
        return DeleteRewardInterface(appId, interfaceId);
    case AGENT:
        return DeleteAgentInterface(appId, interfaceId);
    case ACTION:
        return DeleteActionInterface(appId, interfaceId);
    default:
        NS_ABORT_MSG("Unknown application type " << applicationId.applicationType);
    }
}

void
RlApplication::Send(Ptr<OpenGymDictContainer> data,
                    const std::vector<Ptr<ChannelInterface>>& interfaces)
{
    NS_LOG_FUNCTION(this << data << interfaces);
    if (m_running)
    {
        for (const auto& interface : interfaces)
        {
            interface->Send(data);
        }
    }
}

void
RlApplication::Send(Ptr<OpenGymDictContainer> data,
                    const std::map<uint, Ptr<ChannelInterface>>& interfaces)
{
    NS_LOG_FUNCTION(this << data << m_running);
    if (m_running)
    {
        for (const auto& [_, interface] : interfaces)
        {
            interface->Send(data);
        }
    }
}

void
RlApplication::Send(Ptr<OpenGymDictContainer> data, const InterfaceMap& interfaces)
{
    for (const auto& [_, appInterfaces] : interfaces)
    {
        Send(data, appInterfaces);
    }
}

} // namespace ns3
