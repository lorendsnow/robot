#include "btstack.h"
#include "pico/btstack_run_loop_async_context.h"
#include "pico/printf.h"
#include "pico/sync.h"

#include "thumbstick.h"
#include "bluetooth/ble_server.h"
#include "controller_server.h"

#define APP_AD_FLAGS 0x06

static struct thumbstick_state state;

static const uint8_t adv_data[] = {
    /* Flags general discoverable */
    0x02,
    BLUETOOTH_DATA_TYPE_FLAGS,
    APP_AD_FLAGS,
    /* Name */
    0x0b,
    BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'C',
    'o',
    'n',
    't',
    'r',
    'o',
    'l',
    'l',
    'e',
    'r',
    /* Incomplete list of 16-bit service class UUIDs */
    0x03,
    BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
    0x10,
    0xff,
};
static const uint8_t adv_data_len = sizeof(adv_data);

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;
static hci_con_handle_t                       con_handle;

static uint16_t le_notification_enabled = 0;

// NOLINTBEGIN(*-easily-swappable-parameters)
static uint16_t att_read_callback(hci_con_handle_t connection_handle,
                                  uint16_t att_handle, uint16_t offset,
                                  uint8_t* buffer, uint16_t buffer_size) {
    // NOLINTEND(*-easily-swappable-parameters)
    UNUSED(connection_handle);

    if (att_handle ==
        ATT_CHARACTERISTIC_0f9e4129_0220_4669_82dc_79b378fa1dff_01_VALUE_HANDLE) {
        return att_read_callback_handle_blob(
            (const uint8_t*)&state, sizeof(state), offset, buffer, buffer_size);
    }

    return 0;
}

// NOLINTBEGIN(*-easily-swappable-parameters)
static int att_write_callback(hci_con_handle_t connection_handle,
                              uint16_t att_handle, uint16_t transaction_mode,
                              uint16_t offset, uint8_t* buffer,
                              uint16_t buffer_size) {
    // NOLINTEND(*-easily-swappable-parameters)
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

    switch (att_handle) {
        case ATT_CHARACTERISTIC_0f9e4129_0220_4669_82dc_79b378fa1dff_01_CLIENT_CONFIGURATION_HANDLE:
            le_notification_enabled =
                little_endian_read_16(buffer, 0) ==
                GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
            con_handle = connection_handle;
            if (le_notification_enabled) {
                att_server_request_can_send_now_event(con_handle);
            }
            break;
        default:
            break;
    }

    return 0;
}

// NOLINTBEGIN(*-easily-swappable-parameters)
static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t* packet, uint16_t size) {
    // NOLINTEND(*-easily-swappable-parameters)
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            printf("Just works requested\n");
            sm_just_works_confirm(
                sm_event_just_works_request_get_handle(packet));
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_notification_enabled = 0;
            break;
        case ATT_EVENT_CAN_SEND_NOW:
            thumbstick_read(&state);
            printf("read state - x=%d; y=%d\n", state.x, state.y);
            att_server_notify(
                con_handle,
                ATT_CHARACTERISTIC_0f9e4129_0220_4669_82dc_79b378fa1dff_01_VALUE_HANDLE,
                (uint8_t*)&state, sizeof(state));
            if (le_notification_enabled) {
                att_server_request_can_send_now_event(con_handle);
            }
            break;
        default:
            break;
    }
}

void bt_server_init(async_context_t* ctx) {
    state.x = 0;
    state.y = 0;

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

    printf("att server initiating...\n");
    att_server_init(profile_data, att_read_callback, att_write_callback);

    uint16_t  adv_int_min = 0x00FF;
    uint16_t  adv_int_max = 0x00FF;
    uint8_t   adv_type    = 0;
    bd_addr_t null_addr;
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0,
                                  null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*)adv_data);
    gap_advertisements_enable(1);

    printf("setting callbacks...\n");
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    att_server_register_packet_handler(packet_handler);

    printf("turning on HCI...\n");
    hci_power_control(HCI_POWER_ON);
}