#include "ns3/lte-rrc-sap.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/observation-application.h"

using namespace ns3;

/**
 * @ingroup defiance
 * @brief Child class of ObservationApp that observes measurement reports of UEs.
 */
class HandoverObservationApplication : public ObservationApplication
{
  public:
    HandoverObservationApplication();
    ~HandoverObservationApplication() override;

    void DoInitialize() override;

    static TypeId GetTypeId();
    void Observe(uint64_t imsi, uint16_t rrc, uint16_t rnti, LteRrcSap::MeasurementReport msg);
    void RegisterCallbacks() override;

  private:
    uint32_t m_numBs;
    std::vector<std::pair<int32_t, int32_t>> m_observations;
    Time m_lastObservationTime = Seconds(0);
};

NS_OBJECT_ENSURE_REGISTERED(HandoverObservationApplication);
