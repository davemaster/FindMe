/************************************************************************************************************************************************************************************
 * Project : FindMe - SmartBag
 * Date : 02 October 2016
 * Email : davemaster99@gmail.com
 * 
 *************************************************************************************************************************************************************************************/

#include <MQTTClient.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <string.h>

const char* _SSID     = "xxxxxx"; //Wi-Fi SSID 
const char* _PASSWORD = "xxxxxxxxxx"; // Wi-Fi Password 

// MQTT - Artik Cloud Server params
char Artik_Cloud_Server[]     = "api.artik.cloud"; // Server 
int  Artik_Cloud_Port         = 8883; // MQTT Port
char Client_Name[] = "FindMe - SmartBag"; // Any Name 
char Device_ID[]   = "21xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // DEVICE ID
char Device_TOKEN[] = "6xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; //  DEVICE TOKEN
char MQTT_Publish[] = "/v1.1/messages/21xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // (/v1.1/messages/"DEVICE ID")
char MQTT_GetMessage[] = "/v1.1/actions/21xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // (/v1.1/actions/"DEVICE ID")

char txBuffer[200]; // Json Data to Artik Cloud 
char rxBuffer[300]; // Json Data from Artik Cloud 

// status of SmartBag 
boolean bSmartBagState=false;

// pinouts for button and sound (louder buzzer)
const int buttonPin = 0;  
const int soundPin = 14;  

WiFiClientSecure SSL_ESP; // ESP SSL Connection to Artik cloud 

MQTTClient MQTT_Artik_Client; // MQTT Protocol

/*******************************************************************************************************************************
 
 messageReceived : receives message from MQTT_GetMessage

**********************************************************************************************************************************/
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(payload);  
  
  if(topic == MQTT_GetMessage)
  {
    parseRxBuffer(payload);
  }
}

/*******************************************************************************************************************************
 parseRxBuffer
{"actions":[{"name":"setOff"},{"name":"setOn"}]}

**********************************************************************************************************************************/
void parseRxBuffer(String payload) 
{
  StaticJsonBuffer<300> JsonBuffer;
  String Json = payload;
  
  Json.toCharArray(rxBuffer,300);

  //*****************************
  JsonObject& root = JsonBuffer.parseObject(rxBuffer);
  
  if (!root.success()) 
  {
    Serial.println("parseObject() failed");
  }
  //****************************
  
  //const char* 
  String nameparam = root["actions"][0]["name"];

  Serial.print("action: ");
  Serial.println(nameparam);

  if(nameparam.equals("setOn"))
  { 
    bSmartBagState=true;
    digitalWrite(soundPin, HIGH);  //turns SOUND ON
    sendToArtikCloud(bSmartBagState); 
    
  }
  
  if(nameparam.equals("setOff"))  
  { 
    bSmartBagState=false;
    digitalWrite(soundPin, LOW); //turns SOUND OFF
    sendToArtikCloud(bSmartBagState);
  }
}

/*******************************************************************************************************************************
 
 setup : initial setup of ESP8266
 
**********************************************************************************************************************************/
void setup() 
{ 
  delay(1000);
  Serial.begin(9600);  

  pinMode(buttonPin, INPUT);
  pinMode(soundPin, OUTPUT); 
  
  digitalWrite(buttonPin, HIGH);
  digitalWrite(soundPin, LOW); 
     
  bSmartBagState = false;  
  
  //pinMode(BUILTIN_LED, OUTPUT); 
    
  // Wifi Setting
  WiFi.begin(_SSID, _PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  MQTT_Artik_Client.begin(Artik_Cloud_Server, Artik_Cloud_Port, SSL_ESP); // Connect to Artik Server
  
  TryToConnect();
}

/*******************************************************************************************************************************
 
 putDataInBuffer : charge txBuffer with message to publish
 
**********************************************************************************************************************************/
void putDataInBuffer(boolean bState) 
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& dataPair = jsonBuffer.createObject();
  
  dataPair["STATE"] = bState ;
    
  dataPair.printTo(txBuffer, sizeof(txBuffer));
  Serial.println(txBuffer);
}


/*************************************************************************************************************************************  

sendToArtikCloud: sends message with status of SmartBag

*************************************************************************************************************************************/

void sendToArtikCloud(boolean bState) 
{  
    // Publishing data in txBuffer 
    putDataInBuffer(bState);  
    delay(1000);
    
    MQTT_Artik_Client.publish(MQTT_Publish, txBuffer);// Publishing data to the Artik Cloud 
    delay(1000);

    Serial.println("\nMessage sent!");
}


/***********************************************************************************************************************************
 
 loop : main execution loop code

************************************************************************************************************************************/

void TryToConnect() 
{
  // Loop until we're reconnected
  while (!MQTT_Artik_Client.connected()) 
  {
    Serial.print("Attempting Artik Cloud connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to        
    
    if(MQTT_Artik_Client.connect(Client_Name, Device_ID, Device_TOKEN)) // Connect to Artik IOT Device 
    { 
      MQTT_Artik_Client.subscribe(MQTT_GetMessage);
      Serial.println("Artik Cloud connected...!");     
    }
    else 
    {
      Serial.println("failed...");      
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }    
    
  }
}

/**********************************************************************************************************************************
 
 loop : main execution loop code

**********************************************************************************************************************************/
void loop() 
{  

  int reading = digitalRead(buttonPin);
  
  if (bSmartBagState)
  {
    if (reading == LOW)
    {
    
    bSmartBagState=false;
    digitalWrite(soundPin, LOW);     //turns SOUND OFF
    
    Serial.println("\nButton OFF pressed!");
    
    sendToArtikCloud(bSmartBagState);
    
    }
  }
  
  
  if(! MQTT_Artik_Client.connected()) 
  {
    TryToConnect();
  }
  
  MQTT_Artik_Client.loop();  
  delay(100);  
  
}



