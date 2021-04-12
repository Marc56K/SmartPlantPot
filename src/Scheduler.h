#pragma once

class AppContext;

class Scheduler
{
public:
    Scheduler(AppContext& ctx);
    ~Scheduler();

    void Init();
    void Update();

    long GetNextWakupUtcTime(int& hour, int& minute);
    long GetSecondsUntilWake();

private:
    AppContext& _ctx;
};