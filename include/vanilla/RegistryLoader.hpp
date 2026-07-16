#pragma once

// Ensures the JSON for every Configuration-state registry MC++ needs to sync
// with clients (dimension_type, worldgen/biome, damage_type, etc.) is present
// on disk under registry_data/. If that folder doesn't already exist, extracts
// it from a vanilla server.jar placed next to the server binary. Never
// modifies or redistributes server.jar itself; only reads a locally-supplied
// copy once at startup.
bool ensureRegistryDataExtracted();