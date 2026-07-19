#include <EntityIdAllocator.hpp>

std::atomic<int> EntityIdAllocator::_next{1};

int EntityIdAllocator::next() {
    return _next++;
}