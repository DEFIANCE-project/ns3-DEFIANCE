#ifndef AGENT_APPLICATION_H
#define AGENT_APPLICATION_H

#include "history-container.h"
#include "rl-application.h"

namespace ns3
{
/**
 * @ingroup defiance
 * @class AgentApplication
 * @brief This application is installed on a node on which the RL agent is to be placed in the
 * network.
 *
 * Before the AgentApplication is started \c Setup() should be called to initiate necessary
 * configurations. In order to receive observations or rewards from other applications, the
 * AgentApplication needs to add the according interfaces with \c AddObservationInterface(),
 * \c AddRewardInterface(), \c AddActionInterface() or \c AddAgentInterface(). When an observation
 * or a reward is received, the AgentApplication stores it in a data structure and calls
 * \c OnRecvObs() resp. \c OnRecvReward() with the ID of the interface the observation or reward was
 * received from. The AgentApplication needs to implement \c OnRecvObs() and \c OnRecvReward() to
 * handle received observations and rewards.
 */
class AgentApplication : public RlApplication
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();

    AgentApplication();
    ~AgentApplication() override;

    /**
     * @brief Add a ChannelInterface to this AgentApplication over which observations from an
     * ObservationApplication can be received.
     * @param remoteAppId the ID of the ObservationApplication the interface is connected to.
     * @param interface the interface for receiving observations from specific
     * ObservationApplication.
     * @return the ID of the newly added interface among all interfaces of this AgentApplication
     * which are connected to the specified ObservationApplication.
     */
    uint AddObservationInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface) override;

    /**
     * @brief Add a ChannelInterface to this AgentApplication over which rewards from a
     * RewardApplication can be received.
     * @param remoteAppId the ID of the RewardApplication the interface is connected to.
     * @param interface the interface for receiving rewards from specific RewardApplication.
     * @return the ID of the newly added interface among all interfaces of this
     * AgentApplication which are connected to the specified RewardApplication.
     */
    uint AddRewardInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface) override;

    /**
     * @brief Add a ChannelInterface to this AgentApplication over which it can communicate with
     * another AgentApplication.
     * @param remoteAppId the ID of the AgentApplication the interface is connected to.
     * @param interface the interface for communicating with specific AgentApplication.
     * @return the ID of the newly added interface among all interfaces of this
     * AgentApplication which are connected to the specified (remote) AgentApplication.
     */
    uint AddAgentInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface) override;

    /**
     * @brief Add a ChannelInterface to this AgentApplication over which it can send actions to an
     * ActionApplication.
     * @param remoteAppId the ID of the ActionApplication the interface is connected to.
     * @param interface the interface for sending actions to specific ActionApplication.
     * @return the ID of the newly added interface among all interfaces of this
     * AgentApplication which are connected to the specified ActionApplication.
     */
    uint AddActionInterface(uint32_t remoteAppId, Ptr<ChannelInterface> interface) override;

    /**
     * @brief Delete a ChannelInterface that is currently connected to an ObservationApplication.
     * @param remoteAppId the ID of the ObservationApplication the interface is connected to.
     * @param interfaceId the ID of the ChannelInterface to delete. This is the ID among all
     * interfaces of this AgentApplication which are connected to the specified
     * ObservationApplication.
     */
    void DeleteObservationInterface(uint32_t remoteAppId, uint interfaceId) override;

    /**
     * @brief Delete a ChannelInterface that is currently connected to a RewardApplication.
     * @param remoteAppId the ID of the RewardApplication the interface is connected to.
     * @param interfaceId the ID of the ChannelInterface to delete. This is the ID among all
     * interfaces of this AgentApplication which are connected to the specified
     * RewardApplication.
     */
    void DeleteRewardInterface(uint32_t remoteAppId, uint interfaceId) override;

    /**
     * @brief Delete a ChannelInterface that is currently connected to a remote AgentApplication.
     * @param remoteAppId the ID of the AgentApplication the interface is connected to.
     * @param interfaceId the ID of the ChannelInterface to delete. This is the ID among all
     * interfaces of this AgentApplication which are connected to the specified
     * AgentApplication.
     */
    void DeleteAgentInterface(uint32_t remoteAppId, uint interfaceId) override;

    /**
     * @brief Delete a ChannelInterface that is currently connected to an ActionApplication.
     * @param remoteAppId the ID of the ActionApplication the interface is connected to.
     * @param interfaceId the ID of the ChannelInterface to delete. This is the ID among all
     * interfaces of this AgentApplication which are connected to the specified
     * ActionApplication.
     */
    void DeleteActionInterface(uint32_t remoteAppId, uint interfaceId) override;

    /**
     * @brief Initialize the data structures for storing observations resp. rewards and the
     * observation and action spaces. Can be overridden if needed.
     */
    void Setup() override;

    /**
     * @brief Send a message to the specified AgentApplication via the given
     * ChannelInterface.
     * @param data the message to send.
     * @param appId the ID of the AgentApplication to which the message is sent.
     * @param interfaceId the ID of the interface to use. This is the ID among all interfaces of
     * this AgentApplication which are connected to the specified (receiving) AgentApplication.
     */
    void SendToAgent(Ptr<OpenGymDictContainer> data, uint32_t appId, uint32_t interfaceId);

    /**
     * @brief Send a message to the specified AgentApplication via all according ChannelInterfaces.
     * @param data the message to send.
     * @param appId the ID of the AgentApplication to which the message is sent.
     */
    void SendToAgent(Ptr<OpenGymDictContainer> data, uint32_t appId);

    /**
     * @brief Send a message to all connected AgentApplications (via all according
     * ChannelInterfaces). This method is not atomic. It might happen that the message is only sent
     * to some AgentApplications if \c m_running changes to \c false.
     * @param data the message to send.
     */
    void SendToAgent(Ptr<OpenGymDictContainer> data);

    /**
     * @brief Send an action to the specified ActionApplication via the given
     * ChannelInterface.
     * @param action the action to send.
     * @param appId the ID of the ActionApplication to which the action is sent.
     * @param interfaceId the ID of the interface to use. This is the ID among all interfaces of
     * this AgentApplication which are connected to the specified ActionApplication.
     */
    void SendAction(Ptr<OpenGymDictContainer> action, uint32_t appId, uint32_t interfaceId);

    /**
     * @brief Send an action to the specified ActionApplication via all according ChannelInterfaces.
     * @param action the action to send.
     * @param appId ID of the ActionApplication to which the action is sent.
     */
    void SendAction(Ptr<OpenGymDictContainer> action, uint32_t appId);

    /**
     * @brief Send an action to all connected ActionApplications  (via all according
     * ChannelInterfaces). This method is not atomic. It might happen that the action is only sent
     * to some ActionApplications if \c m_running changes to \c false.
     * @param action the action to send.
     */
    void SendAction(Ptr<OpenGymDictContainer> action);

    /**
     * @brief Requests this agent to be truncated.
     * In the next interaction through ns3-ai, isGameOver will be set.
     * Scheduling this message should be preferred to using \c Simulator::Stop
     */
    void RequestTruncation();

  protected:
    void DoDispose() override;

    uint m_maxObservationHistoryLength; //!< maximum length of the history of each observation deque
                                        //!< to store
    uint m_maxRewardHistoryLength; //!< maximum length of the history of each reward deque to store
    bool m_obsTimestamping;        //!< enable ns3 timestamps for observations
    bool m_rewardTimestamping;     //!< enable ns3 timestamps for rewards
    HistoryContainer
        m_obsDataStruct; //!< a data structure in which received observations are stored
    HistoryContainer m_rewardDataStruct; //!< a data structure in which received rewards are stored
    Ptr<OpenGymDataContainer>
        m_observation; //!< the current observation which is used in \c InferAction()
    float m_reward;    //!< the current reward which is used in \c InferAction()

    InterfaceMap m_observationInterfaces;
    InterfaceMap m_rewardInterfaces;
    InterfaceMap m_agentInterfaces;
    InterfaceMap
        m_actionInterfaces; //!< Interfaces connecting this AgentApplication to ActionApplications

    /**
     * @brief Called when an observation was received. The observation was already stored in
     * \c m_obsDataStruct.
     * Once there are enough observations, \c InferAction() should be called to begin the inference
     * cycle.
     * @param remoteAppId ID of the ObservationApplication from which the observation was received.
     */
    virtual void OnRecvObs(uint remoteAppId) = 0;

    /**
     * @brief Called when a message is received from an agent.
     * @param remoteAppId ID of the RewardApplication from which the reward was received.
     */
    virtual void OnRecvFromAgent(uint remoteAppId, Ptr<OpenGymDictContainer> payload);

    /**
     * @brief Called when a reward was received. The reward was already stored in
     * \c m_rewardDataStruct.
     * @param remoteAppId ID of the RewardApplication from which the reward was received.
     */
    virtual void OnRecvReward(uint remoteAppId) = 0;

    /**
     * Send observation and reward to the python agent and infer an action. This action is passed to
     * \c SendAction() for forwarding. It uses \c m_observation and \c m_reward.
     */
    void InferAction();

    /**
     * Send observation and reward to the python agent and infer an action. This action is passed to
     * \c SendAction() for forwarding. It uses \c m_observation and \c m_reward.
     * @param remoteAppId ID of the ActionApplication to which the action shall be passed
     */
    void InferAction(uint remoteAppId);

  private:
    /**
     * @brief Callback when an observation is received over the ChannelInterface. It stores the
     * observation in \c m_obsDataStruct and calls \c OnRecvObs() with the ID of the interface.
     * @param remoteAppId ID of the ObservationApplication from which the observation was
     * received.
     * @param observation observation that was received.
     */
    void ReceiveObservation(uint remoteAppId, Ptr<OpenGymDictContainer> observation);

    /**
     * @brief Callback when an reward is received over the ChannelInterface. It stores the
     * reward in \c m_rewardDataStruct and calls \c OnRecvReward() with the ID of the interface.
     * @param remoteAppId ID of the \¢ RewardApplication from which the reward was received.
     * @param reward reward that was received.
     */
    void ReceiveReward(uint remoteAppId, Ptr<OpenGymDictContainer> reward);

    virtual Ptr<OpenGymSpace> GetObservationSpace() = 0;
    virtual Ptr<OpenGymSpace> GetActionSpace() = 0;

    /**
     * @brief This method defines what the AgentApplication does with actions it receives from Ray.
     * Usually, it calls \c SendAction(), i.e. the action is sent to all action interfaces. Should
     * be overwritten by the user if only specific ActionApplications should be addressed.
     * @param action action to send to ActionApps directly corresponding to the action space.
     */
    virtual void InitiateAction(Ptr<OpenGymDataContainer> action);

    /**
     * @brief This method defines what the AgentApplication does with actions it receives from Ray.
     * Usually, it calls \c SendAction(), i.e. the action is sent to the specified ActionApplication
     * via all available interfaces.
     * @param remoteAppId ID of the ActionApplication to which the action is sent.
     * @param action action to send to ActionApps directly corresponding to the action space.
     */
    virtual void InitiateActionForApp(uint remoteAppId, Ptr<OpenGymDataContainer> action);

    /**
     * @brief Overwrite this method to send extra information to the tensorflow board.
     */
    virtual std::map<std::string, std::string> GetExtraInfo()
    {
        return {};
    }

    /**
     * @brief Overwrite this method to set the delay for receiving an action from the agent model.
     * Used to model the computation time of inference or local training.
     */
    virtual Time GetActionDelay()
    {
        return Seconds(0);
    }

    bool m_truncationRequested =
        false; /// indicate if game over should be indicated in the next ns3-ai interaction
};

} // namespace ns3

#endif
