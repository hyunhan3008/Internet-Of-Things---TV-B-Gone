/////////////////////////////////////////////////////////////////////////////
// MyOTAThing.ino
// COM3505 lab assessment: Over-The-Air update template; ADD YOUR CODE HERE!
/////////////////////////////////////////////////////////////////////////////

// the wifi and HTTP server libraries //////// ///////////////////////////////
#include <WiFi.h>         // wifi
#include <WebServer.h> // simple webserver
#include <HTTPClient.h>   // ESP32 library for making HTTP requests
#include <Update.h>       // OTA update library

// OTA stuff ////////////////////////////////////////////////////////////////
int doCloudGet(HTTPClient *, String, String); // helper for downloading 'ware
void doOTAUpdate();                           // main OTA logic
int currentVersion = 2; // TODO keep up-to-date! (used to check for updates)
String gitID = "hyunhan"; // TODO change to your team's git ID


// MAC and IP helpers ///////////////////////////////////////////////////////
char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator
void getMAC(char *);
String ip2str(IPAddress);                 // helper for printing IP addresses

// LED utilities, loop slicing ///////////////////////////////////////////////
void ledOn();
void ledOff();
void blink(int = 1, int = 300);
int loopIteration = 0;
//bool update = false; //(if update successful then light on) 

// SETUP: initialisation entry point ////////////////////////////////////////
void setup() {
  // set up GPIO pin for a switch
  pinMode(14, INPUT_PULLUP); // pin 14: digital input, built-in pullup resistor
  
  Serial.begin(115200);         // initialise the serial line
  getMAC(MAC_ADDRESS);          // store the MAC address
  Serial.printf("\nMyOTAThing setup...\nESP32 MAC = %s\n", MAC_ADDRESS);
  Serial.printf("firmware is at version %d\n", currentVersion);
  Serial.printf("Hello Hammish");

  /* Team code.
  Join uos-other network. 
  */
  const char *ssid = "uos-other";
  const char *password = "shefotherkey05";

  //print current status
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  //Call .begin() to init current wifi lib's network settings & give status.
  //Parameters: name of network & WPA encrypted password
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { //wait till wifi status connected
    delay(500);
    //print current status
    Serial.print("...connecting...");
  }

  //print current status
  Serial.println("Connection successful!");
  Serial.print("Device IP: ");
  Serial.println(WiFi.localIP());

  // set up GPIO pin for an external LED (LIGHT: NOT TESTED)
  //pinMode(32, OUTPUT); // set up pin 32 as a digital output
  
  // check for and perform firmware updates as needed
  doOTAUpdate();//perform update
}

// LOOP: task entry point ///////////////////////////////////////////////////
void loop() {
  
  int sliceSize = 500000;
  loopIteration++;
  if(loopIteration % sliceSize == 0) {// a slice every sliceSize iterations
    Serial.println("OTA loop");
    // do other useful stuff here...?
    //(LIGHT: NOT TESTED) (on if update successful)    
    //Serial.printf("light");
    //digitalWrite(32, HIGH);       // on...
    //delay(1000);
  }
}

// OTA over-the-air update stuff ///////////////////////////////////////////
void doOTAUpdate() {             // the main OTA logic
  // materials for doing an HTTP GET on github from the BinFiles/ dir
  HTTPClient http; // manage the HTTP request process
  int respCode;    // the response code from the request (e.g. 404, 200, ...)
  int highestAvailableVersion = -1;  // version of latest firmware on server
  
  // do a GET to read the version file from the cloud
  Serial.println("checking for firmware updates...");
  respCode = doCloudGet(&http, gitID, "version");
  if(respCode == 200) // check response code (-ve on failure)
    highestAvailableVersion = atoi(http.getString().c_str());
  else
    Serial.printf("couldn't get version! rtn code: %d\n", respCode);
  http.end(); // free resources

  // do we know the latest version, and does the firmware need updating?
  if(respCode != 200) {
    Serial.printf("cannot update\n\n");
    return;
  } else if(currentVersion >= highestAvailableVersion) {
    Serial.printf("firmware is up to date\n\n");
    return;
  }          

  // ok, we need to do a firmware update...
  Serial.printf(
    "upgrading firmware from version %d to version %d\n",
    currentVersion, highestAvailableVersion
  );

  /* Team code.
  OTA update logic. 
  */

  //doCloudGet does HTTP GET for .bin file 
  //(checks length of update using HTTPClients getSize(), min = 100k bytes)
  int binCode = doCloudGet(&http, gitID, String(highestAvailableVersion)+".bin");
  if (binCode == 200) { //CHECKS RESPONSE CODE 
    int fileSize = http.getSize();
    if (fileSize >= 100000) { //CHECK FILE SIZE
       Update.begin(fileSize);
       Update.writeStream(http.getStream()); //takes Stream using HTTPClient getStream()
       Serial.printf("update new version: BEGIN");
       Update.end();
       if (Update.isFinished()) {
          //update = true; 
          Serial.println("update new version: SUCCESSFULLY COMPLETED. Rebooting...");
          currentVersion = highestAvailableVersion;
          Serial.println(currentVersion);
          delay(1000);
          ESP.restart(); //restart ESP, if update successful 
       } else {
          Update.getError();
       }
    } else {
    Serial.println("update new version: INCORRECT FILE SIZE");
    }
  } else {
    Serial.println("update new version: UNABLE TO LOCATE UPDATE FILE.");
  }
}


// helper for downloading from cloud firmware server via HTTP GET
int doCloudGet(HTTPClient *http, String gitID, String fileName) {
  // build up URL from components; for example:
  // http://com3505.gate.ac.uk/repos/com3505-labs-2018-adalovelace/BinFiles/2.bin
  String baseUrl =
    "http://com3505.gate.ac.uk/repos/";
  String url =
    baseUrl + "com3505-labs-2018-" + gitID + "/BinFiles/" + fileName;

  // make GET request and return the response code
  http->begin(url);
  http->addHeader("User-Agent", "ESP32");
  return http->GET();
}

// misc utilities //////////////////////////////////////////////////////////
// get the ESP's MAC address
void getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion...
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
}

// LED blinkers
void ledOn()  { digitalWrite(BUILTIN_LED, HIGH); }
void ledOff() { digitalWrite(BUILTIN_LED, LOW); }
void blink(int times, int pause) {
  ledOff();
  for(int i=0; i<times; i++) {
    ledOn(); delay(pause); ledOff(); delay(pause);
  }
}

// utility for printing IP addresses
String ip2str(IPAddress address) {
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}


