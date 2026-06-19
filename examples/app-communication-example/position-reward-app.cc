#include "position-reward-app.h"

#include "ns3/base-test.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("PositionRewardApp");

TypeId
PositionRewardApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PositionRewardApp")
                            .SetParent<RewardApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<PositionRewardApp>();
    return tid;
}

Ptr<OpenGymDictContainer>
PositionRewardApp::CreateDictContainer(float reward)
{
    auto box = MakeBoxContainer<float>(3);
    box->AddValue(reward);

    auto dictContainer = CreateObject<OpenGymDictContainer>();
    dictContainer->Add("distance", box);
    return dictContainer;
}

void
PositionRewardApp::Reward(Ptr<const MobilityModel> observation)
{
    if (IsRunning())
    {
        // The reward is the length of the position vector, i.e. the distance to the origin of
        // coordinates
        float reward = observation->GetPosition().GetLength();
        Send(CreateDictContainer(reward), 0);
        NS_LOG_INFO("Reward app sent to agent app:");
        NS_LOG_INFO("\tReward: " << reward);
    }
}

void
PositionRewardApp::RegisterCallbacks()
{
    NS_LOG_FUNCTION(this);
    Config::ConnectWithoutContext("/NodeList/" + std::to_string(GetNode()->GetId()) +
                                      "/$ns3::MobilityModel/CourseChange",
                                  MakeCallback(&PositionRewardApp::Reward, this));
}

NS_OBJECT_ENSURE_REGISTERED(PositionRewardApp);

} // namespace ns3
