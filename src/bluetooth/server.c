#include "btstack.h"

#include "bluetooth/server.h"

#define RFCOMM_SERVER_CHANNEL 1

static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t* packet, uint16_t size);

static uint16_t                               rfcomm_channel_id;
static uint8_t                                spp_service_buffer[150];
static btstack_packet_callback_registration_t hci_event_callback_registration;
static char                                   lineBuffer[30];

void bt_server_init(void) {
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    l2cap_init();

    rfcomm_init();
    rfcomm_register_service(packet_handler, RFCOMM_SERVER_CHANNEL, 0xffff);

    sdp_init();
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));

    spp_create_sdp_record(spp_service_buffer,
                          sdp_create_service_record_handle(),
                          RFCOMM_SERVER_CHANNEL, "Robot");

    btstack_assert(de_get_len(spp_service_buffer) <=
                   sizeof(spp_service_buffer));

    sdp_register_service(spp_service_buffer);

    gap_set_local_name("Robot");

    hci_power_control(HCI_POWER_ON);
}

static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t* packet, uint16_t size) {
    UNUSED(channel);

    bd_addr_t event_addr;
    uint8_t   rfcomm_channel_nr;
    uint16_t  mtu;
    int       i;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) ==
                        HCI_STATE_WORKING) {
                        gap_discoverable_control(1);
                        gap_connectable_control(1);
                    }
                    break;

                case RFCOMM_EVENT_INCOMING_CONNECTION:
                    rfcomm_event_incoming_connection_get_bd_addr(packet,
                                                                 event_addr);
                    rfcomm_channel_nr =
                        rfcomm_event_incoming_connection_get_server_channel(
                            packet);
                    rfcomm_channel_id =
                        rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
                    printf("RFCOMM channel %u requested for %s\n",
                           rfcomm_channel_nr, bd_addr_to_str(event_addr));
                    rfcomm_accept_connection(rfcomm_channel_id);
                    break;

                case RFCOMM_EVENT_CHANNEL_OPENED:
                    if (rfcomm_event_channel_opened_get_status(packet)) {
                        printf("RFCOMM channel open failed, status 0x%02x\n",
                               rfcomm_event_channel_opened_get_status(packet));
                    } else {
                        rfcomm_channel_id =
                            rfcomm_event_channel_opened_get_rfcomm_cid(packet);
                        mtu = rfcomm_event_channel_opened_get_max_frame_size(
                            packet);
                        printf(
                            "RFCOMM channel open succeeded. New RFCOMM Channel "
                            "ID %u, max frame size %u\n",
                            rfcomm_channel_id, mtu);
                    }
                    break;
                case RFCOMM_EVENT_CAN_SEND_NOW:
                    rfcomm_send(rfcomm_channel_id, (uint8_t*)lineBuffer,
                                (uint16_t)strlen(lineBuffer));
                    break;

                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    printf("RFCOMM channel closed\n");
                    rfcomm_channel_id = 0;
                    break;

                default:
                    break;
            }
            break;

        case RFCOMM_DATA_PACKET:
            printf("RCV: '");
            for (i = 0; i < size; i++) {
                putchar(packet[i]);
            }
            printf("'\n");
            break;

        default:
            break;
    }
}