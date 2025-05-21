
// /* Ethernet Pins */
// #define ETH_PHY_TYPE ETH_PHY_LAN8720
// #define ETH_PHY_ADDR  -1
// #define ETH_PHY_MDC   18
// #define ETH_PHY_MDIO  23
// #define ETH_PHY_POWER -1
// #define ETH_CLK_MODE  ETH_CLOCK_GPIO0_OUT
// // #define ETH_NAME "ESP-Ethernet-2"

// #include <ETH.h>

#include <EthernetESP32.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define USE_SERIAL Serial

// #define ETH_TARGET DBG_SERVER
#define ETH_TARGET MASTER_SLAVE

/* Messurement Pins */
#define MASTER_AND_SLAVE  14
#define SIG_IN            13
#define SIG_OUT           12


typedef enum {NOT_DEFINED, MASTER, SLAVE} MODE;
MODE mode = MASTER;

static bool eth_connected = false;
//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;  
unsigned long last_button_time = 0;
int iteration = 0; 

IPAddress master      (192, 168, 178, 150);
IPAddress slave       (192, 168, 178, 151);
IPAddress dbg_server  (192, 168, 178, 190);

WebServer server(8080);
EMACDriver driver(ETH_PHY_LAN8720, 18, 23, -1, EMAC_APPL_CLK_OUT_GPIO, EMAC_CLK_OUT);

const String payload = "{\"num1\": 10, \"num2\": 20}";
StaticJsonDocument<1024> jsonDocument;

#define BUFFER_SIZE 40
char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));

HTTPClient http_calc;
HTTPClient http_error;

void start_http()
{
  static bool server_running = false;
  if (server_running) return;
#if ETH_TARGET == DBG_SERVER
    // http_calc.begin("http://192.168.178.190:8080/calculate");
    // http_error.begin("http://192.168.178.190:8080/calculate");
    // http_calc.begin("http://www.example.com/index.html");
#elif ETH_TARGET == MASTER_SLAVE
    http_calc.begin("http://192.168.178.20:8080/calculate");
    http_error.begin("http://192.168.178.20:8080/error");
#endif
  if (!server_running) server_running = true;
}

void IRAM_ATTR alarm_isr()
{
  // button_time = millis();
  // if (250 > (button_time - last_button_time))
  // {
  //   return;
  // }
  // last_button_time = button_time;

  // Send Alarm Packet#
  http_error.begin("http://192.168.178.151:8080/error");

  int http_code = http_error.POST("ERROR_ALARM");
  USE_SERIAL.println("Sending Alarm");
  http_error.end();
}

void handle_alarm(void)
{
  USE_SERIAL.println("Error Occured");
  digitalWrite(SIG_OUT, HIGH);
  digitalWrite(SIG_OUT, LOW);
}

void handle_calculation(void)
{
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  int num1 = jsonDocument["num1"];
  int num2 = jsonDocument["num2"];
  int result = num1 + num2;
  USE_SERIAL.printf("%d + %d = %d\n", num1, num2, result);
  sprintf(buffer, "{\"sum\": %d}", result);

  server.send(200, "application/json", buffer);
}


void setup() 
{
  
  USE_SERIAL.begin(115200);
  Ethernet.init(driver);

  pinMode(MASTER_AND_SLAVE, INPUT);
  pinMode(SIG_OUT, OUTPUT);
  pinMode(SIG_IN, OUTPUT);
  digitalWrite(SIG_OUT, LOW);

  // Set Board to master or Slave
  if (HIGH == digitalRead(MASTER_AND_SLAVE))
  {
    mode = MASTER;
  } 
  else 
  {
    mode = SLAVE;
  }

  if (MASTER == mode)
  {
    USE_SERIAL.println("CHIP IN MASTER MODE");
    Ethernet.begin(master);
  } 
  else if (SLAVE == mode)
  {
    USE_SERIAL.println("CHIP IN SLAVE MODE");
    Ethernet.begin(slave);
    server.on("/calculate", HTTP_POST, handle_calculation);
    server.on("/error", HTTP_POST, handle_alarm);
    server.begin();
  }
  else 
  {
    USE_SERIAL.println("ERROR in Master Slave Select");
    while(1);
  }
  USE_SERIAL.println("Setup Complete");
  USE_SERIAL.println(Ethernet);

}

void loop() {


  if (MASTER == mode)
  {
    // start_http();
    http_calc.begin("http://192.168.178.151:8080/calculate");

    USE_SERIAL.println("Start HTTP");
    http_calc.addHeader("Content-Type", "text/plain");
    int http_code = http_calc.POST(payload);

    USE_SERIAL.printf("[HTTP] GET... code: %d\n", http_code);
    if (0 < http_code) 
    {
      if (HTTP_CODE_OK == http_code) 
      {
        String payload = http_calc.getString();
        USE_SERIAL.println(payload);
        digitalWrite(SIG_IN, LOW);

      }
    } 
    else 
    {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http_calc.errorToString(http_code).c_str());
    }
    http_calc.end();
    delay(2500);
    iteration += 1;

    if (5 == iteration)
    {
      USE_SERIAL.println("Sending Alarm");

      digitalWrite(SIG_IN, HIGH);
      digitalWrite(SIG_IN, LOW);
      http_error.begin("http://192.168.178.151:8080/error");
      int http_code = http_error.POST("ERROR_ALARM");
      http_error.end();
      iteration = 0;
    }
    USE_SERIAL.printf("%i \n", iteration);
    // End Master Loop
  }
  else if (SLAVE == mode)
  {
	  server.handleClient();
    // End Slave Loop
  }
}