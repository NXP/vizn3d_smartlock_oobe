/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief  display over usb_cdc implementation.
 */

#include "app_config.h"
#include <FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "fsl_component_serial_manager.h"
#include "fsl_component_serial_port_usb.h"

#include "fsl_shell.h"

#include "fwk_input_manager.h"
#include "fwk_common.h"
#include "fwk_log.h"
#include "hal_event_descriptor_face_rec.h"
#include "hal_input_dev.h"
#include "hal_lpm_dev.h"

#include "sln_flash_config.h"
#include "fica_definition.h"
#include "hal_vision_algo.h"
#include "sln_device_utils.h"
#include "wm_net.h"

#include "face_rec_rt_info.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SHELL_PROMPT "SHELL>> "

#define configLOGGING_MAX_MESSAGE_LENGTH 128
#define LOG_ERROR_PRINT "Error: failed to print a message\r\n"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t _VersionCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _ResetCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _SaveCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _AddCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _DelCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _DisplayOutputCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _WhitePwmCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _IrPwmCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _SpeakerVolumeCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _LpmTriggerCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _ConnectivityCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _WiFiCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _BleAddressCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);

static shell_status_t _GetManagerCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _ListCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _RenameCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _VerboseCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _RecordCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _RtInfoCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _OasisCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);
static shell_status_t _FaceRecThresholdCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv);

static int _FrameworkEventsHandler(framework_events_t eventId,
                                   framework_response_t *response,
                                   unsigned char isFinished);
void APP_InputDev_Shell_RegisterShellCommands(shell_handle_t shellContextHandle,
                                              input_dev_t *shellDev,
                                              input_dev_callback_t callback);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static SHELL_COMMAND_DEFINE(version,
                     (char *)"\r\n\"version oasis \": get the version of the current oasis library\r\n"
                    		 "\"version\": get the version of the current software.\r\n",
                     _VersionCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(reset,
                            (char *)"\r\n\"reset\": resets the board.\r\n",
                            _ResetCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(save,
                            (char *)"\r\n\"save\": Save all registered users to flash\r\n",
                            _SaveCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(add,
                            (char *)"\r\n\"add username\": Add user.\r\n",
                            _AddCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(del, (char *)"\r\n\"del -n <username>\": Del user by username.\r\n"
        "\"del -i <id>\": Del user by id.\r\n"
        "\"del -a \": Del all.\r\n", _DelCommand, SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(display_output,
                     (char *)"\r\n\"display_output <UVC|panel> \": Set display device.\r\n"
                    		 "\"display_output\": Get the display device.\r\n"
                    		 "\"display_output source <RGB|3DIR|3DDEPTH> \": Set display source.\r\n"
                    		 "\"display_output source\": Get display source.\r\n" ,
                     _DisplayOutputCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(
    ir_pwm,
    (char *)"\r\n\"ir_pwm <value>\": PWM pulse width for IR LED, value should "
            "be between 0 (inactive) and 100 (max).\r\n",
    _IrPwmCommand, SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(
    white_pwm,
    (char *)"\r\n\"white_pwm <value>\": PWM pulse width for white LED, value "
            "should be between 0 (inactive) and 100 (max).\r\n",
    _WhitePwmCommand, SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(
    volume,
    (char *)"\r\n\"volume <value>\": Volume of the speaker. Value should be between "
            "0 (muted) and 100 (max).\r\n",
    _SpeakerVolumeCommand, SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(
    lpm,
    (char *)"\r\n\"lpm <enable/disable>\": Enable or disable low power functionality based on timeout"
            "or disable.\r\n"
            "\"lpm\": Return the current low status <enable | disable>\r\n",
    _LpmTriggerCommand, SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(connectivity,
                            (char *)"\r\n\"connectivity type <wifi/ble/none>\": Select type off connectivity .\r\n",
                            _ConnectivityCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(ble,
                            (char *)"\r\n\"ble address\": get ble advertising address.\r\n",
                            _BleAddressCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);

static SHELL_COMMAND_DEFINE(wifi,
                            (char *)"\r\n\"wifi ssid <SSID>\": Set the SSID\r\n"
                                    "\"wifi password <Password>\": Set the Password\r\n"
                                    "\"wifi ip \": Get the IP\r\n"
                                    "\"wifi credentials\": get the current WiFi credentials saved in flash.\r\n"
                                    "\"wifi credentials erase\": Remove the current WiFi credentials. After erase the WiFi will disconnected from the network.\r\n"
                                    "\"wifi state <on/off>\": Turn on and off the WiFi.\r\n"
                                    "\"wifi state\": Get current state of the WiFi.\r\n"
                                    "\"wifi reset\": Reset the WiFi connection.\r\n"
                            		"\"wifi scan\": Start the scan process. This will return a json list with <SSID><signal><channel> after the scan is completed.\r\n"
                                    "\"wifi ftp_server <IP> <PORT>\": Set FTP server IP and port.\r\n"
                                    "\"wifi ftp_server \": Set FTP server IP and port.\r\n"
                            ,
                            _WiFiCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(
    list,
    (char *)"\r\n\"list\": get all users registered.\r\n"
    		"\"list -c\": get number of registered users.\r\n",
    _ListCommand, SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(rename,
                            (char *)"\r\n\"rename <id> new_name\": rename user based on id .\r\n",
                            _RenameCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(log_level,
                     (char *)"\r\n\"log_level <none|error|debug|info|verbose>\": set the log level.\r\n"
                    		 "\"log_level\": get the log level.\r\n",
                     _VerboseCommand,
                     SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(get_manager, (char *)"\r\n\"get_manager\": get list of all active managers.\r\n"
								"\"get_manager <id>\": get devices registered to a specific manager\r\n", _GetManagerCommand, SHELL_IGNORE_PARAMETER_COUNT);

static SHELL_COMMAND_DEFINE(record,
                            (char *)"\r\n\"record <start|stop>\": start/stop the record.\r\n"
                            "\"record info\": get recording info\r\n",
                            _RecordCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);
static SHELL_COMMAND_DEFINE(oasis,
                            (char *)"\r\n\"oasis <start|stop>\": start/stop oasis\r\n"
                            "\"oasis info\": get the state of oasis.\r\n",
                            _OasisCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);

static SHELL_COMMAND_DEFINE(rtinfo,
                            (char *)"\r\n\"rtinfo\": runtime information filter\r\n",
                            _RtInfoCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);

static SHELL_COMMAND_DEFINE(facerec_threshold,
                            (char *)"\r\n\"facerec_threshold\": show the current face recognition threshold\r\n"
                            "\"facerec_threshold <value>\": set the face recognition threshold."
                            " Note: The board will reset to make the updated threshold take effect.\r\n",
                            _FaceRecThresholdCommand,
                            SHELL_IGNORE_PARAMETER_COUNT);

static event_common_t s_CommonEvent;
static event_face_rec_t s_FaceRecEvent;
static event_recording_t s_RecordingEvent;
static input_event_t s_InputEvent;
static framework_request_t s_FrameworkRequest;
static input_dev_callback_t s_InputCallback;
static input_dev_t *s_SourceShell; /* Shell device that commands are sent over */
static shell_handle_t s_ShellHandle;
AT_NONCACHEABLE_SECTION_ALIGN_DTC(static char s_logbuf[configLOGGING_MAX_MESSAGE_LENGTH], 8);
/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_InputDev_Shell_RegisterShellCommands(shell_handle_t shellContextHandle,
                                              input_dev_t *shellDev,
                                              input_dev_callback_t callback)
{
    s_InputCallback            = callback;
    s_SourceShell              = shellDev;
    s_ShellHandle              = shellContextHandle;
    s_FrameworkRequest.respond = _FrameworkEventsHandler;
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(reset));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(save));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(add));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(del));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(rename));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(list));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(log_level));
#if !defined(HEADLESS_ENABLE) || (!HEADLESS_ENABLE)
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(display_output));
#endif
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(ir_pwm));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(white_pwm));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(volume));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(lpm));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(connectivity));
#if ENABLE_WIFI
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(wifi));
#endif
#if ENABLE_QN9090
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(ble));
#endif
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(get_manager));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(record));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(rtinfo));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(oasis));
    SHELL_RegisterCommand(shellContextHandle, SHELL_COMMAND(facerec_threshold));
}

#define PRINT_DEVICE_CONFIG_TABLE_ENTRY(DEV_ID, DEV_NAME, CONFIG_NAME, CONFIG_CUR_VAL, CONFIG_EXPECTED_VALS,        \
                                        CONFIG_DESCRIPTION)                                                         \
    SHELL_Printf(s_ShellHandle, "%-*s%-*s%-*s%-*s%-*s%-*s\r\n", 16, DEV_ID, DEVICE_NAME_MAX_LENGTH, DEV_NAME,       \
                 DEVICE_CONFIG_NAME_MAX_LENGTH, CONFIG_NAME, 32, CONFIG_CUR_VAL,                                    \
                 DEVICE_CONFIG_EXPECTED_VAL_MAX_LENGTH, CONFIG_EXPECTED_VALS, DEVICE_CONFIG_DESCRIPTION_MAX_LENGTH, \
                 CONFIG_DESCRIPTION)

/*!
 * @brief prints the device's name and all of its associated configs + values in as a table entry
 * @param devId id of the device whose configs are being printed
 * @param deviceName name of the device whose configs are being printed
 * @param configs pointer to the array of configs for the given device
 * @param printTableHeader whether to print the table's header b/c this is the first entry to the table
 */

int LOG_SHELL_Printf(const char *formatString, ...)
{
    va_list ap;
    int status;

    va_start(ap, formatString);
    status = vsnprintf(s_logbuf, configLOGGING_MAX_MESSAGE_LENGTH, formatString, ap);
    va_end(ap);

    if (s_ShellHandle)
    {
        if (status >= 0)
        {
            SHELL_Write(s_ShellHandle, s_logbuf, strlen(s_logbuf));
        }
        else
        {
            SHELL_Write(s_ShellHandle, LOG_ERROR_PRINT, strlen(LOG_ERROR_PRINT));
        }
    }

    return 0;
}

static void _PrintDeviceConfigTable(uint32_t devId, char *deviceName, hal_device_config *configs, bool printTableHeader)
{
    char devIdString[5]; // TODO: Use macro(?) Arbitrary size. devId shouldn't be larger than about 20, so 5
                         // should be sufficient
    char configVal[DEVICE_CONFIG_EXPECTED_VAL_MAX_LENGTH];

    if (configs == NULL)
    {
        return;
    }

    /* If first device, print table header */
    if (printTableHeader)
    {
        SHELL_Printf(s_ShellHandle, "\r\n");
        PRINT_DEVICE_CONFIG_TABLE_ENTRY("Device ID", "Device Name", "Config Name", "Config Cur Value",
                                        "Config Expected Values", "Config Description");
    }

    if (deviceName != NULL)
    {
        /* Print device name */
        PRINT_DEVICE_CONFIG_TABLE_ENTRY(itoa(devId, devIdString, 10), deviceName, "", "", "", "");
    }
    else
    {
        /* Print device name */
        PRINT_DEVICE_CONFIG_TABLE_ENTRY(itoa(devId, devIdString, 10), "N/A", "", "", "", "");
    }

    /* If device does not have any runtime configs */
    if (strcmp(configs[0].name, "") == 0)
    {
        PRINT_DEVICE_CONFIG_TABLE_ENTRY("", "", "N/A", "N/A", "N/A", "N/A");
        return;
    }

    /* Print each runtime "dynamic" config associated with device and their related info */
    for (int i = 0; i < MAXIMUM_CONFIGS_PER_DEVICE; i++)
    {
        /* If config name is blank, we have reached end of valid configs */
        if (strcmp(configs[i].name, "") == 0)
        {
            return;
        }
        configs[i].get(configVal);
        PRINT_DEVICE_CONFIG_TABLE_ENTRY("", "", configs[i].name, configVal, configs[i].expectedValue,
                                        configs[i].description);
    }
}

static int _HalEventsHandler(uint32_t event_id, void *response, event_status_t status, unsigned char isFinished)
{
    if (response == NULL)
    {
        return -1;
    }

    switch (event_id)
    {
        case kEventFaceRecID_DelUserAll:
        case kEventFaceRecID_DelUser:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDelete was successful");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nDelete ended with an error");
            }
        }
        break;
        case kEventID_GetLogLevel:
        {
            log_level_event_t logLevel = *(log_level_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nCurrent log_level is %d", logLevel.logLevel);
            }
        }
        break;
        case kEventID_SetLogLevel:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nLog_level set");
            }
            else if (status == kEventStatus_Error)
            {
                SHELL_Printf(s_ShellHandle, "\r\nLog_level set failed");
            }
        }
        break;
        case kEventID_GetSpeakerVolume:
        {
            speaker_volume_event_t volume = *(speaker_volume_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nCurrent Volume is %d", volume.volume);
            }
        }
        break;
        case kEventID_SetSpeakerVolume:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nVolume set");
            }
            else if (status == kEventStatus_Error)
            {
                SHELL_Printf(s_ShellHandle, "\r\nVolume set failed");
            }
        }
        break;
        case kEventFaceRecID_SaveUserList:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nUser list saved");
            }
            else if (status == kEventStatus_Error)
            {
                SHELL_Printf(s_ShellHandle, "\r\nUser list saved failed");
            }
        }
        break;
        case kEventFaceRecID_GetUserList:
        {
            user_info_event_t usersInfo = *(user_info_event_t *)response;
            for (int index = 0; index < usersInfo.count; index++)
            {
                char savedStatus[10];
                face_user_info_t userInfo = usersInfo.userInfo[index];
                strcpy(savedStatus, userInfo.isSaved ? "Saved" : "Not saved");
                SHELL_Printf(s_ShellHandle, "\r\n%-10s - Id %3d \tName - %s", savedStatus, userInfo.id, userInfo.name);
            }
        }
        break;
        case kEventFaceRecID_GetUserCount:
        {
            uint32_t count = *(uint32_t *)response;
            SHELL_Printf(s_ShellHandle, "\r\nUsers registered %d", count);
        }
        break;

        case kEventID_GetIRLedBrightness:
        {
            ir_led_event_t brightness = *(ir_led_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nIR LED Brightness is currently set to: %d", brightness.brightness);
            }
        }
        break;

        case kEventID_GetWhiteLedBrightness:
        {
            white_led_event_t brightness = *(white_led_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWhite LED Brightness is currently set to: %d", brightness.brightness);
            }
        }
        break;
        case kEventID_SetIRLedBrightness:
        case kEventID_SetWhiteLedBrightness:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nBrightness set");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nBrightness set failed");
            }
        }
        break;
        case kEventID_GetBLEConnection:
        {
            ble_address_t bleAddress = *(ble_address_t *)response;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nBLE SSID [%s]", bleAddress.ssid);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nBLE SSID not set");
            }
        }
        break;
        case kEventID_SetDisplayOutputSource:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output source set");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output source set failed.");
            }
        }
        break;
        case kEventID_GetDisplayOutputSource:
        {
            display_output_event_t display = *(display_output_event_t *)response;
            if (display.displayOutputSource == kPixelFormat_UYVY1P422_RGB)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output source is RGB");
            }
            else if (display.displayOutputSource == kPixelFormat_UYVY1P422_Gray)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output source is 2DIR");
            }
            else if (display.displayOutputSource == kPixelFormat_Gray16)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output source is 3DIR");
            }
            else if (display.displayOutputSource == kPixelFormat_Depth16)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output source is 3DDEPTH");
            }
        }
        break;
        case kEventID_GetConnectivityType:
        {
            connectivity_event_t connectivity = *(connectivity_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                char connectivityTypeText[10];
                if (connectivity.connectivityType == kConnectivityType_BLE)
                {
                    strcpy(connectivityTypeText, "BLE");
                }
                else if (connectivity.connectivityType == kConnectivityType_WiFi)
                {
                    strcpy(connectivityTypeText, "WiFi");
                }
                else
                {
                    strcpy(connectivityTypeText, "None");
                }
                SHELL_Printf(s_ShellHandle, "\r\nConnectivity type is currently set to \"%s\".", connectivityTypeText);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get current connectivity type.");
            }
        }
        break;
        case kEventID_SetConnectivityType:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nConnectivity type set. Reset..");
                vTaskDelay(1);
                __NVIC_SystemReset();
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nConnectivity type set failed.");
            }
        }
        break;
        case kEventID_WiFiEraseCredentials:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi Credentials erased.");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi Credentials erased failed.");
            }
        }
        break;
        case kEventID_WiFiSetCredentials:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi Credentials set with success.");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi Credentials set failed.");
            }
        }
        break;
        case kEventID_WiFiGetCredentials:
        {
            wifi_cred_t wifiCred = ((wifi_event_t *)response)->wifiCred;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi Credentials SSID %s PASS %s.", wifiCred.ssid.value,
                             wifiCred.password.value);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get WiFi credentials.");
            }
        }
        break;
        case kEventID_WiFiSetState:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi state set with success");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to set WiFi state.");
            }
        }
        break;
        case kEventID_WiFiGetState:
        {
            wifi_state_t wifiState = ((wifi_event_t *)response)->state;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi state is %s.", (wifiState == kWiFi_On) ? "On" : "Off");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get WiFi state.");
            }
        }
        break;
        case kEventID_WiFiGetIP:
        {
            char *ip = ((wifi_event_t *)response)->ip;
            if ((status == kEventStatus_Ok) && ip != NULL)
            {
                SHELL_Printf(s_ShellHandle, "\r\nWiFi IP is %s", ip);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get WiFi ip.");
            }
        }
        break;
        case kEventID_WiFiScan:
        {
            if (status == kEventStatus_NonBlocking)
            {
                SHELL_Printf(s_ShellHandle, "\r\n WiFi start scanning");
            }
            else if (status == kEventStatus_Error)
            {
                SHELL_Printf(s_ShellHandle, "\r\n WiFi failed to start scanning");
            }
            else if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\n SSID json list %s", (char *)response);
            }
        }
        break;
        case kEventID_FTPSetServerInfo:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nFTP info set with success");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to set FTP info.");
            }
        }
        break;
        case kEventID_FTPGetServerInfo:
        {
            char *server_info = ((wifi_event_t *)response)->ftpServerInfoSerialized;
            if ((status == kEventStatus_Ok) && server_info != NULL)
            {
                SHELL_Printf(s_ShellHandle, "\r\nFTP server info is %s", server_info);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get FTP server info.");
            }
        }
        break;
        case kEventID_FTPSetServerIP:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nFTP ip address set with success");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to set FTP ip address.");
            }
        }
        break;
        case kEventID_FTPGetServerIP:
        {
            char *ip = ((wifi_event_t *)response)->ip;
            if ((status == kEventStatus_Ok) && ip != NULL)
            {
                SHELL_Printf(s_ShellHandle, "\r\nFTP server IP is %s", ip);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get FTP server ip.");
            }
        }
        break;
        case kEventID_FTPSetServerPort:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nFTP server port set with success");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to set FTP server port.");
            }
        }
        break;
        case kEventID_FTPGetServerPort:
        {
            uint16_t port = ((wifi_event_t *)response)->ftpServerInfo.serverPort;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nFTP server port is %d", port);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get FTP server port.");
            }
        }
        break;
        case kEventID_GetDisplayOutput:
        {
            display_output_event_t display = *(display_output_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                char displayText[10];
                if (display.displayOutput == kDisplayOutput_Panel)
                {
                    strcpy(displayText, "PANEL");
                }
                else if (display.displayOutput == kDisplayOutput_UVC)
                {
                    strcpy(displayText, "UVC");
                }
                else
                {
                    strcpy(displayText, "None");
                }
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output is currently set to \"%s\".", displayText);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFailed to get current display output.");
            }
        }
        break;
        case kEventID_SetDisplayOutput:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output set. Reset the board for the change to take effect");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nDisplay output set failed.");
            }
        }
        break;
        case kEventFaceRecID_UpdateUserInfo:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nUpdate was successful.");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nUpdate failed.");
            }
        }
        break;
        case kEventID_SetLPMTrigger:
        {
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nLPM set was successful.");
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nLPM set failed.");
            }
        }
        break;
        case kEventID_GetLPMTrigger:
        {
            lpm_event_t lpmEvent = *(lpm_event_t *)response;
            if (status == kEventStatus_Ok)
            {
                char lpmStatus[10];
                strcpy(lpmStatus, lpmEvent.status == kLPMManagerStatus_SleepEnable ? "enabled" : "disabled");
                SHELL_Printf(s_ShellHandle, "\r\nLPM is currently \"%s\".", lpmStatus);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nGet low power mode status failed.");
            }
        }
        break;
        case kEventID_RecordingInfo:
        {
            recording_info_t recordedInfo = *(recording_info_t *)response;
            if (status == kEventStatus_Ok)
            {
                SHELL_Printf(s_ShellHandle, "\r\nH.264 clip start:0x%x size:0x%x state:%d", recordedInfo.start,
                             recordedInfo.size, recordedInfo.state);
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nRecorded error");
            }
        }
        break;
        case kEventFaceRecID_OasisSetState:
        case kEventFaceRecID_OasisGetState:
        {
            event_face_rec_t oasisResponse = *(event_face_rec_t *)response;
            if (status == kEventStatus_Ok)
            {
                if (oasisResponse.oasisState.state == kOasisState_Running)
                {
                    SHELL_Printf(s_ShellHandle, "\r\nOasis mode: Running");
                }
                else if (oasisResponse.oasisState.state == kOasisState_Stopped)
                {
                    SHELL_Printf(s_ShellHandle, "\r\nOasis mode: Stopped");
                }
            }
            else if (status == kEventStatus_Error)
            {
                if (oasisResponse.oasisState.state == kOasisState_Running)
                {
                    SHELL_Printf(s_ShellHandle, "\r\nOasis it's already running");
                }
                else if (oasisResponse.oasisState.state == kOasisState_Stopped)
                {
                    SHELL_Printf(s_ShellHandle, "\r\nOasis it's already stopped");
                }
            }
        }
        break;
        case kEventFaceRecID_SetFaceRecThreshold:
        case kEventFaceRecID_GetFaceRecThreshold:
        {
            if (status == kEventStatus_Ok)
            {
                faceRecThreshold_event_t *pFaceRecThreshold = (faceRecThreshold_event_t *)response;
                SHELL_Printf(s_ShellHandle, "\r\nFace recognition threshold: %d, range[%d - %d]",
                             pFaceRecThreshold->value, pFaceRecThreshold->min, pFaceRecThreshold->max);

                /* reset the system to make the face recognition threshold setting takes effect */
                if (event_id == kEventFaceRecID_SetFaceRecThreshold)
                {
                    __NVIC_SystemReset();
                }
            }
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\nFace recognition threshold error");
            }
        }
        break;

        default:
            break;
    }

    if (isFinished)
    {
        SHELL_PrintPrompt(s_ShellHandle);
    }
    return 0;
}

static int _FrameworkEventsHandler(framework_events_t eventId, framework_response_t *response, unsigned char isFinished)
{
    if (isFinished)
    {
        SHELL_PrintPrompt(s_ShellHandle);
        return 0;
    }

    switch (eventId)
    {
        case kFrameworkEvents_GetManagerInfo:
        {
            if (isFinished)
            {
                SHELL_PrintPrompt(s_ShellHandle);
                return 0;
            }
            fwk_task_info_t taskInfo = response->fwkTaskInfo;
            SHELL_Printf(s_ShellHandle, "\r\n ID- %d \tpriority- %d \t Name- %s", taskInfo.managerId, taskInfo.priority,
                         taskInfo.name);
        }
        break;

        case kFrameworkEvents_GetManagerComponents:
        {
            if (isFinished)
            {
                SHELL_PrintPrompt(s_ShellHandle);
                return 0;
            }
            fwk_task_component_t taskComponent = response->fwkTaskComponent;

            if (taskComponent.configs != NULL)
            {
                /* Print config table header if receiving first entry of table */
                if (taskComponent.deviceId == 0)
                {
                    _PrintDeviceConfigTable(taskComponent.deviceId, taskComponent.deviceName, taskComponent.configs,
                                            true);
                }
                else
                {
                    _PrintDeviceConfigTable(taskComponent.deviceId, taskComponent.deviceName, taskComponent.configs,
                                            false);
                }
            }
            /* Temporary fix until config refactor is applied to each type of device */
            // TODO: Remove this if-else statement
            else
            {
                SHELL_Printf(s_ShellHandle, "\r\n ManagerID-%d.\t DeviceID %d \t DeviceName - %s  ",
                             taskComponent.managerId, taskComponent.deviceId, taskComponent.deviceName);
            }
        }
        break;

        default:
            break;
    }
    return 0;
}

static shell_status_t _GetManagerCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 1)
    {
        if (s_InputCallback != NULL)
        {
            uint8_t fromISR                     = __get_IPSR();
            s_InputEvent.frameworkRequest       = &s_FrameworkRequest;
            s_FrameworkRequest.frameworkEventId = kFrameworkEvents_GetManagerInfo;
            s_InputCallback(s_SourceShell, kInputEventID_FrameworkRecv, 0, &s_InputEvent, 0, fromISR);
        }
    }
    else if (argc == 2)
    {
        if (s_InputCallback != NULL)
        {
            char *pEnd;
            uint8_t fromISR                     = __get_IPSR();
            s_InputEvent.frameworkRequest       = &s_FrameworkRequest;
            s_FrameworkRequest.frameworkEventId = kFrameworkEvents_GetManagerComponents;
            s_FrameworkRequest.managerId        = (uint8_t)strtol(argv[1], &pEnd, 10);
            s_InputCallback(s_SourceShell, kInputEventID_FrameworkRecv, 0, &s_InputEvent, 0, fromISR);
        }
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _ListCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList = 0;
    if (argc == 1)
    {
        receiverList                     = 1 << kFWKTaskID_VisionAlgo;
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_GetUserList;
        s_FaceRecEvent.eventBase.respond = _HalEventsHandler;
        s_InputEvent.inputData           = &s_FaceRecEvent;
    }
    else if (argc == 2)
    {
        if (strcmp((char *)argv[1], "-c") == 0)
        {
            receiverList                     = 1 << kFWKTaskID_VisionAlgo;
            s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_GetUserCount;
            s_FaceRecEvent.eventBase.respond = _HalEventsHandler;
            s_InputEvent.inputData           = &s_FaceRecEvent;
        }
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR = __get_IPSR();
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _RenameCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList = 0;
    char *pEnd;
    uint32_t id;

    if (argc != 3)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    id = strtol(argv[1], &pEnd, 10);

    if ((pEnd == argv[1]) || (id > (uint16_t)(-1)))
    {
        SHELL_Printf(shellContextHandle, "Not a valid id\r\n");
        return kStatus_SHELL_Error;
    }

    if (strlen(argv[2]) > FACE_NAME_MAX_LEN)
    {
        SHELL_Printf(shellContextHandle, "Name too long\r\n");
        return kStatus_SHELL_Error;
    }
    else if (hasSpecialCharacters(argv[2]))
    {
        SHELL_Printf(shellContextHandle, "Name contains invalid characters\r\n");
        return kStatus_SHELL_Error;
    }

    receiverList                     = 1 << kFWKTaskID_VisionAlgo;
    s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_UpdateUserInfo;
    s_FaceRecEvent.eventBase.respond = _HalEventsHandler;
    s_FaceRecEvent.updateFace.id     = id;
    strcpy(s_FaceRecEvent.updateFace.name, argv[2]);

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_FaceRecEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _AddCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList = 0;

    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }
    else
    {
        receiverList                     = 1 << kFWKTaskID_VisionAlgo;
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_AddUser;
        if (argc == 1)
        {
            s_FaceRecEvent.addFace.hasName = false;
        }
        else
        {
            if (strlen(argv[1]) > FACE_NAME_MAX_LEN)
            {
                SHELL_Printf(shellContextHandle, "Name too long\r\n");
                return kStatus_SHELL_Error;
            }
            if (hasSpecialCharacters(argv[1]))
            {
                SHELL_Printf(shellContextHandle, "Name contains invalid characters\r\n");
                return kStatus_SHELL_Error;
            }
            strcpy(s_FaceRecEvent.addFace.name, argv[1]);
            s_FaceRecEvent.addFace.hasName = true;
        }
    }

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_FaceRecEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _DelCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;

    if (argc > 3)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    memset(&s_FaceRecEvent, 0, sizeof(s_FaceRecEvent));
    receiverList                     = 1 << kFWKTaskID_VisionAlgo;
    s_FaceRecEvent.eventBase.respond = _HalEventsHandler;
    if (argc == 1)
    {
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_DelUser;
        s_FaceRecEvent.delFace.hasName   = false;
        s_FaceRecEvent.delFace.hasID     = false;
    }
    else if (argc == 2)
    {
        if (strcmp((char *)argv[1], "-a") == 0)
        {
            s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_DelUserAll;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (argc == 3)
    {
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_DelUser;

        if (strcmp((char *)argv[1], "-n") == 0)
        {
            if (strlen(argv[2]) > FACE_NAME_MAX_LEN)
            {
                SHELL_Printf(shellContextHandle, "Name too long\r\n");
                return kStatus_SHELL_Error;
            }
            if (hasSpecialCharacters(argv[2]))
            {
                SHELL_Printf(shellContextHandle, "Name contains invalid characters\r\n");
                return kStatus_SHELL_Error;
            }
            strcpy(s_FaceRecEvent.delFace.name, argv[2]);
            s_FaceRecEvent.delFace.hasName = true;
        }
        else if (strcmp((char *)argv[1], "-i") == 0)
        {
            char *pEnd;
            uint32_t id = strtol(argv[2], &pEnd, 10);

            if (pEnd == argv[2] || id > (uint16_t)(-1))
            {
                SHELL_Printf(shellContextHandle, "Not a valid id\r\n");
                return kStatus_SHELL_Error;
            }
            s_FaceRecEvent.delFace.id    = (uint16_t)id;
            s_FaceRecEvent.delFace.hasID = true;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Wrong parameters\r\n");
            return kStatus_SHELL_Error;
        }
    }

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_FaceRecEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _VerboseCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    s_CommonEvent.eventBase.respond = _HalEventsHandler;
    if (argc == 1)
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetLogLevel;
    }
    else
    {
        s_CommonEvent.eventBase.eventId = kEventID_SetLogLevel;
        if (!strcmp((char *)argv[1], "none"))
        {
            s_CommonEvent.logLevel.logLevel = kLOGLevel_None;
        }
        else if (!strcmp((char *)argv[1], "error"))
        {
            s_CommonEvent.logLevel.logLevel = kLOGLevel_Error;
        }
        else if (!strcmp((char *)argv[1], "debug"))
        {
            s_CommonEvent.logLevel.logLevel = kLOGLevel_Debug;
        }
        else if (!strcmp((char *)argv[1], "info"))
        {
            s_CommonEvent.logLevel.logLevel = kLOGLevel_Info;
        }
        else if (!strcmp((char *)argv[1], "verbose"))
        {
            s_CommonEvent.logLevel.logLevel = kLOGLevel_Verbose;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid log_level value!\r\n");
            return kStatus_SHELL_Error;
        }
    }
    receiverList = 1 << kFWKTaskID_Output;
    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_CommonEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }
    return kStatus_SHELL_Success;
}

static shell_status_t _SaveCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    if (argc != 1)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    s_FaceRecEvent.eventBase.respond = _HalEventsHandler;
    s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_SaveUserList;
    receiverList                     = 1 << kFWKTaskID_Output | 1 << kFWKTaskID_VisionAlgo;
    SHELL_Printf(shellContextHandle, "Saving users to flash...\r\n");
    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_FaceRecEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _VersionCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 1)
    {
        /* Find the current running bank by checking the ResetISR Address in the vector table (which is loaded into
         * DTC) */
        uint32_t runningFromBankA = (((*(uint32_t *)(APPLICATION_RESET_ISR_ADDRESS)-FLEXSPI_AMBA_BASE) &
                                      (FICA_IMG_BANK_APP_MASK)) == FICA_IMG_APP_A_ADDR);

        uint32_t runningFromBankB = (((*(uint32_t *)(APPLICATION_RESET_ISR_ADDRESS)-FLEXSPI_AMBA_BASE) &
                                      (FICA_IMG_BANK_APP_MASK)) == FICA_IMG_APP_B_ADDR);


        if (runningFromBankA)
        {
            SHELL_Printf(shellContextHandle, "App running in bankA\r\n");
        }
        else if (runningFromBankB)
        {
            SHELL_Printf(shellContextHandle, "App running in bankB\r\n");
        }
        else
        {
            SHELL_Printf(shellContextHandle, "App running at unknown address\r\n");
        }

        SHELL_Printf(shellContextHandle, "Version %d.%d.%d\r\n", SMART_LOCK_VERSION_MAJOR, SMART_LOCK_VERSION_MINOR,
                     SMART_LOCK_VERSION_HOTFIX);
    }
    else if (!strcmp((char *)argv[1], "oasis"))
    {
        SHELL_Printf(shellContextHandle, "Oasis version %d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_HOTFIX);
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Invalid parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }
    return kStatus_SHELL_Success;
}

static shell_status_t _ResetCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    if (argc > 1)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 1)
    {
        __NVIC_SystemReset();
    }
    return kStatus_SHELL_Success;
}

static shell_status_t _DisplayOutputCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    if (argc > 3)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    s_CommonEvent.eventBase.respond = _HalEventsHandler;
    if (argc == 1)
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetDisplayOutput;
        receiverList                    = 1 << kFWKTaskID_Output;
    }
    else
    {
        if (!strcmp((char *)argv[1], "UVC"))
        {
            s_CommonEvent.displayOutput.displayOutput = kDisplayOutput_UVC;
            s_CommonEvent.eventBase.eventId           = kEventID_SetDisplayOutput;
            receiverList                              = 1 << kFWKTaskID_Output;
        }
        else if (!strcmp((char *)argv[1], "panel"))
        {
            s_CommonEvent.displayOutput.displayOutput = kDisplayOutput_Panel;
            s_CommonEvent.eventBase.eventId           = kEventID_SetDisplayOutput;
            receiverList                              = 1 << kFWKTaskID_Output;
        }
        else if (!strcmp((char *)argv[1], "source"))
        {
            if (argc == 2)
            {
                s_CommonEvent.eventBase.eventId = kEventID_GetDisplayOutputSource;
                receiverList                    = 1 << kFWKTaskID_Output | 1 << kFWKTaskID_Display;
            }
            else
            {
                s_CommonEvent.eventBase.eventId = kEventID_SetDisplayOutputSource;
                receiverList                    = 1 << kFWKTaskID_Output | 1 << kFWKTaskID_Display;
                if (!strcmp((char *)argv[2], "RGB"))
                {
                    s_CommonEvent.displayOutput.displayOutputSource = kPixelFormat_UYVY1P422_RGB;
                }
                else if (!strcmp((char *)argv[2], "2DIR"))
                {
                    s_CommonEvent.displayOutput.displayOutputSource = kPixelFormat_UYVY1P422_Gray;
                }
                else if (!strcmp((char *)argv[2], "3DIR"))
                {
                    s_CommonEvent.displayOutput.displayOutputSource = kPixelFormat_Gray16;
                }
                else if (!strcmp((char *)argv[2], "3DDEPTH"))
                {
                    s_CommonEvent.displayOutput.displayOutputSource = kPixelFormat_Depth16;
                }
                else
                {
                    SHELL_Printf(shellContextHandle, "Invalid display source type\r\n");
                    return kStatus_SHELL_Error;
                }
            }
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid display type\r\n");
            return kStatus_SHELL_Error;
        }
    }

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_CommonEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _IrPwmCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    char *pEnd;

    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    s_CommonEvent.eventBase.respond = _HalEventsHandler;
    if (argc == 1)
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetIRLedBrightness;
    }
    else
    {
        uint32_t brightness = strtol(argv[1], &pEnd, 10);
        if (argv[1] == pEnd)
        {
            SHELL_Printf(shellContextHandle, "\"%s\" not a number.\r\n", argv[1]);
            return kStatus_SHELL_Error;
        }
        else if ((brightness < 0) || (100 < brightness))
        {
            SHELL_Printf(shellContextHandle, "PWM duty of %s outside of acceptable range. Valid values are 0->100\r\n",
                         argv[1]);
            return kStatus_SHELL_Error;
        }
        s_CommonEvent.irLed.brightness  = (uint8_t)brightness;
        s_CommonEvent.eventBase.eventId = kEventID_SetIRLedBrightness;
    }

    receiverList = 1 << kFWKTaskID_Output;

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_CommonEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _WhitePwmCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    char *pEnd;

    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    s_CommonEvent.eventBase.respond = _HalEventsHandler;
    if (argc == 1)
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetWhiteLedBrightness;
    }
    else
    {
        uint32_t brightness = strtol(argv[1], &pEnd, 10);
        if (argv[1] == pEnd)
        {
            SHELL_Printf(shellContextHandle, "\"%s\" not a number.\r\n", argv[1]);
            return kStatus_SHELL_Error;
        }
        else if ((brightness < 0) || (100 < brightness))
        {
            SHELL_Printf(shellContextHandle, "PWM duty of %s outside of acceptable range. Valid values are 0->100\r\n",
                         argv[1]);
            return kStatus_SHELL_Error;
        }
        s_CommonEvent.whiteLed.brightness = (uint8_t)brightness;
        s_CommonEvent.eventBase.eventId   = kEventID_SetWhiteLedBrightness;
    }

    receiverList = 1 << kFWKTaskID_Output;

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_CommonEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _SpeakerVolumeCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    char *pEnd;

    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    s_CommonEvent.eventBase.respond = _HalEventsHandler;
    if (argc == 1)
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetSpeakerVolume;
    }
    else
    {
        uint32_t volume = strtol(argv[1], &pEnd, 10);
        if (argv[1] == pEnd)
        {
            SHELL_Printf(shellContextHandle, "\"%s\" not a number.\r\n", argv[1]);
            return kStatus_SHELL_Error;
        }
        else if ((volume < 0) || (100 < volume))
        {
            SHELL_Printf(shellContextHandle, "Volume %s outside of acceptable range. Valid values are 0->100. \r\n",
                         argv[1]);
            return kStatus_SHELL_Error;
        }
        s_CommonEvent.eventBase.eventId    = kEventID_SetSpeakerVolume;
        s_CommonEvent.speakerVolume.volume = (uint8_t)volume;
    }

    receiverList = 1 << kFWKTaskID_Output;

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_CommonEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _LpmTriggerCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;

    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    receiverList = 1 << kFWKTaskID_VisionAlgo;
    if (argc == 1)
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetLPMTrigger;
        s_CommonEvent.eventBase.respond = _HalEventsHandler;
    }
    else
    {
        s_CommonEvent.eventBase.eventId = kEventID_SetLPMTrigger;
        s_CommonEvent.eventBase.respond = _HalEventsHandler;
        if (!strcmp((char *)argv[1], "enable"))
        {
            s_CommonEvent.lpm.status = kLPMManagerStatus_SleepEnable;
        }
        else if (!strcmp((char *)argv[1], "disable"))
        {
            s_CommonEvent.lpm.status = kLPMManagerStatus_SleepDisable;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid lpm type\r\n");
            return kStatus_SHELL_Error;
        }
    }

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_CommonEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _ConnectivityCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    shell_status_t status = kStatus_SHELL_Success;

    if (argc == 2)
    {
        if (!strcmp((char *)argv[1], "type"))
        {
            s_CommonEvent.eventBase.eventId = kEventID_GetConnectivityType;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid parameters supplied\r\n");
            status = kStatus_SHELL_Error;
        }
    }
    else if (argc == 3)
    {
        if (!strcmp((char *)argv[1], "type"))
        {
            s_CommonEvent.eventBase.eventId = kEventID_SetConnectivityType;
            if (!strcmp((char *)argv[2], "wifi"))
            {
                s_CommonEvent.connectivity.connectivityType = kConnectivityType_WiFi;
            }
            else if (!strcmp((char *)argv[2], "ble"))
            {
                s_CommonEvent.connectivity.connectivityType = kConnectivityType_BLE;
            }
            else if (!strcmp((char *)argv[2], "none"))
            {
                s_CommonEvent.connectivity.connectivityType = kConnectivityType_None;
            }
            else
            {
                SHELL_Printf(shellContextHandle, "Invalid type parameters supplied\r\n");
                status = kStatus_SHELL_Error;
            }
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid parameters supplied\r\n");
            status = kStatus_SHELL_Error;
        }
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        status = kStatus_SHELL_Error;
    }

    if (status == kStatus_SHELL_Success)
    {
        receiverList                    = 1 << kFWKTaskID_Output;
        s_CommonEvent.eventBase.respond = _HalEventsHandler;
        if (s_InputCallback != NULL)
        {
            uint8_t fromISR        = __get_IPSR();
            s_InputEvent.inputData = &s_CommonEvent;
            s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent),
                            fromISR);
        }
    }

    return status;
}

static shell_status_t _WiFiCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;
    shell_status_t status = kStatus_SHELL_Success;

    if (FWK_ConfigGetConnectivityType() != kConnectivityType_WiFi)
    {
        SHELL_Printf(shellContextHandle, "Set connectivity type to WiFi and reset\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc < 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (!strcmp((char *)argv[1], "credentials"))
    {
        receiverList = 1 << kFWKTaskID_Input;

        if (argc == 2)
        {
            s_CommonEvent.eventBase.eventId = kEventID_WiFiGetCredentials;
        }
        else if (argc == 3)
        {
            if (!strcmp((char *)argv[2], "erase"))
            {
                s_CommonEvent.eventBase.eventId = kEventID_WiFiEraseCredentials;
            }
            else
            {
                status = kStatus_SHELL_Error;
            }
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else if (!strcmp((char *)argv[1], "ip"))
    {
        receiverList = 1 << kFWKTaskID_Input;
        if (argc == 2)
        {
            s_CommonEvent.eventBase.eventId = kEventID_WiFiGetIP;
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else if (!strcmp((char *)argv[1], "ssid"))
    {
        if (argc > 2)
        {
            receiverList          = 1 << kFWKTaskID_Input;
            wifi_cred_t *wifiCred = &s_CommonEvent.wifi.wifiCred;
            memset(wifiCred, 0, sizeof(wifi_cred_t));
            s_CommonEvent.eventBase.eventId = kEventID_WiFiSetCredentials;
            wifiCred->ssid.length           = WIFI_SSID_LENGTH;
            wifiCred->ssid.length =
                mergeParameters((char *)&(wifiCred->ssid.value), wifiCred->ssid.length, &argv[2], (uint32_t)(argc - 2));
            if (wifiCred->ssid.length == 0)
            {
                status = kStatus_SHELL_Error;
            }
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else if (!strcmp((char *)argv[1], "password"))
    {
        if (argc > 2)
        {
            receiverList          = 1 << kFWKTaskID_Input;
            wifi_cred_t *wifiCred = &s_CommonEvent.wifi.wifiCred;
            memset(wifiCred, 0, sizeof(wifi_cred_t));
            s_CommonEvent.eventBase.eventId = kEventID_WiFiSetCredentials;
            wifiCred->password.length       = WIFI_PASSWORD_LENGTH;
            wifiCred->password.length = mergeParameters((char *)&(wifiCred->password.value), wifiCred->password.length,
                                                        &argv[2], (uint32_t)(argc - 2));
            if (wifiCred->password.length == 0)
            {
                status = kStatus_SHELL_Error;
            }
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else if (!strcmp((char *)argv[1], "state"))
    {
        receiverList = 1 << kFWKTaskID_Input;
        if (argc == 2)
        {
            s_CommonEvent.eventBase.eventId = kEventID_WiFiGetState;
        }
        else if (argc == 3)
        {
            if (!strcmp((char *)argv[2], "on"))
            {
                s_CommonEvent.eventBase.eventId = kEventID_WiFiSetState;
                s_CommonEvent.wifi.state        = kWiFi_On;
            }
            else if (!strcmp((char *)argv[2], "off"))
            {
                s_CommonEvent.eventBase.eventId = kEventID_WiFiSetState;
                s_CommonEvent.wifi.state        = kWiFi_Off;
            }
            else
            {
                status = kStatus_SHELL_Error;
            }
        }
    }
    else if (!strcmp((char *)argv[1], "reset"))
    {
        receiverList = 1 << kFWKTaskID_Input;
        if (argc == 2)
        {
            s_CommonEvent.eventBase.eventId = kEventID_WiFiReset;
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else if (!strcmp((char *)argv[1], "scan"))
    {
        receiverList = 1 << kFWKTaskID_Input;
        if (argc == 2)
        {
            s_CommonEvent.eventBase.eventId = kEventID_WiFiScan;
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else if (!strcmp((char *)argv[1], "ftp_server"))
    {
        receiverList = 1 << kFWKTaskID_Input;
        if (argc == 2)
        {
            s_CommonEvent.eventBase.eventId = kEventID_FTPGetServerInfo;
        }
        else if (argc == 4)
        {
            char *pEnd;
            s_CommonEvent.eventBase.eventId           = kEventID_FTPSetServerInfo;
            s_CommonEvent.wifi.ftpServerInfo.serverIP = net_inet_aton(argv[2]);

            uint32_t port = strtol(argv[3], &pEnd, 10);
            if (pEnd == argv[3] || port > (uint16_t)(-1))
            {
                SHELL_Printf(shellContextHandle, "Not a valid port\r\n");
                return kStatus_SHELL_Error;
            }

            s_CommonEvent.wifi.ftpServerInfo.serverPort = port;
        }
        else
        {
            status = kStatus_SHELL_Error;
        }
    }
    else
    {
        status = kStatus_SHELL_Error;
    }

    if (status == kStatus_SHELL_Error)
    {
        SHELL_Printf(shellContextHandle, "Invalid parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR                 = __get_IPSR();
        s_InputEvent.inputData          = &s_CommonEvent;
        s_CommonEvent.eventBase.respond = _HalEventsHandler;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return status;
}

static shell_status_t _BleAddressCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;

    if (FWK_ConfigGetConnectivityType() != kConnectivityType_BLE)
    {
        SHELL_Printf(shellContextHandle, "Set connectivity type to BLE and reset\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc != 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (!strcmp((char *)argv[1], "address"))
    {
        s_CommonEvent.eventBase.eventId = kEventID_GetBLEConnection;
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Invalid display type\r\n");
        return kStatus_SHELL_Error;
    }

    receiverList = 1 << kFWKTaskID_Output;

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR                 = __get_IPSR();
        s_InputEvent.inputData          = &s_CommonEvent;
        s_CommonEvent.eventBase.respond = _HalEventsHandler;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_CommonEvent), fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _RecordCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;

    if (argc != 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    if (!strcmp((char *)argv[1], "start"))
    {
        s_RecordingEvent.eventBase.eventId = kEventID_RecordingState;
        s_RecordingEvent.state             = kRecordingState_Start;
    }
    else if (!strcmp((char *)argv[1], "stop"))
    {
        s_RecordingEvent.eventBase.eventId = kEventID_RecordingState;
        s_RecordingEvent.state             = kRecordingState_Stop;
    }
    else if (!strcmp((char *)argv[1], "info"))
    {
        s_RecordingEvent.eventBase.eventId = kEventID_RecordingInfo;
        s_RecordingEvent.eventBase.respond = _HalEventsHandler;
        s_RecordingEvent.state             = kRecordingState_Info;
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Wrong command\r\n");
        return kStatus_SHELL_Error;
    }

    receiverList = 1 << kFWKTaskID_VisionAlgo;

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_RecordingEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_RecordingEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _RtInfoCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    char *pEnd;

    if (argc == 2)
    {
        if (!strcmp((char *)argv[1], "size"))
        {
            /* print the runtime information start address and size */
            unsigned char *pStart = NULL;
            unsigned int size     = 0;
            FaceRecRtInfo_Size(&pStart, &size);
            if (pStart && size)
            {
                SHELL_Printf(shellContextHandle, "\"%s\" start:0x%x size:0x%x\r\n", argv[1], pStart, size);
            }
            return kStatus_SHELL_Success;
        }
        else if (!strcmp((char *)argv[1], "clean"))
        {
            /* clean the captured runtime information */
            FaceRecRtInfo_Clean();
            return kStatus_SHELL_Success;
        }
        else if (!strcmp((char *)argv[1], "disable"))
        {
            /* disable all the filters */
            FaceRecRtInfo_Disable();
            return kStatus_SHELL_Success;
        }
        else
        {
            SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if ((argc == 3) || (argc == 4))
    {
        uint32_t tmp;
        face_rec_rt_info_id_t id = kFaceRecRtInfoId_Count;
        unsigned char enable     = 0;
        unsigned char filter     = 0;

        if (!strcmp((char *)argv[1], "global"))
        {
            id = kFaceRecRtInfoId_Global;
        }
        else if (!strcmp((char *)argv[1], "detect"))
        {
            id = kFaceRecRtInfoId_Detect;
        }
        else if (!strcmp((char *)argv[1], "fake"))
        {
            id = kFaceRecRtInfoId_Fake;
        }
        else if (!strcmp((char *)argv[1], "facerec"))
        {
            id = kFaceRecRtInfoId_FaceFecognize;
        }
        else
        {
            tmp = strtol(argv[1], &pEnd, 10);
            if (argv[1] == pEnd)
            {
                SHELL_Printf(shellContextHandle, "\"%s\" invalid item id.\r\n", argv[1]);
                return kStatus_SHELL_Error;
            }

            if ((tmp >= kFaceRecRtInfoId_Global) && (tmp < kFaceRecRtInfoId_Count))
            {
                id = (face_rec_rt_info_id_t)tmp;
            }
        }

        /* check the item id */
        if (id > kFaceRecRtInfoId_Count)
        {
            SHELL_Printf(shellContextHandle, "\"%s\" invalid item id.\r\n", argv[1]);
            return kStatus_SHELL_Error;
        }

        tmp = strtol(argv[2], &pEnd, 10);
        if ((argv[2] == pEnd) || ((tmp != 0) && (tmp != 1)))
        {
            SHELL_Printf(shellContextHandle, "\"%s\" invalid enable flag.\r\n", argv[2]);
            return kStatus_SHELL_Error;
        }
        enable = (unsigned char)tmp;

        if (argc == 4)
        {
            tmp = strtol(argv[3], &pEnd, 10);
            if ((argv[3] == pEnd) || ((tmp != 0) && (tmp != 1) && (tmp != 2)))
            {
                SHELL_Printf(shellContextHandle, "\"%s\" invalid enable flag.\r\n", argv[3]);
                return kStatus_SHELL_Error;
            }
            filter = tmp;
        }

        SHELL_Printf(shellContextHandle, "id:0x%02x enable:%d filter:%d\r\n", id, enable, filter);
        FaceRecRtInfo_Filter(id, enable, filter);
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _OasisCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    uint32_t receiverList;

    if (argc != 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    memset(&s_FaceRecEvent, 0, sizeof(s_FaceRecEvent));

    if (!strcmp((char *)argv[1], "start"))
    {
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_OasisSetState;
        s_FaceRecEvent.oasisState.state  = kOasisState_Running;
    }
    else if (!strcmp((char *)argv[1], "stop"))
    {
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_OasisSetState;
        s_FaceRecEvent.oasisState.state  = kOasisState_Stopped;
    }
    else if (!strcmp((char *)argv[1], "info"))
    {
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_OasisGetState;
    }
    else
    {
        SHELL_Printf(shellContextHandle, "Wrong command\r\n");
        return kStatus_SHELL_Error;
    }
    s_FaceRecEvent.eventBase.respond = _HalEventsHandler;

    receiverList = 1 << kFWKTaskID_VisionAlgo;

    if (s_InputCallback != NULL)
    {
        uint8_t fromISR        = __get_IPSR();
        s_InputEvent.inputData = &s_FaceRecEvent;
        s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent),
                        fromISR);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t _FaceRecThresholdCommand(shell_handle_t shellContextHandle, int32_t argc, char **argv)
{
    char *pEnd;

    if (argc > 2)
    {
        SHELL_Printf(shellContextHandle, "Invalid # of parameters supplied\r\n");
        return kStatus_SHELL_Error;
    }

    uint32_t receiverList            = 1 << kFWKTaskID_Output;
    s_FaceRecEvent.eventBase.respond = _HalEventsHandler;

    if (argc == 1)
    {
        s_FaceRecEvent.eventBase.eventId = kEventFaceRecID_GetFaceRecThreshold;
        // SHELL_Printf(shellContextHandle, "Current threshold: %d Range:[%d - %d]\r\n");
    }
    else
    {
        uint32_t threshold = strtol(argv[1], &pEnd, 10);
        if (argv[1] == pEnd)
        {
            SHELL_Printf(shellContextHandle, "\"%s\" not a number.\r\n", argv[1]);
            return kStatus_SHELL_Error;
        }
        else if ((threshold < MINIMUM_FACE_REC_THRESHOLD) || (MAXIMUM_FACE_REC_THRESHOLD < threshold))
        {
            SHELL_Printf(shellContextHandle, "Face recognition threshold %s out of range. Valid values are %d-->%d\r\n",
                         argv[1], MINIMUM_FACE_REC_THRESHOLD, MAXIMUM_FACE_REC_THRESHOLD);
            return kStatus_SHELL_Error;
        }
        s_FaceRecEvent.faceRecThreshold.min   = MINIMUM_FACE_REC_THRESHOLD;
        s_FaceRecEvent.faceRecThreshold.max   = MAXIMUM_FACE_REC_THRESHOLD;
        s_FaceRecEvent.faceRecThreshold.value = threshold;
        s_FaceRecEvent.eventBase.eventId      = kEventFaceRecID_SetFaceRecThreshold;
    }

    uint8_t fromISR        = __get_IPSR();
    s_InputEvent.inputData = &s_FaceRecEvent;
    s_InputCallback(s_SourceShell, kInputEventID_Recv, receiverList, &s_InputEvent, sizeof(s_FaceRecEvent), fromISR);

    return kStatus_SHELL_Success;
}
