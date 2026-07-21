#pragma once
#include <Standards.hpp>
#include <string>

// Minimal, narrowly-scoped HTTPS client: a single GET request to a single
// host, used only for Mojang's session-server verification (see
// computeServerHash / Encryption_Response_p). Not a general-purpose client --
// no redirects, no keep-alive, no request body support.
namespace HttpsClient {
    struct Response {
        bool success = false;
        int statusCode = 0;
        std::string body;
    };

    // Blocks the calling thread (connect/TLS handshake/read all use a fixed
    // timeout) -- callers on this project's per-connection ThreadPool tasks
    // must expect this call to take up to a few seconds, same as any other
    // blocking I/O already done on those threads (socket recv, etc.).
    Response Get(const std::string& host, const std::string& path);
}
