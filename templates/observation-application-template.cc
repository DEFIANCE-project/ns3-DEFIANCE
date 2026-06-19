#include "ns3/defiance-module.h"
#include "ns3/observation-application.h"

class TemplateObservationApplication : public ObservationApplication
{
  public:
    TemplateObservationApplication()
        : ObservationApplication() {};

    ~TemplateObservationApplication() override = default;

    TypeId GetTypeId() const
    {
        static TypeId tid = TypeId("TemplateObservationApplication")
                                .SetParent<ObservationApplication>()
                                .AddConstructor<TemplateObservationApplication>();
        return tid;
    }

    void Observe(Ptr<OpenGymDictContainer> observation)
    {
        // This is a custom function. You can rename it and change the signature according to your
        // callback in RegisterCallbacks() that references this function.

        if (IsRunning())
        { // Usually, observations shall only be processed if the application is running
          // NS_LOG_INFO("Received observation");
          // Send(observation, 0);
        }
    }

    void RegisterCallbacks() override
    {
        // register your desired callback here
        // auto trace = DynamicCast<PendulumCart>(GetNode()->m_reportCarStatsTrace);
        // trace.ConnectWithoutContext(MakeCallback(&TemplateObservationApplication::Observe,
        // this));
    }
};
