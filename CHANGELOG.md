- **2024-01-26** A lot of changes and code cleanup.
The register settings now require a preamble before accepting a SyncWord. This means that the false pakets practically are eliminated.
The GDO0 pin now asserts when a syncword is received. The reason for this INCOMPATIBLE change is that with the old settings it was possible for the receiver to exit RX/WoR without reporting this to GDO0 pin
This can lead to high current consumption and/or inability to receive more packets effectivly disabling the node. See the examples for the recommended usage.

- **2024-01-25** New code revision 0.7.4 (just before the incompatible changes related to GDO0)

- **2023-12-21** begin() checks is CC1101 is connected and returns true/false accordingly

- **2022-03-07** Typo fixes

- **2021-07-04** New convenience function whitening(true/false). The default is true

- **2021-06-10** Code cleanup. The examples are converted to platformio format

- **2021-05-27** New function setSyncWord10(sync1,sync0) which makes the order of sync1 and sync0 clear. The old function setSyncWord(sync0, sync1) still works. Note that the use of theese functions is not a good idea as the default SyncWord is the best. The role of syncword if for packet detection and NOT FOR PACKET FILTERING.

- **2019-12-18** The project is created