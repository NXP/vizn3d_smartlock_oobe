---
sidebar_position: 2
---

# Normal Boot

By default, if no other boot flags are set during the boot phase, the Normal boot mode is used.
During Normal boot, the bootloader will simply boot to the "main" application which is flashed at the current application bank flash address (see Application Banks for more information).
For example, if the current flash bank is set to Bank A, then the bootloader will jump to the flash address associated with Bank A and begin running the application at that address.

<!-- If Image Verification is enabled, Normal boot will also check that the image certificate for the firmware image to run has been signed by a trusted certificate authority to ensure that the application comes from a trusted source.
Should the signature check fail, the bootloader will not run the application in order to avoid executing untrusted, potentially malicious firmware.

For more details regarding image verification, see [Image Verification](../security/image-verification.md). -->
