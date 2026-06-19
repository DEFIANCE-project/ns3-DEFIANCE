#ifndef NS3_POSITION_OBSERVATION_APP_H
#define NS3_POSITION_OBSERVATION_APP_H

#include "ns3/mobility-model.h"
#include "ns3/observation-application.h"

namespace ns3
{

/**
 * @ingroup defiance
 * Observation application that registers to a callback to observe
 * the current position and velocity of the node it is installed on. These values are
 * packed into a DictContainer and sent to all registered agent applications.
 */
class PositionObservationApp : public ObservationApplication
{
  public:
    PositionObservationApp() {};
    ~PositionObservationApp() override = default;

    static TypeId GetTypeId();
    void Observe(Ptr<const MobilityModel> observation);
    void RegisterCallbacks() override;
};
} // namespace ns3
#endif // NS3_POSITION_OBSERVATION_APP_H
