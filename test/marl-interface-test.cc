#include "ns3/defiance-module.h"
#include "ns3/ns3-ai-multi-agent-gym-interface.h"
#include "ns3/test.h"

#include <boost/interprocess/exceptions.hpp>

using namespace ns3;

/**
 * @ingroup defiance
 * @ingroup tests
 * @ingroup marl-tests
 * Test suite for simple marl interface tests
 */
class SimpleMarlInterfaceTestSuite : public TestSuite
{
  public:
    SimpleMarlInterfaceTestSuite(std::string name = "marl-interface", Type type = Type::UNIT);

  protected:
    virtual void SetupCallbacks();
    void DoSetup() override;
    void DoTeardown() override;

    virtual std::map<std::string, std::pair<Ptr<OpenGymBoxSpace>, Ptr<OpenGymBoxSpace>>>
    AgentSpaces()
    {
        auto m_box1 = MakeBoxSpace<float>(12, 0, 1);
        auto m_box2 = MakeBoxSpace<float>(5, -1, 1);
        auto m_box3 = MakeBoxSpace<float>(9, 0, 100);
        return {{"agent1", {m_box1, m_box1}},
                {"agent2", {m_box2, m_box2}},
                {"agent3", {m_box3, m_box3}}};
    }

    bool m_exitEarly = false;

  private:
    void DoRun() override;
};

SimpleMarlInterfaceTestSuite::SimpleMarlInterfaceTestSuite(std::string name, TestSuite::Type type)
    : TestSuite(name, type)
{
}

void
SimpleMarlInterfaceTestSuite::DoSetup()
{
    SetupCallbacks();
    try
    {
        OpenGymMultiAgentInterface::Get()->Init();
    }
    catch (boost::interprocess::interprocess_exception& e)
    {
        std::cout << "SKIP " << this->GetName()
                  << " Run this test using pytest from defiance 0.000s" << std::endl;
        m_exitEarly = true;
        return;
    }
}

void
SimpleMarlInterfaceTestSuite::DoTeardown()
{
    Simulator::Destroy();
    if (!m_exitEarly)
    {
        OpenGymMultiAgentInterface::Get()->NotifySimulationEnd();
    }
}

void
SimpleMarlInterfaceTestSuite::DoRun()
{
    std::vector<std::string> agentCycle{"agent1", "agent2", "agent3", "agent2", "agent1"};
    for (const auto& agent : agentCycle)
    {
        auto observation =
            MakeBoxContainer<float>(AgentSpaces()[agent].first->GetShape().front(), 0.1);
        if (!m_exitEarly)
        {
            OpenGymMultiAgentInterface::Get()
                ->NotifyCurrentState(agent, observation, 1, false, {}, Seconds(1), noopCallback);
        }
    }
}

void
SimpleMarlInterfaceTestSuite::SetupCallbacks()
{
    for (const auto& [agent, spaces] : AgentSpaces())
    {
        OpenGymMultiAgentInterface::Get()->SetGetObservationSpaceCb(
            agent,
            [obsSpace = spaces.first]() { return obsSpace; });
        OpenGymMultiAgentInterface::Get()->SetGetActionSpaceCb(
            agent,
            [actionSpace = spaces.second]() { return actionSpace; });
    }
}

/**
 * @ingroup marl-tests
 * Test case for registering callbacks
 */
class EchoMarlInterfaceTestSuite : public SimpleMarlInterfaceTestSuite
{
  public:
    EchoMarlInterfaceTestSuite()
        : SimpleMarlInterfaceTestSuite("marl-echo-action-interface", Type::UNIT)
    {
    }

  protected:
    std::map<std::string, std::pair<Ptr<OpenGymBoxSpace>, Ptr<OpenGymBoxSpace>>> AgentSpaces()
        override
    {
        return {{"agent1", {MakeBoxSpace<float>(12, 0, 1), MakeBoxSpace<int>(20, 7, 10)}},
                {"agent2", {MakeBoxSpace<int>(5, -1, 1), MakeBoxSpace<float>(12, 0, 1)}},
                {"agent3", {MakeBoxSpace<float>(9, 0, 100), MakeBoxSpace<int>(5, -1, 1)}},
                {"agent4", {MakeBoxSpace<int>(3, 7, 10), MakeBoxSpace<float>(9, 0, 100)}},
                {"agent5", {MakeBoxSpace<float>(8, 0, 100), MakeBoxSpace<int>(3, 7, 10)}},
                {"agent6", {MakeBoxSpace<int>(20, 7, 10), MakeBoxSpace<float>(8, 0, 100)}}};
    }

  private:
    std::vector<std::string> m_agents{{"agent1", "agent2", "agent3", "agent4", "agent5", "agent6"}};

    void DoRun() override
    {
        SendNewAction(0, MakeBoxContainer<float>(12, 0.1));
        if (!m_exitEarly)
        {
            Simulator::Run();
        }
    };

    void SendNewAction(int i, Ptr<OpenGymDataContainer> action);
};

void
EchoMarlInterfaceTestSuite::SendNewAction(int i, Ptr<OpenGymDataContainer> action)
{
    if (i > 20)
    {
        Simulator::Stop();
        OpenGymMultiAgentInterface::Get()->NotifySimulationEnd();
    }
    auto agent = m_agents[i % 6];
    float reward = 1;

    switch (i % 2)
    {
    case 0:
        reward = action->GetObject<OpenGymBoxContainer<float>>()->GetValue(0);
        break;
    case 1:
        reward = action->GetObject<OpenGymBoxContainer<int>>()->GetValue(0);
        break;
    };
    std::cout << "Reward = " << reward << std::endl;
    try
    {
        OpenGymMultiAgentInterface::Get()->NotifyCurrentState(
            agent,
            action,
            reward,
            false,
            {},
            Seconds(10),
            MakeCallback(&EchoMarlInterfaceTestSuite::SendNewAction, this, i + 1));
    }
    catch (boost::interprocess::interprocess_exception& e)
    {
        std::cout << "SKIP " << this->GetName()
                  << " Run this test using pytest from defiance 0.000s" << std::endl;
        return;
    }
}

class TestObservation : public ObservationApplication
{
  public:
    TestObservation() {};
    ~TestObservation() override {};
    static TypeId GetTypeId();
    void RegisterCallbacks() override;
};

void
TestObservation::RegisterCallbacks()
{
    Send(MakeDictBoxContainer<float>(1, "floatObs", 0));
    for (int i = 1; i <= 10; i++)
    {
        Simulator::Schedule(
            Seconds(i),
            MakeCallback(*[](TestObservation* obs, Ptr<OpenGymDictContainer> container) {
                obs->Send(container);
            }).Bind(this, MakeDictBoxContainer<float>(1, "floatObs", i)));
    }
}

TypeId
TestObservation::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestObservation")
                            .SetParent<ObservationApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<TestObservation>();
    return tid;
}

class EchoAgentApplication : public AgentApplication
{
  public:
    static TypeId GetTypeId();

  protected:
    void OnRecvObs(uint id) override;
    void OnRecvReward(uint id) override;

    void InitiateAction(Ptr<OpenGymDataContainer> action) override;

  private:
    Ptr<OpenGymSpace> GetObservationSpace() override;
    Ptr<OpenGymSpace> GetActionSpace() override;
};

void
EchoAgentApplication::InitiateAction(Ptr<OpenGymDataContainer> action)
{
    auto box = action->GetObject<OpenGymBoxContainer<float>>();
    auto oldBox = m_observation->GetObject<OpenGymBoxContainer<float>>();
    NS_ABORT_MSG_IF(
        std::equal(box->GetData().begin(), box->GetData().end(), oldBox->GetData().begin()),
        "unexpected data");
}

Ptr<OpenGymSpace>
EchoAgentApplication::GetObservationSpace()
{
    return MakeBoxSpace<float>(2, 0, 10);
}

Ptr<OpenGymSpace>
EchoAgentApplication::GetActionSpace()
{
    return MakeBoxSpace<float>(2, 0, 10);
}

void
EchoAgentApplication::OnRecvObs(uint id)
{
    m_observation =
        MakeBoxContainer<float>(1, m_obsDataStruct.AggregateNewest(id, 1)["floatObs"].GetAvg());
    m_reward = m_obsDataStruct.AggregateNewest(id, 1)["floatObs"].GetAvg();
    InferAction();
}

void
EchoAgentApplication::OnRecvReward(uint id)
{
    // no reward apps registered
}

TypeId
EchoAgentApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TestAgent")
                            .SetParent<AgentApplication>()
                            .SetGroupName("defiance")
                            .AddConstructor<EchoAgentApplication>();
    return tid;
}

NS_OBJECT_ENSURE_REGISTERED(TestObservation);

/**
 * @ingroup marl-tests
 * Test case for registering callbacks
 */
class MarlAgentInterfaceTestSuite : public SimpleMarlInterfaceTestSuite
{
  public:
    MarlAgentInterfaceTestSuite()
        : SimpleMarlInterfaceTestSuite("marl-agent-interface", Type::UNIT)
    {
    }

  protected:
    void SetupCallbacks() override
    {
        NodeContainer nodes(2);
        auto agent = CreateObject<EchoAgentApplication>();

        nodes.Get(0)->AddApplication(agent);
        auto observationApp = CreateObjectWithAttributes<TestObservation>("StartTime",
                                                                          TimeValue(Seconds(0)),
                                                                          "StopTime",
                                                                          TimeValue(Seconds(10)));
        nodes.Get(1)->AddApplication(observationApp);
        CommunicationHelper commHelper;
        commHelper.SetObservationApps(RlApplicationContainer(observationApp));
        commHelper.SetAgentApps(RlApplicationContainer(agent));
        commHelper.SetIds();
        commHelper.AddCommunication({{observationApp->GetId(), agent->GetId(), {}}});
        commHelper.Configure();
    };

    void DoRun() override
    {
        Simulator::Stop(Seconds(1000));
        if (!m_exitEarly)
        {
            Simulator::Run();
        }
    };
};

/**
 * @ingroup defiance-tests
 * Static variables for test initialization
 */
static SimpleMarlInterfaceTestSuite sMarlInterfaceTestSuite;

static EchoMarlInterfaceTestSuite sEchoInterfaceTestSuite;

static MarlAgentInterfaceTestSuite sMarlAgentInterfaceTestSuite;
