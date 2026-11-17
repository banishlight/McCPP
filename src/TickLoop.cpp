#include <TickLoop.hpp>
#include <systems/KeepAliveSystem.hpp>
#include <systems/ItemDespawnSystem.hpp>
#include <systems/ItemPhysicsSystem.hpp>
#include <systems/FallingBlockSystem.hpp>
#include <systems/FluidSystem.hpp>
#include <systems/CropGrowthSystem.hpp>
#include <systems/AutosaveSystem.hpp>
#include <systems/ChunkUnloadSystem.hpp>
#include <systems/DayNightSystem.hpp>
#include <Console.hpp>
#include <chrono>
#include <sstream>
#include <iomanip>

TickLoop& TickLoop::getInstance() {
    static TickLoop instance;
    return instance;
}

TickLoop::~TickLoop() {
    close();
}

void TickLoop::initialize() {
    if (_initialized) return;
    registerSystem(std::make_shared<KeepAliveSystem>());
    registerSystem(std::make_shared<ItemPhysicsSystem>());
    registerSystem(std::make_shared<FallingBlockSystem>());
    registerSystem(std::make_shared<FluidSystem>());
    registerSystem(std::make_shared<CropGrowthSystem>());
    registerSystem(std::make_shared<ItemDespawnSystem>());
    registerSystem(std::make_shared<AutosaveSystem>());
    registerSystem(std::make_shared<ChunkUnloadSystem>());
    registerSystem(std::make_shared<DayNightSystem>());
    _tickThread = std::thread(&TickLoop::tickThreadLoop, this);
    _initialized = true;
}

void TickLoop::close() {
    if (!_initialized) return;
    running = false;
    if (_tickThread.joinable()) _tickThread.join();
    _initialized = false;
}

void TickLoop::registerSystem(std::shared_ptr<TickSystem> system) {
    _systems.push_back(std::move(system));
}

void TickLoop::tickThreadLoop() {
    using Clock = std::chrono::steady_clock;
    auto nextTick = Clock::now();

    // Overrun reporting: entities are simulated client-side at a steady 20
    // ticks/sec, so any tick that runs long makes the server fall behind
    // what clients show (visible as moving entities snapping back). Report
    // overruns at most once per interval, with the worst tick's per-system
    // breakdown so the cause is obvious.
    constexpr double REPORT_INTERVAL_SECONDS = 5.0;
    auto lastReport = Clock::now();
    int ticksSinceReport = 0, overrunsSinceReport = 0;
    double worstTickMs = 0.0;
    std::vector<double> systemMs(_systems.size(), 0.0), worstSystemMs(_systems.size(), 0.0);

    while (running) {
        nextTick += std::chrono::milliseconds(TICK_RATE_MS);
        _tickCount++;
        const auto tickStart = Clock::now();
        for (size_t i = 0; i < _systems.size(); i++) {
            auto& system = _systems[i];
            const auto systemStart = Clock::now();
            // An exception escaping a thread's own function calls std::terminate,
            // crashing the whole process -- for every connected player, not just
            // whatever one system was doing (found via a real crash: a filesystem
            // error from AutosaveSystem's save path, previously uncaught here,
            // took the entire server down). Catching per-system means one
            // system misbehaving for a tick doesn't even block the others from
            // still running that same tick.
            try {
                system->onTick(_tickCount);
            } catch (const std::exception& e) {
                Console::getConsole().Error("TickLoop::tickThreadLoop(): Unhandled exception in system '" + system->getName() + "': " + e.what());
            } catch (...) {
                Console::getConsole().Error("TickLoop::tickThreadLoop(): Unknown unhandled exception in system '" + system->getName() + "'.");
            }
            systemMs[i] = std::chrono::duration<double, std::milli>(Clock::now() - systemStart).count();
        }

        const double tickMs = std::chrono::duration<double, std::milli>(Clock::now() - tickStart).count();
        ticksSinceReport++;
        if (tickMs > TICK_RATE_MS) {
            overrunsSinceReport++;
            if (tickMs > worstTickMs) {
                worstTickMs = tickMs;
                worstSystemMs = systemMs;
            }
        }
        if (std::chrono::duration<double>(Clock::now() - lastReport).count() >= REPORT_INTERVAL_SECONDS) {
            if (overrunsSinceReport > 0) {
                std::ostringstream msg;
                msg << std::fixed << std::setprecision(1)
                    << "Can't keep up! " << overrunsSinceReport << " of the last " << ticksSinceReport
                    << " ticks took longer than " << TICK_RATE_MS << "ms. Worst: " << worstTickMs << "ms (";
                bool first = true;
                for (size_t i = 0; i < _systems.size(); i++) {
                    if (worstSystemMs[i] < 0.1) continue;
                    msg << (first ? "" : ", ") << _systems[i]->getName() << " " << worstSystemMs[i] << "ms";
                    first = false;
                }
                msg << ")";
                Console::getConsole().Error(msg.str());
            }
            lastReport = Clock::now();
            ticksSinceReport = overrunsSinceReport = 0;
            worstTickMs = 0.0;
        }
        #ifdef DEBUG
            if (_tickCount % 200 == 0) {
                Console::getConsole().Entry("TickLoop::tickThreadLoop(): tick " + std::to_string(_tickCount));
            }
        #endif
        auto now = std::chrono::steady_clock::now();
        if (nextTick > now) {
            std::this_thread::sleep_for(nextTick - now);
        } else {
            nextTick = now; // fell behind; don't spiral trying to catch up
        }
    }
}