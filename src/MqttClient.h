#pragma once
#include <memory>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

class AppContext;

class MqttClient
{
public:
    MqttClient(AppContext& ctx);
    ~MqttClient();

    void Init();
    void Update();

private:
    AppContext& _ctx;
    WiFiClient _client;
    std::string _server;
    std::string _user;
    std::string _key;
    std::string _prefix;
    std::shared_ptr<Adafruit_MQTT_Client> _mqtt;
};