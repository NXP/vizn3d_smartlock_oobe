# SLN-VIZN3D-IOT solution release notes



**V1.3.0**

**What’s new**

1. Update OASIS LITE library to V4.82 which enable user to customize more threshold for face
2. Detection/recognition and improves registration procedure.
3. Fix some bugs in command line module.
4. Optimize Flexio camera implementation.

**List of known issues:**

| RTVZN-582 | observed  the screen flicker during the registration         |
| --------- | ------------------------------------------------------------ |
| RTVZN-795 | [headless]  Lots of side face during registration for some face types |
| RTVZN-783 | Fake face  prompt in normal usage                            |
| RTVZN-631 | User can  get registered both remotely and then locally      |
| RTVZN-643 | Face  manager app \| User can sometimes be registered twice  |
| RTVZN-686 | Some  remotely registered users are not recognized by the    |



**V1.2.0**

**What’s new**

1. Enable WiFi
   * Add credentials via the shell terminal
   * Connect to the access point mentioned
   * Reconnection capabilities
2. Save local video, encoded in H.264 in a remote FTP server
   * Add FTP Server info via the shell terminal
3. Implement non-xip feature in which the application is loaded in SDRAM
4. Add support for Chinese font display
5. Update the SDK to 2.11.0
6. Fix a bug with Orbbec camera having overexposure issues
7. Fix a bug that will make the version command not print correct application bank
8. Add `FaceREcRTInfo tool` used for collecting information for face recognition
9. Fix a bug in which LEDs were not working without bootloader.

**List of known issues:**

| RTVZN-783 | Fake face  prompt in normal usage                            |
| --------- | ------------------------------------------------------------ |
| RTVZN-686 | Some  remotely registered users are not recognized by the board |
| RTVZN-660 | "Invalid  bluetooth packet" toast notification in the apk sometimes |
| RTVZN-643 | Face  manager app \| User can sometimes be registered twice  |
| RTVZN-631 | User can  get registered both remotely and then locally      |
| RTVZN-582 | observed  the screen flicker during the registration         |



**V1.1.4**

**What’s new**

1. First official version of SLN-VIZN3D-IOT smart_lock application released

**List of known issues:**

| RTVZN-806 | Display_output  prints panel even if it set to uvc           |
| --------- | ------------------------------------------------------------ |
| RTVZN-795 | [headless]  Lots of side face during registration for some face types |
| RTVZN-783 | Fake face  prompt in normal usage                            |
| RTVZN-778 | Each press  of SW buttons will queue sounds                  |
| RTVZN-686 | Some  remotely registered users are not recognized by the board |
| RTVZN-660 | "Invalid  bluetooth packet" toast notification in the apk sometimes |
| RTVZN-631 | User can  get registered both remotely and then locally      |



