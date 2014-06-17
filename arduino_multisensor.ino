
#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

char website[] PROGMEM = "www.google.com";

byte Ethernet::buffer[500];
BufferFiller bfill;

//pins
const int tempSensorPin = A0;
const int luxSensorPin = A1;


void setup () {
  Serial.begin(57600);
  Serial.println("\n[webClient]");
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println( "Failed to access Ethernet controller");
  //ether.staticSetup(myip);
  
   if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

 // if (!ether.dnsLookup(website))
  //  Serial.println("DNS failed");
    
  //ether.printIp("SRV: ", ether.hisip);

}

static word homePage() {
  long t = millis() / 1000;
  word h = t / 3600;
  byte m = (t / 60) % 60;
  byte s = t % 60;
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<meta http-equiv='refresh' content='1'/>"
    "<title>404 Not found</title>" 
    "Arduino Timer | $D$D:$D$D:$D$D"),
      h/10, h%10, m/10, m%10, s/10, s%10);
  return bfill.position();
}

static word readingPage() {
  //compute measures
   float tempmV = analogRead(tempSensorPin) * (5000/1024);
   Serial.println(tempmV);
   float temp = (tempmV - 500) / 10;
   Serial.println(temp);
      

  int tempValue = abs(((analogRead(tempSensorPin) * (5000/1024)) - 500) / 10) + 20;
  
  int luxValue = analogRead(luxSensorPin)/4; 
  
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<meta http-equiv='refresh' content='1'/>"
    "<title>Arduino Sensor Reading (by rodolfoahsantos@gmail.com)</title>" 
    "temp:$D|lux:$D"),
       tempValue,luxValue);
  return bfill.position();
}

static word historyPage() {
  long t = millis() / 1000;
  word h = t / 3600;
  byte m = (t / 60) % 60;
  byte s = t % 60;
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<meta http-equiv='refresh' content='1'/>"
    "<title>History</title>" 
    "Arduino Timer | $D$D:$D$D:$D$D"),
      h/10, h%10, m/10, m%10, s/10, s%10);
  return bfill.position();
}

void loop () {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if (pos){  // check if valid tcp data is received
  
    char *data = (char *) Ethernet::buffer + pos;
    Serial.println(data);
    
    if (strncmp("GET /reading", data, 12) == 0) {
      Serial.println("Reading request");
      
     
      
      ether.httpServerReply(readingPage());
    }
    else if (strncmp("GET /history", data, 12) == 0) {
      Serial.println("History request");
      ether.httpServerReply(historyPage());
    }
    
    else{
       ether.httpServerReply(homePage());
    }
  
   
  }
   
}
