// Author: Domingos Bruges
//
// websocket arduino client library: https://github.com/djsb/arduino-websocketclient
//
// Copyright (c) 2013 Domingos Bruges
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE. 

#include "sha1.h"
#include "base64.h"
#include <Ethernet.h>
#include <WSClient.h>


bool WSClient::handshake(Client &client) {


    socket_client = &client;

    // If there is a connected client->
    if (socket_client->connected()) {
        // Check request and look for websocket handshake
        if (analyzeRequest()) {
            return true;

        } else {
            // Might just need to break until out of socket_client loop.
            disconnectStream();
            return false;
        }
    } else {
        return false;
    }
}


int WSClient::charinstr(char* text, int len_text, char* string){

    int pos_search = 0;
    int pos_text = 0;
    int len_search = strlen(string);
    char* str;
    str = string;

    for (pos_text = 0; pos_text < len_text - len_search;++pos_text)
    {
        if(text[pos_text] == str[pos_search])
        {
            ++pos_search;
            if(pos_search == len_search)
            {
                // match
                return pos_text;
            }
        }
        else
        {
         pos_text -=pos_search;
         pos_search = 0;
     }
 }
    // no match
 return -1;
}

bool WSClient::array_cmp(char *a, char *b, int len_a, int len_b){
  int n;

      // if their lengths are different, return false
  if (len_a != len_b) 
    return false;

      // test each element to be the same. if not, return false
for (n=0;n<len_a;n++) 
    if (a[n]!=b[n]) 
        return false;

      //ok, if we have not returned yet, they are equal :)
    return true;
}

bool WSClient::analyzeRequest() {
    int bite;
    bool foundupgrade = false;
    bool foundconnection = false;
    char keyStart[17];
    char b64Key[25];
    char key[]="------------------------";

    randomSeed(analogRead(0));
    for (int i=0; i<16; ++i) {
        keyStart[i] = (char)random(1, 256);
    }

    base64_encode(b64Key, keyStart, 16);

    for (int i=0; i<24; ++i) {
        key[i] = b64Key[i];
    }

    socket_client->print(F("GET "));
    socket_client->print(path);
    socket_client->print(F(" HTTP/1.1\r\n"));
    socket_client->print(F("Upgrade: websocket\r\n"));
    socket_client->print(F("Connection: Upgrade\r\n"));
    socket_client->print(F("Host: "));
    socket_client->print(host);
    socket_client->print(CRLF); 
    socket_client->print(F("Sec-WebSocket-Key: "));
    socket_client->print(key);
    socket_client->print(CRLF);
    socket_client->print(F("Sec-WebSocket-Version: 13\r\n"));
    socket_client->print(CRLF);

    // DEBUG ONLY - inspect the handshaking process
    /*
    Serial.print(F("GET "));
    Serial.print(path);
    Serial.print(F(" HTTP/1.1\r\n"));
    Serial.print(F("Upgrade: websocket\r\n"));
    Serial.print(F("Connection: Upgrade\r\n"));
    Serial.print(F("Host: "));
    Serial.print(host);
    Serial.print(CRLF); 
    Serial.print(F("Sec-WebSocket-Key: "));
    Serial.print(key);
    Serial.print(CRLF);
    Serial.print(F("Sec-WebSocket-Version: 13\r\n"));
    Serial.print(CRLF);
    */


    while (socket_client->connected() && !socket_client->available()) {
        delay(50);
    }


    while (!socket_client->available()) {
        delay(50);
    }


    int i=0;
    char temp[80];
    char serverKey[28];

    // TODO: Improve the extraction
    while ((bite = socket_client->read()) != -1) {
        temp[i] = (char)bite; i++;
        if ((char)bite == '\n'){

            int commaPosition = -1;
            if (commaPosition = charinstr(temp, sizeof(temp), "Sec-WebSocket-Accept:") != -1){

              if(commaPosition != -1)
              {
                commaPosition = commaPosition + 19;
                for (int i=2; i<sizeof(temp); i++){
                    serverKey[i-2] = temp[i+commaPosition];

                }
            }

        }

        memset(temp, NULL, sizeof(temp));
        i=0;

    }

}

char magicKey[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

char newKey[60];
for (int i=0; i<sizeof(key); i++){
    newKey[i]=key[i];
}

for (int i=0; i<sizeof(magicKey); i++){
    newKey[sizeof(key)+i-1]=magicKey[i];
}



uint8_t *hash;
char result[21];
char b64Result[28];


Sha1.init();
Sha1.print(newKey);
hash = Sha1.result();

for (int i=0; i<20; ++i) {
    result[i] = (char)hash[i];
}
result[20] = '\0';

base64_encode(b64Result, result, 20);

return array_cmp(serverKey,b64Result,sizeof(serverKey), sizeof(b64Result));
}




void WSClient::disconnect() {
    disconnectStream();
}


void WSClient::disconnectStream() {

    // Should send 0x8700 to server to tell it I'm quitting here.
    socket_client->write((uint8_t) 0x87);
    socket_client->write((uint8_t) 0x00);
    
    socket_client->flush();
    delay(10);
    socket_client->stop();
}


char* WSClient::getData() {
    uint8_t msgtype;
    uint8_t bite;
    unsigned int length;
    uint8_t mask[4];
    uint8_t index;
    unsigned int i;
    bool hasMask = false;

    // char array to hold bytes sent by server to client
    // message could not exceed 256 chars. Array initialized with NULL
    char socketStr[256] = {NULL};



    if (socket_client->connected() && socket_client->available()) {


        msgtype = timedRead();
        if (!socket_client->connected()) {
            return (char*)socketStr;
        }

        length = timedRead();


        if (length > 127) {
            hasMask = true;
            length = length & 127;
        }


        if (!socket_client->connected()) {
            return (char*)socketStr;
        }

        index = 6;



        if (length == 126) {
            length = timedRead() << 8;
            if (!socket_client->connected()) {
                return (char*)socketStr;
            }
            
            length |= timedRead();
            if (!socket_client->connected()) {
                return (char*)socketStr;
            }   

        } else if (length == 127) {

            while(1) {
                // halt, can't handle this case
            }
        }


        if (hasMask) {
            // get the mask
            mask[0] = timedRead();
            if (!socket_client->connected()) {
                return (char*)socketStr;
            }

            mask[1] = timedRead();
            if (!socket_client->connected()) {
                return (char*)socketStr;
            }

            mask[2] = timedRead();
            if (!socket_client->connected()) {
                return (char*)socketStr;
            }

            mask[3] = timedRead();
            if (!socket_client->connected()) {
                return (char*)socketStr;
            }
        }


        if (hasMask) {
            for (i=0; i<length; ++i) {
                socketStr[i] = (char) (timedRead() ^ mask[i % 4]);
                if (!socket_client->connected()) {
                    return (char*)socketStr;
                }
            }
        } else {
            for (i=0; i<length; ++i) {
                socketStr[i] = (char) timedRead();
                if (!socket_client->connected()) {
                    return (char*)socketStr;
                }
            }            
        }

    }

    return (char*)socketStr;
}

void WSClient::sendData(char *str) {
//    Serial.println(F("")); Serial.print(F("TX: "));
//    for (int i=0; i<strlen(str); i++)
//        Serial.print(str[i]);
    if (socket_client->connected()) {
        sendEncodedData(str);       
    }
}



int WSClient::timedRead() {
  while (!socket_client->available()) {
    delay(50);  
}

int a = socket_client->read();
return a;
}

void WSClient::sendEncodedData(char *str) {
 int size = strlen(str);

    // string type
 socket_client->write(0x81);

    // NOTE: no support for > 16-bit sized messages
 if (size > 125) {
    //Serial.println(F("size bigger than 125"));
    socket_client->write(127);
    socket_client->write((uint8_t) (size >> 56) & 255);
    socket_client->write((uint8_t) (size >> 48) & 255);
    socket_client->write((uint8_t) (size >> 40) & 255);
    socket_client->write((uint8_t) (size >> 32) & 255);
    socket_client->write((uint8_t) (size >> 24) & 255);
    socket_client->write((uint8_t) (size >> 16) & 255);
    socket_client->write((uint8_t) (size >> 8) & 255);
    socket_client->write((uint8_t) (size ) & 255);
} else {
    //Serial.println(F("size small than 125"));
    socket_client->write((uint8_t) size);
}

for (int i=0; i<size; ++i) {
    socket_client->write(str[i]);
}

}


