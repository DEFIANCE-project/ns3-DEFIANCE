#include "handover-observation-application.h"

#include "ns3/base-test.h"
#include "ns3/net-device-container.h"

#include <cstdint>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverObservationApplication");

extern NetDeviceContainer g_ueLteDevs;
extern NetDeviceContainer g_enbLteDevs;

HandoverObservationApplication::HandoverObservationApplication()
    : ObservationApplication()
{
}

HandoverObservationApplication::~HandoverObservationApplication()
{
}

void
HandoverObservationApplication::DoInitialize()
{
    ObservationApplication::DoInitialize();
    m_observations = std::vector<std::pair<int32_t, int32_t>>(m_numBs, std::make_pair(-1, -1));
}

TypeId
HandoverObservationApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::HandoverObservationApplication")
            .SetParent<ObservationApplication>()
            .SetGroupName("defiance")
            .AddConstructor<HandoverObservationApplication>()
            .AddAttribute("NumBs",
                          "Number of base stations in the simulation.",
                          UintegerValue(1),
                          MakeUintegerAccessor(&HandoverObservationApplication::m_numBs),
                          MakeUintegerChecker<uint>());
    ;
    return tid;
}

void
HandoverObservationApplication::Observe(uint64_t imsi,
                                        uint16_t cellId,
                                        uint16_t rnti,
                                        LteRrcSap::MeasurementReport report)
{
    // Measurement reports only from our primary UE
    if (imsi != g_ueLteDevs.Get(0)->GetObject<LteUeNetDevice>()->GetImsi())
    {
        return;
    }

    auto rsrp = report.measResults.measResultPCell.rsrpResult;
    auto rsrq = report.measResults.measResultPCell.rsrqResult;

    m_observations[cellId - 1] = std::make_pair(rsrp, rsrq);

    auto listEutra = report.measResults.measResultListEutra;
    for (auto it = listEutra.begin(); it != listEutra.end(); ++it)
    {
        auto secCellId = it->physCellId;
        m_observations[secCellId - 1] = std::make_pair(it->rsrpResult, it->rsrqResult);
    }

    if (Simulator::Now() == m_lastObservationTime)
    {
        NS_LOG_INFO("Already received observation for this time.");
    }
    else
    {
        auto rsrps = CreateObject<OpenGymBoxContainer<int32_t>>();
        auto rsrqs = CreateObject<OpenGymBoxContainer<int32_t>>();
        for (auto obs : m_observations)
        {
            rsrps->AddValue(obs.first);
            rsrqs->AddValue(obs.second);
        }
        NS_LOG_INFO("RSRPs: " << rsrps << " RSRQs: " << rsrqs << " CellId: " << cellId);
        auto observationDict = CreateObject<OpenGymDictContainer>();
        observationDict->Add("rsrps", rsrps);
        observationDict->Add("rsrqs", rsrqs);
        observationDict->Add("cellId", MakeBoxContainer<int32_t>(1, (int32_t)cellId));
        m_lastObservationTime = Simulator::Now();
        Send(observationDict);
    }
}

void
HandoverObservationApplication::RegisterCallbacks()
{
    // this has parameters rnti, cellId, rnti and LteRrcSap::MeasurementReport
    auto nodeId = GetNode()->GetId();
    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(nodeId) +
            "/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbRrc/RecvMeasurementReport",
        MakeCallback(&HandoverObservationApplication::Observe, this));
}
