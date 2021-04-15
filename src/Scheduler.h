#pragma once

class AppContext;

class Scheduler
{
public:
    Scheduler(AppContext& ctx);
    ~Scheduler();

    void Init();
    void Update();

    long GetSleepDuration();

private:
    AppContext& _ctx;
};