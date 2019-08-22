#include <EspNowFloodingMesh.h>
#include <CommandParser.h>
Commands cmd;
int bsid = 0x112233;

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#define ESP_NOW_CHANNEL 1
//AES 128bit
unsigned char secredKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
unsigned char iv[16] = {0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};

void espNowFloodingMeshRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  char replyPrtStr[15];
  sprintf(replyPrtStr, "%lu", replyPrt);
  cmd.send("REC", replyPrtStr, data, len);
}

void setup() {
  Serial.begin(115200);
  cmd.begin(Serial);
  delay(1000);
  //Set device in AP mode to begin with
  espNowFloodingMesh_RecvCB(espNowFloodingMeshRecv);

  espNowFloodingMesh_ErrorDebugCB([](int level, const char *str) {
    if (level == 0) {
      cmd.send("ERROR", str);
    }
    if (level == 1) {
      cmd.send("WRN", str);
    }
    if (level == 2) {
      cmd.send("INFO", str);
    }
  });
  Serial.println();
  Serial.println();
  Serial.println();
  cmd.send("READY");
}

int channel = 1;
char buf[20];
bool initialized = false;
void loop() {
  espNowFloodingMesh_loop();
  cmd.handleInputCommands([](const char* cmdName, const char*p1, const char*p2, const char*p3, const unsigned char*binary, int size) {

    if (strcmp(cmdName, "PING") == 0) {
      cmd.send("ACK");
    } else if (strcmp(cmdName, "CHANNEL") == 0) {
      if (strcmp(p1, "SET") == 0) {
        channel = atoi(p2);
        cmd.send("ACK", itoa(channel, buf, 10));
      } else if (strcmp(p1, "GET") == 0) {
        cmd.send("ACK", itoa(channel, buf, 10));
      } else {
        cmd.send("NACK", "PARAM");
      }
    } else if (strcmp(cmdName, "ROLE") == 0) {
      if (strcmp(p1, "MASTER") == 0) {
        int ttl = (p2 == NULL ? 0 : atoi(p2));
        espNowFloodingMesh_setToMasterRole(true, ttl);
        cmd.send("ACK", p2);
      } else   if (strcmp(p1, "SLAVE") == 0) {
        espNowFloodingMesh_setToMasterRole(false);
        cmd.send("ACK");
      } else {
        cmd.send("NACK", "INVALID ROLE");
      }
    } else if (strcmp(cmdName, "SEND") == 0) {
      int ttl = 0;
      if (p1 != 0) {
        ttl = atoi(p1);
      }
      espNowFloodingMesh_send((uint8_t*)binary, size, ttl);
      cmd.send("ACK");
    } else if (strcmp(cmdName, "REQ") == 0) {
      int ttl = 0;
      if (p1 != 0) {
        ttl = atoi(p1);
      }
      uint32_t replyptr = espNowFloodingMesh_sendAndHandleReply((uint8_t*)binary, size, ttl, NULL);
      sprintf(buf, "%lu", replyptr);
      cmd.send("ACK", buf);
    } else if (strcmp(cmdName, "REPLY") == 0) {
      int ttl = 0;
      uint32_t replyPrt;
      if (p1 == NULL || p2 == NULL) {
        cmd.send("ACK", "INVALID PARAM");
      } else {
        ttl = atoi(p1);
        replyPrt = Commands::sTolUint(p2);
        espNowFloodingMesh_sendReply((uint8_t*)binary, size, ttl, replyPrt);
        cmd.send("ACK", buf);
      }
    } else if (strcmp(cmdName, "STOP") == 0) {
      espNowFloodingMesh_end();
      cmd.send("ACK");
    } else if (strcmp(cmdName, "REBOOT") == 0) {
      cmd.send("ACK", "Rebooting");
      Serial.flush();
      ESP.restart();
      while (1);
    } else if (strcmp(cmdName, "INIT") == 0) {
      if (initialized == false) {
        initialized = true;
        espNowFloodingMesh_secredkey(secredKey);
        espNowFloodingMesh_setAesInitializationVector(iv);
        espNowFloodingMesh_begin(channel, bsid);
        cmd.send("ACK");
      } else {
        cmd.send("NACK", "REBOOT NEEDED");
      }
    } else if (strcmp(cmdName, "RTC") == 0) {
      if (strcmp(p1, "GET") == 0) {
        time_t t = espNowFloodingMesh_getRTCTime();
        sprintf(buf, "%lu", t);
        cmd.send("ACK", buf);
      } else if (strcmp(p1, "SET") == 0) {
        time_t t = Commands::sTolUint(p2);
        espNowFloodingMesh_setRTCTime(t);
        sprintf(buf, "%lu", t);
        cmd.send("ACK", buf);
      } else {
        cmd.send("NACK", "INVALID PARAMETER");
      }
    } else if (strcmp(cmdName, "KEY") == 0) {
      if (strcmp(p1, "SET") == 0) {
        if (size == sizeof(secredKey)) {
          memcpy(secredKey, binary, sizeof(secredKey));
          espNowFloodingMesh_secredkey(secredKey);
          cmd.send("ACK");
        } else {
          cmd.send("NACK", "SIZE!=16");
        }
      } else if (strcmp(p1, "GET") == 0) {
        cmd.send("ACK", secredKey, sizeof(secredKey));
      } else {
        cmd.send("NACK", "PARAM");
      }
    }
    else if (strcmp(cmdName, "MEM") == 0) {
      sprintf(buf, "%d", ESP.getFreeHeap());
      cmd.send("ACK", buf);

    }
    else if (strcmp(cmdName, "IV") == 0) {
      if (strcmp(p1, "SET") == 0) {
        if (size == sizeof(secredKey)) {
          memcpy(iv, binary, sizeof(iv));
          espNowFloodingMesh_setAesInitializationVector(iv);
          cmd.send("ACK");
        } else {
          cmd.send("NACK", "SIZE!=16");
        }
      } else if (strcmp(p1, "GET") == 0) {
        cmd.send("ACK", secredKey, sizeof(secredKey));
      } else {
        cmd.send("NACK", "PARAM");
      }
    }
    else if (strcmp(cmdName, "BSID") == 0) {
      if (strcmp(p1, "SET") == 0) {
        if (strlen(p2)>0) {
          bsid = atoi(p2);
          cmd.send("ACK");
        } else {
          cmd.send("NACK", "INVALID SIZE");
        }
      } else if (strcmp(p1, "GET") == 0) {
        sprintf(buf, "%ud", bsid);
        cmd.send("ACK", buf);
      } else {
        cmd.send("NACK", "PARAM");
      }
    } else if (strcmp(cmdName, "MAC") == 0) {
      String mac = WiFi.macAddress();
      cmd.send("ACK", (const unsigned char *)mac.c_str(), 6);
    }
    else {
      cmd.send("NACK", "INVALID COMMAND"); //Handle invalid command
    }
  });
}
