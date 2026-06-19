#include "ns3/ipv4.h"
#include "ns3/reward-application.h"

#include <cstdint>

namespace ns3
{
class LteHelper;

/**
 * @ingroup defiance
 * @brief Child class of RewardApplication that sends rewards based on the throughput of the UE.
 */
class HandoverRewardApplication : public RewardApplication
{
  public:
    HandoverRewardApplication() {};
    ~HandoverRewardApplication() override {};
    static TypeId GetTypeId();
    /// Measure arriving packet sizes
    void IncreaseReceivedBytes(Ptr<const Packet> packet, Ptr<Ipv4> ipLayer, uint32_t interface);
    /// Send the reward to the agent
    void SendReward();
    void RegisterCallbacks() override;

  private:
    uint32_t m_receivedBytes{0}; // bytes received since the last reward computation
    Time m_calculationInterval{MilliSeconds(1000)};
    Time m_simTime{Seconds(100)};
    Ptr<LteHelper> m_lteHelper;
};
} // namespace ns3
