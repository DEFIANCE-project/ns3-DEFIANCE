#ifndef BASE_TEST_H
#define BASE_TEST_H

#include "action-application.h"
#include "agent-application.h"
#include "observation-application.h"
#include "reward-application.h"

#include "ns3/ai-module.h"
#include "ns3/node-container.h"
#include "ns3/test.h"

#include <vector>

namespace ns3
{
Ptr<OpenGymDictContainer> MakeDictContainer(std::string key, Ptr<OpenGymDataContainer> data);

template <typename T>
Ptr<OpenGymBoxContainer<T>>
MakeBoxContainer(uint32_t shape)
{
    std::vector<uint32_t> vShape = {shape};
    return CreateObject<OpenGymBoxContainer<T>>(vShape);
}

template <class T, class... Params>
Ptr<OpenGymBoxContainer<T>>
MakeBoxContainer(uint32_t shape, Params... params)
{
    auto obj = MakeBoxContainer<T>(shape);
    for (const auto& data : {params...})
    {
        obj->AddValue(data);
    }
    return obj;
}

extern Callback<void, Ptr<OpenGymDataContainer>> noopCallback;

template <typename T>
Ptr<OpenGymBoxSpace>
MakeBoxSpace(uint32_t shape, float low, float high)
{
    std::vector<uint32_t> vShape = {shape};
    return CreateObject<OpenGymBoxSpace>(low, high, vShape, TypeNameGet<T>());
}

template <typename T>
Ptr<OpenGymBoxSpace>
MakeBoxSpace(uint32_t shape, std::vector<float> low, std::vector<float> high)
{
    std::vector<uint32_t> vShape = {shape};
    return CreateObject<OpenGymBoxSpace>(low, high, vShape, TypeNameGet<T>());
}

template <class T, class... Params>
Ptr<OpenGymDictContainer>
MakeDictBoxContainer(uint32_t shape, std::string key, Params... params)
{
    return MakeDictContainer(key, MakeBoxContainer<T>(shape, params...));
}

/**
 * @ingroup defiance-tests
 * @class RlAppBaseTestCase
 * @brief Base test case class that unifies setup functionality for different tests.
 * This is not a full test case yet, actual tests need to inherit from it.
 */
class RlAppBaseTestCase : public TestCase
{
  public:
    RlAppBaseTestCase(std::string name);
    ~RlAppBaseTestCase() override;

    void ReceiveData(int id, Ptr<OpenGymDictContainer> data);
    void CreateInterfaces();

  protected:
    void ScenarioSetup();
    void DoRun() override;
    void DoTeardown() override;
    virtual void Simulate() = 0;

    std::vector<float> m_receivedData;
    std::vector<Ptr<ChannelInterface>> m_sendingInterfaces;
    std::vector<Ptr<ChannelInterface>> m_receivingInterfaces;
    Ptr<Node> m_node;
};

/**
 * @ingroup defiance-tests
 * @class TestDataCollectorApp
 * @brief Simple DataCollecorApplication that can be reused for tests.
 */
class TestDataCollectorApp : public DataCollectorApplication
{
  public:
    TestDataCollectorApp();
    ~TestDataCollectorApp() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();
    void CollectData(float data, int appId, int interfaceIndex);
    /**
     * @brief Executes the callback registered in \c RegisterCallbacks().
     * This allows to simulate the behaviour of a real tracing source by
     * calling \c ExecuteCallback() from outside this class.
     *
     * @param arg argument which the callback will get
     */
    void ExecuteCallback(float arg, int arg2, int arg3);
    void RegisterCallbacks() override;

  protected:
    Callback<void, float, int, int> m_callback;
};

/**
 * @ingroup DEFIANCE-tests
 * @class TestObservationApp
 * @brief Simple ObservationApplication that can be reused for tests.
 */
class TestObservationApp : public ObservationApplication
{
  public:
    TestObservationApp();
    ~TestObservationApp() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();
    virtual void Observe(float observation, int appId, int interfaceIndex);
    /**
     * @brief Executes the callback registered in \c RegisterCallbacks().
     * This allows to simulate the behaviour of a real tracing source by
     * calling \c ExecuteCallback() from outside this class.
     *
     * @param arg argument which the callback will get
     */
    void ExecuteCallback(float arg, int arg2, int arg3);
    void RegisterCallbacks() override;

  protected:
    Callback<void, float, int, int> m_callback;
};

/**
 * @ingroup DEFIANCE-tests
 * @class TestRewardApp
 * @brief Simple RewardApplication that can be reused for tests.
 */
class TestRewardApp : public RewardApplication
{
  public:
    TestRewardApp();
    ~TestRewardApp() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();
    virtual void Reward(float reward, int appId, int interfaceIndex);
    /**
     * @brief Executes the callback registered in \c RegisterCallbacks().
     * This allows to simulate the behaviour of a real tracing source by
     * calling \c ExecuteCallback() from outside this class.
     *
     * @param arg argument which the callback will get
     */
    void ExecuteCallback(float arg, int arg2, int arg3);
    void RegisterCallbacks() override;

  protected:
    Callback<void, float, int, int> m_callback;
};

/**
 * @ingroup DEFIANCE-tests
 * @class TestAgentApp
 * @brief Simple AgentApplication that can be reused for tests.
 */
class TestAgentApp : public AgentApplication
{
  public:
    TestAgentApp() {};

    ~TestAgentApp() override = default;
    static TypeId GetTypeId();

    void OnRecvObs(uint remoteAppId) override;
    void OnRecvReward(uint remoteAppId) override;
    void OnRecvFromAgent(uint remoteAppId, Ptr<OpenGymDictContainer> payload) override;

    Ptr<OpenGymSpace> GetObservationSpace() override;
    Ptr<OpenGymSpace> GetActionSpace() override;

    std::map<uint, std::vector<float>> GetObservation() const;
    std::map<uint, std::vector<float>> GetReward() const;
    std::map<uint, std::vector<float>> GetMessage() const;

  private:
    std::map<uint, std::vector<float>> m_observation;
    std::map<uint, std::vector<float>> m_reward;
    std::map<uint, std::vector<float>> m_agentMessages;
};

/**
 * @ingroup DEFIANCE-tests
 * @class TestActionApp
 * @brief Simple ActionApplication that can be reused for tests.
 */
class TestActionApp : public ActionApplication
{
  public:
    TestActionApp();
    virtual ~TestActionApp();
    /**
     * @brief Get the type ID.
     * @return the object TypeId.
     */
    static TypeId GetTypeId();
    void ExecuteAction(uint remoteAppId, Ptr<OpenGymDictContainer> action) override;
    std::map<uint, std::vector<float>> GetAction() const;

  private:
    std::map<uint, std::vector<float>> m_action;
};

} // namespace ns3
#endif
