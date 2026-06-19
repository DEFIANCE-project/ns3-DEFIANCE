#ifndef NS3_OBSERVATION_SHARING_AGENT_APP_H
#define NS3_OBSERVATION_SHARING_AGENT_APP_H

#include "ns3/agent-application.h"

namespace ns3
{

/**
 * @ingroup defiance
 * Agent application that receives position and velocity values from observation and reward
 * applications and outputs some aggregated statistics about them.
 */
class ObservationSharingAgentApp : public AgentApplication
{
  public:
    ObservationSharingAgentApp() // register custom data structure for agent messages here
        : AgentApplication(),
          m_agentDataStruct(10) {};
    ~ObservationSharingAgentApp() override = default;

    static TypeId GetTypeId();

    void OnRecvObs(uint remoteAppId) override;
    void OnRecvFromAgent(uint remoteAppId, Ptr<OpenGymDictContainer> payload) override;
    void OnRecvReward(uint remoteAppId) override;

    void PrintLatestMessages(uint id);

    Ptr<OpenGymSpace> GetObservationSpace() override;
    Ptr<OpenGymSpace> GetActionSpace() override;

  protected:
    HistoryContainer m_agentDataStruct;
};

} // namespace ns3
#endif // NS3_OBSERVATION_SHARING_AGENT_APP_H
