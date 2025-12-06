#include "agent-application.h"

#include "base-test.h"
#include "rl-application.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AgentApplication");

AgentApplication::AgentApplication()
    : m_obsDataStruct{0},
      m_rewardDataStruct{0}
{
}

AgentApplication::~AgentApplication()
{
}

void
AgentApplication::DoDispose()
{
    for (auto& [appId, interfaceMap] : m_observationInterfaces)
    {
        for (auto& [id, interfacePtr] : interfaceMap)
        {
            interfacePtr->Dispose();
        }
    }
    for (auto& [appId, interfaceMap] : m_rewardInterfaces)
    {
        for (auto& [id, interfacePtr] : interfaceMap)
        {
            interfacePtr->Dispose();
        }
    }
    for (auto& [appId, interfaceMap] : m_agentInterfaces)
    {
        for (auto& [id, interfacePtr] : interfaceMap)
        {
            interfacePtr->Dispose();
        }
    }
    for (auto& [appId, interfaceMap] : m_actionInterfaces)
    {
        for (auto& [id, interfacePtr] : interfaceMap)
        {
            interfacePtr->Dispose();
        }
    }
    RlApplication::DoDispose();
}

TypeId
AgentApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::AgentApplication")
            .SetParent<RlApplication>()
            .SetGroupName("defiance")
            .AddAttribute("MaxObservationHistoryLength",
                          "Maximum length of the history of each observation deque to store.",
                          UintegerValue(10),
                          MakeUintegerAccessor(&AgentApplication::m_maxObservationHistoryLength),
                          MakeUintegerChecker<uint>())
            .AddAttribute("MaxRewardHistoryLength",
                          "Maximum length of the history of each reward deque to store.",
                          UintegerValue(10),
                          MakeUintegerAccessor(&AgentApplication::m_maxRewardHistoryLength),
                          MakeUintegerChecker<uint>())
            .AddAttribute("ObservationTimestamping",
                          "Enable ns3 timestamps for observations.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&AgentApplication::m_obsTimestamping),
                          MakeBooleanChecker())
            .AddAttribute("RewardTimestamping",
                          "Enable ns3 timestamps for rewards.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&AgentApplication::m_rewardTimestamping),
                          MakeBooleanChecker());
    return tid;
}

void
AgentApplication::ReceiveObservation(uint remoteAppId, Ptr<OpenGymDictContainer> observation)
{
    NS_LOG_FUNCTION(this << remoteAppId << observation);
    m_obsDataStruct.Push(observation, remoteAppId);
    OnRecvObs(remoteAppId);
}

void
AgentApplication::OnRecvFromAgent(uint remoteAppId, Ptr<OpenGymDictContainer> payload)
{
    NS_LOG_INFO("In order to use interagent communication, override this in a subclass.");
}

void
AgentApplication::ReceiveReward(uint remoteAppId, Ptr<OpenGymDictContainer> reward)
{
    NS_LOG_FUNCTION(this << remoteAppId << reward);
    m_rewardDataStruct.Push(reward, remoteAppId);
    OnRecvReward(remoteAppId);
}

void
AgentApplication::Setup()
{
    m_obsDataStruct = HistoryContainer(m_maxObservationHistoryLength, m_obsTimestamping);
    m_rewardDataStruct = HistoryContainer(m_maxRewardHistoryLength, m_rewardTimestamping);
    OpenGymMultiAgentInterface::Get()->SetGetObservationSpaceCb(
        GetId().ToString(),
        MakeCallback(&AgentApplication::GetObservationSpace, this));
    OpenGymMultiAgentInterface::Get()->SetGetActionSpaceCb(
        GetId().ToString(),
        MakeCallback(&AgentApplication::GetActionSpace, this));
}

uint
AgentApplication::AddObservationInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_LOG_FUNCTION(this << interface << remoteAppId);
    uint id;
    if (m_observationInterfaces[remoteAppId].empty())
    {
        id = 0;
    }
    else
    {
        auto it = m_observationInterfaces[remoteAppId].rbegin();
        id = (*it).first + 1;
    }
    m_observationInterfaces[remoteAppId][id] = interface;
    interface->AddRecvCallback(
        MakeCallback(&AgentApplication::ReceiveObservation, this, remoteAppId));
    return id;
}

uint
AgentApplication::AddAgentInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_LOG_FUNCTION(this << interface << remoteAppId);
    uint id;
    if (m_agentInterfaces[remoteAppId].empty())
    {
        id = 0;
    }
    else
    {
        auto it = m_agentInterfaces[remoteAppId].rbegin();
        id = (*it).first + 1;
    }
    m_agentInterfaces[remoteAppId][id] = interface;
    interface->AddRecvCallback(MakeCallback(&AgentApplication::OnRecvFromAgent, this, remoteAppId));
    return id;
}

uint
AgentApplication::AddRewardInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_LOG_FUNCTION(this << interface << remoteAppId);
    uint id;
    if (m_rewardInterfaces[remoteAppId].empty())
    {
        id = 0;
    }
    else
    {
        auto it = m_rewardInterfaces[remoteAppId].rbegin();
        id = (*it).first + 1;
    }
    m_rewardInterfaces[remoteAppId][id] = interface;
    interface->AddRecvCallback(MakeCallback(&AgentApplication::ReceiveReward, this, remoteAppId));
    return id;
}

uint
AgentApplication::AddActionInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface)
{
    NS_LOG_FUNCTION(this << interface << remoteAppId);
    uint id;
    if (m_actionInterfaces[remoteAppId].empty())
    {
        id = 0;
    }
    else
    {
        auto it = m_actionInterfaces[remoteAppId].rbegin();
        id = (*it).first + 1;
    }
    m_actionInterfaces[remoteAppId][id] = interface;
    return id;
}

void
AgentApplication::DeleteObservationInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_LOG_FUNCTION(this << interfaceId << remoteAppId);
    m_observationInterfaces[remoteAppId][interfaceId]->Disconnect();
    auto it = m_observationInterfaces[remoteAppId].find(interfaceId);
    m_observationInterfaces[remoteAppId].erase(it);
}

void
AgentApplication::DeleteRewardInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_LOG_FUNCTION(this << interfaceId << remoteAppId);
    m_rewardInterfaces[remoteAppId][interfaceId]->Disconnect();
    auto it = m_rewardInterfaces[remoteAppId].find(interfaceId);
    m_rewardInterfaces[remoteAppId].erase(it);
}

void
AgentApplication::DeleteAgentInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_LOG_FUNCTION(this << interfaceId << remoteAppId);
    m_agentInterfaces[remoteAppId][interfaceId]->Disconnect();
    auto it = m_agentInterfaces[remoteAppId].find(interfaceId);
    m_agentInterfaces[remoteAppId].erase(it);
}

void
AgentApplication::DeleteActionInterface(uint32_t remoteAppId, uint interfaceId)
{
    NS_LOG_FUNCTION(this << interfaceId << remoteAppId);
    m_actionInterfaces[remoteAppId][interfaceId]->Disconnect();
    auto it = m_actionInterfaces[remoteAppId].find(interfaceId);
    m_actionInterfaces[remoteAppId].erase(it);
}

void
AgentApplication::InitiateAction(Ptr<OpenGymDataContainer> action)
{
    SendAction(MakeDictContainer("default", action));
}

void
AgentApplication::InitiateActionForApp(uint remoteAppId, Ptr<OpenGymDataContainer> action)
{
    SendAction(MakeDictContainer("default", action), remoteAppId);
}

void
AgentApplication::SendToAgent(Ptr<OpenGymDictContainer> data,
                              uint32_t appId,
                              uint32_t interfaceIndex)
{
    NS_LOG_FUNCTION(this << data << appId << interfaceIndex);
    NS_ASSERT_MSG((interfaceIndex < m_agentInterfaces[appId].size()),
                  "Interface index " << interfaceIndex
                                     << " not in vector of interfaces of ActionApplication "
                                     << appId);
    Send(data, {m_agentInterfaces[appId][interfaceIndex]});
}

void
AgentApplication::SendToAgent(Ptr<OpenGymDictContainer> data, uint32_t appId)
{
    NS_LOG_FUNCTION(this << data << appId);
    Send(data, m_agentInterfaces[appId]);
}

void
AgentApplication::SendToAgent(Ptr<OpenGymDictContainer> data)
{
    NS_LOG_FUNCTION(this << data);
    Send(data, m_agentInterfaces);
}

void
AgentApplication::SendAction(Ptr<OpenGymDictContainer> action,
                             uint32_t appId,
                             uint32_t interfaceIndex)
{
    NS_LOG_FUNCTION(this << action << appId << interfaceIndex);
    NS_ASSERT_MSG((interfaceIndex < m_actionInterfaces[appId].size()),
                  "Interface index " << interfaceIndex
                                     << " not in vector of interfaces of ActionApplication "
                                     << appId);
    Send(action, {m_actionInterfaces[appId][interfaceIndex]});
}

void
AgentApplication::SendAction(Ptr<OpenGymDictContainer> action, uint32_t appId)
{
    NS_LOG_FUNCTION(this << action << appId);
    Send(action, m_actionInterfaces[appId]);
}

void
AgentApplication::SendAction(Ptr<OpenGymDictContainer> action)
{
    NS_LOG_FUNCTION(this << action);
    Send(action, m_actionInterfaces);
}

void
AgentApplication::InferAction()
{
    NS_ABORT_MSG_IF(!m_observation, "Observation is not set!");
    OpenGymMultiAgentInterface::Get()->NotifyCurrentState(
        GetId().ToString(),
        m_observation,
        m_reward,
        m_truncationRequested,
        GetExtraInfo(),
        GetActionDelay(),
        MakeCallback(&AgentApplication::InitiateAction, this));
}

void
AgentApplication::InferAction(uint remoteAppId)
{
    OpenGymMultiAgentInterface::Get()->NotifyCurrentState(
        GetId().ToString(),
        m_observation,
        m_reward,
        m_truncationRequested,
        GetExtraInfo(),
        GetActionDelay(),
        MakeCallback(&AgentApplication::InitiateActionForApp, this, remoteAppId));
}

void
AgentApplication::RequestTruncation()
{
    m_truncationRequested = true;
}

} // namespace ns3
