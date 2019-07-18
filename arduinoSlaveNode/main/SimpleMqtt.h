#ifndef ___SIMPLE_MQTT_H_
#define ___SIMPLE_MQTT_H_
#include<vector>
#include<string>
class SimpleMQTT {
    public:
        SimpleMQTT();
        ~SimpleMQTT();
        void setDeviceName(const char *name);
        bool publishI(const char* parameterName, long long value);
        bool publishF(const char* parameterName, float value);
        bool publishB(const char* parameterName, bool value);
        bool publishS(const char* parameterName, const char *value);

        bool subscribe(const char* parameterName);
        bool subscribeUri(const char* devName, const char *valName);
        void parse(const unsigned char *data, int size);
        const char* getBuffer();

        void handleSubscribe(void (*cb)(const char *, const char* ));
        void handleSend(void (*cb)(const char *, int));

    private:
        const char *deviceName;
        char buffer[100];
        void (*subscribeCallBack)(const char *uri, const char* value);
        void (*sendCallBack)(const char *data, int len);
        
        void parse2(const char *c,int l);

        std::vector<char*> uriVector;
        void addUriToVector(char *uri);
};
#endif
