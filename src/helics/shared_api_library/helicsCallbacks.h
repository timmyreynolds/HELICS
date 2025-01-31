/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 * @brief Functions dealing with callbacks for the shared library
 */

#ifndef HELICS_APISHARED_CALLBACK_FUNCTIONS_H_
#define HELICS_APISHARED_CALLBACK_FUNCTIONS_H_

#include "helicsCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set the logging callback to a broker.
 *
 * @details Add a logging callback function to a broker.
 *          The logging callback will be called when
 *          a message flows into a broker from the core or from a broker.
 *
 * @param broker The broker object in which to set the callback.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *               the function arguments are loglevel, an identifier, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsBrokerSetLoggingCallback(HelicsBroker broker,
                                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                  void* userdata,
                                                  HelicsError* err);

/**
* Set the logging callback for a core.
*
* @details Add a logging callback function to a core. The logging callback will be called when
*          a message flows into a core from the core or from a broker.
*
* @param core The core object in which to set the callback.
* @param logger A callback with signature void(int, const char *, const char *, void *);
*               The function arguments are loglevel, an identifier, a message string, and a pointer to user data.
* @param userdata A pointer to user data that is passed to the function when executing.
*
* @param[in,out] err A pointer to an error object for catching errors.

*/
HELICS_EXPORT void helicsCoreSetLoggingCallback(HelicsCore core,
                                                void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                void* userdata,
                                                HelicsError* err);

/**
 * Set the logging callback for a federate.
 *
 * @details Add a logging callback function to a federate. The logging callback will be called when
 *          a message flows into a federate from the core or from a federate.
 *
 * @param fed The federate object in which to create a subscription must have been created with
 *            helicsCreateValueFederate or helicsCreateCombinationFederate.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *        The function arguments are loglevel, an identifier string, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void
    helicsFederateSetLoggingCallback(HelicsFederate fed,
                                     void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                     void* userdata,
                                     HelicsError* err);

/**
 * Set a general callback for a custom filter.
 *
 * @details Add a custom filter callback for creating a custom filter operation in the C shared library.
 *
 * @param filter The filter object to set the callback for.
 * @param filtCall A callback with signature helics_message_object(helics_message_object, void *);
 *                 The function arguments are the message to filter and a pointer to user data.
 *                 The filter should return a new message.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterSetCustomCallback(HelicsFilter filter,
                                                 HelicsMessage (*filtCall)(HelicsMessage message, void* userData),
                                                 void* userdata,
                                                 HelicsError* err);

/**
 * Set callback for queries executed against a federate.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param fed The federate to set the callback for.
 * @param queryAnswer A callback with signature const char *(const char *query, int querySize, HelicsQueryBuffer buffer, void *userdata);
 *                 The function arguments include the query string requesting an answer along with its size; the string is not guaranteed to
 be
 * null terminated. HelicsQueryBuffer is the buffer intended to filled out by the userCallback. The buffer can be empty if the query is not
 recognized and HELICS will generate the appropriate response.  The buffer is used to ensure memory ownership separation between user code
 and HELICS code.
 *  The HelicsQueryBufferFill method can be used to load a string into the buffer.
 @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */

HELICS_EXPORT void
    helicsFederateSetQueryCallback(HelicsFederate fed,
                                   void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer buffer, void* userdata),
                                   void* userdata,
                                   HelicsError* err);

/**
 * Set callback for the time request.
 *
 * @details This callback will be executed when a valid time request is made. It is intended for the possibility of embedded data grabbers
 in a callback to simplify user code.
 *
 * @param fed The federate to set the callback for.
 * @param requestTime A callback with signature void(HelicsTime currentTime, HelicsTime requestTime, bool iterating, void *userdata);
 *                 The function arguments are the current time value, the requested time value, a bool indicating that the time is
 iterating, and pointer to the userdata.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */

HELICS_EXPORT void helicsFederateSetTimeRequestEntryCallback(
    HelicsFederate fed,
    void (*requestTime)(HelicsTime currentTime, HelicsTime requestTime, HelicsBool iterating, void* userdata),
    void* userdata,
    HelicsError* err);

/**
 * Set callback for the time update.
 *
 * @details This callback will be executed every time the simulation time is updated starting on entry to executing mode.
 *
 * @param fed The federate to set the callback for.
 * @param timeUpdate A callback with signature void(HelicsTime newTime, bool iterating, void *userdata);
 *                 The function arguments are the new time value, a bool indicating that the time is iterating, and pointer to the userdata.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */

HELICS_EXPORT void helicsFederateSetTimeUpdateCallback(HelicsFederate fed,
                                                       void (*timeUpdate)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                                       void* userdata,
                                                       HelicsError* err);

/**
 * Set callback for the federate mode change.
 *
 * @details This callback will be executed every time the operating mode of the federate changes.
 *
 * @param fed The federate to set the callback for.
 * @param stateChange A callback with signature void(HelicsFederateState newState, HelicsFederateState oldState, void *userdata);
 *                 The function arguments are the new state, the old state, and pointer to the userdata.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void
    helicsFederateSetStateChangeCallback(HelicsFederate fed,
                                         void (*stateChange)(HelicsFederateState newState, HelicsFederateState oldState, void* userdata),
                                         void* userdata,
                                         HelicsError* err);

// Definition of helicsFederateStateChangeCallback located in FederateExport since it makes use of some data only available in that
// compilation unit

/**
 * Set callback for the time request return.
 *
 * @details This callback will be executed after all other callbacks for a time request return.  This callback will be the last thing
 executed before returning control to the user program.
 * The difference between this and the TimeUpdate callback is the order of execution.  The timeUpdate callback is executed prior to
 individual interface callbacks, this callback is executed after all others.
 * @param fed The federate to set the callback for.
 * @param requestTimeReturn A callback with signature void(HelicsTime newTime, bool iterating, void *userdata);
 *                 The function arguments are the new time value, a bool indicating that the time is iterating, and pointer to the userdata.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void
    helicsFederateSetTimeRequestReturnCallback(HelicsFederate fed,
                                               void (*requestTimeReturn)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                               void* userdata,
                                               HelicsError* err);

/**
 * Set the data for a query callback.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param buffer The buffer received in a helicsQueryCallback.
 * @param str Pointer to the data to fill the buffer with.
 * @param strSize The size of the string.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsQueryBufferFill(HelicsQueryBuffer buffer, const char* str, int strSize, HelicsError* err);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
