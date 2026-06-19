#include "simulation-setup.cc"

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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverExample");

std::string handoverAlgorithm = "agent";
uint16_t numberOfUes = 3;
uint16_t numberOfEnbs = 4;
double distance = 1000.0; // m
double speed = 20.0;      // m/s
double simTime = 50.0;    // s
uint32_t stepTime = 420;  // ms
uint32_t delay = 0;       // ms
double enbTxPowerDbm = 10.0;
std::string trialName = "1";
bool visualize = false;
int parallel = 0;
uint32_t seed = 1;
uint32_t runId = 1;
std::string runName = "unknown";
NetDeviceContainer g_ueLteDevs;
NetDeviceContainer g_enbLteDevs;
uint32_t g_totalHandovers = 0;
uint32_t g_noopHandovers = 0;
uint32_t g_sameCellHandovers = 0;

// Variables for saving stats
std::string pathToNs3 = std::getenv("NS3_HOME");
std::string expName = "testExpName";
uint32_t bytesReceived = 0;
Time lastStatCalculation = Seconds(0);
Time statInterval = MilliSeconds(1000);

/**
 * Save throughput stats of an infering agent to a file.
 */
void
SaveStats(Ptr<const Packet> packet, Ptr<Ipv4> ipLayer, uint32_t interface)
{
    bytesReceived += packet->GetSize();
    if (Simulator::Now() - lastStatCalculation >= statInterval)
    {
        std::cout << Simulator::Now().GetSeconds() << "\t Attempting to save stats" << std::endl;
        if (expName.empty())
        {
            std::cout << "No experiment name set, stats will not be saved." << std::endl;
        }
        else
        {
            auto throughput = bytesReceived * 8 / statInterval.GetSeconds(); // in Bit/s
            std::ofstream stats_file(expName, std::ios_base::app);
            auto stats_file_ptr = Create<OutputStreamWrapper>(&stats_file);
            std::ostream* stream = stats_file_ptr->GetStream();
            std::cout << Simulator::Now().GetSeconds() << ", " << throughput << std::endl;
            *stream << Simulator::Now().GetSeconds() << "," << throughput << std::endl;
            stream->flush();
        }
        bytesReceived = 0;
        lastStatCalculation = Simulator::Now();
    }
}

/**
 * Handover playground to test agents ability to perform handovers. Some eNbs are created in a grid,
 * while UEs move in the area between them. UE 0 is controlled by the agent while the rest of the UE
 * will just reconnect after a radio link failure.
 *
 * Start training on this example with the following command:
 * run-agent train -n defiance-handover
 * Start inference of the trained agent with the following command:
 * run-agent infer -n defiance-handover -a /path/to/PPO-run
 * When in a devcontainer, the path would usually be /root/ray_results/PPO-run
 * Specify the beneath cmd line arguments like this:
 * --ns3-settings numberOfUes=4 seed=2
 * Large number of UEs will break the maximum interprocess message size, increase shmSize to fix.
 */
int
main(int argc, char* argv[])
{
    // some default attributes reasonable for this scenario, but before processing command line
    // arguments, so that user can override
    Config::SetDefault("ns3::UdpClient::Interval", TimeValue(MilliSeconds(1)));
    Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(10000000));
    Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
    Config::SetDefault("ns3::LteUePhy::EnableRlfDetection", BooleanValue(false));

    CommandLine cmd(__FILE__);
    cmd.AddValue("handoverAlgorithm",
                 "Handover algorithm to use (agent, a2a4, a3)",
                 handoverAlgorithm);
    cmd.AddValue("numberOfUes", "Number of UEs in the simulation", numberOfUes);
    cmd.AddValue("numberOfEnbs", "Number of eNBs in the simulation", numberOfEnbs);
    cmd.AddValue("distance", "Meters between eNBs (default = 1000)", distance);
    cmd.AddValue("speed", "Meters/second of the UEs (default = 20)", speed);
    cmd.AddValue("simTime", "Total duration of the simulation in seconds (default = 100)", simTime);
    cmd.AddValue("stepTime",
                 "Milliseconds between each step in the simulation (default = 420)",
                 stepTime);
    cmd.AddValue("delay", "Transmission delay of Simple Channel Interfaces in ms", delay);
    cmd.AddValue("enbTxPowerDbm", "TX power [dBm] used by HeNBs (default = 10.0)", enbTxPowerDbm);
    cmd.AddValue("seed", "Seed for random number generator", seed);
    cmd.AddValue("runId",
                 "Counts how often the environment has been reset (used for seeding)",
                 runId);
    cmd.AddValue("parallel", "Number of parallel simulation runs", parallel);
    cmd.AddValue("trial_name", "Trial name", trialName);
    cmd.AddValue("visualize", "Log visualization data", visualize);

    cmd.Parse(argc, argv);

    if (handoverAlgorithm == "agent")
    {
        Config::SetDefault("ns3::LteUePhy::EnableRlfDetection", BooleanValue(false));
    }
    seed += parallel;

    OpenGymMultiAgentInterface::Get();
    Ns3AiMsgInterface::Get()->SetTrialName(trialName);
    std::cout << "Trial name: " << trialName << "\t";
    std::cout << "Seed: " << seed << "\tRun ID: " << runId << std::endl;

    scenarioSetup(numberOfUes,
                  numberOfEnbs,
                  1,
                  distance,
                  speed,
                  simTime,
                  stepTime,
                  delay,
                  enbTxPowerDbm,
                  seed,
                  runId,
                  visualize,
                  trialName,
                  handoverAlgorithm);

    if (visualize)
    {
        expName = pathToNs3 + "/contrib/defiance/examples/handover/handover-stats_" +
                  std::to_string(seed) + ".csv";
        auto ueNode = g_ueLteDevs.Get(0)->GetNode();
        ueNode->GetObject<Ipv4L3Protocol>()->TraceConnectWithoutContext("Rx",
                                                                        MakeCallback(&SaveStats));
        std::string output = pathToNs3 + "/contrib/defiance/examples/handover/inference_" +
                             std::to_string(seed) + ".csv";
        std::ofstream ofs(output, std::ios_base::app);
        Ptr<OutputStreamWrapper> ofs_ptr = Create<OutputStreamWrapper>(&ofs);
        std::ostream* s = ofs_ptr->GetStream();
        *s << handoverAlgorithm << "," << numberOfUes << "," << numberOfEnbs << "," << stepTime
           << "," << seed << "," << runId << std::endl;
        s->flush();
    }

    auto start = std::chrono::high_resolution_clock::now();
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Simulating " << simTime << "s took " << duration.count()
              << "s. Handovers: " << g_totalHandovers << "\tNoops: " << g_noopHandovers
              << "\tSame cell: " << g_sameCellHandovers << std::endl;
    OpenGymMultiAgentInterface::Get()->NotifySimulationEnd(0, {});
    return 0;
}
