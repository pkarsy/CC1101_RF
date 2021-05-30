- **2021-05-30** Now getPacket, sendPacket, automatically handle the address byte. sendPacket adds the address byte and getPacket removes it. See the example with address check. WARNING potential breaking change. If you have an older project and use addressCheck you can use the older branch or update your code

- **2021-05-27** Now getPacket() adds a zero to the end of the received data allowing for example the use of strcmp (if we have text). The packet can still contain NULL characters so we cannot generally use strlen, but getPacket always return the packet size. WARNING potential incompatibility. The buffer now needs an additional byte but if you used a 64 byte buffer(as in the examples), no changes are needed.

- **2021-05-27** New function setSyncWord10(sync1,sync0) which makes the order of sync1 and sync0 very clear. The old function setSyncWord(sync0, sync1) still works but generates a warning. Note that the use of both functions -the old and the new- is not a good idea as the default SyncWord is the best. The role of syncword if for packet detection and NOT FOR PACKET FILTERING.

- **2019-12-18** The project is created