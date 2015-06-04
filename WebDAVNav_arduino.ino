
#include <SD.h>
#include <Ethernet.h>
#include <SPI.h>
#include <avr/pgmspace.h>

#define SS_SD_CARD   4
#define SS_ETHERNET 10
#define FLASH_TEXT(name)   static const unsigned char name[] PROGMEM

byte mac[]  = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 1, 177 };
EthernetServer server(80);

FLASH_TEXT(HTTP_NOT_FOUND) = "HTTP/1.1 404 Not Found";
FLASH_TEXT(HTTP_200_FOUND) = "HTTP/1.1 200 OK";
FLASH_TEXT(HTTP_207_FOUND) = "HTTP/1.1 207 Multi Status";
FLASH_TEXT(HTTP_405_METHOD_NOT_ALLOWED) = "HTTP/1.1 405 Method Not Allowed";
FLASH_TEXT(HTTP_OPTIONS_HEADERS) = "Allow: PROPFIND, GET\nDAV: 1, 2";

FLASH_TEXT(HTTP_XML_CONTENT) = "Content-Type: application/xml;";
FLASH_TEXT(HTTP_HTML_CONTENT) = "Content-Type: text/html";
FLASH_TEXT(HTTP_CONTENT_TYPE) = "Content-Type: ";
FLASH_TEXT(MIME_JPEG) = "image/jpeg";
FLASH_TEXT(MIME_PNG) = "image/jpeg";
FLASH_TEXT(MIME_BIN) = "application/octet-stream";

FLASH_TEXT(MULTISTATUS_START) = "<?xml version=\"1.0\" ?><D:multistatus xmlns:D=\"DAV:\">";
FLASH_TEXT(MULTISTATUS_END) = "</D:multistatus>";
FLASH_TEXT(RESPONSE_START) = "<D:response>";
FLASH_TEXT(RESPONSE_END) = "</D:response>";
FLASH_TEXT(HREF_START) = "<D:href>";
FLASH_TEXT(HREF_END) = "</D:href>";
FLASH_TEXT(PROPSTAT_START) = "<D:propstat>";
FLASH_TEXT(PROPSTAT_END) = "</D:propstat>";
FLASH_TEXT(PROP_START) = "<D:prop>";
FLASH_TEXT(PROP_END) = "</D:prop>";
FLASH_TEXT(CONTENTLEN_START) = "<D:getcontentlength>";
FLASH_TEXT(CONTENTLEN_END) = "</D:getcontentlength>";
FLASH_TEXT(RESOURCETYPE_START) = "<D:resourcetype>";
FLASH_TEXT(RESOURCETYPE_END) = "</D:resourcetype>";
FLASH_TEXT(RESOURCE_COLLECTION) = "<D:collection/>";
FLASH_TEXT(CREATEDATE_START) = "<D:creationdate>";
FLASH_TEXT(CREATEDATE_END) = "</D:creationdate>";
FLASH_TEXT(MODDATE_START) = "<D:getlastmodified>";
FLASH_TEXT(MODDATE_END) = "</D:getlastmodified>";
FLASH_TEXT(STATUS_OK) = "<D:status>HTTP/1.1 200 OK</D:status>";

#define MAX_STRING 60

char stringBuffer[MAX_STRING];

char* getString(const unsigned char* str) {
  strcpy_P(stringBuffer, (char*)str);
  return stringBuffer;
}

void setup() {
  Serial.begin(9600);
  pinMode(SS_SD_CARD, OUTPUT);
  pinMode(SS_ETHERNET, OUTPUT);
  //Disable SD and ETHERNET to begin with, libraries handle after that
  digitalWrite(SS_SD_CARD, HIGH);
  digitalWrite(SS_ETHERNET, HIGH);

  if (!SD.begin(4)) {
    Serial.println("SD fail!");
    return;
  }

  Ethernet.begin(mac, ip);
  server.begin();
}

void ListFiles(EthernetClient client, char *folderPath, File folder, int format) {
  client.println(getString(MULTISTATUS_START));
  folder.rewindDirectory();
  while (true) {

    File entry =  folder.openNextFile();
    if (! entry) {
      folder.rewindDirectory();
      break;
    }
    client.print(getString(RESPONSE_START));
    client.print(getString(HREF_START));

    client.print(folderPath);
    client.print(entry.name());

    client.print(getString(HREF_END));
    client.print(getString(PROPSTAT_START));
    client.print(getString(PROP_START));


    if (entry.isDirectory()) {
      client.print(getString(RESOURCETYPE_START));
      client.print(getString(RESOURCE_COLLECTION));
      client.print(getString(RESOURCETYPE_END));
    } else {
      client.print(getString(CONTENTLEN_START));
      client.print(entry.size(), DEC);
      client.print(getString(CONTENTLEN_END));

    }

    client.print(getString(PROP_END));
    client.print(getString(STATUS_OK));
    client.print(getString(PROPSTAT_END));
    client.println(getString(RESPONSE_END));

    entry.close();

  }
  client.println(getString(MULTISTATUS_END));

}

#define BUFFER_SIZE 110

void not_found_404(EthernetClient client) {
  client.println(getString(HTTP_NOT_FOUND));
  client.println(getString(HTTP_HTML_CONTENT));
  client.println();
}

void not_allowed_405(EthernetClient client) {
  client.println(getString(HTTP_405_METHOD_NOT_ALLOWED));
  client.println();
}
void loop()
{
  char reqest_line[BUFFER_SIZE];
  int index = 0;
  EthernetClient client = server.available();
  if (client) {
    index = 0;
    while (client.connected()) {
      if (client.available()) {
          char c = client.read();
          if (c != '\n' && c != '\r') {
              reqest_line[index] = c;
              index++;
              if (index >= BUFFER_SIZE) {
                index = BUFFER_SIZE - 1;
              }
              continue;
           }
           reqest_line[index] = 0;
           //GET /folder/test.txt HTTP/1.1
           char *filename;

        
           (strstr(reqest_line, " HTTP"))[0] = 0;

           if (strstr(reqest_line, "PROPFIND ") != 0) {
              //curl --data "" --header "depth:1"  --header "Content-Type: text/xml" --request PROPFIND http://192.168.1.177/
              filename = reqest_line + 9; 
              File dataFile = SD.open(filename);
              if (! dataFile) {
                not_found_404(client);
                break;
              }
              if (dataFile.isDirectory()) {
                client.println(getString(HTTP_207_FOUND));
                client.println(getString(HTTP_XML_CONTENT));
                client.println();
                ListFiles(client, filename, dataFile, 0);
              } else {
                not_allowed_405(client);
              }
              dataFile.close();
           } else if (strstr(reqest_line, "GET ") != 0) {
              filename = reqest_line + 5; 
              File dataFile = SD.open(filename);
              if (! dataFile) {
                not_found_404(client);
                break;
              }
              if (dataFile.isDirectory()) {
                ListFiles(client, filename, dataFile, 1);
              } else {
                client.println(getString(HTTP_200_FOUND));
                client.println(getString(HTTP_CONTENT_TYPE));
                if (strstr(reqest_line, ".JPG") != 0) {
                  client.println(getString(MIME_JPEG));
                } else if (strstr(reqest_line, ".PNG") != 0) {
                  client.println(getString(MIME_PNG));
                } else {
                  client.println(getString(MIME_BIN));
                }
                client.println();
                char buf[42];
                int16_t  num_read;
                while (dataFile.available()) {
                  num_read = dataFile.read(buf, 42);
                  client.write(buf, 42);
                }
              }
              dataFile.close();
          } else if (strstr(reqest_line, "OPTIONS ") != 0) {
              client.println(getString(HTTP_200_FOUND));
              client.println(getString(HTTP_OPTIONS_HEADERS));
              client.println();
          } else {
              not_allowed_405(client);
          }
          break;
       }
    }
    delay(1);
    client.stop();
  }
}
