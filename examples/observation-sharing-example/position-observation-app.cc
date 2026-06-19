#include "position-observation-app.h"

#include "ns3/base-test.h"

#include <string>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("PositionObservationApp");

TypeId
PositionObservationApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PositionObservationApp")
                            .SetParent<ObservationApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<PositionObservationApp>();
    return tid;
}

void
PositionObservationApp::Observe(Ptr<const MobilityModel> observation)
{
    if (IsRunning())
    {
        NS_LOG_FUNCTION(this << observation);
        auto position = observation->GetPosition();
        auto box = MakeBoxContainer<float>(3, position.x, position.y, position.z);
        Send(MakeDictContainer("position", box), 0);
    }
}

void
PositionObservationApp::RegisterCallbacks()
{
    NS_LOG_FUNCTION(this);
    // If the app is not yet installed on a node, it observes the course changes of all nodes
    std::string nodeId = "*";
    if (GetNode())
    {
        nodeId = std::to_string(GetNode()->GetId());
    }
    Config::ConnectWithoutContext("/NodeList/" + nodeId + "/$ns3::MobilityModel/CourseChange",
                                  MakeCallback(&PositionObservationApp::Observe, this));
}

NS_OBJECT_ENSURE_REGISTERED(ObservationApplication);
} // namespace ns3
