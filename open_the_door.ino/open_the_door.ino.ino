#include <EtherCard.h>

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface ip address
static byte myip[] = { 192,168,35,99 };
// gateway ip address
static byte gwip[] = { 192,168,35,1 };
// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];

bool RelayStatus = false;
int Relaypin = 2;
int Switch = 6;

void setup() {
  // put your setup code here, to run once:
  pinMode(Relaypin, OUTPUT);
  pinMode(Switch, INPUT_PULLUP);

  Serial.begin(57600);
  Serial.println("[webClient]");

  // Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  // if (!ether.dhcpSetup())
  //   Serial.println(F("DHCP failed"));
  ether.staticSetup(myip, gwip);

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  Serial.println("[DONE]");
}

const char http_OK[] PROGMEM = 
  "HTTP/1.0 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Pragma: no-cache\r\n\r\n";

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(Switch) == LOW) {
    digitalWrite(Relaypin, HIGH);
    Serial.println("OPEN DOOR");
  }
  else {
    digitalWrite(Relaypin, LOW);
  }

  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if(pos) {
    Serial.print("#1 len=");
    Serial.print(len);
    Serial.print(",");
    Serial.print(pos);
    Serial.println();

    BufferFiller bfill = ether.tcpOffset();
    bfill = ether.tcpOffset();
    char * data = (char *) Ethernet::buffer + pos;
    if(strncmp("GET /", data, 5) != 0) { }
    else {
      data += 5;
      if(data[0] == ' ') { }
      else if(strncmp("?relay=on", data, 9) == 0) {
        RelayStatus = true;
        digitalWrite(Relaypin, HIGH);
        Serial.println("OPEN DOOR");
      } 
      else if(strncmp("?relay=off", data, 10) == 0) {
        RelayStatus = false;
        digitalWrite(Relaypin, LOW);
      }
      else { }

      Serial.print("GET=");
      Serial.println(data);

      bfill.emit_p(
        PSTR(
          "<title>Open the Door</title>"
          "<h1>Open the Door</h1>"
          "</br>"
          "<a href=\"?relay=on\"><h1>open</h1></a>"
        ),
        http_OK
      );
      ether.httpServerReply(bfill.position());
    }
  }
  
  delay(100);
}
