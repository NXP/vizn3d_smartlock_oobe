/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __WPL_H
#define __WPL_H

#define WPL_SUCCESS 0
#define WPL_ERROR   1

#define WPL_WIFI_SSID_LENGTH     32
#define WPL_WIFI_PASSWORD_LENGTH 63

enum wifi_connection_event
{
    /* WLAN Connection Manager is in disconnected state */
    WIFI_CONNECTION_DISCONNECTED,
    /* WLAN Connection Manager is in connected state */
    WIFI_CONNECTION_CONNECTED,
};

/** WPL_Init
 *
 * @param cb Callback the upper layer to call if wlan disconnect/connect takes place caused by remote.
 * @return
 */
int WPL_Init(void (*cb)(enum wifi_connection_event reason));

/* WPL_Start()
 *
 * Start WIFI interface
 */
int WPL_Start();

/* WPL_Stop()
 *
 * Stop WIFI interface
 */
int WPL_Stop();

/* WPL_Start_AP()
 *
 * Start WIFI AP
 */
int WPL_Start_AP(char *ssid, char *password, int chan);

/* WPL_Stop_AP()
 *
 * Stop WIFI AP
 */
int WPL_Stop_AP();

/* WPL_Scan()
 *
 * Scan for Wifi APs
 */
int WPL_Scan();

/* WPL_Join()
 *
 * Join given SSID AP
 */
int WPL_Join(char *ssid, char *password);

/* WPL_Leave()
 *
 * Leave the current station network and clean up
 */
int WPL_Leave();

/* WPL_getSSIDs()
 *
 * get SSIDs as JSON
 */
char *WPL_getSSIDs();

/* WPL_StartDHCPServer(char *ip, char *net)
 *
 * Start the DHCP Server on given IP and network
 */
int WPL_StartDHCPServer(char *ip, char *net);

/* WPL_StartDHCPServer(char *ip, char *net)
 *
 * Stop the DHCP server and clean up
 */
int WPL_StopDHCPServer();

int WPL_GetIP(char *ip, int client);
#endif
