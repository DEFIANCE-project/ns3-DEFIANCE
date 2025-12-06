#include "ns3/defiance-module.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup defiance-tests
 *
 * @brief Test case for observation datastructure
 */
class HistoryContainerTest : public TestCase
{
  public:
    HistoryContainerTest();
    virtual ~HistoryContainerTest();

  private:
    void DoRun() override;
    void TestAggregatedInfo();
    void TestDataStruct();
    void TestDataStructAggregation();
    Ptr<OpenGymBoxContainer<float>> ExtractFloatBox(Ptr<OpenGymDictContainer> dict,
                                                    std::string name);
};

HistoryContainerTest::HistoryContainerTest()
    : TestCase("test case for obs structure")
{
}

HistoryContainerTest::~HistoryContainerTest()
{
}

/**
 * @ingroup defiance-tests
 *
 * @brief Test AggregatedInfo class by adding and updating vales and checking the aggregated results
 */
void
HistoryContainerTest::TestAggregatedInfo()
{
    AggregatedInfo t = AggregatedInfo();
    t.UpdateAverage(1.0);
    t.UpdateAverage(2.0);
    t.UpdateMax(0);
    t.UpdateMin(-3.0);
    t.UpdateMax(10.0);
    NS_TEST_ASSERT_MSG_EQ_TOL(t.GetAvg(), 1.5, 0.0001, "Average is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(t.GetMin(), -3.0, 0.0001, "Min is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(t.GetMax(), 10.0, 0.0001, "Max is not correct");
}

Ptr<OpenGymBoxContainer<float>>
HistoryContainerTest::ExtractFloatBox(Ptr<OpenGymDictContainer> dict, std::string name)
{
    return dict->Get(name)->GetObject<OpenGymBoxContainer<float>>();
}

/**
 * @ingroup defiance-tests
 *
 * @brief Test pushing and extracting data from HistoryContainer
 */
void
HistoryContainerTest::TestDataStruct()
{
    HistoryContainer testStructure1 = HistoryContainer(3);
    std::vector<uint32_t> shape = {2, 2};

    auto testDictBox = CreateObject<OpenGymDictContainer>();
    auto innerBox = CreateObject<OpenGymBoxContainer<float>>(shape);
    innerBox->AddValue(2);
    innerBox->AddValue(4);
    testDictBox->Add("sinr", innerBox);

    testStructure1.Push(testDictBox, 0);

    TimestampedData* result = testStructure1.GetNewestByID(0);

    // Check if the type is correct
    NS_TEST_ASSERT_MSG_EQ(result->data->Get("sinr")->GetDataContainerPbMsg().type(),
                          ns3_ai_gym::Box,
                          "Type is not correct");

    // Should return a dict with a box of values 2 and 4
    auto newBox = ExtractFloatBox(result->data, "sinr");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox->GetValue(0), 2.0, 0.0001, "Value is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox->GetValue(1), 4.0, 0.0001, "Value is not correct");

    // Test pushing too many values into the queue, should remove the oldest value

    auto testPushDict2 = CreateObject<OpenGymDictContainer>();
    auto box2 = CreateObject<OpenGymBoxContainer<float>>(shape);
    box2->AddValue(3);
    box2->AddValue(4);
    testPushDict2->Add("sinr", box2);

    auto box3 = CreateObject<OpenGymBoxContainer<float>>(shape);
    auto testPushDict3 = CreateObject<OpenGymDictContainer>();
    box3->AddValue(9);
    box3->AddValue(10);
    testPushDict3->Add("sinr", box3);
    testStructure1.Push(testPushDict2, 0);
    testStructure1.Push(testPushDict2, 0);
    testStructure1.Push(testPushDict3, 0);

    // Get the last dicts and the only boxes under the key sinr in the dicts
    auto resultvec = testStructure1.GetNewestByID(0, 3);
    auto newBox1 = ExtractFloatBox(resultvec[0]->data, "sinr");
    auto newBox2 = ExtractFloatBox(resultvec[1]->data, "sinr");

    NS_TEST_ASSERT_MSG_EQ_TOL(newBox1->GetValue(0), 9.0, 0.0001, "Value is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox1->GetValue(1), 10.0, 0.0001, "Value is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox2->GetValue(0), 3.0, 0.0001, "Value is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox2->GetValue(1), 4.0, 0.0001, "Value is not correct");

    // And check if the oldest value was removed
    int number = testStructure1.GetSize(0);
    NS_TEST_ASSERT_MSG_EQ(number, 3, "Size of ID is not correct");
    auto newBox3 = ExtractFloatBox(resultvec[2]->data, "sinr");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox3->GetValue(0), 3.0, 0.0001, "Value is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(newBox3->GetValue(1), 4.0, 0.0001, "Value is not correct");

    // Test if ns3 simulation time is correctly stored

    auto testStructure2 = HistoryContainer(3, true);
    auto testPushDict4 = CreateObject<OpenGymDictContainer>();
    auto box4 = CreateObject<OpenGymBoxContainer<float>>(shape);
    box4->AddValue(3);
    box4->AddValue(4);
    testPushDict4->Add("sinr", box4);
    testStructure2.Push(testPushDict4, 0);

    auto result2 = testStructure2.GetNewestByID(0);
    NS_TEST_ASSERT_MSG_EQ(result->ns3timestamp, -1, "Time is tracked but shouldn't be");
    NS_TEST_ASSERT_MSG_EQ(result2->ns3timestamp, 0, "Time is not tracked");
}

/**
 * @ingroup defiance-tests
 *
 * @brief Test aggregation over multiple observations
 */
void
HistoryContainerTest::TestDataStructAggregation()
{
    HistoryContainer testStructure = HistoryContainer(3);
    std::vector<uint32_t> shape = {4};

    auto dict1 = CreateObject<OpenGymDictContainer>();
    auto box1 = CreateObject<OpenGymBoxContainer<float>>(shape);
    box1->AddValue(0.5);
    box1->AddValue(23.5);
    box1->AddValue(56.2);
    box1->AddValue(3.2);

    dict1->Add("sinr", box1);
    testStructure.Push(dict1, 0);

    // First test aggregation over one dict with one box of 4 values
    auto info = testStructure.AggregateNewest(0);
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetAvg(), 20.85, 0.0001, "Average is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetMax(), 56.2, 0.0001, "Max is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetMin(), 0.5, 0.0001, "Min is not correct");

    // Test aggregation over two dicts with one box of 4 values
    auto box2 = CreateObject<OpenGymBoxContainer<float>>(shape);
    auto dict2 = CreateObject<OpenGymDictContainer>();
    box2->AddValue(1.0);
    box2->AddValue(2.0);
    box2->AddValue(3.0);
    box2->AddValue(4.0);

    dict2->Add("sinr", box2);
    testStructure.Push(dict2, 0);

    info = testStructure.AggregateNewest(0, 2);

    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetAvg(), 11.675, 0.0001, "Average is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetMax(), 56.2, 0.0001, "Max is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetMin(), 0.5, 0.0001, "Min is not correct");

    // Test pushing a third and a fourth dict, overwriting the oldest dict and aggregating over the
    // last 3 dicts
    auto box3 = CreateObject<OpenGymBoxContainer<float>>(shape);
    auto dict3 = CreateObject<OpenGymDictContainer>();
    box3->AddValue(5.0);
    box3->AddValue(6.0);
    box3->AddValue(7.0);
    box3->AddValue(8.0);

    dict3->Add("sinr", box3);
    testStructure.Push(dict3, 0);

    auto box4 = CreateObject<OpenGymBoxContainer<float>>(shape);
    auto dict4 = CreateObject<OpenGymDictContainer>();
    box4->AddValue(9.0);
    box4->AddValue(10.0);
    box4->AddValue(11.0);
    box4->AddValue(12.0);

    dict4->Add("sinr", box4);
    testStructure.Push(dict4, 0);

    info = testStructure.AggregateNewest(0, 3);

    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetAvg(), 6.5, 0.0001, "Average is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetMax(), 12.0, 0.0001, "Max is not correct");
    NS_TEST_ASSERT_MSG_EQ_TOL(info["sinr"].GetMin(), 1.0, 0.0001, "Min is not correct");
}

void
HistoryContainerTest::DoRun()
{
    TestAggregatedInfo();
    TestDataStruct();
    TestDataStructAggregation();
}

/**
 * @ingroup defiance-tests
 *
 * @brief TestSuite for ObsDatastructure
 */
class HistoryContainerTestSuite : public TestSuite
{
  public:
    HistoryContainerTestSuite();

  protected:
    void DoTeardown() override;
};

HistoryContainerTestSuite::HistoryContainerTestSuite()
    : TestSuite("defiance-history-container", Type::UNIT)
{
    AddTestCase(new HistoryContainerTest, Duration::QUICK);
}

void
HistoryContainerTestSuite::DoTeardown()
{
    Simulator::Destroy();
}

static HistoryContainerTestSuite
    sHistoryContainerTestSuite; //!< Static variable for test initialization
