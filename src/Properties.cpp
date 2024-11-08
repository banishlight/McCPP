#include <Standards.hpp>
#include <Properties.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <Console.hpp>


Properties::Properties() {
    // if the file exists, loadValues().
    // if it does not, generateDefault().
    std::ifstream file("server.properties");
    if (file.is_open()) { loadValues(file); }
    else { generateDefault(); }
}

Properties::~Properties() {
    // maybe save changes to the properties that might've taken place while it was running?
}

// This  is a thread safe singleton starting with C++11
Properties& Properties::getProperties() {
    static Properties singleton;
    return singleton;
}

string Properties::difficulty_to_string(Difficulties d) {
    switch(d) {
        case Easy: return "easy";
        case Normal: return "normal";
        case Hard: return "hard";
        case Peaceful: return "peaceful";
        default: return "normal";
    }
}

void Properties::generateDefault() {
    Console::getConsole().Entry("generating default properties file");
    std::ofstream MyFile("server.properties");
    MyFile << "allow-flight=" + std::to_string(allow_flight) + "\n";
    MyFile << "allow-nether=" + std::to_string(allow_nether) + "\n";
    MyFile << "broadcast-console-to-ops=" + std::to_string(broadcast_console_to_ops) + "\n";
    MyFile << "broadcast-rcon-to-ops=" + std::to_string(broadcast_rcon_to_ops) + "\n";
    MyFile << "difficulty=" + difficulty_to_string(difficulty) + "\n";
    MyFile << "enable-command-block=" + std::to_string(enable_command_block) + "\n";
    MyFile << "enable-jmx-monitoring=" + std::to_string(enable_jmx_monitoring) + "\n";
    MyFile << "enable-query=" + std::to_string(enable_query) + "\n";
    MyFile << "enable-status=" + std::to_string(enable_status) + "\n";
    MyFile << "enfore-whitelist=" + std::to_string(enforce_whitelist) + "\n";
    MyFile << "entity-broadcast-range-percentage=" + std::to_string(entity_broadcast_range_percentage) + "\n";
    MyFile << "force-gamemode=" + std::to_string(force_gamemode) + "\n";
    MyFile << "function-permission-level=" + std::to_string(function_permission_level) + "\n";
    MyFile << "gamemode=" + gamemode + "\n";
    MyFile << "generate-structures=" + std::to_string(generate_structures) + "\n";
    MyFile << "generator-settings=" + generator_settings + "\n";
    MyFile << "hardcore=" + std::to_string(hardcore) + "\n";
    MyFile << "hide-online-players=" + std::to_string(hide_online_players) + "\n";
    MyFile << "level-name=" + level_name + "\n";
    MyFile << "level-seed=" + level_seed + "\n";
    MyFile << "level-type=" + level_type + "\n";
    MyFile << "max-players=" + std::to_string(max_players) + "\n";
    MyFile << "max-tick-time=" + std::to_string(max_tick_time) + "\n";
    MyFile << "max-world-size=" + std::to_string(max_world_size) + "\n";
    MyFile << "motd=" + motd + "\n";
    MyFile << "network-compression-threshold=" + std::to_string(network_compression_threshold) + "\n";
    MyFile << "online-mode=" + std::to_string(online_mode) + "\n";
    MyFile << "op-permission-level=" + std::to_string(op_permission_level) + "\n";
    MyFile << "player-idle-timeout=" + std::to_string(player_idle_timeout) + "\n";
    MyFile << "prevent-proxy-connections=" + std::to_string(prevent_proxy_connections) + "\n";
    MyFile << "pvp=" + std::to_string(pvp) + "\n";
    MyFile << "query.port=" + query_port + "\n";
    MyFile << "rate-limit=" + std::to_string(rate_limit) + "\n";
    MyFile << "rcon.password=" + rcon_password + "\n";
    MyFile << "rcon.port=" + rcon_port + "\n";
    MyFile << "require-resource-pack=" + std::to_string(require_resource_pack) + "\n";
    MyFile << "resource-pack=" + resource_pack + "\n";
    MyFile << "resource-pack-prompt=" + resource_pack_prompt + "\n";
    MyFile << "resource-pack-sha1=" + resource_pack_sha1 + "\n";
    MyFile << "server-ip=" + server_ip + "\n";
    MyFile << "server-port=" + server_port + "\n";
    MyFile << "simulation-distance=" + std::to_string(simulation_distance) + "\n";
    MyFile << "spawn-animals=" + std::to_string(spawn_animals) + "\n";
    MyFile << "spawn-monsters=" + std::to_string(spawn_monsters) + "\n";
    MyFile << "spawn-npcs=" + std::to_string(spawn_npcs) + "\n";
    MyFile << "spawn-protection=" + std::to_string(spawn_protection) + "\n";
    MyFile << "sync-chunk-writes=" + std::to_string(sync_chunk_writes) + "\n";
    MyFile << "text-filtering-config=" + text_filtering_config + "\n";
    MyFile << "use-native-transport=" + std::to_string(use_native_transport) + "\n";
    MyFile << "view-distance=" + std::to_string(view_distance) + "\n";
    MyFile << "white-list=" + std::to_string(white_list) + "\n";
}

void Properties::loadValues(std::ifstream& file) {
    string line;
    while (getline(file, line)) {
        checkString(line);
    }
    file.close();
}

void Properties::checkString(string input) {
    // Split identifier and data here
    string identifier, value;
    std::istringstream iss(input);
    getline(iss, identifier, '=');
    getline(iss, value);
    // This could get gross, hopefully there is a better way.
    #ifdef DEBUG
        Console::getConsole().Entry("Identifier found: [" + identifier + "]");
        Console::getConsole().Entry("Value found: [" + value + "]");
    #endif
    if (input[0] == '#') {
        return; // Ignore comment line in file
    }
    else if (identifier.compare("allow-flight") == 0) {
        if (value.compare("true")) {
            this->allow_flight = true;
        }
    }
    else if (identifier.compare("allow-nether") == 0) {
        if (value.compare("false")) {
            this->allow_nether = false;
        }
    }
    else if (identifier.compare("broadcast-console-to-ops") == 0) {
        if (value.compare("false")) {
            this->broadcast_console_to_ops = false;
        }
    }
    else if (identifier.compare("broadcast-rcon-to-ops") == 0) {
        if (value.compare("false")) {
            this->broadcast_rcon_to_ops = false;
        }
    }
    else if (identifier.compare("difficulty") == 0) {
        if (value.compare("peaceful") == 0) {
            this->difficulty = Peaceful;
        }
        else if (value.compare("normal") == 0) {
            this->difficulty = Normal;
        }
        else if (value.compare("hard") == 0) {
            this->difficulty = Hard;
        }
    }
    else if (identifier.compare("enable-command-block") == 0) {
        if (value.compare("true") == 0) {
            this->enable_command_block = true;
        }
    }
    else if (identifier.compare("enable-query") == 0) {
        if (value.compare("true") == 0) {
            this->enable_query = true;
        }
    }
    else if (identifier.compare("enable-status") == 0) {
        if (value.compare("false") == 0) {
            this->enable_status = true;
        }
    }
    else if (identifier.compare("enforce-whitelist") == 0) {
        if (value.compare("true") == 0) {
            this->enforce_whitelist = true;
        }
    }
    else if (identifier.compare("force-gamemode") == 0) {
        if (value.compare("true") == 0) {
            this->force_gamemode = true;
        }
    }
    else if (identifier.compare("function-permission-level") == 0) {
        this->function_permission_level = std::stoi(value);
    }
    else if (identifier.compare("gamemode") == 0) {
        if (value.compare("creative") == 0 || value.compare("adventure") == 0 || 
        value.compare("spectator") == 0) {
            this->gamemode = value;
        }
    }
    else if (identifier.compare("generate-structures") == 0) {
        if (value.compare("false") == 0) {
            this->generate_structures = false;
        }
    }
    else if (identifier.compare("generator-settings") == 0) {
        // TODO: This takes a JSON string
    }
    else if (identifier.compare("hardcore") == 0) {
        if (value.compare("true") == 0) {
            this->hardcore = true;
        }
    }
    else if (identifier.compare("hide-online-players") == 0) {
        if (value.compare("true") == 0) {
            this->hide_online_players = true;
        }
    }
    else if (identifier.compare("level-name") == 0) {
        this->level_name = value;
    }
    else if (identifier.compare("level-seed") == 0) {
        this->level_seed = value;
    }
    else if (identifier.compare("level-type") == 0) {
        this->level_type = value;
    }
    else if (identifier.compare("max-players") == 0) {
        this->max_players = std::stoi(value);
    }
    else if (identifier.compare("max-tick-time") == 0) {
        this->max_tick_time = std::stoi(value);
    }
    else if (identifier.compare("max-world-size") == 0) {
        this->max_world_size = std::stoi(value);
    }
    else if (identifier.compare("motd") == 0) {
        this->motd = value;
    }
    else if (identifier.compare("network-compression-threshold") == 0) {
        this->network_compression_threshold = std::stoi(value);
    }
    else if (identifier.compare("online-mode") == 0) {
        if (value.compare("false") == 0) {
            this->online_mode = false;
        }
    }
    else if (identifier.compare("op-permission-level") == 0) {
        this->op_permission_level = std::stoi(value);
    }
    else if (identifier.compare("player-idle-timeout") == 0) {
        this->player_idle_timeout = std::stoi(value);
    }
    else if (identifier.compare("prevent-proxy-connections") == 0) {
        if (value.compare("true") == 0) {
            this->prevent_proxy_connections = std::stoi(value);
        }
    }
    else if (identifier.compare("pvp") == 0) {
        if (value.compare("false") == 0) {
            this->pvp = false;
        }
    }
    else if (identifier.compare("query.port") == 0) {
        this->query_port = value;
    }
    else if (identifier.compare("rate-limit") == 0) {
        this->rate_limit = std::stoi(value);
    }
    else if (identifier.compare("rcon.password") == 0) {
        this->rcon_password = value;
    }
    else if (identifier.compare("rcon.port") == 0) {
        this->rcon_port = value;
    }
    else if (identifier.compare("require-resource-pack") == 0) {
        if (value.compare("true") == 0) {
            this->require_resource_pack = true;
        }
    }
    else if (identifier.compare("resource-pack") == 0) {
        this->resource_pack = value;
    }
    else if (identifier.compare("resource-pack-prompt") == 0) {
        this->resource_pack_prompt = value;
    }
    else if (identifier.compare("resource-pack-sha1") == 0) {
        this->resource_pack_sha1 = value;
    }
    else if (identifier.compare("server-ip") == 0) {
        this->server_ip = value;
    }
    else if (identifier.compare("server-port") == 0) {
        this->server_port = value;
    }
    else if (identifier.compare("simulation-distance") == 0) {
        this->simulation_distance = std::stoi(value);
    }
    else if (identifier.compare("spawn-animals") == 0) {
        if (value.compare("false") == 0) {
            this->spawn_animals = false;
        }
    }
    else if (identifier.compare("spawn-monsters") == 0) {
        if (value.compare("false") == 0) {
            this->spawn_monsters = false;
        }
    }
    else if (identifier.compare("spawn-npcs") == 0) {
        if (value.compare("false") == 0) {
            this->spawn_npcs = false;
        }
    }
    else if (identifier.compare("spawn-protection") == 0) {
        this->spawn_protection = std::stoi(value);
    }
    else if (identifier.compare("sync-chunk-writes") == 0) {
        if (value.compare("false") == 0) {
            this->sync_chunk_writes = false;
        }
    }
    else if (identifier.compare("text-filtering-config") == 0) {
        this->text_filtering_config = value;
    }
    else if (identifier.compare("use-native-transport") == 0) {
        if (value.compare("false") == 0) {
            this->use_native_transport = false;
        }
    }
    else if (identifier.compare("view-distance") == 0) {
        this->view_distance = std::stoi(value);
    }
    else if (identifier.compare("white-list") == 0) {
        if (value.compare("true") == 0) {
            this->white_list = true;
        }
    }
    else {
        // unknown identifier read.
    }
}

string Properties::getIP() {
    if (this->server_ip.compare("") == 0) {
        // ERROR no ip set, throw exception
        Console::getConsole().Error("Properties::getIP() : No IP set.");
    }
    return this->server_ip;
}

string Properties::getPort() {
    return this->server_port;
}

