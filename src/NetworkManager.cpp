#include "NetworkManager.h"
#include "AppContext.h"

NetworkManager::NetworkManager(AppContext& ctx) :
    _ctx(ctx)
{
    _eventIds.push_back(
        WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
        {
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
        }, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP));

    _eventIds.push_back(
        WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
        {
            WiFi.reconnect();
        }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED));

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
}

NetworkManager::~NetworkManager()
{
    for (auto& eventId : _eventIds)
    {
        WiFi.removeEvent(eventId);
    }
}

bool NetworkManager::WifiConnected()
{
    return WiFi.isConnected();
}

void NetworkManager::Init()
{
    Update();
    uint32_t t = 2000;
    while (!_wifiSSID.empty() && !WifiConnected() && t > 0)
    {
        const uint32_t waitTime = std::min(10u, t);
        vTaskDelay(waitTime);
        t -= waitTime;
    }
}

void NetworkManager::Update()
{
    auto wifiSSID = _ctx.GetSettingsMgr().GetStringValue(Setting::WIFI_SSID);
    auto wifiKey = _ctx.GetSettingsMgr().GetStringValue(Setting::WIFI_KEY);
    if (_wifiSSID != wifiSSID || _wifiKey != wifiKey)
    {
        _wifiSSID = wifiSSID;
        _wifiKey = wifiKey;

	    WiFi.begin(_wifiSSID.c_str(), _wifiKey.c_str());
    }
}