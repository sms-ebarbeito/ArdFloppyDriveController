char *read_user_cmd(void *buffer, int buflen) {
  char *buf = (char *) buffer;
  byte l = 0;
  do {
      int i = Serial.read();

      if( (i==13 || i==10) ) { Serial.println(); break; }
      else if( i==27 ) { l=0; Serial.println(); break; }
      else if( i==8 ) { 
          if( l>0 ){ Serial.write(8); Serial.write(' '); Serial.write(8); l--; }
        }
      else if( isprint(i) && l<buflen-1 ) { buf[l++] = i; Serial.write(i); }
      
      if( motor_timeout>0 && millis() > motor_timeout ) { ArduinoFDC.motorOff(); motor_timeout = 0; }
    }
  while(true);

  while( l>0 && isspace(buf[l-1]) ) l--;
  buf[l] = 0;
  return buf;
}


void dump_buffer(int offset, byte *buf, int n) {
  int i = 0;
  while( i<n ) {
      print_hex((offset+i)/256); 
      print_hex((offset+i)&255); 
      Serial.write(':');

      for(int j=0; j<16; j++) {
          if( (j&7)==0  ) Serial.write(' ');
          if( i+j<n ) print_hex(buf[i+j]); else Serial.print(F("  "));
          Serial.write(' ');
        }

      Serial.write(' ');
      for(int j=0; j<16; j++) {
          if( (j&7)==0  ) Serial.write(' ');
          if( i+j<n ) Serial.write(isprint(buf[i+j]) ? buf[i+j] : '.'); else Serial.write(' ');
        }

      Serial.println();
      i += 16;
    }
}

void set_drive_type(int n) {
  ArduinoFDC.setDriveType((ArduinoFDCClass::DriveType) n);
  Serial.print(F("Setting disk type for drive ")); Serial.write('A'+ArduinoFDC.selectedDrive());
  Serial.print(F(" to ")); print_drive_type(ArduinoFDC.getDriveType());
  Serial.println();
}

byte go_track(byte track) {
  return ArduinoFDC.readSector(track, 0, 0, tempbuffer);
}
