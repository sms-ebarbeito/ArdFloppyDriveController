
#include "ArduinoFDC.h"
#include "ff.h"

#define USE_MONITOR

#define TEMPBUFFER_SIZE 80
byte tempbuffer[TEMPBUFFER_SIZE];
static byte databuffer[516];
unsigned long motor_timeout = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");
  ArduinoFDC.begin(ArduinoFDCClass::DT_3_DD, ArduinoFDCClass::DT_3_DD);

  Serial.print(F("Drive A: ")); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
  if( ArduinoFDC.selectDrive(1) ) {
      Serial.print(F("Drive B: ")); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
      ArduinoFDC.selectDrive(0);
  }

  //---------------------------------------------------------------------//
  // INIT DEMO -> red first sector of all tracks and return to track 0
  //---------------------------------------------------------------------//
  ArduinoFDC.motorOn();
  ArduinoFDC.selectDrive(0);
  byte tracks = ArduinoFDC.numTracks();
  byte sectors = ArduinoFDC.numSectors();

  for (int track = 0; track < tracks; track++) {
    byte result = ArduinoFDC.readSector(track, 0, 0, tempbuffer) ;
    print_error(result);
  }
  go_track(0);
  
}

void loop() {
    monitor();
}
