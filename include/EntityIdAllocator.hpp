#pragma once
#include <atomic>

// Thread-safe monotonic entity ID source, shared by every kind of entity
// (players, item drops, ...) so two different entity kinds can never collide
// on the wire -- promoted out of Player, which used to own this counter alone.
class EntityIdAllocator {
    public:
        static int next();
    private:
        static std::atomic<int> _next;
};