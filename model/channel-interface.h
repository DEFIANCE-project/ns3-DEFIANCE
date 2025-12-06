#ifndef CHANNEL_INTERFACE_H
#define CHANNEL_INTERFACE_H

#include "ns3/container.h"
#include "ns3/traced-callback.h"

namespace ns3
{
enum ConnectionStatus
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
};

/**
 * @ingroup defiance
 * @class ChannelInterface
 * @brief The ChannelInterface class provides the basic abstraction of one side of a communication
 * channel in the defiance framework. It must always be used in connection with another
 * ChannelInterface object.
 */

class ChannelInterface : public Object
{
  public:
    /*
     * @warning The default constructor must not be used.
     */
    ChannelInterface();
    ~ChannelInterface() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * @brief Add a callback that is called when a message is received at the interface.
     * @param callback the callback to be called with the content of the received message.
     */
    void AddRecvCallback(Callback<void, Ptr<OpenGymDictContainer>> callback);

    /**
     * @brief Remove a callback from the list of callbacks that are called when a message is
     * received at the interface.
     * @param callback the callback to be removed.
     */
    void RemoveRecvCallback(Callback<void, Ptr<OpenGymDictContainer>> callback);

    /**
     * @brief Send data to the communication partner.
     * @param data the data to be sent.
     */
    virtual int Send(Ptr<OpenGymDictContainer> data) = 0;

    /**
     * @brief Establish a connection to the communication partner.
     */
    virtual ConnectionStatus Connect(Ptr<ChannelInterface> otherInterface) = 0;

    /**
     * @brief Disconnect from the communication partner (both sides will be disconnected).
     */
    virtual void Disconnect() = 0;

    /**
     * @brief Get the trace source from which receive callbacks are executed.
     */
    TracedCallback<Ptr<OpenGymDictContainer>> GetReceiveCallbacks() const;

    /**
     * @brief Get the connection status of the channel.
     */
    ConnectionStatus GetConnectionStatus() const;

    /**
     * @brief Set the connection status of the channel.
     * @param status the new connection status.
     */
    void SetConnectionStatus(ConnectionStatus status);

  private:
    ConnectionStatus m_connectionStatus{DISCONNECTED}; ///< The connection status of the channel
    TracedCallback<Ptr<OpenGymDictContainer>>
        m_receiveCallbacks; ///< The callbacks to be called when a message is received
};
} // namespace ns3

#endif /* CHANNEL_INTERFACE_H */
