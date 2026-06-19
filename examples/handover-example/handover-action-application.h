#include "ns3/action-application.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-ue-net-device.h"

using namespace ns3;

extern NetDeviceContainer g_ueLteDevs;
extern NetDeviceContainer g_enbLteDevs;
extern uint32_t g_totalHandovers;
extern uint32_t g_noopHandovers;
extern uint32_t g_sameCellHandovers;

/**
 * @ingroup defiance
 * @brief Child class of ActionApplication that checks validity of and executes handover actions.
 */
class HandoverActionApplication : public ActionApplication
{
  public:
    HandoverActionApplication();

    ~HandoverActionApplication() override;

    static TypeId GetTypeId();

    void ExecuteAction(uint32_t remoteAppId, Ptr<OpenGymDictContainer> action) override;

    void SetLteHelper(Ptr<LteHelper> helper);

  private:
    uint32_t m_numBs;
    Ptr<LteHelper> m_lteHelper;
    std::string m_handoverAlgorithm;
};
