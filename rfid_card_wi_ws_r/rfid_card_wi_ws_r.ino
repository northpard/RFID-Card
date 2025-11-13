#include <SPI.h>
#include <MFRC522.h>

const int RST_PIN = 9;
const int SS_PIN = 10;
MFRC522 rc522(SS_PIN, RST_PIN);

// [CH7] composite data structure to store on tag
struct TagData
{
  char name[16];
  long total;
  long payment;
};

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  SPI.begin();
  rc522.PCD_Init();

  Serial.println("start!");
}

void loop()
{
  // input command
  String cmd = "";
  while (Serial.available() > 0)
  {
    cmd = Serial.readStringUntil('\n');
  }

  if (!rc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rc522.PICC_ReadCardSerial()) {
    return;
  }

  // [CH2] direct handling of 'w' command and write
  // if (cmd.length() > 0 && cmd == "w")
  // {
  //   Serial.print("cmd : ");
  //   Serial.println(cmd);
  // } else { return; }
  // const int index = 60; // block index
  // MFRC522::StatusCode status;
  // // set key value
  // MFRC522::MIFARE_Key key;
  // for (int i = 0; i < 6; i++) { key.keyByte[i] = 0xFF; }
  // // check auth
  // status = rc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, index, &key, &(rc522.uid));
  // if (status != MFRC522::STATUS_OK) {
  //   Serial.print("Authentication Failed : ");
  //   Serial.println(rc522.GetStatusCodeName(status));
  //   return;
  // }
  // // define buffer
  // char data[16];
  // memset(data, 0x00, sizeof(data));
  // // string to char array
  // String name = "nomaefg";
  // name.toCharArray(data, name.length() + 1);
  // // write data
  // status = rc522.MIFARE_Write(index, (byte*)&data, 16);
  // if (status != MFRC522::STATUS_OK) {
  //   Serial.print("Write Failed : ");
  //   Serial.println(rc522.GetStatusCodeName(status));
  //   return;
  // }
  // rc522.PICC_DumpToSerial(&(rc522.uid));
  // delay(100);

  // [CH3] refactored: build key, switch by command, call writeString()
  MFRC522::MIFARE_Key key;
  for (int i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  MFRC522::StatusCode status;
  // [CH6] temp holders for read data
  String s_data; // string data
  int i_data;    // integer data
  // [CH7] temp holder for struct write
  TagData t_data;
  String s_temp;
  if (cmd.length() > 0)
  {
    Serial.print("cmd : ");

    // [CH3] original: switch using index operator
    // switch (cmd[0])
    // {
    //   case 'w':
    //     Serial.println("write");
    //     status = writeString(60, key, "nomaefg");
    //     break;
    //   default:
    //     Serial.println("unknown");
    //     status = MFRC522::STATUS_ERROR;
    //     break;
    // }

    // [CH4] updated: use charAt, add 'r' to read
    switch (cmd.charAt(0))
    {
      case 'w':
        // [CH4] original write branch
        // Serial.println("write");
        // status = writeString(60, key, "nomaefg");
        // break;

        // [CH5] support subtype: ws (string) / wi (integer)
        Serial.print("write ");
        switch (cmd.charAt(1))
        {
          case 's':
            Serial.println("string");
            status = writeString(60, key, "nomaefg");
            break;
          case 'i':
            Serial.println("integer");
            status = writeInteger(61, key, 32767);
            rc522.PICC_DumpToSerial(&(rc522.uid));
            break;
          case 't':
            Serial.println("struct");
            s_temp = "nomaefg";
            s_temp.toCharArray(t_data.name, s_temp.length() + 1);
            t_data.total = -2147483647L;
            t_data.payment = 2000000000L;
            status = writeTagData(56, key, t_data);
            rc522.PICC_DumpToSerial(&(rc522.uid));
            break;
          default:
            Serial.println("unknown type");
            status = MFRC522::STATUS_ERROR;
            break;
        }
        break;
      case 'r':
        // [CH4] original simple read string branch
        // Serial.println("read");
        // {
        //   String data;
        //   status = readString(60, key, data);
        //   Serial.println(data);
        // }
        // break;

        // [CH6] support subtype: rs (string) / ri (integer)
        Serial.print("read ");
        switch (cmd.charAt(1))
        {
          case 's':
            Serial.println("string");
            status = readString(60, key, s_data);
            Serial.println(s_data);
            break;
          case 'i':
            Serial.println("integer");
            status = readInteger(61, key, i_data);
            Serial.println(i_data);
            break;
          case 't':
            // [CH7] read composite struct across two blocks
            Serial.println("struct");
            status = readTagData(56, key, t_data);
            Serial.print("name : "); Serial.println(String(t_data.name));
            Serial.print("total : "); Serial.println(t_data.total);
            Serial.print("payment : "); Serial.println(t_data.payment);
            break;
          default:
            Serial.println("unknown type");
            status = MFRC522::STATUS_ERROR;
            break;
        }
        break;
      default:
        Serial.println("unknown");
        status = MFRC522::STATUS_ERROR;
        break;
    }

    if (status == MFRC522::STATUS_OK)
    {
      Serial.println("success!");
    }
  }
}

// [CH3] helper: authenticate given block with provided key
MFRC522::StatusCode checkAuth(int index, MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status =
      rc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, index, &key, &(rc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication Failed : ");
    Serial.println(rc522.GetStatusCodeName(status));
  }

  return status;
}

// [CH5] helper: encode 16-bit int into buffer (little-endian)
void toBytes(byte* buffer, int data, int offset = 0)
{
  buffer[offset] = data & 0xFF;
  buffer[offset + 1] = (data >> 8) & 0xFF;
}

// [CH6] helper: decode 16-bit int from buffer (little-endian)
int toInteger(byte* buffer, int offset = 0)
{
  return ((buffer[offset + 1] << 8) | buffer[offset]);
}

// [CH5] helper: write integer value to a block
MFRC522::StatusCode writeInteger(int index, MFRC522::MIFARE_Key key, int data)
{
  // check auth
  MFRC522::StatusCode status = checkAuth(index, key);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }

  // write integer
  byte buffer[16];
  memset(buffer, 0x00, sizeof(buffer));
  toBytes(buffer, data);

  status = rc522.MIFARE_Write(index, buffer, sizeof(buffer));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Write Failed : ");
    Serial.println(rc522.GetStatusCodeName(status));
  }

  return status;
}

// [CH7] helper: write composite TagData across two consecutive blocks
MFRC522::StatusCode writeTagData(int index, MFRC522::MIFARE_Key key, TagData data)
{
  // check auth
  MFRC522::StatusCode status = checkAuth(index, key);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }

  // write struct in two 16-byte chunks
  byte buffer[32];
  memset(buffer, 0x00, sizeof(buffer));
  memcpy(buffer, &data, sizeof(data));

  for (int i = 0; i < 2; i++)
  {
    status = rc522.MIFARE_Write(index + i, buffer + (i * 16), 16); // sizeof(data)
    if (status != MFRC522::STATUS_OK)
    {
      Serial.print("Write Failed : ");
      Serial.println(rc522.GetStatusCodeName(status));
    }
  }

  return status;
}

// [CH7] helper: read composite TagData from two consecutive blocks
MFRC522::StatusCode readTagData(int index, MFRC522::MIFARE_Key key, TagData& data)
{
  // check auth
  MFRC522::StatusCode status = checkAuth(index, key);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }

  // read data
  byte buffer[34];
  byte length = 18;

  for (int i = 0; i < 2; i++)
  {
    status = rc522.MIFARE_Read(index + i, buffer + (i * 16), &length);
    if (status != MFRC522::STATUS_OK)
    {
      Serial.print("Read Failed : ");
      Serial.println(rc522.GetStatusCodeName(status));
    }
  }

  memcpy(&data, buffer, sizeof(data));
  return status;
}

// [CH6] helper: read integer from a block
MFRC522::StatusCode readInteger(int index, MFRC522::MIFARE_Key key, int& data)
{
  // check auth
  MFRC522::StatusCode status = checkAuth(index, key);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }

  // read data
  byte buffer[18];
  byte length = 18;

  status = rc522.MIFARE_Read(index, buffer, &length);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Read Failed : ");
    Serial.println(rc522.GetStatusCodeName(status));
  }
  else
  {
    data = toInteger(buffer);
  }

  return status;
}

// [CH3] helper: write a String to a block
MFRC522::StatusCode writeString(int index, MFRC522::MIFARE_Key key, String data)
{
  // check auth
  MFRC522::StatusCode status = checkAuth(index, key);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }

  // convert string to char array
  char buffer[16];
  memset(buffer, 0x00, sizeof(buffer));
  data.toCharArray(buffer, data.length() + 1);

  // write data
  status = rc522.MIFARE_Write(index, (byte*)&buffer, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Write Failed : ");
    Serial.println(rc522.GetStatusCodeName(status));
  }

  return status;
}

// [CH4] helper: read a String from a block
MFRC522::StatusCode readString(int index, MFRC522::MIFARE_Key key, String& data)
{
  // check auth
  MFRC522::StatusCode status = checkAuth(index, key);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }

  // read data
  byte buffer[18];
  byte length = 18;

  status = rc522.MIFARE_Read(index, buffer, &length);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Read Failed : ");
    Serial.println(rc522.GetStatusCodeName(status));
  }
  else
  {
    data = String((char*)buffer);
  }

  return status;
}
