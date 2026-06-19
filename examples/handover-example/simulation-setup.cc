#include "handover-action-application.h"
#include "handover-agent-application.h"
#include "handover-observation-application.h"
#include "handover-reward-application.h"

#include "ns3/applications-module.h"
#include "ns3/communication-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/rl-application-helper.h"

#include <cstdint>
#include <string>
#include <vector>

inline void
scenarioSetup(uint16_t numberOfUes,
              uint16_t numberOfEnbs,
              uint16_t numBearersPerUe = 1,
              double distance = 1000.0,
              double speed = 20.0,
              double simTime = 100.0,
              uint32_t stepTime = 420,
              uint32_t delay = 0,
              double enbTxPowerDbm = 10.0,
              uint32_t seed = 0,
              uint32_t runId = 0,
              bool visualize = false,
              std::string trialName = "1",
              std::string handoverAlgorithm = "agent")
{
    auto lteHelper = CreateObject<LteHelper>();
    auto epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);
    lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
    if (handoverAlgorithm == "agent") // disable automatic handover
    {
        lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm");
    }
    else if (handoverAlgorithm == "a2a4")
    {
        lteHelper->SetHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
    }
    else if (handoverAlgorithm == "a3")
    {
        lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");
    }
    else
    {
        NS_FATAL_ERROR("Unknown handover algorithm");
    }
    lteHelper->SetPathlossModelType(LogDistancePropagationLossModel::GetTypeId());
    lteHelper->SetPathlossModelAttribute("Exponent", DoubleValue(2.2));
    lteHelper->SetPathlossModelAttribute("ReferenceDistance", DoubleValue(1.0));

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Create a single RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

    // Routing of the Internet Host (towards the LTE network)
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    // interface 0 is localhost, 1 is the p2p device
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create(numberOfEnbs);
    ueNodes.Create(numberOfUes);

    // Install Mobility Model in eNB
    auto enbPositionAlloc = CreateObject<ListPositionAllocator>();
    for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
        double xPos = (i % 2 == 0) ? 0 : distance;
        double yPos = (i / 2) * distance;
        Vector enbPosition(xPos, yPos, 0);
        enbPositionAlloc->Add(enbPosition);
    }
    MobilityHelper enbMobility;
    enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbMobility.SetPositionAllocator(enbPositionAlloc);
    enbMobility.Install(enbNodes);

    RngSeedManager::SetSeed(seed == 0U ? time(nullptr) : seed);
    RngSeedManager::SetRun(runId == 0U ? 1 : runId);
    ObjectFactory pos;
    pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
    pos.Set(
        "X",
        StringValue("ns3::UniformRandomVariable[Min=0.0|Max=" + std::to_string(distance) + "]"));
    NS_ASSERT_MSG(numberOfEnbs > 0, "Number of eNBs cannot be 0.");
    auto yBound = distance * ((numberOfEnbs - 1) / 2);
    pos.Set("Y",
            StringValue("ns3::UniformRandomVariable[Min=0.0|Max=" + std::to_string(yBound) + "]"));

    Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();

    // Install Mobility Model in UE
    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel(
        "ns3::RandomWaypointMobilityModel",
        "Speed",
        StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(speed) + "]"),
        "Pause",
        StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),
        "PositionAllocator",
        PointerValue(taPositionAlloc));
    ueMobility.SetPositionAllocator(taPositionAlloc);
    ueMobility.Install(ueNodes);

    // Install LTE Devices in eNB and UEs
    Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(enbTxPowerDbm));
    g_enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    g_ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

    // Install the IP stack on the UEs
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIfaces;
    ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(g_ueLteDevs));

    // Attach all UEs to their respective closest eNodeB
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
        Ptr<Node> ueNode = ueNodes.Get(i);
        Ptr<NetDevice> ueLteDev = g_ueLteDevs.Get(i);

        double minDistance = std::numeric_limits<double>::max();
        Ptr<NetDevice> closestEnbLteDev;

        for (uint32_t j = 0; j < enbNodes.GetN(); ++j)
        {
            Ptr<Node> enbNode = enbNodes.Get(j);
            Ptr<NetDevice> enbLteDev = g_enbLteDevs.Get(j);

            Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel>();
            Ptr<MobilityModel> enbMobility = enbNode->GetObject<MobilityModel>();
            double distance = ueMobility->GetDistanceFrom(enbMobility);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestEnbLteDev = enbLteDev;
            }
        }

        lteHelper->Attach(ueLteDev, closestEnbLteDev);
    }

    // Install and start applications on UEs and remote host
    uint16_t dlPort = 10000;

    // Randomize a bit start times to avoid simulation artifacts
    // (e.g., buffer overflows due to packet transmissions happening
    // exactly at the same time)
    auto startTimeSeconds = CreateObject<UniformRandomVariable>();
    startTimeSeconds->SetAttribute("Min", DoubleValue(0));
    startTimeSeconds->SetAttribute("Max", DoubleValue(0.010));

    for (uint32_t u = 0; u < numberOfUes; ++u)
    {
        Ptr<Node> ue = ueNodes.Get(u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

        for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
            ++dlPort;

            ApplicationContainer clientApps;
            ApplicationContainer serverApps;

            UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);
            dlClientHelper.SetAttribute("Interval", TimeValue(Seconds(0.01)));
            dlClientHelper.SetAttribute("PacketSize", UintegerValue(655));
            clientApps.Add(dlClientHelper.Install(remoteHost));
            PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory",
                                                InetSocketAddress(Ipv4Address::GetAny(), dlPort));
            serverApps.Add(dlPacketSinkHelper.Install(ue));

            Time startTime = Seconds(startTimeSeconds->GetValue());
            serverApps.Start(startTime);
            clientApps.Start(startTime);
        }
    }

    lteHelper->AddX2Interface(enbNodes);

    // Add measurement configuration
    LteRrcSap::ReportConfigEutra reportConfig;
    reportConfig.reportInterval = LteRrcSap::ReportConfigEutra::MS120;
    NetDeviceContainer::Iterator it;
    for (it = g_enbLteDevs.Begin(); it != g_enbLteDevs.End(); ++it)
    {
        Ptr<NetDevice> netDevice = *it;
        Ptr<LteEnbNetDevice> enbLteNetDevice = netDevice->GetObject<LteEnbNetDevice>();
        Ptr<LteEnbRrc> enbRrc = enbLteNetDevice->GetRrc();
        enbRrc->AddUeMeasReportConfig(reportConfig);
    }

    // Framework code
    RlApplicationHelper appHelper(HandoverRewardApplication::GetTypeId());
    appHelper.SetAttribute("StartTime", TimeValue(Seconds(1)));
    appHelper.SetAttribute("StopTime", TimeValue(Seconds(simTime)));
    appHelper.SetAttribute("SimulationTime", TimeValue(Seconds(simTime)));
    appHelper.SetAttribute("StepTime", TimeValue(MilliSeconds(stepTime)));
    appHelper.SetAttribute("LteHelper", PointerValue(lteHelper));
    // UE 0 will be the one we control
    auto rewardApps = appHelper.Install(ueNodes.Get(0));

    appHelper.SetTypeId(HandoverAgentApplication::GetTypeId());
    appHelper.SetAttribute("NumUes", UintegerValue(numberOfUes));
    appHelper.SetAttribute("NumBs", UintegerValue(numberOfEnbs));
    appHelper.SetAttribute("StepTime", UintegerValue(stepTime));
    auto agentApps = appHelper.Install(remoteHost);

    appHelper.SetTypeId(HandoverObservationApplication::GetTypeId());
    appHelper.SetAttribute("NumBs", UintegerValue(numberOfEnbs));
    auto observationApps = appHelper.Install(enbNodes);

    appHelper.SetTypeId(HandoverActionApplication::GetTypeId());
    appHelper.SetAttribute("HandoverAlgorithm", StringValue(handoverAlgorithm));
    appHelper.SetAttribute("NumBs", UintegerValue(numberOfEnbs));
    appHelper.SetAttribute("LteHelper", PointerValue(lteHelper));
    auto actionApps = appHelper.Install(enbNodes);

    CommunicationHelper commHelper;
    commHelper.SetAgentApps(agentApps);
    commHelper.SetActionApps(actionApps);
    commHelper.SetObservationApps(observationApps);
    commHelper.SetRewardApps(rewardApps);
    commHelper.SetIds();

    // Set the communication attributes
    for (uint32_t i = 0; i < numberOfEnbs; i++)
    {
        commHelper.AddCommunication(
            {CommunicationPair{observationApps.GetId(i),
                               agentApps.GetId(0),
                               CommunicationAttributes{MilliSeconds(delay)}}});

        commHelper.AddCommunication(
            {CommunicationPair{actionApps.GetId(i),
                               agentApps.GetId(0),
                               CommunicationAttributes{MilliSeconds(delay)}}});
    }
    commHelper.AddCommunication({CommunicationPair{rewardApps.GetId(0),
                                                   agentApps.GetId(0),
                                                   CommunicationAttributes{MilliSeconds(delay)}}});

    commHelper.Configure();
}
