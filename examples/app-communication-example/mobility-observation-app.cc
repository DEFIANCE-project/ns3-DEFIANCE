#include "mobility-observation-app.h"

#include "ns3/base-test.h"
#include "ns3/mobility-model.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("MobilityObservationApp");

TypeId
MobilityObservationApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::MobilityObservationApp")
                            .SetParent<ObservationApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<MobilityObservationApp>();
    return tid;
}

Ptr<OpenGymDictContainer>
MobilityObservationApp::CreateDictContainer(Vector position, double velocity)
{
    auto dictContainer = CreateObject<OpenGymDictContainer>();
    dictContainer->Add("position", MakeBoxContainer<float>(2, position.x, position.y));
    dictContainer->Add("velocity", MakeBoxContainer<double>(1, velocity));
    return dictContainer;
}

void
MobilityObservationApp::Observe(Ptr<const MobilityModel> observation)
{
    if (IsRunning())
    {
        auto position = observation->GetPosition();
        auto totalVelocity = observation->GetVelocity().GetLength();
        Send(CreateDictContainer(position, totalVelocity), 0);
        NS_LOG_INFO("Observation app " << GetId().applicationId << " sent to agent app:");
        NS_LOG_INFO("\tPosition: " << position);
        NS_LOG_INFO("\tVelocity: " << totalVelocity);
    }
}

void
MobilityObservationApp::RegisterCallbacks()
{
    NS_LOG_FUNCTION(this);
    // If the app is not yet installed on a node, it observes the course changes of all nodes
    std::string nodeId = "*";
    if (GetNode())
    {
        nodeId = std::to_string(GetNode()->GetId());
    }
    Config::ConnectWithoutContext("/NodeList/" + nodeId + "/$ns3::MobilityModel/CourseChange",
                                  MakeCallback(&MobilityObservationApp::Observe, this));
}

NS_OBJECT_ENSURE_REGISTERED(MobilityObservationApp);
} // namespace ns3
