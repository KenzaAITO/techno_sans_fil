/*
 ============================================================================
 Name        : application.c
 Author      : Polytech
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include "application.h"
#include "encryption.h"

#include "app_timer.h"
#include "app_button.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "ble_lbs.h"
#include "boards.h"
#include "nrf_sdm.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include <string.h>

static app_state_t app_state = APP_INITIALISE;
static uint8_t app_buttons = 0;

/**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define CENTRAL_LINK_COUNT              0
/**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1
/**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_PRESCALER             0
/**< Size of timer operation queues. */
#define APP_TIMER_OP_QUEUE_SIZE         4
/**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_INTERVAL                   0x00A0
/**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_WINDOW                     0x0050
/**< Timeout when scanning. 0x0000 disables timeout. */
#define SCAN_TIMEOUT                    0x0000
/**< Active scanning is not set. */
#define SCAN_REQUEST                    0
/**< We will not ignore unknown devices. */
#define SCAN_WHITELIST_ONLY             0
/**< The SoftDevice BLE configuration id. */
#define APP_BLE_CONN_CFG_TAG            1
/**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_OBSERVER_PRIO           3

/**< The Bluetooth scan parameters */
static const ble_gap_scan_params_t app_scan_param =
{
    SCAN_REQUEST,
    SCAN_WHITELIST_ONLY,
    NULL,
    (uint16_t)SCAN_INTERVAL,
    (uint16_t)SCAN_WINDOW,
    (uint16_t)SCAN_TIMEOUT
};

const uint8_t ble_adv_data[] = {0x0A, 0x09, 'P','O','L','Y','T','E','C','H','x',     /* Complete local name */
                                0x02, 0x01, BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED,   /* Flags */
                                0x03, 0x19, 0xc8, 0x03,                              /* Appearance */
                                0x03, 0x03, 0x1c, 0x18};                             /* Complete list of 16 bits UUID */
static ble_gap_adv_params_t ble_adv_params = {0};

static uint8_t MESSAGE[] = "CE MESSAGE NE DOIT PAS ETRE VISIBLE";
static uint8_t KEY[] = {0x0A, 0x1B, 0xFE, 0xDE, 0xFD, 0xAB, 0xB1, 0x04};

static uint8_t ble_adv[31];
static uint8_t ble_adv_length;
static uint8_t ble_mess_count;

// Initialise the log system used by the drivers to report errors and warnings
app_error_t app_initialise_hardware(void)
{
    app_error_t ret = APP_NO_ERROR;
    ret_code_t errcode;

    errcode = NRF_LOG_INIT(NULL);
    if(errcode != NRF_SUCCESS)
    {
        printf("Error returned by NRF_LOG_INIT: %lu\n", errcode);
        ret = APP_ERROR_INITIALISE_LOG;
        goto end;
    }
    else
    {
        // And the backend log system to use (in our case it is RTT)
        NRF_LOG_DEFAULT_BACKENDS_INIT();
    }

    // Initialise the timer
    errcode = app_timer_init();
    if(errcode != NRF_SUCCESS)
    {
        printf("Error returned by app_timer_init: %lu\n", errcode);
        ret = APP_ERROR_INITIALISE_TIMER;
        goto end;
    }

    // Install the system callback to get notified on button changes and reset the LEDs
    bsp_event_t event;
    errcode = bsp_init(BSP_INIT_BUTTONS, app_event_handler);
    if(errcode != NRF_SUCCESS)
    {
        printf("Error returned by bsp_init: %lu\n", errcode);
        ret = APP_ERROR_INITIALISE_BUTTONS;
        goto end;
    }

    // Configure LED-pins as outputs and turn them off
    LEDS_CONFIGURE(LEDS_MASK);
    LEDS_OFF(LEDS_MASK);

end:
    return ret;
}

app_error_t app_uninitialise_hardware(void)
{
    app_error_t ret = APP_NO_ERROR;

    // Turn all LEDs off
    LEDS_OFF(LEDS_MASK);

    return ret;
}

app_error_t app_update(uint8_t *end)
{
    app_error_t ret = APP_NO_ERROR;

    //Machine d'etats
    switch(app_state)
    {
        case APP_INITIALISE:
            // Initialise the hardware required by the application
            ret = app_initialise_hardware();
            if(ret != APP_NO_ERROR)
            {
                printf("Error returned by app_initialise_hardware(): %d\n", ret);
            }
            printf("Hardware initialised\n");
            ble_mess_count = 0;
            app_state = APP_RUNNING;
            break;

        case APP_DESTROY:
            // Uninitialise the hardware required by the application
            ret = app_uninitialise_hardware();
            if(ret != APP_NO_ERROR)
            {
                printf("Error returned by app_uninitialise_hardware(): %d\n", ret);
            }
            printf("Hardware uninitialised\n");
            app_state = APP_ENDED;
            if(end != NULL) *end = 1;
            break;

        case APP_RUNNING:
            // Run the application in running state
            break;

        case APP_SCANNING:
            // Run the application in scanning state
            break;

        default:
            // Handle unexpected states
            break;
    };

    return ret;
}

void app_event_handler(bsp_event_t event)
{
    switch(event)
    {
        case BSP_EVENT_KEY_0:
            // Button 1 pressed
            printf("BUTTON-1 PRESSED\n");
            app_buttons |= APP_BUTTON1_PRESSED;
            break;

        default:
            // Handle unmanaged events
            printf("Unmanaged event in app_event_handler: %d\n", event);
            break;
    };
}

void app_display_info(void)
{
    ble_gap_addr_t addr;
    uint32_t err;
    uint8_t i;

    // Retrieve the BLE address
    err = sd_ble_gap_addr_get(&addr);
    if(err != NRF_SUCCESS)
    {
        printf("Error returned by sd_ble_gap_address_get(): %lu\n", err);
    }
    else
    {
        // Display the local BLE address
        printf("Local BLE address is ");
        for(i = sizeof(addr.addr) / sizeof(addr.addr[0]); i > 0; i--)
        {
            printf((i > 1) ? "%.2x:" : "%.2x", addr.addr[i - 1]);
        }
        printf("\n");
    }
}
