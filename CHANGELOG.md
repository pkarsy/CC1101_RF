- 2021-05-27 Now getPacket adds a zero to the end of the received getPacket allowing for example the use of strcmp if we have textual data. The packet can still contain NULL characters so we cannot generally use strlen, but it is not needed as getPacket always return the packet size. Of course the zero at the end means we neet a buffer with one more byte, but using a buffer with 64 bytes as in the examples works.

- 2021-05-27 new function setSyncWord10(sync1,sync0) which makes it very clear which is sync1 and sync0. The old function setSyncWord(sync0, sync1) works but generates a warning. Note that the use of the function the old and the new is not a good idea as the default syncword is the best an syncword SHOULD NOT BE USED FOR PACKET FILTERING

- 2019-12-18 The project is created