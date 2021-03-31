#include "MqttClient.h"
#include "AppContext.h"

RTC_DATA_ATTR long lastMqttPublishTime[4] = { 0, 0, 0, 0 };

MqttClient::MqttClient(AppContext& ctx) :
    _ctx(ctx)
{
}

MqttClient::~MqttClient()
{
}

void MqttClient::Init()
{
    auto& sm = _ctx.GetSettingsMgr();
    if (sm.GetIntValue(Setting::MQTT_ENABLED) != 0)
    {
        auto port = sm.GetIntValue(Setting::MQTT_PORT);
        _server = sm.GetStringValue(Setting::MQTT_SERVER);        
        _user = sm.GetStringValue(Setting::MQTT_USER);
        _key = sm.GetStringValue(Setting::MQTT_KEY);
        _prefix = sm.GetStringValue(Setting::MQTT_TOPIC);
        _mqtt = std::make_shared<Adafruit_MQTT_Client>(&_client, _server.c_str(), port, _user.c_str(), _key.c_str());
    }
}

void MqttClient::Update()
{
    if (_mqtt != nullptr && _ctx.GetNetworkMgr().WifiConnected())
    {
        if (!_mqtt->connected())
        {
            int8_t ret = _mqtt->connect();
            if (ret != 0)
            {
                Serial.println(_mqtt->connectErrorString(ret));
                return;
            }
        }
        
        auto now = _ctx.GetClock().Now().utcTime;
        auto& sensors = _ctx.GetSensorMgr();
        auto publishValue = [&](const char* name, float value, int cacheIdx)
        {
            lastMqttPublishTime[cacheIdx] = std::min(lastMqttPublishTime[cacheIdx], now);
            if (lastMqttPublishTime[cacheIdx] + 60 < now)
            {
                std::string topic = _prefix + name;
                Serial.println(String("MQTT publish: ") + topic.c_str() + " = " + value);
                Adafruit_MQTT_Publish pub(_mqtt.get(), topic.c_str());
                if (pub.publish(value))
                {
                    lastMqttPublishTime[cacheIdx] = now;
                }
                else
                {
                    Serial.println("MQTT publish failed");
                }
            }   
        };

        publishValue("soilmoisture", sensors.GetSoilMoisture(), 0);
        publishValue("watertanklevel", sensors.GetWaterTankLevel(), 1);
        publishValue("batteryvoltage", sensors.GetBatVoltage(), 2);
        publishValue("temperature", sensors.GetTemperature(), 3);
    }
}