/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Translator.hpp"

#include "CoreApp.hpp"
#include "TranslatorOperations.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

namespace helics {
static const std::map<std::string, TranslatorTypes> translatorTypes{{"json", TranslatorTypes::JSON},
                                                                    {"binary",
                                                                     TranslatorTypes::BINARY},
                                                                    {"custom",
                                                                     TranslatorTypes::CUSTOM}};

TranslatorTypes translatorTypeFromString(const std::string& translatorType) noexcept
{
    auto fnd = translatorTypes.find(translatorType);
    if (fnd != translatorTypes.end()) {
        return fnd->second;
    }
    auto nfilt = translatorType;
    std::transform(nfilt.begin(), nfilt.end(), nfilt.begin(), ::tolower);
    fnd = translatorTypes.find(nfilt);
    if (fnd != translatorTypes.end()) {
        return fnd->second;
    }
    return TranslatorTypes::UNRECOGNIZED;
}

void addOperations(Translator* trans, TranslatorTypes type)
{
    switch (type) {
        case TranslatorTypes::CUSTOM:
        default:
            break;
        case TranslatorTypes::JSON: {
            auto op = std::make_shared<JsonTranslatorOperation>();
            trans->setTranslatorOperations(std::move(op));
        } break;
        case TranslatorTypes::BINARY: {
            auto op = std::make_shared<BinaryTranslatorOperation>();
            trans->setTranslatorOperations(std::move(op));
        } break;
    }
}

Translator::Translator(Federate* ffed, std::string_view translatorName, InterfaceHandle ihandle):
    Interface(ffed, ihandle, translatorName)
{
}

Translator::Translator(Core* core, std::string_view translatorName, InterfaceHandle ihandle):
    Interface(core, ihandle, translatorName)
{
}

Translator::Translator(Core* core, const std::string_view translatorName):
    Interface(core, InterfaceHandle(), translatorName)
{
    if (cr != nullptr) {
        handle = cr->registerTranslator(translatorName, std::string_view{}, std::string_view{});
    }
}

void Translator::setOperator(std::shared_ptr<TranslatorOperator> mo)
{
    if (mo) {
        setTranslatorOperations(std::make_shared<CustomTranslatorOperation>(mo));
    } else {
        setTranslatorOperations(nullptr);
    }
}

void Translator::setTranslatorOperations(std::shared_ptr<TranslatorOperations> transOps)
{
    transOp = std::move(transOps);
    if (cr != nullptr) {
        cr->setTranslatorOperator(handle, (transOp) ? transOp->getOperator() : nullptr);
    }
}

static const std::string emptyStr;

void Translator::set(const std::string& property, double val)
{
    if (transOp) {
        transOp->set(property, val);
    }
}

void Translator::setString(const std::string& property, const std::string& val)
{
    if (transOp) {
        transOp->setString(property, val);
    }
}

void Translator::setTranslatorType(std::int32_t type)
{
    addOperations(this, static_cast<TranslatorTypes>(type));
}

}  // namespace helics
