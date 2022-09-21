//----------------------------------
//            LIBS
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//----------------------------------
//        MQTT CONTROLLER
const char* ssid = "WIFI-SSID";
const char* password =  "WIFI-PASSWORD";
const char* mqttServer = "192.168.0.101";
const int mqttPort = 1883;
String msg;

//----------------------------------
//        OUTPUT PIN
const int REDD = 13;//D7
const int GREENN = 12;//D6
const int BLUEE = 14;//D5
const int RELAI = 16;//D0

//----------------------------------
//        GENERAL VARIABLES
bool relaiState;
int redVal, greenVal, blueVal;

WiFiClient espClient;
PubSubClient client(espClient);

//----------------------------------
//     WIFI CONNECT / RECONNECT
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP18")) {
      Serial.println("connected");
      client.subscribe("ligths");
      client.subscribe("ligths/switch");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//----------------------------------
//            CALLBACK
void callback(String topic, byte* message, unsigned int length) {
  Serial.print(topic);
  Serial.print(" : ");
  //Serial.print(". Message: ");
  String messageTemp, red, green, blue;
  int rl, gl, bl = 0;
  bool r, g, b = false;

  //=== RELAY SIWTCH ===
  if (topic == "ligths/switch") {
    for (int i = 0; i < length; i++) {
      //Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    if (messageTemp) {
      relaiState = !relaiState;
      digitalWrite(RELAI, relaiState);
    }
  }
  //=== RGB INCOME MSG ===
  else {
    //----------------------------------
    //      -1/3- MSG INCOME
    for (int i = 0; i < length; i++) {
      if ((char)message[i] == 'r' ) {
        r = true;
      } else if ((char)message[i] == 'g') {
        r = false; g = true;
      } else if ((char)message[i] == 'b') {
        g = false; b = true;
      } else if ((char)message[i] == 'a') {
        b = false;
      }
      if (r) {
        red += (char)message[i];
      }
      if (g) {
        green += (char)message[i];
      }
      if (b) {
        blue += (char)message[i];
      }
    }
  }
  //----------------------------------
  //       -2/3- FORMAT INCOME
  red.remove(0, 3); green.remove(0, 3); blue.remove(0, 3);
  rl = red.length(); gl = green.length(); bl = blue.length();
  red.remove(rl - 2, 2); green.remove(gl - 2, 2); blue.remove(bl - 2, 2);
  redVal = red.toInt(), greenVal = green.toInt(); blueVal = blue.toInt();

  //----------------------------------
  //       -3/3- WRITE PIN
  analogWrite(REDD, redVal); analogWrite(GREENN, greenVal); analogWrite(BLUEE, blueVal);
  //Serial.print("RED : "); Serial.print(red); Serial.print(" - GREEN : "); Serial.print(green); Serial.print(" - BLUE : "); Serial.print(blue);
  //Serial.print(redVal); Serial.print(", "); Serial.print(greenVal); Serial.print(", "); Serial.print(blueVal);
  //Serial.println();
}

void setup() {
  pinMode(REDD, OUTPUT); pinMode(GREENN, OUTPUT); pinMode(BLUEE, OUTPUT); pinMode(RELAI, OUTPUT);
  digitalWrite(REDD, LOW); digitalWrite(GREENN, LOW); digitalWrite(BLUEE, LOW);

  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
