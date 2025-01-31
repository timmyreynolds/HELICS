/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"
#include "helics/helics.h"

#include <future>
#include <gtest/gtest.h>
#include <thread>
/** these test cases test out the message federates
 */

class filter_simple_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

/*
class filter_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};
*/

class filter: public FederateTestFixture, public ::testing::Test {};

/** test registration of filters*/

TEST_P(filter_simple_type_tests, registration)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    helicsFederateRegisterGlobalEndpoint(mFed, "port2", nullptr, &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err));
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    EXPECT_TRUE(f1 != nullptr);
    CE(auto f2 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter2", &err));
    CE(helicsFilterAddDestinationTarget(f2, "port2", &err));
    EXPECT_TRUE(f2 != f1);
    CE(auto ep1 = helicsFederateRegisterEndpoint(fFed, "fout", "", &err));
    EXPECT_TRUE(ep1 != nullptr);
    CE(auto f3 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "c4", &err));
    EXPECT_EQ(helicsFilterIsValid(f3), HELICS_TRUE);
    helicsFilterAddSourceTarget(f3, "filter0/fout", nullptr);
    EXPECT_TRUE(f3 != f2);

    auto f1_b = helicsFederateGetFilter(fFed, "filter1", &err);
    const char* tmp;
    tmp = helicsFilterGetName(f1_b);
    EXPECT_STREQ(tmp, "filter0/filter1");

    CE(helicsFilterSetTag(f1_b, "tag1", "tagvalue", &err));
    EXPECT_STREQ(helicsFilterGetTag(f1_b, "tag1"), "tagvalue");

    auto f1_c = helicsFederateGetFilterByIndex(fFed, 2, &err);
    tmp = helicsFilterGetName(f1_c);
    EXPECT_STREQ(tmp, "filter0/c4");

    auto f1_n = helicsFederateGetFilterByIndex(fFed, -2, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(f1_n, nullptr);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

/**
Test filter info fields
*/
TEST_P(filter_simple_type_tests, info_tests)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", nullptr, &err);

    CE(helicsEndpointSetInfo(p1, "p1_test", &err));
    CE(helicsEndpointSetInfo(p2, "p2_test", &err));

    CE(auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err));
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    CE(helicsFilterSetInfo(f1, "f1_test", &err));

    CE(auto f2 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter2", &err));
    CE(helicsFilterAddDestinationTarget(f2, "port2", &err));
    CE(helicsFilterSetInfo(f2, "f2_test", &err));

    CE(auto ep1 = helicsFederateRegisterEndpoint(fFed, "fout", "", &err));
    CE(helicsEndpointSetInfo(ep1, "ep1_test", &err));
    CE(auto f3 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "c4", &err));
    helicsFilterAddSourceTarget(f3, "filter0/fout", nullptr);
    CE(helicsFilterSetInfo(f3, "f3_test", &err));

    // Check endpoints
    EXPECT_STREQ(helicsEndpointGetInfo(p1), "p1_test");
    EXPECT_STREQ(helicsEndpointGetInfo(p2), "p2_test");
    EXPECT_STREQ(helicsEndpointGetInfo(ep1), "ep1_test");

    // Check filters
    EXPECT_STREQ(helicsFilterGetInfo(f1), "f1_test");
    EXPECT_STREQ(helicsFilterGetInfo(f2), "f2_test");
    EXPECT_STREQ(helicsFilterGetInfo(f3), "f3_test");

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
}

TEST_F(filter, core_filter_reg)
{
    CE(auto core1 = helicsCreateCore("test", "core1", "--autobroker", &err));

    CE(auto core2 = helicsCoreClone(core1, &err));

    std::string core1IdentifierString = helicsCoreGetIdentifier(core1);

    EXPECT_EQ(core1IdentifierString, "core1");

    CE(auto sourceFilter1 = helicsCoreRegisterFilter(
           core1, HelicsFilterTypes::HELICS_FILTER_TYPE_DELAY, "core1SourceFilter", &err));

    CE(helicsFilterAddSourceTarget(sourceFilter1, "ep1", &err));
    CE(auto destinationFilter1 = helicsCoreRegisterFilter(
           core1, HelicsFilterTypes::HELICS_FILTER_TYPE_DELAY, "core1DestinationFilter", &err));

    helicsFilterAddDestinationTarget(destinationFilter1, "ep2", &err);
    CE(auto cloningFilter1 = helicsCoreRegisterCloningFilter(core1, "ep3", &err));

    helicsFilterRemoveDeliveryEndpoint(cloningFilter1, "ep3", &err);
    int core1IsConnected = helicsCoreIsConnected(core1);
    EXPECT_NE(core1IsConnected, HELICS_FALSE);
    helicsCoreSetReadyToInit(core1, &err);
    helicsCoreDisconnect(core1, &err);
    helicsCoreDisconnect(core2, &err);
    helicsCoreFree(core1);
    helicsCoreFree(core2);
    helicsCloseLibrary();
}

TEST_P(filter_simple_type_tests, message_filter_function)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_P(filter_simple_type_tests, function_mObj)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}
/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/
TEST_P(filter_simple_type_tests, function_two_stage)
{
    HelicsBroker broker = AddBroker(GetParam(), 3);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter2");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto fFed2 = GetFederateAt(1);
    auto mFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));

    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 1.25, &err));

    CE(auto f2 = helicsFederateRegisterFilter(fFed2, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    CE(helicsFilterAddSourceTarget(f2, "port1", &err));
    EXPECT_TRUE(f2 != nullptr);
    CE(helicsFilterSet(f2, "delay", 1.25, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(fFed2, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed2, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, .0, &err));
    CE(helicsFederateRequestTimeAsync(fFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed2, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, .0, &err));
    CE(helicsFederateRequestTimeAsync(fFed2, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed2, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTimeAsync(fFed2, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    if (helicsEndpointHasMessage(p2) == HELICS_FALSE) {
        printf("missing message\n");
    }
    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed2, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalizeAsync(fFed, &err));
    CE(helicsFederateFinalize(fFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(helicsFederateFinalizeComplete(fFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_simple_type_tests, function2)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err));
    helicsFilterAddSourceTarget(f1, "port1", nullptr);
    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 2.5, &err));

    CE(auto f2 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    helicsFilterAddSourceTarget(f2, "port2", nullptr);
    EXPECT_TRUE(f2 != nullptr);
    CE(helicsFilterSet(f2, "delay", 2.5, &err));
    // this is expected to fail since a regular filter doesn't have a delivery endpoint
    helicsFilterAddDeliveryEndpoint(f2, "port1", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);
    CE(helicsEndpointSendBytesTo(p2, data.c_str(), static_cast<int>(data.size()), "port1", &err));
    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));
    // there may be something wrong here yet but this test isn't the one to find it and
    // this may prevent spurious errors for now.
    std::this_thread::yield();
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    EXPECT_TRUE(!helicsEndpointHasMessage(p1));
    CE(helicsFederateRequestTime(mFed, 4.0, &err));
    EXPECT_TRUE(helicsEndpointHasMessage(p1));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_P(filter_simple_type_tests, message_filter_function3)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "random", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(auto f1 =
           helicsFederateRegisterGlobalFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err));
    EXPECT_TRUE(f1 != nullptr);
    helicsFilterAddSourceTarget(f1, "port1", nullptr);
    CE(auto f2 =
           helicsFederateRegisterGlobalFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    helicsFilterAddSourceTarget(f2, "port1", nullptr);

    helicsFederateRegisterEndpoint(fFed, "fout", "", &err);
    CE(auto f3 =
           helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_RANDOM_DELAY, "filter3", &err));
    helicsFilterAddSourceTarget(f3, "filter0/fout", nullptr);

    CE(helicsFilterSet(f2, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data = "hello world";
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);
    CE(helicsEndpointSendBytesTo(p2, data.c_str(), static_cast<int>(data.size()), "port1", &err));
    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    EXPECT_TRUE(!helicsEndpointHasMessage(p2));
    // there may be something wrong here yet but this test isn't the one to find it and
    // this may prevent spurious errors for now.
    std::this_thread::yield();
    CE(helicsFederateRequestTimeAsync(mFed, 3.0, &err));
    CE(helicsFederateRequestTime(fFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    EXPECT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    EXPECT_TRUE(helicsEndpointHasMessage(p1));
    CE(helicsFederateFinalize(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto p3 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto f1 = helicsFederateRegisterCloningFilter(dcFed, nullptr, &err);
    CE(helicsFilterAddDeliveryEndpoint(f1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "src", &err));

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));

    CE(helicsFederateEnterExecutingModeAsync(dFed, &err));

    CE(helicsFederateEnterExecutingModeComplete(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p2);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p3);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto p3 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto f1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(f1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    auto cr = helicsFederateGetCore(sFed, &err);

    CE(helicsCoreAddSourceFilterToEndpoint(cr, "filt1", "src", &err));

    // error test
    helicsCoreAddSourceFilterToEndpoint(cr, nullptr, "src", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    // this is testing the filtered_endpoints query for cloning source filters
    auto q = helicsCreateQuery("", "filtered_endpoints");
    std::string filteredEndpoints = helicsQueryExecute(q, sFed, nullptr);
    // std::cout << filteredEndpoints << std::endl;
    EXPECT_TRUE(filteredEndpoints.find("(cloning)") != std::string::npos);
    EXPECT_TRUE(filteredEndpoints.find("srcFilters") != std::string::npos);
    helicsQueryFree(q);

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p2);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p3);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test_broker_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto p3 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto f1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(f1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    CE(helicsBrokerAddSourceFilterToEndpoint(brokers[0], "filt1", "src", &err));

    // error test
    helicsBrokerAddSourceFilterToEndpoint(brokers[0], nullptr, "src", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p2);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p3);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int>(data.size()));
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

// this tests using a remote core to connect an endpoint to a cloning destination filter
TEST_F(filter, clone_test_dest_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 2.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);
    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto p3 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto f1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(f1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    auto cr = helicsFederateGetCore(sFed, &err);

    CE(helicsCoreAddDestinationFilterToEndpoint(cr, "filt1", "dest", &err));

    // error test
    helicsCoreAddDestinationFilterToEndpoint(cr, nullptr, "dest", &err);

    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsCoreFree(cr);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    auto q = helicsCreateQuery("", "filtered_endpoints");
    std::string filteredEndpoints = helicsQueryExecute(q, dFed, nullptr);
    // std::cout << filteredEndpoints << std::endl;
    EXPECT_TRUE(filteredEndpoints.find("cloningdestFilter") != std::string::npos);
    helicsQueryFree(q);

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateFinalize(sFed, nullptr));

    HelicsMessage m2;
    auto dFedExec = [&]() {
        helicsFederateRequestTime(dFed, 1.0, nullptr);
        m2 = helicsEndpointGetMessage(p2);
        helicsFederateFinalize(dFed, nullptr);
    };

    HelicsMessage m3;
    auto dcFedExec = [&]() {
        helicsFederateRequestTime(dcFed, 2.0, nullptr);
        auto res = helicsFederateHasMessage(dcFed);
        if (res == HELICS_FALSE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            helicsFederateRequestTime(dcFed, 4.0, nullptr);
        }
        m3 = helicsEndpointGetMessage(p3);
        helicsFederateFinalize(dcFed, nullptr);
    };

    auto threaddFed = std::thread(dFedExec);
    auto threaddcFed = std::thread(dcFedExec);

    threaddFed.join();
    EXPECT_STREQ(helicsMessageGetSource(m2), "src");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));

    threaddcFed.join();

    EXPECT_STREQ(helicsMessageGetSource(m3), "src");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m3), "src");
    EXPECT_STREQ(helicsMessageGetDestination(m3), "cm");
    EXPECT_STREQ(helicsMessageGetOriginalDestination(m3), "dest");
    EXPECT_EQ(helicsMessageGetByteCount(m3), static_cast<int64_t>(data.size()));

    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test_broker_dest_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto p3 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto f1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(f1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    CE(helicsBrokerAddDestinationFilterToEndpoint(brokers[0], "filt1", "dest", &err));

    // error test
    helicsBrokerAddDestinationFilterToEndpoint(brokers[0], nullptr, "dest", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p2);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    }
    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));

    // now check the message clone
    auto res2 = helicsFederateHasMessage(dcFed);

    if (res2 == HELICS_FALSE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        CE(helicsFederateRequestTime(dcFed, 2.0, &err));
        res2 = helicsFederateHasMessage(dcFed);
    }

    EXPECT_EQ(res2, HELICS_TRUE);

    if (res2 == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p3);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, multi_clone_test)
{
    extraBrokerArgs = " --globaltime";
    HelicsBroker broker = AddBroker("test", 4);
    AddFederates(helicsCreateMessageFederate, "test", 2, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto sFed2 = GetFederateAt(1);
    auto dFed = GetFederateAt(2);
    auto dcFed = GetFederateAt(3);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    ASSERT_EQ(err.error_code, 0);
    auto p2 = helicsFederateRegisterGlobalEndpoint(sFed2, "src2", "", &err);
    ASSERT_EQ(err.error_code, 0);
    auto p3 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    ASSERT_EQ(err.error_code, 0);
    auto p4 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);
    ASSERT_EQ(err.error_code, 0);

    auto f1 = helicsFederateRegisterCloningFilter(dcFed, "", &err);
    helicsFilterAddDeliveryEndpoint(f1, "cm", nullptr);
    ASSERT_EQ(err.error_code, 0);
    CE(helicsFilterAddSourceTarget(f1, "src", &err));
    CE(helicsFilterAddSourceTarget(f1, "src2", &err));

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(sFed2, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    std::string data2(400, 'b');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "dest", &err));
    CE(helicsEndpointSendBytesTo(p2, data2.c_str(), static_cast<int>(data2.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(sFed2, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(sFed2, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto mcnt = helicsEndpointPendingMessageCount(p3);
    EXPECT_EQ(mcnt, 2);
    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto m2 = helicsEndpointGetMessage(p3);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
        res = helicsFederateHasMessage(dFed);
        EXPECT_EQ(res, HELICS_TRUE);

        if (res == HELICS_TRUE) {
            m2 = helicsFederateGetMessage(dFed);
            EXPECT_STREQ(helicsMessageGetSource(m2), "src2");
            EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src2");
            EXPECT_STREQ(helicsMessageGetDestination(m2), "dest");
            EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data2.size()));
        }
    }

    // now check the message clone
    mcnt = helicsEndpointPendingMessageCount(p4);
    EXPECT_EQ(mcnt, 2);
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res != HELICS_FALSE) {
        auto m2 = helicsFederateGetMessage(dcFed);
        EXPECT_STREQ(helicsMessageGetSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(m2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
        res = helicsFederateHasMessage(dcFed);
        EXPECT_EQ(res, HELICS_TRUE);

        if (res != HELICS_FALSE) {
            m2 = helicsFederateGetMessage(dcFed);
            EXPECT_STREQ(helicsMessageGetSource(m2), "src2");
            EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "src2");
            EXPECT_STREQ(helicsMessageGetDestination(m2), "cm");
            EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "dest");
            EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data2.size()));
        }
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(sFed2, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(sFed2, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, file_load)
{
    std::string filename = std::string(TEST_DIR) + "/example_filters.json";
    auto mFed = helicsCreateMessageFederateFromConfig(filename.c_str(), &err);

    const char* name = helicsFederateGetName(mFed);
    EXPECT_STREQ(name, "filterFed");

    EXPECT_EQ(helicsFederateGetEndpointCount(mFed), 3);
    helicsFederateFinalize(mFed, nullptr);
    helicsFederateFree(mFed);
    // auto id = mFed.getEndpointId ("ept1");
    // EXPECT_EQ (mFed.getEndpointType (id), "genmessage");

    // EXPECT_EQ (mFed.filterObjectCount (), 3);

    // auto filt = mFed.getFilterObject (2);

    // auto cloneFilt = std::dynamic_pointer_cast<helics::CloningFilter> (filt);
    // EXPECT_TRUE (cloneFilt);
    // mFed.disconnect ();
}

static HelicsMessage filterFunc1(HelicsMessage mess, void* /*unused*/)
{
    auto time = helicsMessageGetTime(mess);
    helicsMessageSetTime(mess, time + 2.5, nullptr);
    return mess;
}

TEST_F(filter, callbacks)
{
    HelicsBroker broker = AddBroker("test", 2);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err);
    auto f2 = helicsFederateRegisterFilter(mFed, HELICS_FILTER_TYPE_DELAY, "dfilter", &err);

    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    CE(helicsFilterSetCustomCallback(f1, filterFunc1, nullptr, &err));
    EXPECT_EQ(err.error_code, 0);

    helicsFilterSetCustomCallback(f2, filterFunc1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(filter, filter_simple_type_tests, ::testing::ValuesIn(CoreTypes_simple));
/*
INSTANTIATE_TEST_SUITE_P(filter, filter_type_tests, ::testing::ValuesIn(CoreTypes));
*/
