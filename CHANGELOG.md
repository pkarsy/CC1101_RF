- 2021-05-27 Now getPacket() adds a zero to the end of the received data allowing for example the use of strcmp (if we have text). The packet can still contain NULL characters so we cannot generally use strlen, but getPacket always return the packet size

- 2021-05-27 new function setSyncWord10(sync1,sync0) which makes it very clear which is sync1 and sync0. The old function setSyncWord(sync0, sync1) works but generates a warning. Note that the use of the function the old and the new is not a good idea as the default syncword is the best an syncword SHOULD NOT BE USED FOR PACKET FILTERING

- 2019-12-18 The project is created