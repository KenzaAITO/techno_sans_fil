#ifndef APPLICATION_H
#define APPLICATION_H


#define APP_BUTTON1_PRESSED ((uint8_t) 1)
#define APP_BUTTON2_PRESSED ((uint8_t) 2)
#define APP_BUTTON3_PRESSED ((uint8_t) 4)
#define APP_BUTTON4_PRESSED ((uint8_t) 8)
/**
 * @brief Code d'erreurs de l'application
 */
typedef enum
{
	APP_NO_ERROR = 0,
	APP_ERROR_INIT_TIMER,
	APP_ERROR_INIT_BUTTONS,
	APP_ERROR_INIT_SOFTDEVICE_HANDLER,
	APP_ERROR_UNINIT_TIMER,
	APP_ERROR_UNINIT_BUTTONS,
	APP_ERROR_UNINIT_SOFTDEVICE_HANDLER,
	APP_ERROR_ENABLE_SOFTDEVICE_CONFIG,
	APP_ERROR_ENABLE_SOFTDEVICE,
	APP_ERROR_DISABLE_SOFTDEVICE,
	APP_ERROR_SOFTDEVICE_BLE_HANDLER,
	APP_ERROR_SOFTDEVICE_BLE_GAP,
	APP_ERROR_SOFTDEVICE_BLE_ADVERTISING,
	APP_ERROR_SOFTDEVICE_BLE_SCAN,
	APP_ERROR_EVENT,
} app_error_t;
/**
 * @brief Etats de l'application
 */
typedef enum
{
	APP_INITIALISE = 0,
	APP_RUNNING,
	APP_SCANNING,
	APP_DESTROY,
	APP_ENDED,
} app_state_t;
/**
 * @brief Fonction d'initialisation du matériel utilisé par l'application.
 * @return APP_NO_ERROR si aucune erreur, sinon une erreur
 */
app_error_t app_initialise_hardware(void);
/**
 * @brief Fonction de déinitialisation du matériel utilisé par l'application.
 * @return APP_NO_ERROR si aucune erreur, sinon une erreur
 */
app_error_t app_uninitialise_hardware(void);
/**
 * @brief Fonction de mise à jour de l'application.
 * @return APP_NO_ERROR si aucune erreur, sinon une erreur
 */
app_error_t app_update(uint8_t *end);

void app_event_handler(bsp_event_t event);

void app_display_info(void);
