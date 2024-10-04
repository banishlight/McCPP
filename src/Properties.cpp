#include <Standards.hpp>
#include <Properties.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
// using namespace std;


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
    // TODO write all default variables to file server.properties
    std::cout << "generate default properties file";
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
    string identifier;
    string value;
    splitLine(input, identifier, value);
    // This could get gross, hopefully there is a better way.
    if (input[0] == '#') {
        return; // Ignore comment line in file
    }
    else if (identifier.compare("allow-flight")) {
        if (value.compare("true")) {
            this->allow_flight = true;
        }
    }
    else if (identifier.compare("allow-nether")) {
        if (value.compare("false")) {
            this->allow_nether = false;
        }
    }
    else if (identifier.compare("broadcast-console-to-ops")) {
        if (value.compare("false")) {
            this->broadcast_console_to_ops = false;
        }
    }
    else if (identifier.compare("broadcast-rcon-to-ops")) {
        if (value.compare("false")) {
            this->broadcast_rcon_to_ops = false;
        }
    }
    else if (identifier.compare("difficulty")) {
        if (value.compare("peaceful")) {
            this->difficulty = Peaceful;
        }
        else if (value.compare("normal")) {
            this->difficulty = Normal;
        }
        else if (value.compare("hard")) {
            this->difficulty = Hard;
        }
    }
    else if (identifier.compare("enable-command-block")) {
        if (value.compare("true")) {
            this->enable_command_block = true;
        }
    }
    else if (identifier.compare("enable-query")) {
        if (value.compare("true")) {
            this->enable_query = true;
        }
    }
    else if (identifier.compare("enable-status")) {
        if (value.compare("false")) {
            this->enable_status = true;
        }
    }
    else if (identifier.compare("enforce-whitelist")) {
        if (value.compare("true")) {
            this->enforce_whitelist = true;
        }
    }
    else if (identifier.compare("force-gamemode")) {
        if (value.compare("true")) {
            this->force_gamemode = true;
        }
    }
    else if (identifier.compare("function-permission-level")) {
        this->function_permission_level = std::stoi(value);
    }
    else if (identifier.compare("gamemode")) {
        if (value.compare("creative") || value.compare("adventure") || 
        value.compare("spectator")) {
            this->gamemode = value;
        }
    }
    else if (identifier.compare("generate-structures")) {
        if (value.compare("false")) {
            this->generate_structures = false;
        }
    }
    else if (identifier.compare("generator-settings")) {
        // TODO: This takes a JSON string
    }
    else if (identifier.compare("hardcore")) {
        if (value.compare("true")) {
            this->hardcore = true;
        }
    }
    else if (identifier.compare("hide-online-players")) {
        if (value.compare("true")) {
            this->hide_online_players = true;
        }
    }
    else if (identifier.compare("level-name")) {
        this->level_name = value;
    }
    else if (identifier.compare("level-seed")) {
        this->level_seed = value;
    }
    else if (identifier.compare("level-type")) {
        this->level_type = value;
    }
    else if (identifier.compare("max-players")) {
        this->max_players = std::stoi(value);
    }
    else if (identifier.compare("max-tick-time")) {
        this->max_tick_time = std::stoi(value);
    }
    else if (identifier.compare("max-world-size")) {
        this->max_world_size = std::stoi(value);
    }
    else if (identifier.compare("motd")) {
        this->motd = value;
    }
    else if (identifier.compare("network-compression-threshold")) {
        this->network_compression_threshold = std::stoi(value);
    }
    else if (identifier.compare("online-mode")) {
        if (value.compare("false")) {
            this->online_mode = false;
        }
    }
    else if (identifier.compare("op-permission-level")) {
        this->op_permission_level = std::stoi(value);
    }
    else if (identifier.compare("player-idle-timeout")) {
        this->player_idle_timeout = std::stoi(value);
    }
    else if (identifier.compare("prevent-proxy-connections")) {
        if (value.compare("true")) {
            this->prevent_proxy_connections = std::stoi(value);
        }
    }
    else if (identifier.compare("pvp")) {
        if (value.compare("false")) {
            this->pvp = false;
        }
    }
    else if (identifier.compare("query.port")) {
        this->query_port = value;
    }
    else if (identifier.compare("rate-limit")) {
        this->rate_limit = std::stoi(value);
    }
    else if (identifier.compare("rcon.password")) {
        this->rcon_password = value;
    }
    else if (identifier.compare("rcon.port")) {
        this->rcon_port = value;
    }
    else if (identifier.compare("require-resource-pack")) {
        if (value.compare("true")) {
            this->require_resource_pack = true;
        }
    }
    else if (identifier.compare("resource-pack")) {
        this->resource_pack = value;
    }
    else if (identifier.compare("resource-pack-prompt")) {
        this->resource_pack_prompt = value;
    }
    else if (identifier.compare("resource-pack-sha1")) {
        this->resource_pack_sha1 = value;
    }
    else if (identifier.compare("server-ip")) {
        this->server_ip = value;
    }
    else if (identifier.compare("server-port")) {
        this->server_port = value;
    }
    else if (identifier.compare("simulation-distance")) {
        this->simulation_distance = std::stoi(value);
    }
    else if (identifier.compare("spawn-animals")) {
        if (value.compare("false")) {
            this->spawn_animals = false;
        }
    }
    else if (identifier.compare("spawn-monsters")) {
        if (value.compare("false")) {
            this->spawn_monsters = false;
        }
    }
    else if (identifier.compare("spawn-npcs")) {
        if (value.compare("false")) {
            this->spawn_npcs = false;
        }
    }
    else if (identifier.compare("spawn-protection")) {
        this->spawn_protection = std::stoi(value);
    }
    else if (identifier.compare("sync-chunk-writes")) {
        if (value.compare("false")) {
            this->sync_chunk_writes = false;
        }
    }
    else if (identifier.compare("text-filtering-config")) {
        this->text_filtering_config = value;
    }
    else if (identifier.compare("use-native-transport")) {
        if (value.compare("false")) {
            this->use_native_transport = false;
        }
    }
    else if (identifier.compare("view-distance")) {
        this->view_distance = std::stoi(value);
    }
    else if (identifier.compare("white-list")) {
        if (value.compare("true")) {
            this->white_list = true;
        }
    }
    else {
        // unknown identifier read.
    }
}

void Properties::splitLine(string line, string identifier, string value) {
    std::istringstream iss(line);
    getline(iss, identifier, '=');
    getline(iss, value);
}

string Properties::getIP() {
    if (server_ip.compare("")) {
        // ERROR no ip set, throw exception
    }
    return server_ip;
}

