#ifndef RL_APPLICATION_H
#define RL_APPLICATION_H

#include "channel-interface.h"

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"

namespace ns3
{

typedef std::unordered_map<uint32_t, std::map<uint, Ptr<ChannelInterface>>> InterfaceMap;

enum ApplicationType
{
    OBSERVATION,
    REWARD,
    AGENT,
    ACTION
};

std::string ApplicationTypeToString(ApplicationType type);

/**
 * @ingroup defiance
 *
 * @brief An unique identifier for every RlApplication.
 * Used e.g. by the ChannelInterface to differentiate different RlApplications.
 */
struct RlApplicationId
{
    ApplicationType applicationType; //!< Code for RL application type
    uint32_t applicationId;          //!< Assumes that we have less than 2^32 RL applications

    std::string ToString() const;
};

/**
 * @ingroup defiance
 * @class RlApplication
 * @brief This class embodies the core functionality of all RL applications.
 *
 */
class RlApplication : public Application
{
  public:
    RlApplication();
    ~RlApplication() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * @brief Set ID that is unique among all RL applications.
     * @param id unique ID among RLApplications.
     */
    void SetId(RlApplicationId id);

    /**
     * @brief Retrieve the RlApplicationId of a specific RL application.
     * @return the desired RlApplicationId.
     */
    RlApplicationId GetId();

    /**
     * @brief Set default IP address that is used for new interfaces.
     * @param address default address for new interfaces.
     */
    void SetDefaultAddress(Ipv4Address address);

    /**
     * @brief Retrieve default IP address for new interfaces.
     * @return default address.
     */
    Ipv4Address GetDefaultAddress();

    /**
     * @brief Called after configuration is finished.
     */
    virtual void Setup();

    /**
     * @brief Add a new interface to this app.
     * @param applicationId ID identifying the remote application sending and receiving through the
     * \c interface.
     * @param interface interface for this connection to the app.
     * @return the index of the newly added interface in the list of interfaces connected to the
     * same remote application.
     */
    uint AddInterface(RlApplicationId applicationId, Ptr<ChannelInterface> interface);

    /**
     * Delete an interface of this app.
     * @param applicationId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interfaceId index of the interface among all interfaces which are connected to the
     * specified RlApplication and registered in this RlApplication.
     */
    void DeleteInterface(RlApplicationId applicationId, uint interfaceId);

  protected:
    /**
     * @brief Send data to the specified interfaces if the application is currently running.
     * @param data the data to send.
     * @param interfaces interfaces over which the data is sent.
     */
    virtual void Send(Ptr<OpenGymDictContainer> data,
                      const std::vector<Ptr<ChannelInterface>>& interfaces);

    /**
     * @brief Send data to the specified interfaces if the application is currently running.
     * @param data the data to send.
     * @param interfaces interfaces over which the data is sent.
     */
    virtual void Send(Ptr<OpenGymDictContainer> data,
                      const std::map<uint, Ptr<ChannelInterface>>& interfaces);

    /**
     * @brief Send data to the specified interfaces if the application is currently running.
     * This method is not atomic. It might happen that \c data is only sent
     * to some RlApplications if \c m_running changes to \c false.
     * @param data the data to send.
     * @param interfaces interfaces over which the data is sent.
     */
    virtual void Send(Ptr<OpenGymDictContainer> data, const InterfaceMap& interfaces);

    /**
     * @brief Set the application to running mode so that observations can be sent.
     */
    void StartApplication() override;

    /**
     * @brief Stop running mode so that observations cannot be sent anymore.
     */
    void StopApplication() override;

    /**
     * @brief Return whether application is currently in running mode.
     */
    bool IsRunning();

    bool m_running; //!< If true, messages to other RlApplications can be sent, if false, Send
                    //!< methods throw all messages away

  private:
    RlApplicationId m_id;         //!< unique ID among RL applications
    Ipv4Address m_defaultAddress; //!< default IP address

    /**
     * @brief Called whenever an observation app connects to this application. By default, fail on
     * incoming connections, as subclasses have to implement this method.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interface interface for this connection to the app.
     * @return the ID of the newly added interface in the list of interfaces connected to the same
     * remote application.
     */
    virtual uint AddObservationInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface);
    /**
     * Called whenever an reward app connects to this application. By default, fail on incoming
     * connections, as subclasses have to implement this method.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interface interface for this connection to the app.
     * @return the ID of the newly added interface in the list of interfaces connected to the same
     * remote application.
     */
    virtual uint AddRewardInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface);
    /**
     * Called whenever an action app connects to this application. By default, fail on incoming
     * connections, as subclasses have to implement this method.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interface interfac efor this connection to the app.
     * @return the ID of the newly added interface in the list of interfaces connected to the same
     * remote application.
     */
    virtual uint AddActionInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface);
    /**
     * Called whenever an agent app connects to this application. By default, fail on incoming
     * connections, as subclasses have to implement this method.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interface interface for this connection to the app.
     * @return the ID of the newly added interface in the list of interfaces connected to the same
     * remote application.
     */
    virtual uint AddAgentInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface);

    /**
     * Called whenever an observation app disconnects from this application.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interfaceId among all interfaces which are connected to the specified
     * \c RlApplication and registered in this \c RlApplication.
     */
    virtual void DeleteObservationInterface(uint32_t remoteAppId, uint interfaceId);
    /**
     * Called whenever an reward app disconnects from this application.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interfaceId among all interfaces which are connected to the specified
     * \c RlApplication and registered in this \c RlApplication.
     */
    virtual void DeleteRewardInterface(uint32_t remoteAppId, uint interfaceId);
    /**
     * Called whenever an action app disconnects from this application.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interfaceId among all interfaces which are connected to the specified
     * \c RlApplication and registered in this \c RlApplication.
     */
    virtual void DeleteActionInterface(uint32_t remoteAppId, uint interfaceId);
    /**
     * Called whenever an agent app disconnects from this application.
     *
     * @param remoteAppId to identify the remote application sending and receiving through the
     * \c interface.
     * @param interfaceId among all interfaces which are connected to the specified
     * \c RlApplication and registered in this \c RlApplication.
     */
    virtual void DeleteAgentInterface(uint32_t remoteAppId, uint interfaceId);
};

} // namespace ns3

#endif /* RL_APPLICATION_H */
