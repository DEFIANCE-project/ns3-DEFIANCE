#include "handover-action-application.h"

#include "ns3/base-test.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-enb-rrc.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-rrc.h"

#include <cstdint>
#include <sys/types.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverActionApplication");

HandoverActionApplication::HandoverActionApplication()
    : ActionApplication()
{
}

HandoverActionApplication::~HandoverActionApplication()
{
}

TypeId
HandoverActionApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::HandoverActionApplication")
            .SetParent<ActionApplication>()
            .SetGroupName("defiance")
            .AddConstructor<HandoverActionApplication>()
            .AddAttribute("NumBs",
                          "Number of base stations in the simulation.",
                          UintegerValue(1),
                          MakeUintegerAccessor(&HandoverActionApplication::m_numBs),
                          MakeUintegerChecker<uint>())
            .AddAttribute("LteHelper",
                          "The LteHelper object to use for handover.",
                          PointerValue(),
                          MakePointerAccessor(&HandoverActionApplication::m_lteHelper),
                          MakePointerChecker<LteHelper>())
            .AddAttribute("HandoverAlgorithm",
                          "The algorithm to use for handover.",
                          StringValue("agent"),
                          MakeStringAccessor(&HandoverActionApplication::m_handoverAlgorithm),
                          MakeStringChecker());
    return tid;
}

void
HandoverActionApplication::ExecuteAction(uint32_t remoteAppId, Ptr<OpenGymDictContainer> action)
{
    if (m_handoverAlgorithm != "agent")
    {
        NS_LOG_INFO("Handover algorithm not agent, skipping action execution.");
        return;
    }

    // 1: Is UE ready for a handover?
    auto currentCellId = g_ueLteDevs.Get(0)->GetObject<LteUeNetDevice>()->GetRrc()->GetCellId();
    auto newCellId = DynamicCast<OpenGymBoxContainer<float>>(action->Get("newCellId"))->GetValue(0);
    NS_LOG_DEBUG("currentCellId: " << currentCellId << "\tnewCellId: " << newCellId);
    uint32_t i = 0;
    for (; i < g_enbLteDevs.GetN(); i++)
    {
        if (g_enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetCellId() == currentCellId)
        {
            break;
        }
    }
    auto enbRrc = GetNode()->GetDevice(0)->GetObject<LteEnbNetDevice>()->GetRrc();
    auto rnti = g_ueLteDevs.Get(0)->GetObject<LteUeNetDevice>()->GetRrc()->GetRnti();
    if (!enbRrc->HasUeManager(rnti))
    { // enb does not have the UeManager for the UE
        // g_actionMap["other"]++;
        return;
    }
    if (enbRrc->GetUeManager(rnti)->GetState() != UeManager::CONNECTED_NORMALLY)
    { // UE amidst handover
        // g_actionMap["during"]++;
        return;
    }

    // 2: Was no-op action taken?
    if (newCellId == 0)
    { // no-op
        NS_LOG_DEBUG(Simulator::Now().GetSeconds() << "\t2: No-op handover.");
        g_noopHandovers++;
        return;
    }

    // 3: Is the requested cell the one that the UE is already connected to?
    if (newCellId == currentCellId)
    { // no-op
        g_sameCellHandovers++;
        return;
    }

    // 4: Is this node the one that the UE is connected to? Check the IMSI
    auto ueImsi = g_ueLteDevs.Get(0)->GetObject<LteUeNetDevice>()->GetImsi();
    auto enbRntiImsi = enbRrc->GetUeManager(rnti)->GetImsi();
    if (ueImsi != enbRntiImsi)
    { // UE RNTI at this eNB relates to a different UE
        // g_actionMap["other"]++;
        return;
    }

    g_totalHandovers++;
    // X2-based Handover
    NS_LOG_DEBUG(Simulator::Now().GetSeconds()
                 << "\tAttempting handover for UE " << g_ueLteDevs.Get(0)->GetNode()->GetId()
                 << ", RNTI " << rnti << "; cell " << currentCellId << " --> " << newCellId);
    m_lteHelper->HandoverRequest(Seconds(0), g_ueLteDevs.Get(0), g_enbLteDevs.Get(i), newCellId);
}

void
HandoverActionApplication::SetLteHelper(Ptr<LteHelper> helper)
{
    m_lteHelper = helper;
}
