/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BLE/ble.h"


#include "ble/DiscoveredCharacteristic.h"

#include "core-util/FunctionPointer.h"
#include "core-util/SharedPointer.h"

#include "mbed-block/BlockDynamic.h"

using namespace mbed::util;

class ANCSClient
{
public:
    typedef enum {
        CategoryIDOther              = 0,
        CategoryIDIncomingCall       = 1,
        CategoryIDMissedCall         = 2,
        CategoryIDVoicemail          = 3,
        CategoryIDSocial             = 4,
        CategoryIDSchedule           = 5,
        CategoryIDEmail              = 6,
        CategoryIDNews               = 7,
        CategoryIDHealthAndFitness   = 8,
        CategoryIDBusinessAndFinance = 9,
        CategoryIDLocation           = 10,
        CategoryIDEntertainment      = 11
    } category_id_t;

    typedef enum {
        EventIDNotificationAdded    = 0,
        EventIDNotificationModified = 1,
        EventIDNotificationRemoved  = 2
    } event_id_t;

    typedef enum {
        EventFlagSilent         = (1 << 0),
        EventFlagImportant      = (1 << 1),
        EventFlagPreExisting    = (1 << 2),
        EventFlagPositiveAction = (1 << 3),
        EventFlagNegativeAction = (1 << 4)
    } event_flags_t;

    typedef enum {
        CommandIDGetNotificationAttributes  = 0,
        CommandIDGetAppAttributes           = 1,
        CommandIDPerformNotificationAction  = 2
    } command_id_t;

    typedef enum {
        NotificationAttributeIDAppIdentifier       = 0,
        NotificationAttributeIDTitle               = 1, // (Needs to be followed by a 2-bytes max length parameter)
        NotificationAttributeIDSubtitle            = 2, // (Needs to be followed by a 2-bytes max length parameter)
        NotificationAttributeIDMessage             = 3, // (Needs to be followed by a 2-bytes max length parameter)
        NotificationAttributeIDMessageSize         = 4,
        NotificationAttributeIDDate                = 5,
        NotificationAttributeIDPositiveActionLabel = 6,
        NotificationAttributeIDNegativeActionLabel = 7
    } notification_attribute_id_t;

    typedef enum {
        ActionIDPositive = 0,
        ActionIDNegative = 1
    } action_id_t;

    typedef enum {
        AppAttributeIDDisplayName = 0
    } app_attribute_id_t;

    typedef struct {
        uint8_t eventID;
        uint8_t eventFlags;
        uint8_t categoryID;
        uint8_t categoryCount;
        uint32_t notificationUID;
    } Notification_t;

    ANCSClient();

    void init(Gap::Handle_t connection);

    /*
        Register callback for when notifications are received.
    */
    void registerNotificationHandler(void (*callback)(Notification_t))
    {
        notificationHandler.attach(callback);
    }

    template <typename T>
    void registerNotificationHandler(T* object, void (T::*member)(Notification_t))
    {
        notificationHandler.attach(object, member);
    }

    /*
        Register callback for when data is received.
    */
    void registerDataHandler(void (*callback)(SharedPointer<Block>))
    {
        dataHandler.attach(callback);
    }

    template <typename T>
    void registerDataHandler(T* object, void (T::*member)(SharedPointer<Block>))
    {
        dataHandler.attach(object, member);
    }

    /*
        Get notification attribute.
    */
    void getNotificationAttribute(uint32_t notificationUID, notification_attribute_id_t, uint16_t length = 0);



    void characteristicDiscoveryCallback(const DiscoveredCharacteristic* characteristicP);

private:

    typedef enum {
        FLAG_NOTIFICATION           = 0x01,
        FLAG_CONTROL                = 0x02,
        FLAG_DATA                   = 0x04,
        FLAG_ENCRYPTION             = 0x08,
        FLAG_NOTIFICATION_SUBSCRIBE = 0x10,
        FLAG_DATA_SUBSCRIBE         = 0x20
    } flags_t;

    void whenDisconnected(const Gap::DisconnectionCallbackParams_t* params);
    void subscribe();
    void hvxCallback(const GattHVXCallbackParams* params);
    void dataSent(unsigned count);

    void securityInitiated(Gap::Handle_t, bool, bool, SecurityManager::SecurityIOCapabilities_t);
    void securityCompleted(Gap::Handle_t, SecurityManager::SecurityCompletionStatus_t);
    void linkSecured(Gap::Handle_t, SecurityManager::SecurityMode_t);
    void contextStored(Gap::Handle_t);
    void passkeyDisplay(Gap::Handle_t, const SecurityManager::Passkey_t);

private:
    uint8_t state;

    Gap::Handle_t connectionHandle;

    DiscoveredCharacteristic notificationSource;
    DiscoveredCharacteristic controlPoint;
    DiscoveredCharacteristic dataSource;

    FunctionPointer1<void, Notification_t> notificationHandler;

    // variables for assembling data fragments
    uint16_t dataLength;
    uint16_t dataOffset;
    SharedPointer<Block> dataPayload;
    FunctionPointer1<void, SharedPointer<Block> > dataHandler;
};
