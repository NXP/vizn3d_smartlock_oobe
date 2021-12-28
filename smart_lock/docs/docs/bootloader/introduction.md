---
sidebar_position: 1
---

# Introduction

The Smart Lock project uses a "bootloader + main application" architecture to provide additional security and update-related functionality to the main application.
The bootloader handles all boot-related tasks including, but not limited to:

* Launching the main application and, if necessary, initializing peripherals
* Firmware updates using either the Mass Storage Device (MSD), Over-the-Air, or Over-the-Wire update method
  * Protects against update failures by using a primary and backup application "flash bank"
* Image certification/verification[^1]

[^1] The SLN-VIZN3D-IOT does not currently support any bootloader security features.

## Why use a bootloader?

By separating the boot process from the main application,
the main application can be safely updated and verified without the risk of creating an irrecoverable state due to a failed update, or running a malicious,
unauthorized and unsigned firmware binary flashed by a bad actor.
It is essential in any production application that precautions be taken to ensure the integrity and stability of the firmware before, during, and after an update,
and the bootloader application is simply one measure to help provide this assurance.

The following sections will describe how to use many of the bootloader's primary features in order to assist developer's interested in understanding, utilizing, and expanding them.

## Application Banks

The bootloader file system uses dual application "banks" referred to as "Bank A" and "Bank B" to provide a backup/redundancy "known good" application to prevent bricking when flashing an update via either the MSD, OTA, or OTW update method.
For example,
if an application update is being flashed via MSD to the Bank A application bank, even if that update should fail midway through Bank B will still contain a fully operational backup.

In the SLN-VIZN3D-IOT, Bank A is located at `0x30200000` while Bank B is located at `0x30800000`.
Specifying an application's flash address can be done prior to compilation of the application via the `Properties->MCU Settings` menu as shown in the figure below:

<!-- ![Configure App Bank in MCUXpresso IDE](configureAppBank.png) -->

Providing an application binary built for the proper application bank address is crucial during MSD, OTA, and OTW updates, and failure to do so will result in a failure to flash the binary.

:::note
The bootloader will not automatically recover from a botched flashing procedure,
but will instead revert to the alternate working application flash bank instead.
:::

## Logging

The bootloader supports debug logging over UART to help diagnose and debug issues that may arise while using or modifying the bootloader.
For example,
the debug logger can be helpful when trying to understand why an application update might have failed.

<!-- ![Example Log Message](logMsgExample.png) -->

Logging is enabled by default in the `Debug` build mode configuration.
The logging functionality, however, comes with an increase in bootloader performance, and can slow down the boot process by as much as 200ms.
As a result,
it may be desirable to disable debug logging in production applications.
To disable logging in the bootloader,
simply build and run the bootloader in the `Release` build mode configuration.
This can be done by right-clicking on the bootloader project in the `Project Explorer` view
and navigating to `Build Configurations -> Set Active -> Release` as shown in the figure below:

<!-- ![Set Build Config in MCUXpresso IDE](buildConfig.png) -->

To make use of the debug logging feature,
use a UART->USB converter to:

* Connect `GND` pin of converter to `J202: Pin 8`
* Connect `TX`  pin of converter to `J202: Pin 3`
* Connect `RX`  pin of converter to `J202: Pin 4`

<!-- ![Uart Pin Connections](uartPinConnections.png) -->

Once the converter has been properly attached,
connect to the board using a serial terminal emulator like *PuTTY* or *Tera Term* configured with the following serial settings:

* Speed: `115200`
* Data: `8 Bit`
* Parity: `None`
* Stop Bits: `1 bit`
* Flow Control: `None`
