#pragma once

class AppContext;

class Scheduler
{
public:
    Scheduler(AppContext& ctx);
    ~Scheduler();

    void Init();
    void Update();

    void GetNextWakupUtcTime(int& hour, int& minute);

private:
    AppContext& _ctx;
};