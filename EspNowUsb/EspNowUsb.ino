#include <EspNowAESBroadcast.h>
#include <CommandParser.h>
Commands cmd;

#define ESP_NOW_CHANNEL 1
//AES 128bit
unsigned char secredKey[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

void espNowAESBroadcastRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  char replyPrtStr[10];
  sprintf(replyPrtStr, "%lu", replyPrt);
  cmd.send("REC", replyPrtStr, data, len);
}

void setup() {
  Serial.begin(115200);
  cmd.begin(Serial);
  delay(1000);
  //Set device in AP mode to begin with
  espNowAESBroadcast_RecvCB(espNowAESBroadcastRecv);

  espNowAESBroadcast_ErrorDebugCB([](int level, const char *str) {
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
  cmd.send("READY");
}

int channel = 1;
char buf[20];
bool initialized = false;

void loop() {
  espNowAESBroadcast_loop();
  delay(1);

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
        int ttl = p2 == NULL ? 0 : atoi(p2);
        espNowAESBroadcast_setToMasterRole(true, ttl);
        cmd.send("ACK");
      } else   if (strcmp(p1, "SLAVE") == 0) {
        espNowAESBroadcast_setToMasterRole(false);
        cmd.send("ACK");
      } else {
        cmd.send("NACK", "INVALID ROLE");
      }
    } else if (strcmp(cmdName, "SEND") == 0) {
      int ttl = 0;
      if (p1 != 0) {
        ttl = atoi(p1);
      }
      espNowAESBroadcast_send((uint8_t*)binary, size, ttl);
      cmd.send("ACK");
    } else if (strcmp(cmdName, "REQ") == 0) {
      int ttl = 0;
      if (p1 != 0) {
        ttl = atoi(p1);
      }
      uint32_t replyptr = espNowAESBroadcast_sendAndHandleReply((uint8_t*)binary, size, ttl, NULL);
      sprintf(buf, "%lu", replyptr);
      cmd.send("ACK", buf);
    } else if (strcmp(cmdName, "REPLY") == 0) {
      int ttl = 0;
      uint32_t replyPrt;
      if (p1 == NULL || p2 == NULL) {
        cmd.send("ACK", "INVALID PARAM");
      } else {
        ttl = atoi(p1);
        sscanf(p2, "%u", &replyPrt);

        espNowAESBroadcast_sendReply((uint8_t*)binary, size, ttl, replyPrt);
        cmd.send("ACK", buf);
      }
    } else if (strcmp(cmdName, "STOP") == 0) {
      espNowAESBroadcast_end();
      cmd.send("ACK");
    } else if (strcmp(cmdName, "REBOOT") == 0) {
      cmd.send("ACK", "Rebooting");
      Serial.flush();
      ESP.restart();
      while (1);
    } else if (strcmp(cmdName, "INIT") == 0) {
      if (initialized == false) {
        initialized = true;
        espNowAESBroadcast_secredkey(secredKey);
        espNowAESBroadcast_begin(channel);
        cmd.send("ACK");
      } else {
        cmd.send("NACK", "REBOOT NEEDED");
      }
    } else if (strcmp(cmdName, "RTC") == 0) {
      if (strcmp(p1, "GET") == 0) {
        time_t t = espNowAESBroadcast_getRTCTime();
        sprintf(buf, "%lu", t);
        cmd.send("ACK", buf);
      } else if (strcmp(p1, "SET") == 0) {
        time_t t;
        sscanf(p2, "%lu", &t);
        espNowAESBroadcast_setRTCTime(t);
        sprintf(buf, "%lu", t);
        cmd.send("ACK", buf);
      } else {
        cmd.send("NACK", "INVALID PARAMETER");
      }
    } else if (strcmp(cmdName, "KEY") == 0) {
      if (strcmp(p1, "SET") == 0) {
        if (size == sizeof(secredKey)) {
          memcpy(secredKey, binary, size);
          espNowAESBroadcast_secredkey(secredKey);
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
    else {
      cmd.send("NACK", "INVALID COMMAND"); //Handle invalid command
    }
  });
}
