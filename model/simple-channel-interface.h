#ifndef NS3_SIMPLE_CHANNEL_INTERFACE_H
#define NS3_SIMPLE_CHANNEL_INTERFACE_H

#include "channel-interface.h"

#include "ns3/nstime.h"

namespace ns3
{

/**
 * @ingroup defiance
 * @class SimpleChannelInterface
 * @brief A very basic implementation of a communication channel that allows the user to specify a
 * delay for propagation. It must always be used in connection with another simple channel interface
 * object.
 */

class SimpleChannelInterface : public ChannelInterface
{
  public:
    SimpleChannelInterface();
    ~SimpleChannelInterface() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * @brief Send data to the communication partner. This data transfer is delayed by the
     * propagation delay this interface has.
     * @param data data to send.
     * @return the number of bytes accepted for transmission if no error occurs, and -1 otherwise
     */
    int Send(Ptr<OpenGymDictContainer> data) override;

    /**
     * @brief Establish a connection to the communication partner.
     * @param otherInterface the interface representing the other end of the communication channel
     * @return the own connection status after the connection attempt (0 DISCONNECTED, 1
     * CONNECTING, 2 CONNECTED)
     */
    ConnectionStatus Connect(Ptr<ChannelInterface> otherInterface) override;

    /**
     * @brief Disconnect from the communication partner (both sides will be disconnected).
     */
    void Disconnect() override;

    /**
     * @brief Set the propagation delay of the channel when sending from this side of the channel.
     * @param delay the time the messages are delayed
     */
    void SetPropagationDelay(Time delay);

    /**
     * @brief Get the propagation delay of the channel when sending from this side of the channel.
     * @return the propagation delay.
     */
    Time GetPropagationDelay() const;

    /**
     * @brief Get the ChannelInterface representing the other end of the communication channel.
     * @return the communication partner.
     */
    Ptr<SimpleChannelInterface> GetCommunicationPartner() const;

    /**
     * @brief Set the communication partner.
     * @param communicationPartner the ChannelInterface representing the other end of the
     * communication channel.
     */
    void SetCommunicationPartner(Ptr<SimpleChannelInterface> communicationPartner);

  private:
    /**
     * @brief Disconnect from the communication partner without disconnecting the other side.
     */
    void PartialDisconnect();

    Ptr<SimpleChannelInterface> m_communicationPartner; ///< The ChannelInterface representing other
                                                        ///< end of the communication channel
    Time m_propagationDelay; ///< The time it takes for a message to be transmitted from one
                             ///< end of the channel to the other
};
} // namespace ns3

#endif // NS3_SIMPLE_CHANNEL_INTERFACE_H
