#ifndef WSCLIENT_H_
#define WSCLIENT_H_

#define CRLF "\r\n" // CRLF characters to terminate lines/handshakes in headers.

#ifndef CALLBACK_FUNCTIONS
#define CALLBACK_FUNCTIONS 1 // default 5 unless specified
#endif

#define SIZE(array) (sizeof(array) / sizeof(*array))

class WSClient {
public:

    bool handshake(Client &client);     // Handle connection requests to validate and process/refuse connections.
    char* getData(); // Get data off of the stream
    void sendData(char *str); // Write data to the stream
    void disconnect();
    char *path;
    char *host;

    void sendEncodedData(char *str);


private:
    Client *socket_client;
    bool analyzeRequest();
    void disconnectStream();    // Disconnect user gracefully.
    int timedRead();
    int charinstr(char* text, int size, char* string);
    bool array_cmp(char *a, char *b, int len_a, int len_b);
};

#endif