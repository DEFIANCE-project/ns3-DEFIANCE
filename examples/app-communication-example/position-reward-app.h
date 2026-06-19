#ifndef NS3_POSITION_REWARD_APP_H
#define NS3_POSITION_REWARD_APP_H

#include "ns3/mobility-model.h"
#include "ns3/reward-application.h"

namespace ns3
{

/**
 * @ingroup defiance
 * Reward application that observes the position of the node it is installed on.
 * The distance to the origin of coordinates is sent as a reward to all registered
 * agent applications.
 */
class PositionRewardApp : public RewardApplication
{
  public:
    PositionRewardApp() {};
    ~PositionRewardApp() override {};
    static TypeId GetTypeId();
    Ptr<OpenGymDictContainer> CreateDictContainer(float reward);
    void Reward(Ptr<const MobilityModel> observation);
    void RegisterCallbacks() override;
};
} // namespace ns3
#endif // NS3_POSITION_REWARD_APP_H
