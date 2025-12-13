#include "channel-interface.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ChannelInterface");

TypeId
ChannelInterface::GetTypeId()
{
    static TypeId tid = TypeId("ChannelInterface").SetParent<Object>().SetGroupName("defiance");
    return tid;
}

ChannelInterface::ChannelInterface()
{
    NS_LOG_FUNCTION(this);
}

ChannelInterface::~ChannelInterface()
{
    NS_LOG_FUNCTION(this);
}

void
ChannelInterface::AddRecvCallback(Callback<void, Ptr<OpenGymDictContainer>> callback)
{
    NS_LOG_FUNCTION(this);
    m_receiveCallbacks.ConnectWithoutContext(callback);
}

void
ChannelInterface::RemoveRecvCallback(Callback<void, Ptr<OpenGymDictContainer>> callback)
{
    NS_LOG_FUNCTION(this);
    m_receiveCallbacks.DisconnectWithoutContext(callback);
}

TracedCallback<Ptr<OpenGymDictContainer>>
ChannelInterface::GetReceiveCallbacks() const
{
    NS_LOG_FUNCTION(this);
    return m_receiveCallbacks;
}

ConnectionStatus
ChannelInterface::GetConnectionStatus() const
{
    return m_connectionStatus;
}

void
ChannelInterface::SetConnectionStatus(ConnectionStatus status)
{
    m_connectionStatus = status;
}

/**
 * @ingroup defiance
 * Unnamed namespace
 */
namespace
{

/**
 * Helper class to automatically shutdown protobuf library on program end.
 */
class ProtobufShutdown
{
  public:
    ~ProtobufShutdown()
    {
        google::protobuf::ShutdownProtobufLibrary();
    };
};

ProtobufShutdown sProtobufShutdown; //!< Static variable for protobuf shutdown

} // namespace
