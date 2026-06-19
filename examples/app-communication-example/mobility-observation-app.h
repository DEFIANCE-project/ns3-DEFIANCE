#ifndef NS3_MOBILITY_OBSERVATION_APP_H
#define NS3_MOBILITY_OBSERVATION_APP_H

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
class MobilityObservationApp : public ObservationApplication
{
  public:
    MobilityObservationApp() {};
    ~MobilityObservationApp() override {};
    static TypeId GetTypeId();
    void Observe(Ptr<const MobilityModel> observation);
    void RegisterCallbacks() override;

  private:
    Ptr<OpenGymDictContainer> CreateDictContainer(Vector position, double velocity);
};
} // namespace ns3
#endif // NS3_MOBILITY_OBSERVATION_APP_H
