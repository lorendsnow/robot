#include "btstack.h"
#include "pico/btstack_run_loop_async_context.h"
#include "pico/printf.h"
#include "pico/stdlib.h"
#include "string.h"

#include "bluetooth/ble_client.h"
#include "thumbstick.h"
#include "motor_control.h"

// Service UUID: 48dc6454-5b36-425d-b974-5880528c99db
// BTstack discovery/event APIs use big-endian (textual) byte order.
static const uint8_t controller_service_uuid[16] = {
    0x48, 0xdc, 0x64, 0x54, 0x5b, 0x36, 0x42, 0x5d,
    0xb9, 0x74, 0x58, 0x80, 0x52, 0x8c, 0x99, 0xdb};

// Characteristic UUID: 0f9e4129-0220-4669-82dc-79b378fa1dff
// BTstack discovery/event APIs use big-endian (textual) byte order.
static const uint8_t thumbstick_characteristic_uuid[16] = {
    0x0f, 0x9e, 0x41, 0x29, 0x02, 0x20, 0x46, 0x69,
    0x82, 0xdc, 0x79, 0xb3, 0x78, 0xfa, 0x1d, 0xff};

static const char* controller_name = "Controller";

typedef enum {
    CLIENT_OFF,
    CLIENT_W4_SCAN_RESULT,
    CLIENT_W4_CONNECT,
    CLIENT_W4_SERVICE_RESULT,
    CLIENT_W4_CHARACTERISTIC_RESULT,
    CLIENT_W4_ENABLE_NOTIFICATIONS_COMPLETE,
    CLIENT_READY
} client_state_t;

static client_state_t client_state = CLIENT_OFF;

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

static bd_addr_t        controller_addr;
static bd_addr_type_t   controller_addr_type;
static hci_con_handle_t connection_handle;

static gatt_client_service_t        controller_service;
static gatt_client_characteristic_t thumbstick_characteristic;
static gatt_client_notification_t   notification_listener;
static int                          notification_listener_registered = 0;
static int                          thumbstick_characteristic_found  = 0;
static indicator_pins_t*            indicator_pins                   = NULL;

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel,
                                     uint8_t* packet, uint16_t size);

static bool advertisement_contains_name(const char* name, uint8_t adv_len,
                                        const uint8_t* adv_data) {
    uint16_t     name_len = (uint8_t)strlen(name);
    ad_context_t context;
    for (ad_iterator_init(&context, adv_len, adv_data);
         ad_iterator_has_more(&context); ad_iterator_next(&context)) {
        uint8_t        data_type = ad_iterator_get_data_type(&context);
        uint8_t        data_size = ad_iterator_get_data_len(&context);
        const uint8_t* data      = ad_iterator_get_data(&context);
        switch (data_type) {
            case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
            case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
                if (data_size < name_len) break;
                if (memcmp(data, name, name_len) == 0) return true;
                break;
            default:
                break;
        }
    }
    return false;
}

static void client_start_scanning(void) {
    printf("Start scanning for Controller...\n");
    client_state = CLIENT_W4_SCAN_RESULT;
    gap_set_scan_parameters(0, 0x0030, 0x0030);
    gap_start_scan();
}

static void client_connect_to_controller(void) {
    client_state = CLIENT_W4_CONNECT;
    gap_stop_scan();
    printf("Stop scan. Connect to device with addr %s.\n",
           bd_addr_to_str(controller_addr));
    gap_connect(controller_addr, controller_addr_type);
}

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel,
                                     uint8_t* packet, uint16_t size) {
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    uint8_t att_status;

    switch (client_state) {
        case CLIENT_W4_SERVICE_RESULT:
            switch (hci_event_packet_get_type(packet)) {
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    gatt_event_service_query_result_get_service(
                        packet, &controller_service);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    att_status =
                        gatt_event_query_complete_get_att_status(packet);
                    if (att_status != ATT_ERROR_SUCCESS) {
                        printf("SERVICE_QUERY_RESULT, ATT Error 0x%02x.\n",
                               att_status);
                        gap_disconnect(connection_handle);
                        break;
                    }
                    if (controller_service.start_group_handle == 0) {
                        printf(
                            "Controller service not found. Disconnecting.\n");
                        gap_disconnect(connection_handle);
                        break;
                    }
                    printf(
                        "Controller service found (0x%04x-0x%04x). Discovering "
                        "characteristics...\n",
                        controller_service.start_group_handle,
                        controller_service.end_group_handle);
                    thumbstick_characteristic_found = 0;
                    client_state = CLIENT_W4_CHARACTERISTIC_RESULT;
                    gatt_client_discover_characteristics_for_service(
                        handle_gatt_client_event, connection_handle,
                        &controller_service);
                    break;
                default:
                    break;
            }
            break;

        case CLIENT_W4_CHARACTERISTIC_RESULT:
            switch (hci_event_packet_get_type(packet)) {
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT: {
                    gatt_client_characteristic_t characteristic;
                    gatt_event_characteristic_query_result_get_characteristic(
                        packet, &characteristic);
                    if (characteristic.uuid16 == 0 &&
                        memcmp(characteristic.uuid128,
                               thumbstick_characteristic_uuid, 16) == 0) {
                        thumbstick_characteristic       = characteristic;
                        thumbstick_characteristic_found = 1;
                        printf("Thumbstick characteristic matched.\n");
                    }
                    break;
                }
                case GATT_EVENT_QUERY_COMPLETE:
                    att_status =
                        gatt_event_query_complete_get_att_status(packet);
                    if (att_status != ATT_ERROR_SUCCESS) {
                        printf(
                            "CHARACTERISTIC_QUERY_RESULT, ATT Error 0x%02x.\n",
                            att_status);
                        gap_disconnect(connection_handle);
                        break;
                    }
                    if (!thumbstick_characteristic_found) {
                        printf(
                            "Thumbstick characteristic not found in service. "
                            "Disconnecting.\n");
                        gap_disconnect(connection_handle);
                        break;
                    }
                    printf(
                        "Thumbstick characteristic found. Enabling "
                        "notifications...\n");
                    notification_listener_registered = 1;
                    gatt_client_listen_for_characteristic_value_updates(
                        &notification_listener, handle_gatt_client_event,
                        connection_handle, &thumbstick_characteristic);
                    client_state = CLIENT_W4_ENABLE_NOTIFICATIONS_COMPLETE;
                    gatt_client_write_client_characteristic_configuration(
                        handle_gatt_client_event, connection_handle,
                        &thumbstick_characteristic,
                        GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                default:
                    break;
            }
            break;

        case CLIENT_W4_ENABLE_NOTIFICATIONS_COMPLETE:
            switch (hci_event_packet_get_type(packet)) {
                case GATT_EVENT_QUERY_COMPLETE:
                    att_status =
                        gatt_event_query_complete_get_att_status(packet);
                    printf("Notifications enabled, ATT status 0x%02x\n",
                           att_status);
                    if (att_status != ATT_ERROR_SUCCESS) {
                        gap_disconnect(connection_handle);
                        break;
                    }
                    printf("Ready to receive thumbstick updates.\n");
                    client_state = CLIENT_READY;
                    gpio_put(indicator_pins->green, true);
                    gpio_put(indicator_pins->red, false);
                    break;
                default:
                    break;
            }
            break;

        case CLIENT_READY:
            switch (hci_event_packet_get_type(packet)) {
                case GATT_EVENT_NOTIFICATION: {
                    uint16_t value_length =
                        gatt_event_notification_get_value_length(packet);
                    const uint8_t* value =
                        gatt_event_notification_get_value(packet);
                    if (value_length == sizeof(struct thumbstick_state)) {
                        struct thumbstick_state state;
                        memcpy(&state, value, sizeof(state));
                        set_motors_from_joystick_coords(&state);
                    } else {
                        printf(
                            "Received notification with unexpected length %u\n",
                            value_length);
                    }
                    break;
                }
                case GATT_EVENT_QUERY_COMPLETE:
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t* packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    const uint8_t* adv_data;
    uint8_t        adv_len;

    switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                client_start_scanning();
            } else {
                client_state = CLIENT_OFF;
            }
            break;

        case GAP_EVENT_ADVERTISING_REPORT:
            if (client_state != CLIENT_W4_SCAN_RESULT) return;
            adv_data = gap_event_advertising_report_get_data(packet);
            adv_len  = gap_event_advertising_report_get_data_length(packet);
            if (!advertisement_contains_name(controller_name, adv_len,
                                             adv_data))
                return;
            gap_event_advertising_report_get_address(packet, controller_addr);
            controller_addr_type =
                gap_event_advertising_report_get_address_type(packet);
            client_connect_to_controller();
            break;

        case HCI_EVENT_META_GAP:
            switch (hci_event_gap_meta_get_subevent_code(packet)) {
                case GAP_SUBEVENT_LE_CONNECTION_COMPLETE:
                    if (client_state != CLIENT_W4_CONNECT) return;
                    connection_handle =
                        gap_subevent_le_connection_complete_get_connection_handle(
                            packet);
                    printf(
                        "Connected to Controller. Searching for service...\n");
                    client_state = CLIENT_W4_SERVICE_RESULT;
                    gatt_client_discover_primary_services_by_uuid128(
                        handle_gatt_client_event, connection_handle,
                        controller_service_uuid);
                    break;
                default:
                    break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            gpio_put(indicator_pins->green, false);
            gpio_put(indicator_pins->red, true);
            printf("Disconnected from Controller.\n");
            connection_handle = HCI_CON_HANDLE_INVALID;
            if (notification_listener_registered) {
                notification_listener_registered = 0;
                gatt_client_stop_listening_for_characteristic_value_updates(
                    &notification_listener);
            }
            if (client_state == CLIENT_OFF) break;
            client_start_scanning();
            break;

        case SM_EVENT_JUST_WORKS_REQUEST:
            printf("Just works requested\n");
            sm_just_works_confirm(
                sm_event_just_works_request_get_handle(packet));
            break;

        default:
            break;
    }
}

void init_indicators(indicator_pins_t* pins) {
    indicator_pins = pins;
    gpio_init_mask((1 << pins->red) | (1 << pins->green));
    gpio_set_dir_out_masked((1 << pins->red) | (1 << pins->green));
    gpio_put(pins->red, true);
    gpio_put(pins->green, false);
}

const btstack_run_loop_t* bt_client_init(async_context_t*  ctx,
                                         indicator_pins_t* pins) {
    init_indicators(pins);

    printf("setting up run loop...\n");
    const btstack_run_loop_t* runloop =
        btstack_run_loop_async_context_get_instance(ctx);
    if (runloop == NULL) {
        printf("runloop is null!\n");
    }
    printf("got run loop\n");
    btstack_run_loop_init(runloop);
    printf("initiated runloop\n");

    printf("l2cap initiating...\n");
    l2cap_init();

    printf("sm initiating...\n");
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(0);

    printf("gatt client initiating...\n");
    gatt_client_init();

    printf("setting callbacks...\n");
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    printf("turning on HCI...\n");
    hci_power_control(HCI_POWER_ON);

    return runloop;
}
