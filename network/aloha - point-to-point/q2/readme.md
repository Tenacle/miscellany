# A1 point-to-point communication
Most of written stuff here are theoretical. Implementation is messy and incomplete. I've built the data and functions but didn't fully combine/implement them together.


## Re: [10pts] Byte stuffing with flags (implemented)
Max frame size is 1024 as specified in `physical.h`. Contents of a frame is as follows:

`Preamble | Ctrl | Info | FCS | Postamble`

| Name          | Size (bytes) | Contents                                         |
|---------------|--------------|--------------------------------------------------|
| Preamble      | 2 bytes      | 0x7EA5                                           |
| Ctrl          | 2 bytes      | type, P/F, SS, Tx/Rx seqno            |
| Info          | 1017 bytes   | Data                                             |
| FCS           | 1 byte       | counts ctrl & info bits and keeps the total even |
| Postamble     | 2 bytes      | 0x7EA6                                           |

### Ctrl
each section is 4 bits in size.

Two types:
1. Information (0x0):  `Type | P/F | TxSeqno| RxSeqno`
   - P/F is for polling(0x0) or final(0xF). Defaults to final as to declare there is nothing to inquire. Polling is to signal the Tx to transmit held any held frames.
   - TxSeqno is the current frame seqno
   - RxSeqno is the current frame expected

2. Supervision (0x8):  `Type | P/F | SS | RxSeqno`
   - SS are special instructions to the other peer.
     - ACK (0x1)  - checks RxSeqno for the NFE
     - NAK (0x2)  - checks RxSeqno for the NFE

## Re: [10pts] Selective repeat flow control (not implemented)
* 4-bit sequence number
  - Found in the 2-byte ctrl section of the frame.
* Window size of 8 frames
## Re: [10pts] Timeout
Fixed timeout of 5ms as per requirements.
## Re: Errors and logging (no errors and logging)
Any errors and retransmissions will be logged.
* Checksum
  - Simple checksum using bit counting. Keeps the bits even by adding 0x1 to the FCS if bit count is odd.
* Stuffing (malformed frames)
  - Drops the frame when a single 0x7E is found (not stuffed) or when a preamble is found without a postamble.
* Timeouts
  - Retransmits the frame when timeout is reached.
## Re: [5pts] Physical layer (somewhat done)
To my utmost ability, proper use of mutex and conditions are used. Mutex and conditions provided by `physical.h` are used.
## Re: [5pts] State machine
State machine (hopefully correct/enough): ***(TODO)***
