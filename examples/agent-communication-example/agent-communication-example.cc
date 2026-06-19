#include "ns3/core-module.h"
#include "ns3/defiance-module.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AgentCommunicationExample");

/**
 * @ingroup defiance
 * Agent application that receives float values from another agent application.
 * When such a message is received stores it in a history container and logs the average of the
 * received values.
 */
class LoggingAgentApp : public AgentApplication
{
  public:
    LoggingAgentApp()
        : AgentApplication(),
          // Register a custom data structure for agent messages
          m_agentDataStruct(10) {};

    ~LoggingAgentApp() override = default;

    void OnRecvObs(uint id) override
    {
    }

    void OnRecvFromAgent(uint id, Ptr<OpenGymDictContainer> payload) override
    {
        NS_LOG_FUNCTION(this << id << payload);
        m_agentDataStruct.Push(payload, id);
        auto latestMessage = m_agentDataStruct.AggregateNewest(id);
        NS_LOG_INFO("Average of latest float-values: " << latestMessage["float-values"].GetAvg());
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

NS_OBJECT_ENSURE_REGISTERED(LoggingAgentApp);

} // namespace ns3

using namespace ns3;

// Run this example with 'ns3 run defiance-agent-agent-communication'

int
main(int argc, char* argv[])
{
    LogComponentEnable("AgentApplication", LOG_LEVEL_FUNCTION);
    LogComponentEnable("AgentCommunicationExample", LOG_LEVEL_FUNCTION);

    // Create agent apps on two nodes and connect them via simple channel interfaces
    auto agent0 = CreateObject<LoggingAgentApp>();
    auto agent1 = CreateObject<LoggingAgentApp>();

    NodeContainer nodes{2};

    auto channelInterface0_1 = CreateObject<SimpleChannelInterface>();
    auto channelInterface1_0 = CreateObject<SimpleChannelInterface>();

    channelInterface0_1->Connect(channelInterface1_0);

    agent0->AddAgentInterface(1, channelInterface0_1);
    agent0->SetId(RlApplicationId{AGENT, 0});
    agent0->Setup();
    agent1->AddAgentInterface(0, channelInterface1_0);
    agent1->SetId(RlApplicationId{AGENT, 1});
    agent1->Setup();

    nodes.Get(0)->AddApplication(agent0);
    nodes.Get(1)->AddApplication(agent1);

    // Create messages and send them between the agent apps
    auto msg = MakeDictBoxContainer<float>(2, "float-values", 1.0, 2.0);
    auto msg2 = MakeDictBoxContainer<float>(3, "float-values", 1.5, 3.5, 4.0);

    // This message will not be received by agent1 as it is scheduled before starting the agent apps
    agent0->SendToAgent(msg, 1, 0);

    // The following messages are scheduled to be sent during the simulation

    // Send msg from agent0 to agent1
    // Expected logging output: 'Average of latest float-values: 1.5'
    Simulator::Schedule(Seconds(1),
                        MakeCallback(*[](Ptr<AgentApplication> app, Ptr<OpenGymDictContainer> msg) {
                            app->SendToAgent(msg, 1, 0);
                        }).Bind(agent0, msg));
    // Send msg from agent1 to agent0
    // Expected logging output: 'Average of latest float-values: 3'
    Simulator::Schedule(Seconds(1),
                        MakeCallback(*[](Ptr<AgentApplication> app, Ptr<OpenGymDictContainer> msg) {
                            app->SendToAgent(msg, 0, 0);
                        }).Bind(agent1, msg2));
    // The following sends no message as agent1 has no connection to itself
    Simulator::Schedule(Seconds(1),
                        MakeCallback(*[](Ptr<AgentApplication> app, Ptr<OpenGymDictContainer> msg) {
                            app->SendToAgent(msg, 1);
                        }).Bind(agent1, msg));

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
