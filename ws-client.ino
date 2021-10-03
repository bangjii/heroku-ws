/*
  Author : bangjii
  Board : ESP8266 (WeMOS D1 R2 Mini)
  Library : WebSockets
  Updated : 03 Oct 2021
  
  ESP8266 client connected to heroku websocket server.
  at begin connection use {"user":"youruser"} for allowed connection
  your-url-heroku.com/{"user":"youruser"}
  
  send message using JSON format
  - broadcast : {"to": "brdALL", "textMsg": "holaaa"}
    arduino : webSocket.sendTXT("{\"to\": \"brdALL\", \"textMsg\": \"holaaa\"}");
    return message {"tipe":"broadcast","from":"jason","textMsg":"holaaa"}
    
  - private : {"to": "john", "textMsg": "holaaa"}
    arduino : webSocket.sendTXT("{\"to\": \"john\", \"textMsg\": \"holaaa\"}");
    return message {"tipe":"private","from":"jason","textMsg":"holaaa"}
    
  Arduino library for websocket : https://github.com/Links2004/arduinoWebSockets
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial
String inputString;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
      
		case WStype_CONNECTED: {
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
			// send message to server when Connected
      webSocket.sendTXT("{\"to\": \"brdALL\", \"textMsg\": \"holaaa from ESP ;)\"}");
		}
			break;
      
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
			// send message to server
			// webSocket.sendTXT("message here");
			break;
      
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);
			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
      
    case WStype_PING:
      // pong will be send automatically
      //USE_SERIAL.printf("[WSc] get ping\n");
      break;
    case WStype_PONG:
      // answer to a ping we send
      //USE_SERIAL.printf("[WSc] get pong\n");
      break;
    }

}

void setup() {
	USE_SERIAL.begin(115200);
	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("Your Wifi Name", "somepassword");

	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}
  
  //begin websocket connection
  String user = "/{\"user\":\"jason\"}";
  webSocket.begin("my-greatest-app.herokuapp.com", 80, user);
	webSocket.onEvent(webSocketEvent);
	webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);

}

void loop() {
	webSocket.loop();
  while (Serial.available()) {
      char inChar = (char)Serial.read();
      inputString += inChar;
      if (inChar == '\n') {
        inputString.trim();
        //broadcast to all client
        String text = "{\"to\": \"brdALL\", \"textMsg\": \""+ inputString +"\"}";
        webSocket.sendTXT(text);
        inputString = "";
      }
  }
}
