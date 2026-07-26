#pragma once
#include <Standards.hpp>

// Name<->itemId lookup backed by ItemTable, mirroring BlockNames' shape --
// used by PlayerDataPersistence (and anywhere else that needs to save/load
// an item by its real name rather than raw numeric id).
namespace ItemNames {
    // Unknown names return -1 ("no item"/empty slot), NOT any specific
    // fallback item -- unlike BlockNames' stone fallback, there's no safe
    // "default item" to substitute, and an empty slot on a lookup failure is
    // strictly safer than a wrong item silently appearing in someone's
    // inventory. Logged once per distinct unknown name.
    Int32 itemNameToId(const string& name);

    // Returns "" for itemId -1 (empty slot) or any id this project never
    // produces -- callers must treat an empty result as "omit this slot"
    // (matching vanilla's own convention of not listing empty slots).
    string itemIdToName(Int32 itemId);
}
