#include "ns3/agent-application.h"
#include "ns3/base-test.h"
#include "ns3/lte-helper.h"

using namespace ns3;

/**
 * @ingroup defiance
 * @brief Child class of AgentApplication that receives all observations and distributes actions.
 */
class HandoverAgentApplication : public AgentApplication
{
  public:
    HandoverAgentApplication();
    ~HandoverAgentApplication() override;

    static TypeId GetTypeId();

    void Setup() override;

    void OnRecvObs(uint id) override;

    void OnRecvReward(uint id) override;

    void InitiateAction(Ptr<OpenGymDataContainer>) override;

    Ptr<OpenGymSpace> GetObservationSpace() override;
    Ptr<OpenGymSpace> GetActionSpace() override;

    // Return the initial observation that is used after resetting the environment.
    Ptr<OpenGymDictContainer> GetResetObservation() const;
    // Return the initial reward that is used after resetting the environment.
    float GetResetReward();

  private:
    uint32_t m_numUes;
    uint32_t m_numBs;
    uint32_t m_stepTime;
    Time m_lastInferredActionTime = Seconds(0);
};
