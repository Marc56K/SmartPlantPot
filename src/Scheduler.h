#pragma once

class AppContext;

class Scheduler
{
public:
    Scheduler(AppContext& ctx);
    ~Scheduler();

    void Init();
    void Update();

private:
    AppContext& _ctx;
};