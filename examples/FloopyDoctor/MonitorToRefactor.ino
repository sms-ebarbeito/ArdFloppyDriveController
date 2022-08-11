
void print_error(byte n) {
  Serial.print(F("Error: "));
  switch( n ) {
    case S_OK        : Serial.print(F("No error")); break;
    case S_NOTINIT   : Serial.print(F("ArduinoFDC.begin() was not called")); break;
    case S_NOTREADY  : Serial.print(F("Drive not ready")); break;
    case S_NOSYNC    : Serial.print(F("No sync marks found")); break;
    case S_NOHEADER  : Serial.print(F("Sector header not found")); break;
    case S_INVALIDID : Serial.print(F("Data record has unexpected id")); break;
    case S_CRC       : Serial.print(F("Data checksum error")); break;
    case S_NOTRACK0  : Serial.print(F("No track 0 signal detected")); break;
    case S_VERIFY    : Serial.print(F("Verify after write failed")); break;
    case S_READONLY  : Serial.print(F("Disk is write protected")); break;
    default          : Serial.print(F("Unknonwn error")); break;
    }
  Serial.println('!');
}

void monitor() 
{
  char cmd;
  int a1, a2, a3, head, track, sector, n;
  
  ArduinoFDC.motorOn();
  while( true )
    {
      Serial.print(F("\r\n\r\nCommand: "));
      n = sscanf(read_user_cmd(tempbuffer, 512), "%c%i,%i,%i", &cmd, &a1, &a2, &a3);
      if( n<=0 || isspace(cmd) ) continue;

      if( cmd=='r' && n>=3 )
        {
          track=a1; sector=a2; head= (n==3) ? 0 : a3;
          if( head>=0 && head<2 && track>=0 && track<ArduinoFDC.numTracks() && sector>=1 && sector<=ArduinoFDC.numSectors() )
            {
              Serial.print(F("Reading track ")); Serial.print(track); 
              Serial.print(F(" sector ")); Serial.print(sector);
              Serial.print(F(" side ")); Serial.println(head);
              Serial.flush();

              byte status = ArduinoFDC.readSector(track, head, sector, databuffer);
              if( status==S_OK )
                {
                  dump_buffer(0, databuffer+1, 512);
                  Serial.println();
                }
              else
                print_error(status);
            }
          else
            Serial.println(F("Invalid sector specification"));
        }
      else if( cmd=='r' && n==1 )
        {
          ArduinoFDC.motorOn();
          for(track=0; track<ArduinoFDC.numTracks(); track++)
            for(head=0; head<2; head++)
              {
                sector = 1;
                for(byte i=0; i<ArduinoFDC.numSectors(); i++)
                  {
                    byte attempts = 0;
                    while( true )
                      {
                        Serial.print(F("Reading track ")); Serial.print(track); 
                        Serial.print(F(" sector ")); Serial.print(sector);
                        Serial.print(F(" side ")); Serial.print(head);
                        Serial.flush();
                        byte status = ArduinoFDC.readSector(track, head, sector, databuffer);
                        if( status==S_OK )
                          {
                            Serial.println(F(" => ok"));
                            break;
                          }
                        else if( (status==S_INVALIDID || status==S_CRC) && (attempts++ < 10) )
                          Serial.println(F(" => CRC error, trying again"));
                        else
                          {
                            Serial.print(F(" => "));
                            print_error(status);
                            break;
                          }
                      }

                    sector+=2;
                    if( sector>ArduinoFDC.numSectors() ) sector = 2;
                  }
              }
        }
      else if( cmd=='w' && n>=3 )
        {
          track=a1; sector=a2; head= (n==3) ? 0 : a3;
          if( head>=0 && head<2 && track>=0 && track<ArduinoFDC.numTracks() && sector>=1 && sector<=ArduinoFDC.numSectors() )
            {
              Serial.print(F("Writing and verifying track ")); Serial.print(track); 
              Serial.print(F(" sector ")); Serial.print(sector);
              Serial.print(F(" side ")); Serial.println(head);
              Serial.flush();
          
              byte status = ArduinoFDC.writeSector(track, head, sector, databuffer, true);
              if( status==S_OK )
                Serial.println(F("Ok."));
              else
                print_error(status);
            }
          else
            Serial.println(F("Invalid sector specification"));
        }
      else if( cmd=='w' && n>=1 )
        {
          bool verify = n>1 && a2>0;
          char c;
          Serial.print(F("Write current buffer to all sectors in drive "));
          Serial.write('A' + ArduinoFDC.selectedDrive());
          Serial.println(F(". Continue (y/n)?"));
          while( (c=Serial.read())<0 );
          if( c=='y' )
            {
              ArduinoFDC.motorOn();
              for(track=0; track<ArduinoFDC.numTracks(); track++)
                for(head=0; head<2; head++)
                  {
                    sector = 1;
                    for(byte i=0; i<ArduinoFDC.numSectors(); i++)
                      {
                        Serial.print(verify ? F("Writing and verifying track ") : F("Writing track ")); Serial.print(track);
                        Serial.print(F(" sector ")); Serial.print(sector);
                        Serial.print(F(" side ")); Serial.print(head);
                        Serial.flush();
                        byte status = ArduinoFDC.writeSector(track, head, sector, databuffer, verify);
                        if( status==S_OK )
                          Serial.println(F(" => ok"));
                        else
                          {
                            Serial.print(F(" => "));
                            print_error(status);
                          }

                        sector+=2;
                        if( sector>ArduinoFDC.numSectors() ) sector = 2;
                      }
                  }
            }
        }
      else if( cmd=='b' )
        {
          Serial.println(F("Buffer contents:"));
          dump_buffer(0, databuffer+1, 512);
        }
      else if( cmd=='B' )
        {
          Serial.print(F("Filling buffer"));
          if( n==1 )
            {
              for(int i=0; i<512; i++) databuffer[i+1] = i;
            }
          else
            {
              Serial.print(F(" with 0x"));
              Serial.print(a1, HEX);
              for(int i=0; i<512; i++) databuffer[i+1] = a1;
            }
          Serial.println();
        }
      else if( cmd=='m' )
        {
          if( n==1 )
            {
              Serial.print(F("Drive "));
              Serial.write('A' + ArduinoFDC.selectedDrive());
              Serial.print(F(" motor is "));
              Serial.println(ArduinoFDC.motorRunning() ? F("on") : F("off"));
            }
          else
            {
              Serial.print(F("Turning drive "));
              Serial.write('A' + ArduinoFDC.selectedDrive());
              Serial.print(F(" motor "));
              if( n==1 || a1==0 )
                { 
                  Serial.println(F("off")); 
                  ArduinoFDC.motorOff();
                }
              else
                { 
                  Serial.println(F("on")); 
                  ArduinoFDC.motorOn();
                }
            }
        }
      else if( cmd=='s' )
        {
          if( n==1 )
            {
              Serial.print(F("Current drive is "));
              Serial.write('A' + ArduinoFDC.selectedDrive());
            }
          else
            {
              Serial.print(F("Selecting drive "));
              Serial.write(a1>0 ? 'B' : 'A');
              Serial.println();
              ArduinoFDC.selectDrive(n>1 && a1>0);
              ArduinoFDC.motorOn();
            }
        }
      else if( cmd=='t' && n>1 )
        {
          set_drive_type(a1);
        }
      else if( cmd=='f' )
        {
          if( confirm_formatting() )
            {
              Serial.println(F("Formatting disk..."));
              byte status = ArduinoFDC.formatDisk(databuffer, n>1 ? a1 : 0, n>2 ? a2 : 255);
              if( status!=S_OK ) print_error(status);
              memset(databuffer, 0, 513);
            }
        }
        else
        Serial.println(F("Invalid command"));
    }
}

bool confirm_formatting() {
  int c;
  Serial.print(F("Formatting will erase all data on the disk in drive "));
  Serial.write('A' + ArduinoFDC.selectedDrive());
  Serial.print(F(". Continue (y/n)?"));
  while( (c=Serial.read())<0 );
  do { delay(1); } while( Serial.read()>=0 );
  Serial.println();
  return c=='y';
}
