# RFID Card Read/Write (Arduino + MFRC522)

This sketch demonstrates writing/reading multiple data types to a MIFARE Classic card using MFRC522.

Chapters implemented in-code with comments:
- CH2: Basic write flow
- CH3: Refactor to helpers `checkAuth`, `writeString`
- CH4: Read string `readString`
- CH5: Write integer `writeInteger`
- CH6: Read integer `readInteger`
- CH7: Composite struct `TagData` write/read across two blocks

Commands via Serial (9600 baud):
- `ws` → write string to block 60
- `wi` → write 16-bit integer to block 61
- `rs` → read string from block 60
- `ri` → read integer from block 61
- `wt` → write struct `TagData` to blocks 56–57
- `rt` → read struct `TagData` from blocks 56–57

Wiring
- `SS` (SDA) → D10
- `RST` → D9
- Uses hardware `SPI` pins for your board.

Dependencies
- Arduino library: `MFRC522`

Notes
- The sketch preserves earlier chapter code as comments with tags `[CH2]..[CH7]` for learning diff.
- Adjust block indices as needed based on your card layout.
