#include "data-collector-application.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("DataCollectorApplication");

DataCollectorApplication::DataCollectorApplication()
    : m_interfaces({})
{
}

DataCollectorApplication::~DataCollectorApplication()
{
}

void
DataCollectorApplication::DoDispose()
{
    for (auto& [appId, interfaceMap] : m_interfaces)
    {
        for (auto& [id, interfacePtr] : interfaceMap)
        {
            interfacePtr->Dispose();
        }
    }
}

TypeId
DataCollectorApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::DataCollectorApplication").SetParent<RlApplication>().SetGroupName("defiance");
    return tid;
}

void
DataCollectorApplication::Setup()
{
    RegisterCallbacks();
}

uint
DataCollectorApplication::AddAgentInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_LOG_FUNCTION(this << interface << remoteAppId);
    uint id;
    if (m_interfaces[remoteAppId].empty())
    {
        id = 0;
    }
    else
    {
        auto it = m_interfaces[remoteAppId].rbegin();
        id = (*it).first + 1;
    }
    NS_LOG_INFO(id);
    m_interfaces[remoteAppId][id] = interface;
    return id;
}

void
DataCollectorApplication::DeleteAgentInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_LOG_FUNCTION(this << interfaceId << remoteAppId);
    m_interfaces[remoteAppId][interfaceId]->Disconnect();
    auto it = m_interfaces[remoteAppId].find(interfaceId);
    m_interfaces[remoteAppId].erase(it);
}

void
DataCollectorApplication::Send(Ptr<OpenGymDictContainer> data,
                               uint32_t remoteAppId,
                               uint32_t interfaceId)
{
    NS_LOG_FUNCTION(this << data << remoteAppId << interfaceId);
    RlApplication::Send(data, {m_interfaces[remoteAppId][interfaceId]});
}

void
DataCollectorApplication::Send(Ptr<OpenGymDictContainer> data, uint32_t remoteAppId)
{
    NS_LOG_FUNCTION(this << data << remoteAppId);
    RlApplication::Send(data, m_interfaces[remoteAppId]);
}

void
DataCollectorApplication::Send(Ptr<OpenGymDictContainer> data)
{
    NS_LOG_FUNCTION(this << data);
    for (const auto& [_, interface] : m_interfaces)
    {
        RlApplication::Send(data, interface);
    }
}

} // namespace ns3
