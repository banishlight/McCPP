#include <network/HttpsClient.hpp>
#include <Console.hpp>
#include <cstring>
#include <algorithm>
#include <cctype>

#ifdef LINUX
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace {
    constexpr int TIMEOUT_SECONDS = 5;

    // RAII wrapper for the socket fd + OpenSSL handles so every early-return
    // failure path in HttpsClient::Get still cleans up, without a goto/cleanup-
    // label chain -- mirrors the project's other single-purpose RAII wrappers
    // (e.g. StreamCipher's EVP_CIPHER_CTX ownership).
    struct TlsConnection {
        int fd = -1;
        SSL_CTX* ctx = nullptr;
        SSL* ssl = nullptr;

        ~TlsConnection() {
            if (ssl) {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            if (ctx) SSL_CTX_free(ctx);
            if (fd >= 0) close(fd);
        }
    };

    bool connectSocket(const std::string& host, int& outFd) {
        struct addrinfo hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* result = nullptr;
        if (getaddrinfo(host.c_str(), "443", &hints, &result) != 0 || !result) {
            Console::getConsole().Error("HttpsClient::Get(): DNS resolution failed for " + host);
            return false;
        }

        int fd = -1;
        for (struct addrinfo* p = result; p != nullptr; p = p->ai_next) {
            fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd < 0) continue;
            struct timeval timeout{TIMEOUT_SECONDS, 0};
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
            close(fd);
            fd = -1;
        }
        freeaddrinfo(result);
        if (fd < 0) {
            Console::getConsole().Error("HttpsClient::Get(): Failed to connect to " + host);
            return false;
        }
        outFd = fd;
        return true;
    }

    // Case-insensitive search for a header's value on its own line within the
    // raw header block (headers are ASCII, so a simple lowercase compare is enough).
    bool findHeaderValue(const std::string& headers, const std::string& name, std::string& value) {
        std::string lowerHeaders = headers;
        std::transform(lowerHeaders.begin(), lowerHeaders.end(), lowerHeaders.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        size_t pos = lowerHeaders.find(lowerName + ":");
        if (pos == std::string::npos) return false;
        size_t valueStart = pos + lowerName.size() + 1;
        size_t lineEnd = headers.find("\r\n", valueStart);
        if (lineEnd == std::string::npos) lineEnd = headers.size();
        value = headers.substr(valueStart, lineEnd - valueStart);
        // Trim leading/trailing whitespace
        size_t first = value.find_first_not_of(" \t");
        size_t last = value.find_last_not_of(" \t");
        value = (first == std::string::npos) ? "" : value.substr(first, last - first + 1);
        return true;
    }

    // Decodes a chunked-transfer-encoded body (hex size line, that many bytes,
    // repeated until a zero-size chunk) -- Mojang's response framing isn't
    // guaranteed to be Content-Length, so this has to be handled too.
    std::string decodeChunked(const std::string& raw) {
        std::string decoded;
        size_t pos = 0;
        while (pos < raw.size()) {
            size_t lineEnd = raw.find("\r\n", pos);
            if (lineEnd == std::string::npos) break;
            std::string sizeLine = raw.substr(pos, lineEnd - pos);
            size_t chunkSize = 0;
            try {
                chunkSize = std::stoul(sizeLine, nullptr, 16);
            } catch (...) {
                break;
            }
            if (chunkSize == 0) break;
            size_t dataStart = lineEnd + 2;
            if (dataStart + chunkSize > raw.size()) break;
            decoded.append(raw, dataStart, chunkSize);
            pos = dataStart + chunkSize + 2; // skip trailing \r\n after the chunk data
        }
        return decoded;
    }
}

namespace HttpsClient {
    Response Get(const std::string& host, const std::string& path) {
        Response response;

        TlsConnection conn;
        if (!connectSocket(host, conn.fd)) return response;

        conn.ctx = SSL_CTX_new(TLS_client_method());
        if (!conn.ctx) {
            Console::getConsole().Error("HttpsClient::Get(): Failed to create SSL context");
            return response;
        }
        SSL_CTX_set_min_proto_version(conn.ctx, TLS1_2_VERSION);
        SSL_CTX_set_verify(conn.ctx, SSL_VERIFY_PEER, nullptr);
        if (SSL_CTX_set_default_verify_paths(conn.ctx) != 1) {
            Console::getConsole().Error("HttpsClient::Get(): Failed to load system trust store");
            return response;
        }

        conn.ssl = SSL_new(conn.ctx);
        if (!conn.ssl) {
            Console::getConsole().Error("HttpsClient::Get(): Failed to create SSL handle");
            return response;
        }
        SSL_set_fd(conn.ssl, conn.fd);
        SSL_set_tlsext_host_name(conn.ssl, host.c_str()); // SNI
        SSL_set1_host(conn.ssl, host.c_str()); // hostname verification against the cert

        if (SSL_connect(conn.ssl) != 1) {
            Console::getConsole().Error("HttpsClient::Get(): TLS handshake failed for " + host);
            return response;
        }
        if (SSL_get_verify_result(conn.ssl) != X509_V_OK) {
            Console::getConsole().Error("HttpsClient::Get(): Certificate verification failed for " + host);
            return response;
        }

        std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host +
                               "\r\nConnection: close\r\nUser-Agent: MCCPP\r\n\r\n";
        if (SSL_write(conn.ssl, request.data(), static_cast<int>(request.size())) <= 0) {
            Console::getConsole().Error("HttpsClient::Get(): Failed to send request to " + host);
            return response;
        }

        std::string raw;
        char buffer[4096];
        while (true) {
            int n = SSL_read(conn.ssl, buffer, sizeof(buffer));
            if (n <= 0) break; // peer closed (or a real error -- either way, nothing more to read)
            raw.append(buffer, n);
        }

        size_t headerEnd = raw.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            Console::getConsole().Error("HttpsClient::Get(): Malformed HTTP response from " + host);
            return response;
        }
        std::string headers = raw.substr(0, headerEnd);
        std::string rawBody = raw.substr(headerEnd + 4);

        size_t statusStart = headers.find(' ');
        if (statusStart != std::string::npos) {
            response.statusCode = std::atoi(headers.c_str() + statusStart + 1);
        }

        std::string transferEncoding, contentLength;
        if (findHeaderValue(headers, "Transfer-Encoding", transferEncoding) &&
            transferEncoding.find("chunked") != std::string::npos) {
            response.body = decodeChunked(rawBody);
        } else if (findHeaderValue(headers, "Content-Length", contentLength)) {
            size_t len = std::stoul(contentLength);
            response.body = rawBody.substr(0, std::min(len, rawBody.size()));
        } else {
            response.body = rawBody; // read-until-EOF case
        }

        response.success = true;
        return response;
    }
}
#endif

#ifdef WINDOWS
namespace HttpsClient {
    Response Get(const std::string& host, const std::string& path) {
        return Response{};
    }
}
#endif
