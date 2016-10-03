/*
 * Project : FindMe - SmartBag
 * Date : 02 October 2016
 * Email : davemaster99@gmail.com
 * 
 */


#include <MQTTClient.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <string.h>


const char* _SSID     = "XXXXXXXXXX"; 		//Wi-Fi SSID 
const char* _PASSWORD = "xxxxxxxxxxxx"; 	// Wi-Fi Password 


// MQTT - Artik Cloud Server params
char Artik_Cloud_Server[]     = "api.artik.cloud"; // Server 
int  Artik_Cloud_Port         = 8883; // MQTT Port
char Client_Name[] = "FindMe - SmartBag"; // Any Name 
char Device_ID[]   = "85xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // DEVICE ID
char Device_TOKEN[] = "29xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; //  DEVICE TOKEN
char MQTT_Publish[] = "/v1.1/messages/85xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // (/v1.1/messages/"DEVICE ID")
char MQTT_GetMessage[] = "/v1.1/actions/85xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // (/v1.1/actions/"DEVICE ID")

char txBuffer[200]; // Json Data to Artik Cloud 
char rxBuffer[300]; // Json Data from Artik Cloud 

// status of SmartBag 
//boolean bButtonState=true;
boolean bSmartBagState=false;

// pinouts for button and sound (louder buzzer)
const int buttonPin = 0;  
const int soundPin = 2;  

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
    //bButtonState=true;
    digitalWrite(soundPin, LOW);  //turns SOUND ON
    //sendToArtikCloud(bSmartBagState); 
    //delay(4000);
    
  }
  
  if(nameparam.equals("setOff"))  
  { 
    bSmartBagState=false;
    //bButtonState=false;
    digitalWrite(soundPin, HIGH); //turns SOUND OFF
    sendToArtikCloud(bSmartBagState);
    //delay(4000);
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
  digitalWrite(soundPin, HIGH); 
      
  //bButtonState = false;  
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
  
  while (!MQTT_Artik_Client.connect(Client_Name, Device_ID, Device_TOKEN)) // Connect to Artik IOT Device 
  { 
    Serial.print("*");
    delay(1000);
  }

  MQTT_Artik_Client.subscribe(MQTT_GetMessage);

  Serial.println("Artik Cloud connected...!");
  
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

/*******************************************************************************************************************************
 loop : main execution loop code
**********************************************************************************************************************************/
void loop() 
{  

  int reading = digitalRead(buttonPin);
 
  if (reading == LOW)
  {
    //bButtonState=false;
    bSmartBagState=false;
    digitalWrite(soundPin, HIGH);     //turns SOUND OFF
    
    Serial.println("\nButton OFF pressed!");

    sendToArtikCloud(bSmartBagState);
       
  }
  
  MQTT_Artik_Client.loop();  
  delay(100);  
    
}

/***********************************************************************************   

sendToArtikCloud: sends message with status of SmartBag

***********************************************************************************/

void sendToArtikCloud(boolean bState) 
{  
    // Publishing data in txBuffer 
    putDataInBuffer(bState);  
    delay(1000);
    
    MQTT_Artik_Client.publish(MQTT_Publish, txBuffer);// Publishing data to the Artik Cloud 
    delay(4000);

    Serial.println("\nMessage sent!");
}



