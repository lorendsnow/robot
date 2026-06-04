#include "btstack.h"
#include "pico/printf.h"

#include "bluetooth/client.h"

typedef enum {
    W4_PEER_COD,
    W4_SCAN_COMPLETE,
    W4_SDP_RESULT,
    W2_SEND_SDP_QUERY,
    W4_RFCOMM_CHANNEL,
    SENDING,
    DONE
} state_t;

static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t* packet, uint16_t size);

static uint8_t rfcomm_server_channel;

static btstack_packet_callback_registration_t  hci_event_callback_registration;
static btstack_context_callback_registration_t handle_sdp_client_query_request;

static bd_addr_t peer_addr;
static state_t   state;

// SPP
static uint16_t rfcomm_mtu;
static uint16_t rfcomm_cid = 0;

// Find remote peer by COD
#define INQUIRY_INTERVAL 5

static void start_scan(void) {
    printf("Starting inquiry scan..\n");
    state = W4_PEER_COD;
    gap_inquiry_start(INQUIRY_INTERVAL);
}

static void stop_scan(void) {
    printf("Stopping inquiry scan..\n");
    state = W4_SCAN_COMPLETE;
    gap_inquiry_stop();
}

#define REPORT_INTERVAL_MS 3000
static uint32_t test_data_transferred;
static uint32_t test_data_start;

static void test_reset(void) {
    test_data_start       = btstack_run_loop_get_time_ms();
    test_data_transferred = 0;
}

static void test_track_transferred(int bytes_sent) {
    test_data_transferred += bytes_sent;
    // evaluate
    uint32_t now         = btstack_run_loop_get_time_ms();
    uint32_t time_passed = now - test_data_start;
    if (time_passed < REPORT_INTERVAL_MS) return;
    // print speed
    int bytes_per_second = test_data_transferred * 1000 / time_passed;
    printf("%u bytes -> %u.%03u kB/s\n", (int)test_data_transferred,
           (int)bytes_per_second / 1000, bytes_per_second % 1000);

    // restart
    test_data_start       = now;
    test_data_transferred = 0;
}

static void handle_query_rfcomm_event(uint8_t packet_type, uint16_t channel,
                                      uint8_t* packet, uint16_t size) {
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    switch (hci_event_packet_get_type(packet)) {
        case SDP_EVENT_QUERY_RFCOMM_SERVICE:
            rfcomm_server_channel =
                sdp_event_query_rfcomm_service_get_rfcomm_channel(packet);
            break;
        case SDP_EVENT_QUERY_COMPLETE:
            if (sdp_event_query_complete_get_status(packet)) {
                printf("SDP query failed, status 0x%02x\n",
                       sdp_event_query_complete_get_status(packet));
                break;
            }
            if (rfcomm_server_channel == 0) {
                printf("No SPP service found\n");
                break;
            }
            printf("SDP query done, channel 0x%02x.\n", rfcomm_server_channel);
            rfcomm_create_channel(packet_handler, peer_addr,
                                  rfcomm_server_channel, NULL);
            break;
        default:
            break;
    }
}

static void handle_start_sdp_client_query(void* context) {
    UNUSED(context);
    if (state != W2_SEND_SDP_QUERY) return;
    state = W4_RFCOMM_CHANNEL;
    sdp_client_query_rfcomm_channel_and_name_for_uuid(
        &handle_query_rfcomm_event, peer_addr,
        BLUETOOTH_SERVICE_CLASS_SERIAL_PORT);
}

static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t* packet, uint16_t size) {
    UNUSED(channel);

    bd_addr_t event_addr;
    uint8_t   status;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            printf("handling HCI event packet...\n");
            switch (hci_event_packet_get_type(packet)) {
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) !=
                        HCI_STATE_WORKING) {
                        printf("HCI State not working...\n");
                        return;
                    }

                    printf("starting scan...\n");
                    start_scan();
                    printf("done scanning\n");
                    break;

                case GAP_EVENT_INQUIRY_RESULT: {
                    printf("got an inquiry result event\n");
                    if (state != W4_PEER_COD) break;
                    uint8_t len = gap_event_inquiry_result_get_name_len(packet);
                    uint8_t device_name[len + 1];
                    for (uint8_t i = 0; i < len; i++) {
                        device_name[i] =
                            gap_event_inquiry_result_get_name(packet)[i];
                    }
                    device_name[len] = 0;

                    gap_event_inquiry_result_get_bd_addr(packet, event_addr);
                    printf("Device found: %s with name %s\n",
                           bd_addr_to_str(event_addr), device_name);
                    break;
                }

                case GAP_EVENT_INQUIRY_COMPLETE:
                    printf("got inquiry complete event\n");
                    switch (state) {
                        case W4_PEER_COD:
                            printf("Inquiry complete\n");
                            printf("Peer not found, starting scan again\n");
                            start_scan();
                            break;
                        case W4_SCAN_COMPLETE:
                            printf(
                                "Start to connect and query for SPP service\n");
                            state = W2_SEND_SDP_QUERY;
                            handle_sdp_client_query_request.callback =
                                &handle_start_sdp_client_query;
                            (void)sdp_client_register_query_callback(
                                &handle_sdp_client_query_request);
                            break;
                        default:
                            printf("got non-recognized state %d\n", state);
                            break;
                    }
                    if (state == W4_PEER_COD) {
                    }
                    break;

                case HCI_EVENT_PIN_CODE_REQUEST:
                    // inform about legacy pairing with pin code - should only
                    // happen before Core v2.1
                    printf(
                        "Pin code request for Legacy Pairing received -> abort "
                        "pairing'\n");
                    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                    gap_pin_code_negative(event_addr);
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // inform about user confirmation request
                    printf(
                        "Accepting Pairing - TODO: require actual user "
                        "action\n");
                    hci_event_user_confirmation_request_get_bd_addr(packet,
                                                                    event_addr);
                    gap_ssp_confirmation_response(event_addr);
                    break;

                case RFCOMM_EVENT_CHANNEL_OPENED:
                    printf("got an RFCOMM channel opened event\n");
                    status = rfcomm_event_channel_opened_get_status(packet);
                    switch (status) {
                        case ERROR_CODE_SUCCESS:
                            rfcomm_cid =
                                rfcomm_event_channel_opened_get_rfcomm_cid(
                                    packet);
                            rfcomm_mtu =
                                rfcomm_event_channel_opened_get_max_frame_size(
                                    packet);
                            printf(
                                "RFCOMM channel open succeeded. New RFCOMM "
                                "Channel ID 0x%02x, max frame size %u\n",
                                rfcomm_cid, rfcomm_mtu);
                            test_reset();

                            // disable page/inquiry scan to get max performance
                            gap_discoverable_control(0);
                            gap_connectable_control(0);
                            break;
                        default:
                            printf(
                                "RFCOMM channel open failed, status 0x%02x\n",
                                status);
                            break;
                    }
                    break;

                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    printf("RFCOMM channel closed\n");
                    rfcomm_cid = 0;

                    // re-enable page/inquiry scan again
                    gap_discoverable_control(1);
                    gap_connectable_control(1);
                    break;

                default:
                    break;
            }
            break;

        case RFCOMM_DATA_PACKET:
            test_track_transferred(size);
            break;

        default:
            break;
    }
}

void bt_client_init(void) {
    printf("initing rfcomm...\n");
    rfcomm_init();
    printf("rfcomm inited!\n");

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    printf("packet handler activated\n");

    // init SDP
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
    printf("inited SDP\n");

    // turn on!
    hci_power_control(HCI_POWER_ON);
    printf("powered on HCI\n");
}