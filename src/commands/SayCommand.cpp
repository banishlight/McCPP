#include <commands/SayCommand.hpp>
#include <CommandSender.hpp>
#include <network/packets/Play.hpp>
#include <Console.hpp>
#include <sstream>

void SayCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    if (args.empty()) {
        sender.sendMessage("Usage: /say <message>");
        return;
    }
    std::ostringstream oss;
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) oss << " ";
        oss << args[i];
    }
    string message = oss.str();
    // minecraft:say_command's decoration ("chat.type.announcement") renders
    // as "[sender] message" on connected clients -- echoed here in the same
    // form so the console log matches what players actually see. Broadcasting
    // alone doesn't reach the console: the console isn't a Play connection,
    // so BroadcastDisguisedChat never touches it (same reason Chat_Message_p
    // logs player chat to console itself instead of relying on the broadcast).
    Console::getConsole().Entry("[" + sender.getName() + "] " + message);
    BroadcastDisguisedChat(sender.getName(), message, "minecraft:say_command");
}

string SayCommand::getName() const {
    return "say";
}

string SayCommand::getDescription() const {
    return "Broadcasts a message to all players as the server";
}
