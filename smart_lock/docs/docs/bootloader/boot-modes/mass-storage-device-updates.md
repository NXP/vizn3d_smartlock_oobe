---
sidebar_position: 3
---

# Mass Storage Device Updates (MSD)

The Mass Storage Device (MSD) boot mode is a means by which application binaries can be flashed to the board via a drag-and-drop interface like one would use with a USB flash drive or similar device.
MSD mode can be useful for deploying quick updates to marketers and engineers in the field without access to debugging tools like a Segger J-Link.

<!-- ![Drag and Drop Interface](dragAndDropInterface.png) -->

## Enabling MSD Mode

To enable MSD mode on the SLN-VIZN3D-IOT,
simply press and hold the `SW1` button while powering on the board.
If done correctly,
the board's onboard LED will change to purple and begin blinking at an interval of roughly 1 second.

<div style={{textAlign: 'center'}}>

![Mass Storage Device Enabled](../../../static/img/bootloader/msd.gif)

</div>

Additionally, if connected to a Windows PC, your computer should make a sound indicating a new USB device has been connected and a new USB Storage Device will be shown in the file explorer.

## Flashing a New Binary

To flash a new binary while Mass Storage device mode is enabled, you must first verify the application bank which is currently in use.
This information can be discovered by using the `version` shell command while the main app is running.

```bash
version
App running in Bank A
Version 1.0.4
Shell>>
```

Once the current application bank in use has been identified, you must compile a binary for the alternate flash bank.
For example,
if Bank A is currently in use,
you must compile a Bank B binary and vice versa.
Instructions on compiling for a specific flash bank can this can be found in the [Application Banks](../application-banks.md) section.

:::info
The requirement to provide a binary for the alternate flash bank is designed to prevent corrupting the active flash bank
and accidentally create an unrecoverable state
which would require erasing and reflashing everything.
:::

After compiling a binary for the proper flash bank,
activate MSD mode by following the [instructions above](#enabling-msd-mode).

To begin flashing the binary,
simply drag and drop the binary onto the device listing for the USB Storage device associated with your board.
While flashing is in progress, a pop-up window will indicate the current progress of the firmware download.

<!-- ![Flashing in Progress](msdFlashInProgress.png) -->

Upon completion,
the board will automatically reboot itself into the new firmware which was just flashed.
This can be verified by opening the serial CLI and typing the `version` command again
and checking that the application is running from the alternate flash bank.
