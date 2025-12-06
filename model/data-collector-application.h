#ifndef DATA_COLLECTOR_APPLICATION_H
#define DATA_COLLECTOR_APPLICATION_H

#include "rl-application.h"

namespace ns3
{
/**
 * @ingroup defiance
 * @class DataCollectorApplication
 * @brief This application is installed on a node where something is observed, possibly processed
 * and sent to an AgentApplication.
 *
 * Instances of this class do not send any data if they are not registered at a callback. To send
 * data this class should be inherited by a class which is registered at a callback and which sends
 * data received from the callback. In the child class, a custom method should be created and
 * registered at a callback to make sure it is regularly called. This method's signature has to
 * conform to the signature expected by the callback. The purpose of the custom method is to be able
 * to process data received from the callback and specify to which ChannelInterfaces it shall be
 * sent. Therefore, in the end of the custom method, \c Send() should be called with the according
 * data and InterfaceIds. If the data shall be sent to all ChannelInterfaces, the overloaded \c
 * Send() method without specifying InterfaceIds should be called.
 */
class DataCollectorApplication : public RlApplication
{
  public:
    DataCollectorApplication();
    ~DataCollectorApplication() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * @brief Calls \c RegisterCallbacks(). Can be overwritten if more setup is necessary. An
     * overwriting method has to call this method.
     */
    void Setup() override;

    /**
     * @brief Overwrite this method to register custom methods as callbacks.
     */
    virtual void RegisterCallbacks() = 0;

    /**
     * @brief Add a ChannelInterface to this DataCollectorApplication over which data can be sent to
     * an AgentApplication. \param remoteAppId the ID of the AgentApplication the interface is
     * connected to. \param interface the ChannelInterface connected to the specified
     * AgentApplication. \return the ID of the newly added interface among all interfaces of this
     * DataCollectorApplication which are connected to the specified AgentApplication.
     */
    uint AddAgentInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface) override;

    /**
     * @brief Delete a ChannelInterface that is currently connected to an AgentApplication.
     * @param remoteAppId the ID of the AgentApplication the interface is connected to.
     * @param interfaceId the ID of the ChannelInterface to delete. This is the ID among all
     * interfaces of this DataCollectorApplication which are connected to the specified
     * AgentApplication.
     */
    void DeleteAgentInterface(uint32_t remoteAppId, uint interfaceId) override;

  protected:
    /**
     * @brief Send data to the specified AgentApplication via the given
     * ChannelInterface.
     * @param data the data to send.
     * @param remoteAppId the ID of the AgentApplication to which the data is sent.
     * @param interfaceId the ID of the interface to use. This is the ID among all interfaces of
     * this DataCollectorApplication which are connected to the specified
     * AgentApplication.
     */
    void Send(Ptr<OpenGymDictContainer> data, uint32_t remoteAppId, uint32_t interfaceId);

    /**
     * @brief Send data to the specified AgentApplication via all according ChannelInterfaces.
     * @param data the data to send.
     * @param remoteAppId the ID of the AgentApplication to which the data is sent.
     */
    void Send(Ptr<OpenGymDictContainer> data, uint32_t remoteAppId);

    /**
     * @brief Send data to all connected AgentApplications (via all according
     * ChannelInterfaces). This method is not atomic. It might happen that the message is only sent
     * to some AgentApplications if \c m_running changes to \c false.
     * @param data the data to send.
     */
    void Send(Ptr<OpenGymDictContainer> data);

    using RlApplication::Send;

    InterfaceMap m_interfaces; //!< All interfaces connected to this DataCollectorApplication for
                               //!< sending to AgentApplications

    void DoDispose() override;
};

} // namespace ns3

#endif /* DATA_COLLECTOR_APPLICATION_H */
