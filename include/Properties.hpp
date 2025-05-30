#pragma once
#include <fstream>
#include <Standards.hpp>

// Requires initialization
class Properties {
    public:
        static Properties& getProperties();
        string getIP();
        string getPort();
        string getMotd();
        string getMaxPlayers();
        void initialize();

        enum Difficulties {
            Peaceful,
            Easy,
            Normal,
            Hard
        };

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
        Properties();
        ~Properties();
        string difficulty_to_string(Difficulties d);
        void generateDefault();
        void loadValues(std::ifstream& file);
        void checkString(string input);

        string server_ip = "";
        string server_port = "25565";
};
