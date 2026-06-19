#include "addition-agent-app.h"

#include "ns3/base-test.h"
#include "ns3/node-list.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AdditionAgentApp");

AdditionAgentApp::AdditionAgentApp()
    : m_last_action(0),
      m_last_observation(0)
{
    NS_LOG_FUNCTION(this);
}

TypeId
AdditionAgentApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::AdditionAgentApp")
                            .SetParent<AgentApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<AdditionAgentApp>();
    return tid;
}

void
AdditionAgentApp::OnRecvObs(uint id)
{
    auto obs = m_obsDataStruct.GetNewestByID(id)
                   ->data->Get("random_number")
                   ->GetObject<OpenGymDiscreteContainer>();
    NS_LOG_INFO("Received observation: " << obs->GetValue());
    m_observation = obs;
}

void
AdditionAgentApp::OnRecvReward(uint remoteAppId)
{
    auto container = m_rewardDataStruct.GetNewestByID(remoteAppId)->data->Get("reward_hint");
    int reward_hint = container->GetObject<OpenGymDiscreteContainer>()->GetValue();
    NS_LOG_INFO("Reward hint: " << reward_hint);
    // the agent should have given (observation + reward_hint)
    auto difference = std::abs(m_last_action - m_last_observation - reward_hint);
    m_reward = std::exp(-difference);
    NS_LOG_INFO(m_last_observation << "+" << reward_hint << "=" << m_last_action << "?");
    NS_LOG_INFO("Received normalized reward: " << m_reward);
    m_last_observation = m_observation->GetObject<OpenGymDiscreteContainer>()->GetValue();
    // as rewards will arrive later than observations infer action now
    InferAction();
}

Ptr<OpenGymSpace>
AdditionAgentApp::GetObservationSpace()
{
    return CreateObject<OpenGymDiscreteSpace>(10);
}

Ptr<OpenGymSpace>
AdditionAgentApp::GetActionSpace()
{
    return CreateObject<OpenGymDiscreteSpace>(NodeList::GetNNodes() + 9);
}

void
AdditionAgentApp::InitiateAction(Ptr<OpenGymDataContainer> action)
{
    m_last_action = action->GetObject<OpenGymDiscreteContainer>()->GetValue();
    NS_LOG_INFO("Action: " << m_last_action);
    SendAction(MakeDictContainer("default", action));
}

NS_OBJECT_ENSURE_REGISTERED(AdditionAgentApp);
} // namespace ns3
