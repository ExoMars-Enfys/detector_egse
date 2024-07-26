#include <Arduino.h>
#include <SPI.h>

const int cs_swir_dac = 10;
const int cs_mwir_dac = 9;
const int cs_adc = 8;

char inBuffer[20] = {};
const byte EOS = 0x0A; // the LF (line feed) character is used as an end-of-command-string indicator

// set up the speed, mode and endianness of each device
SPISettings settingsA(1000000, MSBFIRST, SPI_MODE0);

// sweep characteristics
const int swir_dac_min = 0;
const int swir_dac_max = 4095;
const int swir_dac_inc = 100;

const int mwir_dac_min = 0;
const int mwir_dac_max = 4095;
const int mwir_dac_inc = 100;

bool swir_sweep_en = false;
bool mwir_sweep_en = true;

// w - sweep short wave
// snumber - set DAC number
// k - sweep mid wave
// mnumber - set DAC number
// r - read ADC channels

// ------------------- Declare Internal Functions ----------------------------------------------------------------------

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

void readADC()
{
  int adc_val;
  Serial.println("Reading ADC");

  SPI.beginTransaction(settingsA);
  digitalWrite(cs_adc, LOW);
  SPI.transfer16(0);
  digitalWrite(cs_adc, HIGH);
  delayMicroseconds(50);

  for (int i = 1; i < 9; i++)
  {
    digitalWrite(cs_adc, LOW);
    adc_val = SPI.transfer16(i << 11);
    digitalWrite(cs_adc, HIGH);
    Serial.print(i - 1, DEC);
    Serial.print(": ");
    Serial.print(adc_val, HEX);
    Serial.print(", ");
    delayMicroseconds(50);
  }
  Serial.println();
  SPI.endTransaction();
}

// ------------------- Main UI Function --------------------------------------------------------------------------------

// void writeInstructions()
// TODO: Chloe to update for this EGSE.
// {
//   Serial.println("");
//   Serial.print("LGR Anode Board EGSE controller - Version ");
//   Serial.println(ver);
//   Serial.println("BJW - Mullard Space Science Laboratory");
//   Serial.println("'I' = Prints these instructions");
//   Serial.println("'L' = List all current settings");
//   Serial.println("'X' = Restore power-up settings");
//   Serial.println();
//   Serial.println("'TnXXX' = Set the DAC n where n = A, B, or C, and XXX is the 12-bit value (in decimal)");
//   Serial.println();
//   Serial.println("'R' = Read all the channel counters");
//   Serial.println("'C' = Clear all channel counters");
//   Serial.println("'E' = Enable all channel counters");
//   Serial.println("'D' = Disable all channel counters");
//   Serial.println();
//   Serial.println("'SA' = Initiate STIMs on side A");
//   Serial.println("'SC' = Initiate STIMs on side C");
//   Serial.println("'N'  = Stop STIM pulse generation");
//   Serial.println();
//   Serial.println("'F1' = Set frequency of STIM pulses to 5MHz");
//   Serial.println("'F2' = Set frequency of STIM  pulses to 500kHz");
//   Serial.println("'F3' = Set frequency of STIM pulses to 50kHz");
//   Serial.println("'F4' = Set frequency of STIM pulses to 5kHz");
//   Serial.println();
//   Serial.println("'A' = Initiate a read of the Anode board temperature");
//   Serial.println("'Wxxx'  = Start stream of anode temperature readings for xxx milliseconds (500 max.)");
//   Serial.println();
//   Serial.println("'Qxxxx' = Acquire counts for xxxx milliseconds (4095 max.)");
//   Serial.println("'Pxxxx' = Same as Q but automatically clears and reads counters.");
//   Serial.println("'Oxxxx' = Same as P but streams indefinitely until interrupted.");
//   Serial.println("'Kxxxx' = Initiate a sweep with steps set by J command and count for xxxx ms (4095 max.)");
//   Serial.println("'Jxxxx' = Set the step size for the automatic sweep [Default = 50].");
//   Serial.println("");
//   Serial.println("CR/LF (enter key) is the command string termination character");
//   Serial.println("");
//   return;
// }

void parseCommand()
{
  unsigned char temp = Serial.read();
  int DACwrite;

  switch (temp)
  {
  case 'w':
    swir_sweep_en = !swir_sweep_en;
    Serial.println("Toggled SWIR Sweep");
    break;

  case 'k':
    mwir_sweep_en = !mwir_sweep_en;
    Serial.println("Toggled MWIR Sweep");
    break;

  case 's':
    DACwrite = parseDAC();

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

    SPI.beginTransaction(settingsA);
    digitalWrite(cs_mwir_dac, LOW);
    SPI.transfer16(DACwrite);
    digitalWrite(cs_mwir_dac, HIGH);
    SPI.endTransaction();

    Serial.print("Set MWIR DAC to: ");
    Serial.println(DACwrite, DEC);
    break;

  case 'i':
    // write instructions
    break;

  case 'r':
    readADC();
    break;

  default:
    Serial.println("ERR: Invalid CMD selected");
    break;
  }
}

// --------------------------- Standard Functions ----------------------------------------------------------------------
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

  //
}

void loop()
{
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