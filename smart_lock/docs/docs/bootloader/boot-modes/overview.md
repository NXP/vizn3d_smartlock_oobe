---
sidebar_position: 1
---

# Overview

The bootloader employs several different boot-up methods to augment the boot-up behavior.
Currently, the bootloader supports two primary boot modes:

* **[Normal Mode](normal-boot.md)**
* **[Mass Storage Device (MSD) Update Mode](mass-storage-device-updates.md)**

Normal mode, as the name would imply, is the default boot mode in which the bootloader simply loads the main application.

Mass Storage Device Update mode is a special boot mode in which the board enters an update state where the board appears as a mass storage device to a host PC device.
In this mode,
the bootloader is capable of receiving and flashing a new binary by copying that binary to the board as one would for a regular USB storage device.

More information on each of these modes can be found in the subsequent sections of this document.

## How is boot mode determined?

To determine the boot mode to be enter,
the bootloader checks several different boot flags which get set based on various conditions being met.

For each different boot mode (excluding Normal boot which is taken by default),
there is a different corresponding boot flag.
The means by which a boot flag gets set depends on the boot mode in question and the platform being used.
On the SLN-VIZN3D-IOT for example,
the MSD boot flag is set when the `SW1` button is held during bootup.
