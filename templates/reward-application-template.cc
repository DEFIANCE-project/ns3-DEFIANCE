#include "ns3/defiance-module.h"
#include "ns3/reward-application.h"

class TemplateRewardApplication : public RewardApplication
{
  public:
    TemplateRewardApplication() {};
    ~TemplateRewardApplication() override {};

    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::TemplateRewardApplication")
                                .SetParent<RewardApplication>()
                                .SetGroupName("defiance")
                                .AddConstructor<TemplateRewardApplication>();
        return tid;
    }

    void Reward(Ptr<OpenGymDictContainer> observation)
    {
        // This is a custom function. You can rename it and change the signature according to your
        // callback in RegisterCallbacks() that references this function.

        if (IsRunning())
        { // Usually, observations shall only be processed if the application is running
          // NS_LOG_INFO("Received observation");
          // auto reward = Ptr<OpenGymDictContainer>();
          // Send(reward, 0);
        }
    }

    void RegisterCallbacks() override
    {
        // register your desired callback here
        // auto trace = DynamicCast<PendulumCart>(GetNode()->m_reportCarStatsTrace);
        // trace.ConnectWithoutContext(MakeCallback(&TemplateRewardApplication::Reward, this));
    }
};

NS_OBJECT_ENSURE_REGISTERED(TemplateRewardApplication);
