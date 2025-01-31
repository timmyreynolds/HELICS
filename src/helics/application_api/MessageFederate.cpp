/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MessageFederate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../common/addTargets.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helics_definitions.hpp"
#include "Endpoints.hpp"
#include "MessageFederateManager.hpp"

#include <utility>

namespace helics {
MessageFederate::MessageFederate(const std::string& fedName, const FederateInfo& fi):
    Federate(fedName, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}
MessageFederate::MessageFederate(const std::string& fedName,
                                 const std::shared_ptr<Core>& core,
                                 const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}

MessageFederate::MessageFederate(const std::string& fedName, CoreApp& core, const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}

MessageFederate::MessageFederate(const std::string& fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
    if (looksLikeFile(configString)) {
        MessageFederate::registerInterfaces(configString);
    }
}

MessageFederate::MessageFederate(const std::string& configString):
    MessageFederate(std::string{}, configString)
{
}

MessageFederate::MessageFederate(const char* configString):
    MessageFederate(std::string{}, std::string{configString})
{
}

MessageFederate::MessageFederate()
{
    // default constructor
}

MessageFederate::MessageFederate(bool /*unused*/)
{  // this constructor should only be called by child class that has already constructed the
   // underlying federate in
    // a virtual inheritance
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}
MessageFederate::MessageFederate(MessageFederate&&) noexcept = default;

MessageFederate& MessageFederate::operator=(MessageFederate&& mFed) noexcept
{
    mfManager = std::move(mFed.mfManager);
    if (getID() != mFed.getID()) {  // the id won't be moved, as it is copied so use it as a test if
                                    // it has moved already
        Federate::operator=(std::move(mFed));
    }
    return *this;
}

MessageFederate::~MessageFederate() = default;

void MessageFederate::disconnect()
{
    Federate::disconnect();
    mfManager->disconnect();
}

void MessageFederate::updateTime(Time newTime, Time oldTime)
{
    mfManager->updateTime(newTime, oldTime);
}

void MessageFederate::startupToInitializeStateTransition()
{
    mfManager->startupToInitializeStateTransition();
}
void MessageFederate::initializeToExecuteStateTransition(IterationResult result)
{
    mfManager->initializeToExecuteStateTransition(result);
}

std::string MessageFederate::localQuery(const std::string& queryStr) const
{
    return mfManager->localQuery(queryStr);
}

Endpoint& MessageFederate::registerEndpoint(const std::string& eptName, const std::string& type)
{
    return mfManager->registerEndpoint((!eptName.empty()) ?
                                           (getName() + nameSegmentSeparator + eptName) :
                                           eptName,
                                       type);
}

Endpoint& MessageFederate::registerTargetedEndpoint(const std::string& eptName,
                                                    const std::string& type)
{
    return mfManager->registerTargetedEndpoint((!eptName.empty()) ?
                                                   (getName() + nameSegmentSeparator + eptName) :
                                                   eptName,
                                               type);
}

Endpoint& MessageFederate::registerGlobalEndpoint(const std::string& eptName,
                                                  const std::string& type)
{
    return mfManager->registerEndpoint(eptName, type);
}

Endpoint& MessageFederate::registerGlobalTargetedEndpoint(const std::string& eptName,
                                                          const std::string& type)
{
    return mfManager->registerTargetedEndpoint(eptName, type);
}

void MessageFederate::registerInterfaces(const std::string& configString)
{
    registerMessageInterfaces(configString);
    Federate::registerFilterInterfaces(configString);
}

void MessageFederate::registerMessageInterfaces(const std::string& configString)
{
    if (fileops::hasTomlExtension(configString)) {
        registerMessageInterfacesToml(configString);
    } else {
        registerMessageInterfacesJson(configString);
    }
}

static const std::string emptyStr;
template<class Inp>
static void loadOptions(MessageFederate* fed, const Inp& data, Endpoint& ept)
{
    using fileops::getOrDefault;
    addTargets(data, "flags", [&ept, fed](const std::string& target) {
        auto oindex = getOptionIndex((target.front() != '-') ? target : target.substr(1));
        int val = (target.front() != '-') ? 1 : 0;
        if (oindex == HELICS_INVALID_OPTION_INDEX) {
            fed->logWarningMessage(target + " is not a recognized flag");
            return;
        }
        ept.setOption(oindex, val);
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&ept](int32_t option, int32_t value) { ept.setOption(option, value); });

    auto info = getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        ept.setInfo(info);
    }
    loadTags(data, [&ept](const std::string& tagname, const std::string& tagvalue) {
        ept.setTag(tagname, tagvalue);
    });
    addTargets(data, "subscriptions", [&ept](const std::string& sub) { ept.subscribe(sub); });
    addTargets(data, "filters", [&ept](const std::string& filt) { ept.addSourceFilter(filt); });
    addTargets(data, "sourceFilters", [&ept](const std::string& filt) {
        ept.addSourceFilter(filt);
    });
    addTargets(data, "destFilters", [&ept](const std::string& filt) {
        ept.addDestinationFilter(filt);
    });

    auto defTarget = getOrDefault(data, "target", emptyStr);
    fileops::replaceIfMember(data, "destination", defTarget);
    if (!defTarget.empty()) {
        ept.setDefaultDestination(defTarget);
    }
}

void MessageFederate::registerMessageInterfacesJson(const std::string& jsonString)
{
    auto doc = fileops::loadJson(jsonString);
    bool defaultGlobal = false;
    fileops::replaceIfMember(doc, "defaultglobal", defaultGlobal);
    if (doc.isMember("endpoints")) {
        for (const auto& ept : doc["endpoints"]) {
            auto eptName = fileops::getName(ept);
            auto type = fileops::getOrDefault(ept, "type", emptyStr);
            bool global = fileops::getOrDefault(ept, "global", defaultGlobal);
            Endpoint& epObj =
                (global) ? registerGlobalEndpoint(eptName, type) : registerEndpoint(eptName, type);

            loadOptions(this, ept, epObj);
        }
    }
}

void MessageFederate::registerMessageInterfacesToml(const std::string& tomlString)
{
    toml::value doc;
    try {
        doc = fileops::loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }
    bool defaultGlobal = false;
    fileops::replaceIfMember(doc, "defaultglobal", defaultGlobal);

    if (fileops::isMember(doc, "endpoints")) {
        auto epts = toml::find(doc, "endpoints");
        if (!epts.is_array()) {
            throw(helics::InvalidParameter("endpoints section in toml file must be an array"));
        }
        auto& eptArray = epts.as_array();
        for (auto& ept : eptArray) {
            auto key = fileops::getName(ept);
            auto type = fileops::getOrDefault(ept, "type", emptyStr);
            bool global = fileops::getOrDefault(ept, "global", defaultGlobal);
            Endpoint& epObj =
                (global) ? registerGlobalEndpoint(key, type) : registerEndpoint(key, type);

            loadOptions(this, ept, epObj);
        }
    }
}

void MessageFederate::subscribe(const Endpoint& ept, std::string_view key)
{
    coreObject->addSourceTarget(ept, key);
}

bool MessageFederate::hasMessage() const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->hasMessage();
    }
    return false;
}

bool MessageFederate::hasMessage(const Endpoint& ept) const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->hasMessage(ept);
    }
    return false;
}

uint64_t MessageFederate::pendingMessageCount(const Endpoint& ept) const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->pendingMessageCount(ept);
    }
    return 0;
}

uint64_t MessageFederate::pendingMessageCount() const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->pendingMessageCount();
    }
    return 0;
}

std::unique_ptr<Message> MessageFederate::getMessage()
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->getMessage();
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederate::getMessage(const Endpoint& ept)
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->getMessage(ept);
    }
    return nullptr;
}

Endpoint& MessageFederate::getEndpoint(const std::string& eptName) const
{
    auto& id = mfManager->getEndpoint(eptName);
    if (!id.isValid()) {
        return mfManager->getEndpoint(getName() + nameSegmentSeparator + eptName);
    }
    return id;
}

Endpoint& MessageFederate::getEndpoint(int index) const
{
    return mfManager->getEndpoint(index);
}

void MessageFederate::setMessageNotificationCallback(
    const std::function<void(Endpoint& ept, Time)>& callback)
{
    mfManager->setEndpointNotificationCallback(callback);
}
void MessageFederate::setMessageNotificationCallback(
    const Endpoint& ept,
    const std::function<void(Endpoint& ept, Time)>& callback)
{
    mfManager->setEndpointNotificationCallback(ept, callback);
}

/** get a count of the number endpoints registered*/
int MessageFederate::getEndpointCount() const
{
    return mfManager->getEndpointCount();
}

}  // namespace helics
