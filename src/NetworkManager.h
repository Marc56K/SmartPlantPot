#pragma once
#include <string>
#include <vector>
#include <WiFi.h>

class AppContext;

class NetworkManager
{
public:
    NetworkManager(AppContext& ctx);
    ~NetworkManager();

    bool WifiConnected();

    void Init();
    void Update();

private:
    AppContext& _ctx;
    std::string _wifiSSID;
    std::string _wifiKey;
    std::vector<WiFiEventId_t> _eventIds;
};