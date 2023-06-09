#include <lorawan.h>
#include <TinyGPS++.h>
#include <TimeLib.h>

//Jika Menggunakan Software Serial
#include <SoftwareSerial.h>
SoftwareSerial serialGps(35, 33); //RX ESP, TX ESP

//Jika Menggunakan Hardware Serial
// #define serialGps Serial

TinyGPSPlus gps;
#define time_offset 25200 //tentukan jam offset 25200 detik (7 jam) ==> UTC +7
byte last_second, Second, Minute, Hour, Day, Month;
int Year, DayOfTheWeek, SatellitesValue;
float Latitude, Longitude, Altitude, Speed;

//ABP Credentials
const char *devAddr = "d66e8016";
const char *nwkSKey = "9df906f8e80ba8520000000000000000";
const char *appSKey = "0000000000000000cdd731f0e954eb72";


const unsigned long interval = 10000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter
String dataSend = "";

char myStr[50];
byte outStr[255];
byte recvStatus = 0;
int port, channel, freq;
bool newmessage = false;

const sRFM_pins RFM_pins = {
  .CS = 5,
  .RST = 0,
  .DIO0 = 27,
  .DIO1 = 2,
};

void setup() {
  // Setup loraid access
  Serial.begin(9600);
  serialGps.begin(9600);
  delay(2000);
  if (!lora.init()) {
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_A);

  // Set Data Rate
  lora.setDataRate(SF10BW125);

  // Set FramePort Tx
  lora.setFramePortTx(5);

  // set channel to random
  lora.setChannel(MULTI);

  // Set TxPower to 15 dBi (max)
  lora.setTxPower(15);

  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
}

void loop() {
  lacak();
  // Check interval overflow
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    int t,h,a;
    t = gps.location.lat();
    h = gps.location.lng();
    a = gps.altitude.meters();


    //dataSend = "{\"Latitude\": " + (String)t + ", \"Longitude\": " + (String)h + ", \"Altitude\": " + (String)a + ", \"Location\": " + (String)"https://maps.google.com/maps?q=" + (String)t + "," + (String)h"}";
    //dataSend = "{\"https://maps.google.com/maps?q=\"" + (String)t + "," + (String)h"}";
    dataSend = "{\"Latitude\": " + (String)t + ", \"Longitude\": " + (String)h + ", \"Altitude\": " + (String)a + "}";
    //dataSend = "{\"Location\": " + (String)"https://maps.google.com/maps?q=" + (String)t + ","+ (String)h + ", \"Latitude\": " + (String)t + ", \"Longitude\": " + (String)h + ", \"Altitude\": " + (String)a + "}";
    dataSend.toCharArray(myStr,50);

    Serial.print("Sending: ");
    Serial.println(dataSend);
    lora.sendUplink(myStr, strlen(myStr), 0);

  }

  // Check Lora RX
  lora.update();

  recvStatus = lora.readDataByte(outStr);
  if (recvStatus) {
    newmessage = true;
    int counter = 0;
    port = lora.getFramePortRx();
    channel = lora.getChannelRx();
    freq = lora.getChannelRxFreq(channel);

    for (int i = 0; i < recvStatus; i++)
    {
      if (((outStr[i] >= 32) && (outStr[i] <= 126)) || (outStr[i] == 10) || (outStr[i] == 13))
        counter++;
    }
    if (port != 0)
    {
      if (counter == recvStatus)
      {
        Serial.print(F("Received String : "));
        for (int i = 0; i < recvStatus; i++)
        {
          Serial.print(char(outStr[i]));
        }
      }
      else
      {
        Serial.print(F("Received Hex: "));
        for (int i = 0; i < recvStatus; i++)
        {
          Serial.print(outStr[i], HEX); Serial.print(" ");
        }
      }
      Serial.println();
      Serial.print(F("fport: "));    Serial.print(port);Serial.print(" ");
      Serial.print(F("Ch: "));    Serial.print(channel);Serial.print(" ");
      Serial.print(F("Freq: "));    Serial.println(freq);Serial.println(" ");
    }
    else
    {
      Serial.print(F("Received Mac Cmd : "));
      for (int i = 0; i < recvStatus; i++)
      {
        Serial.print(outStr[i], HEX); Serial.print(" ");
      }
      Serial.println();
      Serial.print(F("fport: "));    Serial.print(port);Serial.print(" ");
      Serial.print(F("Ch: "));    Serial.print(channel);Serial.print(" ");
      Serial.print(F("Freq: "));    Serial.println(freq);Serial.println(" ");
    }
  }

}


void lacak() {

  while (serialGps.available() > 0) { //KETIKA GPS TERSAMBUNG DENGAN ARDUINO....

    if (gps.encode(serialGps.read())) { //KETIKA GPS MENDAPATKAN DATA / SIGNAL...

      //Dapatkan Data Satellite.......................................
      if (gps.satellites.isValid()) {
        SatellitesValue = gps.satellites.value();
      }

      //Dapatkan Data Lokasi (Latitude & Longitude)...................
      if (gps.location.isValid()) {
        Latitude = gps.location.lat();
        Longitude = gps.location.lng();
        Altitude = gps.altitude.meters();
      }

      //Dapatkan Data Altitude........................................
      if (gps.altitude.isValid()) {
        Altitude = gps.altitude.meters();
      }

      //Dapatkan Data Kecepatan........................................
      if (gps.speed.isValid()) {
        Speed = gps.speed.kmph();
      }

      //Dapatkan Data Waktu...........................................
      if (gps.time.isValid()) {
        Minute = gps.time.minute();
        Second = gps.time.second();
        Hour   = gps.time.hour();
      }

      //Dapatkan Data Tanggal.........................................
      if (gps.date.isValid()) {
        Day   = gps.date.day();
        Month = gps.date.month();
        Year  = gps.date.year();
      }

      //Tampilkan Waktu dan Tanggal yang Sudah Sesuai Offset / UTC yang diatur...
      if (last_second != gps.time.second()) {
        last_second = gps.time.second();
        // set current UTC time
        setTime(Hour, Minute, Second, Day, Month, Year);
        // add the offset to get local time
        adjustTime(time_offset);
        Hour   = hour();
        Minute = minute();
        Second = second();
        Day   = day();
        Month = month();
        Year  = year();
        DayOfTheWeek = weekday();
        Serial.println("======= MENAMPILKAN DATA GPS =======");
        Serial.println("\tSatellites Value: " + String(SatellitesValue, 8));
        Serial.println("\tLatitude\t: " + String(Latitude, 8));
        Serial.println("\tLongitude\t: " + String(Longitude, 8));
        Serial.println("\tAltitude\t: " + String(Altitude, 5));
        Serial.println("\tSpeed\t\t: " + String(Speed, 5));
        Serial.println(String() + "\tTime\t\t: " + Hour + ":" + Minute + ":" + Second );
        Serial.println(String() + "\tDate\t\t: " + Day + "-" + Month + "-" + Year );
        Serial.println(String() + "\tDay\t\t: " + DayOfTheWeek);
        Serial.println();
      }

    } //END IF - KETIKA GPS MENDAPATKAN DATA / SIGNAL...

  } //END WHILE - KETIKA GPS TERSAMBUNG DENGAN ARDUINO....

}
