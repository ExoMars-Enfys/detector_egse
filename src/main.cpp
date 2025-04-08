#include <Arduino.h>
#include <SPI.h>

const int ver = 5; // version number

// TODO: Handle line feed

// ------------------- Declare Constants -----------------------------------------------------------
// --- Configure pins
const int cs_swir_dac = 10;
const int cs_mwir_dac = 9;
const int cs_adc = 8;
const int adc_pwr = 23;

// --- Configure SPI settings
// ADC128
// SCLK is normally HIGH (CPOL=1)
// Data is sampled on rising edge of clock (CPHA=1)
// Therefore SPI Mode 3 is used, data is shifted out on falling SCLK and data is sampled on rising SCLK
SPISettings ADCsettings(1000000, MSBFIRST, SPI_MODE3);

// DAC121
// SCLK is normally LOW (CPOL=0)
// Data is sampled on falling edge of clock (CPHA=1)
// Therefore SPI Mode 1 is used, data is shifted out on rising SCLK and data is sampled on falling SCLK
SPISettings DACsettings(1000000, MSBFIRST, SPI_MODE1);

// -- Enfys ADC sampling characteristics
const int adc_discard = 100; // number of samples to discard
const int adc_average = 256; // number of samples to average

// -- Constants for program operation
char inBuffer[20] = {};
const byte EOS = 0x0A; // the LF (line feed) character is used as an end-of-command-string indicator

bool pwr_en = false;

// ------------------- Declare Internal Functions --------------------------------------------------

char *readSerialBuffer()
{
  int bytesRead = 0;
  int readLen = 20;

  while (bytesRead < readLen)
  { // get a serial command
    if (Serial.available() > 0)
    {
      inBuffer[bytesRead] = Serial.read();
      if (inBuffer[bytesRead] == EOS)
      {
        inBuffer[bytesRead] = '\0';
        break;
      }
      bytesRead++;
    }
  }

  return inBuffer;
}

int parseDAC()
{
  char *serialBuffer = readSerialBuffer();
  int DACval = atoi(serialBuffer);

  if (DACval > 4095)
  {
    DACval = 4095;
  }

  return DACval;
}

void readSingleSampleADC()
{
  int adc_val;
  Serial.println("Reading ADC");

  SPI.beginTransaction(ADCsettings);
  digitalWrite(cs_adc, LOW);
  SPI.transfer16(0);
  digitalWrite(cs_adc, HIGH);
  delayMicroseconds(50);

  for (int ch = 1; ch < 9; ch++)
  {
    digitalWrite(cs_adc, LOW);
    adc_val = SPI.transfer16(ch << 11);
    digitalWrite(cs_adc, HIGH);
    Serial.print(ch - 1, DEC);
    Serial.print(": ");
    Serial.print(adc_val, HEX);
    Serial.print(", ");
    delayMicroseconds(50);
  }
  Serial.println();
  SPI.endTransaction();
  return;
}

void readEnfysADC()
{
  long adc_val; // Long to store the cumalitive value of the ADC
  Serial.println("Reading Enfys ADC");
  Serial.print("Number to be discarded per channel: ");
  Serial.println(adc_discard, DEC);
  Serial.print("Number to average per channel: ");
  Serial.println(adc_average, DEC);

  SPI.beginTransaction(ADCsettings);
  digitalWrite(cs_adc, LOW);

  for (int ch = 1; ch < 9; ch++)
  {
    adc_val = 0; // Reset the ADC value for each channel
    for (int i = 0; i < adc_discard; i++)
    {
      SPI.transfer16(ch << 11);
    }

    for (int i = 0; i < adc_average; i++)
    {
      adc_val += SPI.transfer16(ch << 11);
    }

    Serial.print(ch - 1, DEC);
    Serial.print(": ");
    Serial.println(adc_val >> 8, HEX);
  }
  digitalWrite(cs_adc, HIGH);
  SPI.endTransaction();
  delayMicroseconds(50);
  Serial.println("ADC Read Complete");
  return;
}

// ------------------- Main UI Function ------------------------------------------------------------

void writeInstructions()
{
  Serial.println("");
  Serial.print("Enfys Detector EGSE controller - Version: ");
  Serial.println(ver);
  Serial.println("BJW - Mullard Space Science Laboratory");
  Serial.println("'i' = Prints these instructions");
  Serial.println();
  Serial.println("'p' = Toggle 3V3 power to ADC(on/off)");
  Serial.println("'r' = Read ADC channels using Enfys default sampling");
  Serial.println("'t' = Read a single sample of each of the ADC channels");

  Serial.println("'sXXXX' = Set SWIR DAC to XXXX (0-4095)");
  Serial.println("'mXXXX' = Set MWIR DAC to XXXX (0-4095)");
  Serial.println();
  Serial.println("'x' = Set both DACs back to 0");
  Serial.println();
  Serial.println();
  return;
}

void parseCommand()
{
  unsigned char temp = Serial.read();
  int DACwrite;

  switch (temp)
  {
  case 'i':
    writeInstructions();
    break;

  case 'p':
    pwr_en = !pwr_en;
    Serial.print("Toggle Power - ");
    Serial.println(pwr_en, DEC);
    break;

  case 'r':
    if (pwr_en == true)
    {
      readEnfysADC();
    }
    else
    {
      Serial.println("ERR: Power is off, please turn on 3V3 power before reading");
    }
    break;

  case 't':
    if (pwr_en == true)
    {
      readSingleSampleADC();
    }
    else
    {
      Serial.println("ERR: Power is off, please turn on power before reading");
    }
    break;

  case 's':
    if (pwr_en == true)
    {
      DACwrite = parseDAC();

      SPI.beginTransaction(DACsettings);
      digitalWrite(cs_swir_dac, LOW);
      SPI.transfer16(DACwrite);
      digitalWrite(cs_swir_dac, HIGH);
      SPI.endTransaction();

      Serial.print("Set SWIR DAC to: ");
      Serial.println(DACwrite, DEC);
    }
    else
    {
      Serial.println("ERR: Power is off, please turn on power before reading");
    }
    break;

  case 'm':
    if (pwr_en == true)
    {
      DACwrite = parseDAC();

      SPI.beginTransaction(DACsettings);
      digitalWrite(cs_mwir_dac, LOW);
      SPI.transfer16(DACwrite);
      digitalWrite(cs_mwir_dac, HIGH);
      SPI.endTransaction();

      Serial.print("Set MWIR DAC to: ");
      Serial.println(DACwrite, DEC);
    }
    else
    {
      Serial.println("ERR: Power is off, please turn on power before reading");
    }
    break;

  case 'x':
    if (pwr_en == true)
    {
      DACwrite = 0;

      SPI.beginTransaction(DACsettings);
      digitalWrite(cs_swir_dac, LOW);
      SPI.transfer16(DACwrite);
      digitalWrite(cs_swir_dac, HIGH);
      SPI.endTransaction();

      Serial.print("Set SWIR DAC to: ");
      Serial.println(DACwrite, DEC);

      SPI.beginTransaction(DACsettings);
      digitalWrite(cs_mwir_dac, LOW);
      SPI.transfer16(DACwrite);
      digitalWrite(cs_mwir_dac, HIGH);
      SPI.endTransaction();

      Serial.print("Set MWIR DAC to: ");
      Serial.println(DACwrite, DEC);
    }
    else
    {
      Serial.println("ERR: Power is off, please turn on power before reading");
    }
    break;

  default:
    Serial.println("ERR: Invalid CMD selected");
    break;
  }

  // Clear serial buffer
  while (Serial.available() > 0)
  {
    Serial.read();
  }
  return;
}

// --------------------------- Standard Functions --------------------------------------------------
void setup()
{
  // Configure SPI pins
  pinMode(cs_swir_dac, OUTPUT);
  pinMode(cs_mwir_dac, OUTPUT);
  pinMode(cs_adc, OUTPUT);

  // Configure CS pins
  digitalWrite(cs_swir_dac, LOW);
  digitalWrite(cs_mwir_dac, LOW);
  digitalWrite(cs_adc, LOW);
  digitalWrite(adc_pwr, LOW);

  writeInstructions();

  SPI.begin();
}

void loop()
{
  if (pwr_en == true)
  {
    digitalWrite(cs_swir_dac, HIGH);
    digitalWrite(cs_mwir_dac, HIGH);
    digitalWrite(cs_adc, HIGH);
    digitalWrite(adc_pwr, HIGH);
  }
  else
  {
    digitalWrite(cs_swir_dac, LOW);
    digitalWrite(cs_mwir_dac, LOW);
    digitalWrite(cs_adc, LOW);
    digitalWrite(adc_pwr, LOW);
  }

  if (Serial.available() > 0)
  {
    parseCommand();
  }
  else
  {
    delay(100);
  }
}