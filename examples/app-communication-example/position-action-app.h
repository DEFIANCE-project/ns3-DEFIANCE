#ifndef NS3_POSITION_ACTION_APP_H
#define NS3_POSITION_ACTION_APP_H

#include "ns3/action-application.h"

namespace ns3
{

class PositionActionApp : public ActionApplication
{
  public:
    PositionActionApp() {};
    ~PositionActionApp() override = default;
    static TypeId GetTypeId();
    void ExecuteAction(uint32_t remoteAppId, Ptr<OpenGymDictContainer> action) override;
};
} // namespace ns3
#endif // NS3_POSITION_ACTION_APP_H
