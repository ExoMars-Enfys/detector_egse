#include <Arduino.h>
#include <SPI.h>

const int ver = 1; // version number

const int cs_swir_dac = 10;
const int cs_mwir_dac = 9;
const int cs_adc = 8;
const int adc_pwr = 23;

char inBuffer[20] = {};
const byte EOS = 0x0A; // the LF (line feed) character is used as an end-of-command-string indicator

// set up the speed, mode and endianness of each device
SPISettings settingsA(1000000, MSBFIRST, SPI_MODE3);

// Enfys ADC characteristics
const int adc_discard = 128; // number of samples to discard
const int adc_average = 256; // number of samples to average
// sweep characteristics
const int swir_dac_min = 0;
const int swir_dac_max = 4095;
const int swir_dac_inc = 100;

const int mwir_dac_min = 0;
const int mwir_dac_max = 4095;
const int mwir_dac_inc = 100;

bool swir_sweep_en = false;
bool mwir_sweep_en = false;
bool pwr_en = false;

// w - sweep short wave
// snumber - set DAC number
// k - sweep mid wave
// mnumber - set DAC number
// r - read ADC channels

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

void sweepSWIR()
{
  // TODO: Chloe create a function that sweeps between min, and max constants at the top of the page, using the increment
  // Previous code:
  for (int value = 0; value < 4095; value += 100)
  {
    digitalWrite(cs_swir_dac, LOW);
    SPI.transfer16(value);
    digitalWrite(cs_swir_dac, HIGH);
    delay(50);
  }
}

void sweepMWIR()
{
  // TODO: Chloe create a function that sweeps between min, and max constants at the top of the page, using the increment
  for (int value = 0; value < 4095; value += 100)
  {
    digitalWrite(cs_mwir_dac, LOW);
    SPI.transfer16(value);
    digitalWrite(cs_mwir_dac, HIGH);
    delay(50);
  }
}

void readSingleSampleADC()
{
  int adc_val;
  Serial.println("Reading ADC");

  SPI.begin();
  SPI.beginTransaction(settingsA);
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
}

void readEnfysADC()
{
  long adc_val; // Long to store the cumalitive value of the ADC
  Serial.println("Reading Enfys ADC");
  Serial.print("Number to be discarded per channel: ");
  Serial.println(adc_discard, DEC);
  Serial.print("Number to average per channel: ");
  Serial.println(adc_average, DEC);

  SPI.begin();
  SPI.beginTransaction(settingsA);
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
}

// ------------------- Main UI Function --------------------------------------------------------------------------------

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
    readEnfysADC();
    break;

  case 't':
    readSingleSampleADC();
    break;

  case 's':
    DACwrite = parseDAC();

    SPI.begin();
    SPI.beginTransaction(settingsA);
    digitalWrite(cs_swir_dac, LOW);
    SPI.transfer16(DACwrite);
    digitalWrite(cs_swir_dac, HIGH);
    SPI.endTransaction();

    Serial.print("Set SWIR DAC to: ");
    Serial.println(DACwrite, DEC);
    break;

  case 'm':
    DACwrite = parseDAC();

    SPI.begin();
    SPI.beginTransaction(settingsA);
    digitalWrite(cs_mwir_dac, LOW);
    SPI.transfer16(DACwrite);
    digitalWrite(cs_mwir_dac, HIGH);
    SPI.endTransaction();

    Serial.print("Set MWIR DAC to: ");
    Serial.println(DACwrite, DEC);
    break;

  case 'x':
    DACwrite = 0;

    SPI.begin();
    SPI.beginTransaction(settingsA);
    digitalWrite(cs_swir_dac, LOW);
    SPI.transfer16(DACwrite);
    digitalWrite(cs_swir_dac, HIGH);
    SPI.endTransaction();

    Serial.print("Set SWIR DAC to: ");
    Serial.println(DACwrite, DEC);

    SPI.begin();
    SPI.beginTransaction(settingsA);
    digitalWrite(cs_mwir_dac, LOW);
    SPI.transfer16(DACwrite);
    digitalWrite(cs_mwir_dac, HIGH);
    SPI.endTransaction();

    Serial.print("Set MWIR DAC to: ");
    Serial.println(DACwrite, DEC);
    break;

  case 'w':
    swir_sweep_en = !swir_sweep_en;
    Serial.println("Toggled SWIR Sweep");
    break;

  case 'k':
    mwir_sweep_en = !mwir_sweep_en;
    Serial.println("Toggled MWIR Sweep");
    break;

  default:
    Serial.println("ERR: Invalid CMD selected");
    break;
  }
}

// --------------------------- Standard Functions ----------------------------------------------------------------------
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
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
  }

  if (swir_sweep_en == true)
  {
    // Then run the sweepSWIR function
    sweepSWIR();
  }

  if (mwir_sweep_en == true)
  {
    // Then run the sweepMWIR function
    sweepMWIR();
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