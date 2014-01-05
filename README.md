## Websocket Client for Arduino

This library implements a websocket client for arduino.
This library supports Sec-WebSocket-Version: 13 and binary **multiple** binary frame messaging.

### Limitations

This library:
  * <del>only supports sending a max 128 byte message (including headers);</del> - Now supporting Binary Data Framing & compliant with RFC6455 sections 5.6 / 5.7
  * has a considerable memory and sketch size footprint as it uses heavy computation on Sha1, Base64 or MD5;

### Testing Environment

This library was tested and should run well on the following boards:
  * Arduino Uno with Ethernet Shield
  * Arduino Duemilanove w/ ATmega328
  * Arduino Ethernet
  * Arduino Mini w/ ATmega328
  * others with ATmega328 should work ok
  * others probably won't work ;) - send me some hardware and I'll try to port it

This library was developed to support:
  * Muzzley human interface for the Internet of Things - http://www.muzzley.com/

This library was tested against the following websocket servers:
  * muzzley - http://www.muzzley.com
  * socket.io, sockjs, WebSocket-Node & should work fine on any WS implementation;
  * kaazing gateway as used on http://www.websocket.org/echo.html (cheers kaazing for rocking the web)
  * pusher - http://www.pusher.com/
  * pubnub - http://www.pubnub.com/

### Getting started

Install the library to "libraries" folder in your Arduino sketchbook folder. 

Try the example that is shown bellow.

### Example
``` arduino
#include <Ethernet.h>
#include <SPI.h>
#include <WSClient.h>

// Ethernet Configuration
EthernetClient client;
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x00, 0xF2, 0x78 };

// Websocket initialization
WSClient websocket;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Demo example on WSClient usage"));
  Ethernet.begin(mac); // initialize ethernet
  Serial.println(Ethernet.localIP()); // printout IP address for debug purposes
  delay(300); // this is arduino baby ;-)

  // Connect and test websocket server connectivity
  if (client.connect("echo.websocket.org", 80)) {
    Serial.println("Connected");
  } 
  else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  // Define path and host for Handshaking with the server
  websocket.path = "/";
  websocket.host = "echo.websocket.org";

  if (websocket.handshake(client)) {
    Serial.println("Handshake successful");
  } 
  else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }
  }

}


void loop() {
  String data;

  if (client.connected()) {
    data = websocket.getData();
    if (data.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(data);
    }

    Serial.println(F("")); Serial.println(F("Sending Data"));
    websocket.sendData("echo test");
  } else {
    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }

  delay(3000);  // wait to fully let the client disconnect

}
```

### Credits
Thank you to:
  - github user brandenhall for his library that I used to better understand the handshaking process;
  - developers of the third party libraries included by this websocket client library: MD5, sha1 & Base64;

This library is general purpose but was made to support the muzzley arduino library (also available on github). Check http://www.muzzley.com for further reference.


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/djsb/arduino-websocketclient/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

