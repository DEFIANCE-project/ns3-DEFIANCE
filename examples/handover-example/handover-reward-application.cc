#include "handover-reward-application.h"

#include "ns3/base-test.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-net-device.h"
#include "ns3/lte-rlc-sap.h"
#include "ns3/lte-rlc.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-rrc.h"

#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverRewardApplication");

TypeId
HandoverRewardApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("HandoverRewardApplication")
            .SetParent<RewardApplication>()
            .AddConstructor<HandoverRewardApplication>()
            .AddAttribute("SimulationTime",
                          "The total simulation time",
                          TimeValue(Seconds(100)),
                          MakeTimeAccessor(&HandoverRewardApplication::m_simTime),
                          MakeTimeChecker())
            .AddAttribute("StepTime",
                          "Interval for calculation reward",
                          TimeValue(MilliSeconds(1000)),
                          MakeTimeAccessor(&HandoverRewardApplication::m_calculationInterval),
                          MakeTimeChecker())
            .AddAttribute("LteHelper",
                          "The LteHelper object to use for rewards.",
                          TimeValue(MilliSeconds(1000)),
                          MakePointerAccessor(&HandoverRewardApplication::m_lteHelper),
                          MakePointerChecker<LteHelper>());
    return tid;
}

extern NetDeviceContainer g_ueLteDevs;

void
HandoverRewardApplication::SendReward()
{
    auto imsi = g_ueLteDevs.Get(0)->GetObject<LteUeNetDevice>()->GetImsi();

    auto throughput = m_lteHelper->GetRlcStats()->GetDlRxData(imsi, 3) /
                      m_calculationInterval.GetSeconds() / m_simTime.GetSeconds();
    m_receivedBytes = 0;
    auto reward = MakeDictBoxContainer<double>(1, "reward", throughput);
    NS_LOG_INFO("Reward: " << throughput);
    Send(reward);
    Simulator::Schedule(m_calculationInterval, &HandoverRewardApplication::SendReward, this);
}

void
HandoverRewardApplication::RegisterCallbacks()
{
    if (!m_lteHelper->GetRlcStats())
    {
        m_lteHelper->EnableRlcTraces();
    }
    auto stats = m_lteHelper->GetRlcStats();
    stats->SetEpoch(m_calculationInterval);
    stats->SetStartTime(NanoSeconds(1));
    Simulator::Schedule(m_calculationInterval, &HandoverRewardApplication::SendReward, this);
}
