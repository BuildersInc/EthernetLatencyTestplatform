#include <ArduinoJson.h>
#include <EthernetESP32.h>
#include <HTTPClient.h>
#include <WebServer.h>

#define USE_SERIAL Serial

#define DBG_SERVER 0
#define MASTER_SLAVE 1

// #define ETH_TARGET DBG_SERVER
#define ETH_TARGET MASTER_SLAVE


/* Messurement Pins */
#define MASTER_AND_SLAVE 14
#define SIG_IN 13
#define SIG_OUT 12

#define UDP_TX_PACKET_MAX_SIZE 860

typedef enum {
  NOT_DEFINED,
  MASTER,
  SLAVE
} MODE;

MODE mode = MASTER;

int iteration = 0;

IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(0, 0, 0, 0);
IPAddress dns(0, 0, 0, 0);
byte macMaster[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte macSlave[] = { 0xC0, 0x0F, 0xFE, 0xEB, 0xAB, 0xE0 };
IPAddress master(192, 168, 178, 150);

#if ETH_TARGET == DBG_SERVER
IPAddress slave(169, 254, 235, 86);
#else
IPAddress slave(192, 168, 178, 151);

// IPAddress slave(255, 255, 255, 255);
#endif

EMACDriver driver(
  ETH_PHY_LAN8720, 18, 23, -1, EMAC_APPL_CLK_OUT_GPIO, EMAC_CLK_OUT);

unsigned int udp_port = 8080;
uint8_t packetBuffer[UDP_TX_PACKET_MAX_SIZE];
uint8_t NormalMessage[] = "YouThere?";
uint8_t NormalAnswer[] = "YesIAm!";
uint8_t AlarmMsg[] = "Alarm!";

EthernetUDP Udp;

void handle_alarm(void) {
}

int handle_calc(int a, int b) {
  return a + b;
}

void ping_on_pin(uint8_t pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void setup() {

  USE_SERIAL.begin(115200);
  Ethernet.init(driver);

  pinMode(MASTER_AND_SLAVE, INPUT);
  pinMode(SIG_OUT, OUTPUT);
  pinMode(SIG_IN, OUTPUT);
  digitalWrite(SIG_OUT, LOW);

  // Set Board to master or Slave
  if (HIGH == digitalRead(MASTER_AND_SLAVE)) {
    mode = MASTER;
  } else {
    mode = SLAVE;
  }

  if (MASTER == mode) {
    USE_SERIAL.println("CHIP IN MASTER MODE");
    Ethernet.begin(macMaster, master, dns, gateway, subnet);
    USE_SERIAL.printf("Target will be, %d.%d.%d.%d\n", slave[0], slave[1], slave[2], slave[3]);
  } else if (SLAVE == mode) {
    USE_SERIAL.println("CHIP IN SLAVE MODE");
    Ethernet.begin(macSlave, slave, dns, gateway, subnet);
    USE_SERIAL.printf("Master will be, %d.%d.%d.%d\n", master[0], master[1], master[2], master[3]);

    Udp.begin(udp_port);
  } else {
    USE_SERIAL.println("ERROR in Master Slave Select");
    while (1)
      ;
  }
  USE_SERIAL.println("Setup Complete");
  USE_SERIAL.println(Ethernet);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (MASTER == mode) {
    Udp.beginPacket(slave, udp_port);
    if (5 == iteration) {
      iteration = 0;
      ping_on_pin(SIG_IN);
      if (Udp.write(AlarmMsg, sizeof(AlarmMsg))) {
        USE_SERIAL.printf("Alarm Send\r\n");
      } else {
        USE_SERIAL.printf("Failed Alarm Send\r\n");
      }
    } else {
      // Udp.beginPacket(slave, udp_port);
      if (Udp.write(NormalMessage, sizeof(NormalMessage))) {
        USE_SERIAL.printf("Send normal packet number: %i\r\n", iteration);
        iteration++;
      } else {
        USE_SERIAL.printf("FAILED packet number: %i\r\n", iteration);
      }
      // Udp.endPacket();
      delay(2500);
    }
    Udp.endPacket();

  }

  else if (SLAVE == mode) {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

      if (NULL != strstr("Alarm!", (const char*)packetBuffer)) {
        // Alarm Message recv
        ping_on_pin(SIG_OUT);
        return;
      }
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(NormalAnswer, sizeof(NormalAnswer));
      Udp.endPacket();
      USE_SERIAL.printf("Recv Packet with message: %s \r\n", packetBuffer);
    }
  }
}
