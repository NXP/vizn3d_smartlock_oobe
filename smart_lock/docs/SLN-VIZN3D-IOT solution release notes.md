# SLN-VIZN3D-IOT solution release notes

**V1.4.0**

**What’s new**

1. Update OASIS LITE library to V4.92/V1.57.
2. Modify memory settings to support more users.
3. Fix bugs in facedb to support more users.
4. Modify fica definition to support more users.
5. Refine brightness control for 2D face rec with RGB+IR dual camera.
6. Add face rec start/stop cmds in module mode.

**Note**

Such scenarios are no longer supported.
Remote registration using RGB camera, local recognition using IR camera; Alternatively, remote registration using IR camera and local recognition using RGB camera.

**How to enable 2D application:**

1. Please get and follow up the AN (https://www.nxp.com.cn/docs/en/application-note/AN13705.pdf) to rework HW for 2D application.
2. Use the corresponding preprocessor definition and oasis link lib in the project settings for 2D/3D applications. 
   There is a slight difference between the library name described in the AN.

   | Board          | Preprocessor    | Oasis Lib         |
   | -------------  | -------------   | ------------------------------------------------------- |
   | VIZN3D Board   | SMART_LOCK_3D   | liboasis_lite3D_ELOCK_117f_ae.a |
   | Reworked Board | SMART_LOCK_2D   | liboasis_lite2D_ELOCK_117f_ae.a |
   | Reworked Board | SMART_ACCESS_2D | liboasis_lite2D_ACCESS_117f_ae.a |


**How to enable more users:**

1. MAX_FACE_DB_SIZE is defined in hal_sln_facedb.h.
2. If more data needs to be stored, we can use 32MB flash size instead of 16MB.
   Please change the preprocessor definition of 'BOARD_FLASH_SIZE=0x1000000U' to
   'BOARD_FLASH_SIZE=0x2000000U' in both bootloader and smartlock project settings.
3. Flash map can be saw in fica_definition.h.

**How to use face rec start/stop cmds:**

1. Please set the ENABLE_FACEID_MODULE_MODE to 1 in board_define.h to build new firmware.
2. Use shell cmd to start/stop face rec.

   "oasis startRec|stopRec" : start/stop rec.

   "oasis stopReg|stopDeReg": stop reg/dereg.

**List of known issues:**

| RTVZN-1104  | Fake face sometimes prompts in normal usage at 2D face rec                                          |
|-------------|-----------------------------------------------------------------------------------------------------|
| RTVZN-1084  | Face manager app \| Apk is not synchronized with the board when 3000 users are saved                |
| RTVZN-1079  | Face manager app \| Apk doesn't work on all phones                                                  |
| RTVZN-1074  | Limitation on remote registration and local recognition, only support RGB vs RGB and IR vs IR cases |
| RTVZN-795   | [headless]  Lots of side face during registration for some face types |
| RTVZN-783   | Fake face  prompt in normal usage                            |
| RTVZN-686   | Some  remotely registered users are not recognized by the board |
| RTVZN-643   | Face  manager app \| User can sometimes be registered twice  |
| RTVZN-631   | User can  get registered both remotely and then locally      |
| RTVZN-582   | observed  the screen flicker during the registration         |

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
| RTVZN-686 | Some  remotely registered users are not recognized by the board   |



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



