#include <Arduino.h>
#include <SPI.h>

const int cs_swir_dac = 10;
const int cs_mwir_dac = 9;
const int cs_adc = 8;

char inBuffer[20] = {};
const byte EOS = 0x0A;         // the LF (line feed) character is used as an end-of-command-string indicator
unsigned int tempDAC = 0x0000; // current setting for the DAC value

// set up the speed, mode and endianness of each device
SPISettings settingsA(1000000, MSBFIRST, SPI_MODE1);

// w - sweep short wave
// snumber - set DAC number
// k - sweep mid wave
// mnumber - set DAC number
// r - read ADC channels

// ------------------- Declare Internal Functions ------------------------------

// char *readSerialBuffer()
// {
//   int bytesRead = 0;
//   int readLen = 20;

//   while (bytesRead < readLen)
//   { // get a serial command
//     if (Serial.available() > 0)
//     {
//       inBuffer[bytesRead] = Serial.read();
//       if (inBuffer[bytesRead] == EOS)
//       {
//         inBuffer[bytesRead] = '\0';
//         break;
//       }
//       bytesRead++;
//     }
//   }

//   return inBuffer;
// }

// void parseDAC()
// {
//   char *serialBuffer = readSerialBuffer();
//   char serDAC = serialBuffer[0];
//   serialBuffer[0] = ' ';

//   tempDAC = atoi(serialBuffer);

//   return;
// }

// void parseCommand()
// {
//   unsigned char temp = Serial.read();

//   switch (temp)
//   {
//   case 'w':
//     SPI.beginTransaction(settingsA);
//     for (int value = 0; value < 4095; value += 100)
//     {
//       digitalWrite(cs_swir_dac, LOW);
//       SPI.transfer16(value);
//       digitalWrite(cs_swir_dac, HIGH);
//       delay(100);
//     }
//     SPI.endTransaction();
//     break;

//   case 'k':
//     SPI.beginTransaction(settingsA);
//     for (int value = 0; value < 4095; value += 100)
//     {
//       digitalWrite(cs_mwir_dac, LOW);
//       SPI.transfer16(value);
//       digitalWrite(cs_mwir_dac, HIGH);
//       delay(100);
//     }
//     SPI.endTransaction();
//     break;

//   case 's':
//     parseDAC();
//     break;

//   case 't':
//     parseDAC();
//     break;
//   }
// }

// --------------------------- Standard Functions ------------------------------
void setup()
{
  pinMode(cs_swir_dac, OUTPUT);
  pinMode(cs_mwir_dac, OUTPUT);
  pinMode(cs_adc, OUTPUT);

  digitalWrite(cs_swir_dac, HIGH);
  digitalWrite(cs_mwir_dac, HIGH);
  digitalWrite(cs_adc, HIGH);

  SPI.begin();
  SPI.beginTransaction(settingsA);
}

void loop()
{
  SPI.beginTransaction(settingsA);
  // for (int value = 1000; value < 2000; value += 50)
  // {
  //   digitalWrite(cs_swir_dac, LOW);
  //   SPI.transfer16(value);
  //   digitalWrite(cs_swir_dac, HIGH);
  //   delay(100);
  // }
  int value = 1800;
  digitalWrite(cs_swir_dac, LOW);
  SPI.transfer16(value);
  digitalWrite(cs_swir_dac, HIGH);
  delay(1000);

  value = 480;
  digitalWrite(cs_mwir_dac, LOW);
  SPI.transfer16(value);
  digitalWrite(cs_mwir_dac, HIGH);
  delay(200);
}

// void loop()
// {
// int value = 1870;
// digitalWrite(cs_swir_dac, LOW);
// SPI.transfer16(value);
// digitalWrite(cs_swir_dac, HIGH);

// value = 480;
// digitalWrite(cs_mwir_dac, LOW);
// SPI.transfer16(value);
// digitalWrite(cs_mwir_dac, HIGH);
// delay(200);

// if (Serial.available() > 0)
// {
//   parseCommand();
// }
// else
// {
//   delay(100);
// }
// }