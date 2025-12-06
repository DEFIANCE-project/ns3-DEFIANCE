#include "ns3/ai-module.h"
#include "ns3/core-module.h"
#include "ns3/defiance-module.h"

#include <iostream>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AgentAgentCommunication");

class AgentAgentCommunicationTestAgent : public AgentApplication
{
  public:
    AgentAgentCommunicationTestAgent()
        : AgentApplication(),
          // register custom data structure for agent messages here
          m_agentDataStruct(10) {};

    ~AgentAgentCommunicationTestAgent() override = default;

    void OnRecvObs(uint id) override
    {
    }

    void OnRecvFromAgent(uint id, Ptr<OpenGymDictContainer> payload) override
    {
        m_agentDataStruct.Push(payload, id);
        NS_LOG_FUNCTION(this << "received message from agent " << id);
        auto latestMessage = m_agentDataStruct.AggregateNewest(id);
        NS_LOG_INFO("floatObs: " << latestMessage["floatObs"].GetAvg());
    }

    void OnRecvReward(uint id) override
    {
    }

  protected:
    HistoryContainer m_agentDataStruct;

  private:
    Ptr<OpenGymSpace> GetObservationSpace() override
    {
        return {};
    }

    Ptr<OpenGymSpace> GetActionSpace() override
    {
        return {};
    }
};
} // namespace ns3

// run this example with 'ns3 run defiance-agent-agent-communication'

int
main(int argc, char* argv[])
{
    using namespace ns3;
    LogComponentEnable("AgentApplication", LOG_LEVEL_FUNCTION);
    LogComponentEnable("AgentAgentCommunication", LOG_LEVEL_FUNCTION);

    auto agent0 = CreateObject<AgentAgentCommunicationTestAgent>();
    auto agent1 = CreateObject<AgentAgentCommunicationTestAgent>();

    NodeContainer nodes{2};

    auto channelInterface0_1 = CreateObject<SimpleChannelInterface>();
    auto channelInterface1_0 = CreateObject<SimpleChannelInterface>();

    channelInterface0_1->Connect(channelInterface1_0);

    agent0->AddAgentInterface(1, channelInterface0_1);
    agent0->Setup();
    agent1->AddAgentInterface(0, channelInterface1_0);
    agent1->Setup();

    nodes.Get(0)->AddApplication(agent0);
    nodes.Get(1)->AddApplication(agent1);

    auto msg = MakeDictBoxContainer<float>(2, "floatObs", 1.0, 2.0);
    auto msg2 = MakeDictBoxContainer<float>(3, "floatObs", 1.5, 3.5, 4.0);

    // These messages should be received by the agents with floatObs: 1.5 and floatObs: 3.0
    agent0->SendToAgent(msg, 1, 0);
    agent1->SendToAgent(msg2, 0, 0);

    // The following messages are scheduled to be sent during the simulation
    Simulator::Schedule(Seconds(1),
                        MakeCallback(*[](Ptr<AgentApplication> app, Ptr<OpenGymDictContainer> msg) {
                            app->SendToAgent(msg, 1, 0);
                        }).Bind(agent0, msg));
    // no message is sent as agent1 has no connection to itself
    Simulator::Schedule(Seconds(1),
                        MakeCallback(*[](Ptr<AgentApplication> app, Ptr<OpenGymDictContainer> msg) {
                            app->SendToAgent(msg, 1);
                        }).Bind(agent1, msg));

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
