#include "../include/Standards.hpp"
#include "../include/Properties.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

class Properties {

    public:
        enum Difficulties {
            Peaceful,
            Easy,
            Normal,
            Hard
        };

        Properties::Properties() {
            // if the file exists, loadValues().
            // if it does not, generateDefault().
            std::ifstream file("server.properties");
            if (file.is_open()) { loadValues(file); }
            else { generateDefault(); }
        }

        ~Properties() {
            
        }

        // Defaults for Vanilla found here https://minecraft.fandom.com/wiki/Server.properties
        bool allow_flight = false; 
        bool allow_nether = true; 
        bool broadcast_console_to_ops = true; 
        bool broadcast_rcon_to_ops = true; 
        Difficulties difficulty = Easy; 
        bool enable_command_block = false; 
        bool enable_jmx_monitoring = false; 
        bool enable_query = false; 
        bool enable_status = true; 
        bool enforce_whitelist = false; 
        int entity_broadcast_range_percentage = 100; 
        bool force_gamemode = false; 
        int function_permission_level = 2; 
        string gamemode = "survival"; 
        bool generate_structures = true; 
        string generator_settings = ""; 
        bool hardcore = false;
        bool hide_online_players = false; 
        string level_name = "world"; 
        string level_seed = ""; 
        string level_type = "minecraft:normal"; 
        int max_players = 20; 
        int max_tick_time = 60000; 
        int max_world_size = 29999984; 
        string motd = "Minecraft++ Server"; 
        int network_compression_threshold = 256; 
        bool online_mode = true; 
        int op_permission_level = 4;
        int player_idle_timeout = 0; 
        bool prevent_proxy_connections = false; 
        bool pvp = true; 
        string query_port = "25565"; 
        int rate_limit = 0;
        string rcon_password = ""; 
        string rcon_port = "25575";
        bool require_resource_pack = false; 
        string resource_pack = ""; 
        string resource_pack_prompt = ""; 
        string resource_pack_sha1 = ""; 
        string server_ip = ""; 
        string server_port = "25565"; 
        int simulation_distance = 10; 
        bool spawn_animals = true; 
        bool spawn_monsters = true; 
        bool spawn_npcs = true; 
        int spawn_protection = 16; 
        bool sync_chunk_writes = true; 
        string text_filtering_config = ""; 
        bool use_native_transport = true; 
        int view_distance = 10; 
        bool white_list = false; 

    private:
        void generateDefault() {
            // write all default variables to file server.properties
            std::cout << "generate default properties file";
        }

        void loadValues(std::ifstream& file) {

            string line;
            while (getline(file, line)) {
                checkString(line);
            }
            file.close();
        }

        void checkString(string input) {
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
                    allow_flight = true;
                }
            }
            else if (identifier.compare("allow-nether")) {
                if (value.compare("false")) {
                    allow_nether = false;
                }
            }
            else if (identifier.compare("broadcast-console-to-ops")) {
                if (value.compare("false")) {
                    broadcast_console_to_ops = false;
                }
            }
            else if (identifier.compare("broadcast-rcon-to-ops")) {
                if (value.compare("false")) {
                    broadcast_rcon_to_ops = false;
                }
            }
            else if (identifier.compare("difficulty")) {
                if (value.compare("peaceful")) {
                    difficulty = Peaceful;
                }
                else if (value.compare("normal")) {
                    difficulty = Normal;
                }
                else if (value.compare("hard")) {
                    difficulty = Hard;
                }
            }
            else if (identifier.compare("enable-command-block")) {
                if (value.compare("true")) {
                    enable_command_block = true;
                }
            }
            else if (identifier.compare("enable-query")) {
                if (value.compare("true")) {
                    enable_query = true;
                }
            }
            else if (identifier.compare("enable-status")) {
                if (value.compare("false")) {
                    enable_status = true;
                }
            }
            else if (identifier.compare("enforce-whitelist")) {
                if (value.compare("true")) {
                    enforce_whitelist = true;
                }
            }
            else if (identifier.compare("force-gamemode")) {
                if (value.compare("true")) {
                    force_gamemode = true;
                }
            }
            else if (identifier.compare("function-permission-level")) {
                function_permission_level = std::stoi(value);
            }
            else if (identifier.compare("gamemode")) {
                if (value.compare("creative") || value.compare("adventure") || 
                value.compare("spectator")) {
                    gamemode = value;
                }
            }
            else if (identifier.compare("generate-structures")) {
                if (value.compare("false")) {
                    generate_structures = false;
                }
            }
            else if (identifier.compare("generator-settings")) {
                // TODO: This takes a JSON string
            }
            else if (identifier.compare("hardcore")) {
                if (value.compare("true")) {
                    hardcore = true;
                }
            }
            else if (identifier.compare("hide-online-players")) {
                if (value.compare("true")) {
                    hide_online_players = true;
                }
            }
            else if (identifier.compare("level-name")) {
                level_name = value;
            }
            else if (identifier.compare("level-seed")) {
                level_seed = value;
            }
            else if (identifier.compare("level-type")) {
                level_type = value;
            }
            else if (identifier.compare("max-players")) {
                max_players = std::stoi(value);
            }
            else if (identifier.compare("max-tick-time")) {
                max_tick_time = std::stoi(value);
            }
            else if (identifier.compare("max-world-size")) {
                max_world_size = std::stoi(value);
            }
            else if (identifier.compare("motd")) {
                motd = value;
            }
            else if (identifier.compare("network-compression-threshold")) {
                network_compression_threshold = std::stoi(value);
            }
            else if (identifier.compare("online-mode")) {
                if (value.compare("false")) {
                    online_mode = false;
                }
            }
            else if (identifier.compare("op-permission-level")) {
                op_permission_level = std::stoi(value);
            }
            else if (identifier.compare("player-idle-timeout")) {
                player_idle_timeout = std::stoi(value);
            }
            else if (identifier.compare("prevent-proxy-connections")) {
                if (value.compare("true")) {
                    prevent_proxy_connections = std::stoi(value);
                }
            }
            else if (identifier.compare("pvp")) {
                if (value.compare("false")) {
                    pvp = false;
                }
            }
            else if (identifier.compare("query-port")) {
                query_port = value;
            }
            else if (identifier.compare("rate-limit")) {
                rate_limit = std::stoi(value);
            }
            else if (identifier.compare("rcon-password")) {
                rcon_password = value;
            }
            else if (identifier.compare("rcon-port")) {
                rcon_port = value;
            }
            else if (identifier.compare("require-resource-pack")) {
                if (value.compare("true")) {
                    require_resource_pack = true;
                }
            }
            else if (identifier.compare("resource-pack")) {
                resource_pack = value;
            }
            else if (identifier.compare("resource-pack-prompt")) {
                resource_pack_prompt = value;
            }
            else if (identifier.compare("resource-pack-sha1")) {
                resource_pack_sha1 = value;
            }
            else if (identifier.compare("server-ip")) {
                server_ip = value;
            }
            else if (identifier.compare("server-port")) {
                server_port = value;
            }
            else if (identifier.compare("simulation-distance")) {
                simulation_distance = std::stoi(value);
            }
            else if (identifier.compare("spawn-animals")) {
                if (value.compare("false")) {
                    spawn_animals = false;
                }
            }
            else if (identifier.compare("spawn-monsters")) {
                if (value.compare("false")) {
                    spawn_monsters = false;
                }
            }
            else if (identifier.compare("spawn-npcs")) {
                if (value.compare("false")) {
                    spawn_npcs = false;
                }
            }
            else if (identifier.compare("spawn-protection")) {
                spawn_protection = std::stoi(value);
            }
            else if (identifier.compare("sync-chunk-writes")) {
                if (value.compare("false")) {
                    sync_chunk_writes = false;
                }
            }
            else if (identifier.compare("text-filtering-config")) {
                text_filtering_config = value;
            }
            else if (identifier.compare("use-native-transport")) {
                if (value.compare("false")) {
                    use_native_transport = false;
                }
            }
            else if (identifier.compare("view-distance")) {
                view_distance = std::stoi(value);
            }
            else if (identifier.compare("white-list")) {
                if (value.compare("true")) {
                    white_list = true;
                }
            }
            else {
                // unknown identifier read.
            }
        }

        void splitLine(string line, string identifier, string value) {
            std::istringstream iss(line);
            getline(iss, identifier, '=');
            getline(iss, value);
        }

};