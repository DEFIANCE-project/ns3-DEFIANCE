#ifndef NS3_SOCKET_CHANNEL_INTERFACE_H
#define NS3_SOCKET_CHANNEL_INTERFACE_H

#include "channel-interface.h"

#include "ns3/socket.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"

namespace ns3
{

/**
 * @ingroup defiance
 * @class SocketChannelInterface
 * @brief The socket channel interface class provides the basic abstraction of one side of a socket
 * communication channel in the defiance framework. It must always be used in connection with
 * another socket channel interface object.
 */

// TODO: Overall provide tests for this class. The tests for the simple channel interface could be a
// good starting point.
class SocketChannelInterface : public ChannelInterface
{
  public:
    SocketChannelInterface(Ptr<Node> localNode,
                           Ipv4Address localAddress,
                           TypeId transmissionProtocol);
    ~SocketChannelInterface() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * @brief Sends data to the communication partner.
     * @param data data to send.
     * @return the number of bytes accepted for transmission if no error occurs, and -1 otherwise
     */
    int Send(Ptr<OpenGymDictContainer> data) override;

    /**
     * @brief Establish a connection to the communication partner.
     * @param otherInterface the ChannelInterface representing the other end of the communication
     * channel.
     */
    ConnectionStatus Connect(Ptr<ChannelInterface> otherInterface) override;

    /**
     * @brief Disconnects from the communication partner (both sides will be disconnected).
     */
    void Disconnect() override;

  protected:
    /**
     * @brief Serialize the content of a message to a buffer that can be sent as payload.
     * @param data the message to serialize.
     * @return the serialized message in the form of a packet.
     */
    Ptr<Packet> Serialize(Ptr<OpenGymDictContainer> data);

    /**
     * @brief Deserialize the content of a message from a packet into a OpenGymContainer.
     * @param packet the packet containing the message to deserialize.
     * @return the deserialized packet in form of an OpenGymContainer.
     */
    Ptr<OpenGymDictContainer> Deserialize(Ptr<Packet> packet);

    void DoDispose() override;

  private:
    /**
     * @brief Register new connections on this channel.
     * @param socket The new active connection socket.
     * @param address The address of the connected client on this socket.
     */
    void AddConnection(Ptr<Socket> socket, const Address& address);

    /**
     * @brief Unregister existing connection from this channel.
     * @param socket the local socket from the old connection.
     */
    void RemoveConnection(Ptr<Socket> socket);

    /**
     * @brief Send raw uint8 data over the sockets to the communication partners.
     * @param packet the packet containing the data to send.
     * @return the number of bytes accepted for transmission; -1 on error.
     */
    int SendRaw(Ptr<Packet> packet);

    /**
     * @brief Receive packets at a socket and initiates the processing of the received packet
     * payload.
     * @param socket the socket from which the packet is received.
     */
    void Receive(Ptr<Socket> socket);

    /**
     * @brief Implement a one-sided connection attempt to the provided interface.
     * @param otherInterface the interface to which the connection should be established.
     * @return \c true if the connection was successful, \c false otherwise.
     */
    bool DoConnect(Ptr<SocketChannelInterface> otherInterface);

    /**
     * @brief Disconnect from the communication partner without disconnecting the other side.
     */
    void PartialDisconnect();

    Ptr<SocketChannelInterface> m_communicationPartner{
        nullptr};               ///< The other end of the communication channel
    Ptr<Socket> m_localSocket;  ///< The socket used for receiving connections
    Ptr<Socket> m_remoteSocket; ///< The socket used for sending messages (in case of TCP)
    Address m_localAddress;     ///< The address that the local socket is bound to.
};
} // namespace ns3

#endif // NS3_SOCKET_CHANNEL_INTERFACE_H
