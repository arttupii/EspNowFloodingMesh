#ifndef ___SIMPLE_MQTT_H_
#define ___SIMPLE_MQTT_H_
#include<vector>
#include<string>
class SimpleMQTT {
    public:
        SimpleMQTT(int ttl);
        ~SimpleMQTT();
        void setDeviceName(const char *name);
        bool publishI(const char* parameterName, long long value);
        bool publishF(const char* parameterName, float value);
        bool publishB(const char* parameterName, bool value);
        bool publishS(const char* parameterName, const char *value);

        bool subscribeTopic(const char* devName, const char *valName);
        void parse(const unsigned char *data, int size, uint32_t replyId);
        const char* getBuffer();

        void handleSubscribe(void (*cb)(const char *, const char* ));

        bool compareTopic(const char* topic, const char* deviceName, const char* t);
    private:
        const char *deviceName;
        char buffer[100];
        uint32_t replyId;
        
        void (*subscribeCallBack)(const char *topic, const char* value);
        
        void parse2(const char *c,int l);
        bool send(const char *mqttMsg, int len, uint32_t replyId);

        std::vector<char*> topicVector;
        void addtopicToVector(char *topic);
        int ttl;
};
#endif
