#include "no-op-action-app.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("NoOpActionApp");

NoOpActionApp::NoOpActionApp()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NoOpActionApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NoOpActionApp")
                            .SetParent<ActionApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<NoOpActionApp>();
    return tid;
}

void
NoOpActionApp::ExecuteAction(uint32_t remoteAppId, Ptr<ns3::OpenGymDictContainer> action)
{
}

NS_OBJECT_ENSURE_REGISTERED(NoOpActionApp);
} // namespace ns3
