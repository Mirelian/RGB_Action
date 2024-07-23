#include "header.h"

WebServer server;

const char *htmlForm = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>ESP32 OTA Update</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
          }
          .container {
            background-color: #fff;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
          }
          h1 {
            font-size: 24px;
            margin-bottom: 20px;
          }
          input[type="file"] {
            margin-bottom: 10px;
          }
          input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
          }
          input[type="submit"]:hover {
            background-color: #45a049;
          }
        </style>
      </head>
      <body>
        <div class="container">
          <h1>ESP32 OTA Update</h1>
          <form method="POST" action="/update" enctype="multipart/form-data">
            <input type="file" name="update">
            <br>
            <input type="submit" value="Update">
          </form>
        </div>
      </body>
      </html>
    )rawliteral";

void beginWebServer()
{
    server.on("/", HTTP_GET, []()
              { server.send(200, "text/html", htmlForm); });
    server.on("/update", HTTP_POST, []()
              {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); }, []()
              {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { // true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    } });
    server.begin();
}