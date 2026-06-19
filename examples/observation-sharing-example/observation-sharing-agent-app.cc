#include "observation-sharing-agent-app.h"

#include "ns3/agent-application.h"
#include "ns3/base-test.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("ObservationSharingAgentApp");

TypeId
ObservationSharingAgentApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ObservationSharingAgentApp")
                            .SetParent<AgentApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<ObservationSharingAgentApp>();
    return tid;
}

void
ObservationSharingAgentApp::OnRecvObs(uint remoteAppId)
{
    NS_LOG_FUNCTION(this << remoteAppId);
    auto info = m_obsDataStruct.AggregateNewest(remoteAppId);
    // Send to all other agents
    auto newest = m_obsDataStruct.GetNewestByID(remoteAppId);
    SendToAgent(newest->data);
}

void
ObservationSharingAgentApp::OnRecvFromAgent(uint remoteAppId, Ptr<OpenGymDictContainer> payload)
{
    NS_LOG_FUNCTION(this << remoteAppId << payload);
    m_agentDataStruct.Push(payload, remoteAppId);
    auto latestMessage = m_agentDataStruct.AggregateNewest(remoteAppId);
    auto message = latestMessage["position"].GetAvg();
    NS_LOG_INFO("Message: " << message);
}

void
ObservationSharingAgentApp::OnRecvReward(uint remoteAppId)
{
    NS_LOG_FUNCTION(this << remoteAppId);
}

void
ObservationSharingAgentApp::PrintLatestMessages(uint id = 0)
{
    std::cout << "Observations:" << std::endl;
    m_obsDataStruct.Print(std::cout);
    m_obsDataStruct.PrintHistory(std::cout, 0, ns3_ai_gym::Box);
    std::cout << "Agent messages:" << std::endl;
    m_agentDataStruct.Print(std::cout);
    m_agentDataStruct.PrintHistory(std::cout, id, ns3_ai_gym::Box);
}

Ptr<OpenGymSpace>
ObservationSharingAgentApp::GetObservationSpace()
{
    return {};
}

Ptr<OpenGymSpace>
ObservationSharingAgentApp::GetActionSpace()
{
    return {};
}

NS_OBJECT_ENSURE_REGISTERED(ObservationSharingAgentApp);
} // namespace ns3
