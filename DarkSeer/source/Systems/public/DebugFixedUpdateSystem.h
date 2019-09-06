#pragma once

struct DebugFixedUpdateSystem
{
        static constexpr std::chrono::milliseconds TickRate = std::chrono::milliseconds(100);

        static void FixedUpdate(EntityAdmin* entityAdmin);
};