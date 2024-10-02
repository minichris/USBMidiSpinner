#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// Smooth buttons and potentiometers, see the libraries for examples on how to use them.
//#include <Bounce2.h>
//#include <ResponsiveAnalogRead.h>

const int HallAMax = 2100;
const int HallAMin = 2040;
const int HallBMax = 2040;
const int HallBMin = 1980;

// USB MIDI object
Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

void setup() {
  Serial.begin(115200);

  TinyUSBDevice.setManufacturerDescriptor("Christopher Lee");
  TinyUSBDevice.setProductDescriptor("Fidget 2000");

  usbMidi.begin();
  MIDI.begin();

  pinMode(A2, INPUT);
  pinMode(A1, INPUT);

  pinMode(10, INPUT_PULLDOWN);
  pinMode(11, INPUT_PULLDOWN);

  pinMode(22, OUTPUT_4MA);
  digitalWrite(22, HIGH);

  pinMode(21, OUTPUT_4MA);
  digitalWrite(21, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  analogReadResolution(12);
}

int MapToMidi(int input, int inputMin, int inputMax) {
  return min(127, max(0, map(input, inputMin, inputMax, 0, 127)));
}

int LastSentMidiA = -255;
int LastSentMidiB = -255;
int MidiA, MidiB;
int analogA;
int analogB;



bool WaitingForDrop = false;
bool BReachedMax = false;
void PickupAndDropMode(){
    if(!WaitingForDrop){
		Serial.print("   Rising, waiting for drop...");
		if (MidiA != LastSentMidiA) {
			MIDI.send(midi::ControlChange, 1, MidiA, 12U);
			LastSentMidiA = MidiA;
				if(LastSentMidiA > 120){
					WaitingForDrop = true;
				}
		}
	}
	else{
		Serial.print("   dropping, waiting for rise...");
		if (MidiB != LastSentMidiB) {
			if(MidiB > 120){
				BReachedMax = true;
			}
			if(BReachedMax == true){
				MIDI.send(midi::ControlChange, 1, MidiB, 12U);
				LastSentMidiB = MidiB;
				if(LastSentMidiB < 5){
				BReachedMax = false;
				WaitingForDrop = false;
				}
			}		
		}
	}
}

void SendTogether(){ //sends both pickups on the same channel
  if (MidiA != LastSentMidiA) {
    MIDI.send(midi::ControlChange, 1, MidiA, 12U);
    LastSentMidiA = MidiA;
  }

  if (MidiB != LastSentMidiB) {
    MIDI.send(midi::ControlChange, 1, MidiB, 12U);
    LastSentMidiB = MidiB;
  }
}

void SendSeperately(){ //sends each pickup as its own channel
  if (MidiA != LastSentMidiA) {
    MIDI.send(midi::ControlChange, 1, MidiA, 12U);
    LastSentMidiA = MidiA;
  }

  if (MidiB != LastSentMidiB) {
    MIDI.send(midi::ControlChange, 1, MidiB, 14U);
    LastSentMidiB = MidiB;
  }
}

auto pastLEDState = true;
int Mode = -1;
void loop() {
  analogA = analogRead(A2);
  analogB = analogRead(A1);
  MidiA = MapToMidi(analogA, HallAMin, HallAMax);
  MidiB = MapToMidi(analogB, HallBMin, HallBMax);
  Serial.print("\nAnalog A value: ");
  Serial.print(analogA);
  Serial.print("\nAnalog B value: ");
  Serial.print(analogB);
  Serial.print("\nMapped A value: ");
  Serial.print(MidiA);
  Serial.print("\nMapped B value: ");
  Serial.print(MidiB);
  Serial.print("\nLast sent channel A: ");
  Serial.print(LastSentMidiA);
  Serial.print("\nLast sent channel B: ");
  Serial.print(LastSentMidiB);

  if(HIGH == digitalRead(10)){
	Mode = 0;
  }
  else if(HIGH == digitalRead(11)){
	Mode = 1;
  }
  else{
	Mode = 2;
  }
  
  switch(Mode){
	case 0:
	PickupAndDropMode();
	Serial.println("\nPickupAndDropMode");
	break;
	case 1:
	SendTogether();
	Serial.println("\nSendTogether");
	break;
	case 2:
	SendSeperately();
	Serial.println("\nSendSeperately");
	break;
  }

  delay(10);
}
