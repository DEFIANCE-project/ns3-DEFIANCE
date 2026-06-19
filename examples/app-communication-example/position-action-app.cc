#include "position-action-app.h"

#include "ns3/mobility-model.h"
#include "ns3/vector.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("PositionActionApp");

TypeId
PositionActionApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PositionActionApp")
                            .SetParent<ActionApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<PositionActionApp>();
    return tid;
}

void
PositionActionApp::ExecuteAction(uint32_t remoteAppId, Ptr<ns3::OpenGymDictContainer> action)
{
    NS_LOG_FUNCTION(this << remoteAppId << action);
    // Extract positions of the two observation nodes from the dict container
    auto box0 = action->Get("position-0")->GetObject<OpenGymBoxContainer<float>>();
    auto position0 = Vector(box0->GetValue(0), box0->GetValue(1), 0);
    auto box1 = action->Get("position-1")->GetObject<OpenGymBoxContainer<float>>();
    auto position1 = Vector(box1->GetValue(0), box1->GetValue(1), 0);

    // Set new position of action node to the midpoint between the two observation nodes
    auto newPosition = position0 + 0.5 * (position1 - position0);
    GetNode()->GetObject<MobilityModel>()->SetPosition(newPosition);
    NS_LOG_INFO("Set position of action node to " << newPosition);
}

NS_OBJECT_ENSURE_REGISTERED(PositionActionApp);
} // namespace ns3
