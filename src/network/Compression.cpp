#include <network/Compression.hpp>
#include <Standards.hpp>
#include <zlib.h>


std::vector<Byte> decompressData(const std::vector<Byte>& compressedData) {
    std::vector<Byte> decompressed;
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(compressedData.data());
    stream.avail_in = static_cast<uInt>(compressedData.size());
    if (inflateInit(&stream) != Z_OK) {
        // Console::getConsole().Error("Failed to initialize zlib decompression");
        return decompressed;
    }
    const size_t CHUNK_SIZE = 4096;
    std::vector<Byte> buffer(CHUNK_SIZE);
    int ret;
    do {
        stream.next_out = buffer.data();
        stream.avail_out = CHUNK_SIZE;
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            inflateEnd(&stream);
            // Console::getConsole().Error("Zlib decompression error: " + std::to_string(ret));
            return std::vector<Byte>();
        }
        size_t bytesProduced = CHUNK_SIZE - stream.avail_out;
        decompressed.insert(decompressed.end(), buffer.begin(), buffer.begin() + bytesProduced);
    } while (ret != Z_STREAM_END);
    inflateEnd(&stream);
    return decompressed;
}

std::vector<Byte> compressData(const std::vector<Byte>& data) {
    std::vector<Byte> compressed;
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(data.data());
    stream.avail_in = static_cast<uInt>(data.size());
    
    if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK) {
        // Console::getConsole().Error("Failed to initialize zlib compression");
        return compressed;
    }
    
    const size_t CHUNK_SIZE = 4096;
    std::vector<Byte> buffer(CHUNK_SIZE);
    
    int ret;
    do {
        stream.next_out = buffer.data();
        stream.avail_out = CHUNK_SIZE;
        ret = deflate(&stream, Z_FINISH);
        
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            deflateEnd(&stream);
            // Console::getConsole().Error("Zlib compression error: " + std::to_string(ret));
            return std::vector<Byte>();
        }
        
        size_t bytesProduced = CHUNK_SIZE - stream.avail_out;
        compressed.insert(compressed.end(), buffer.begin(), buffer.begin() + bytesProduced);
        
    } while (ret != Z_STREAM_END);
    
    deflateEnd(&stream);
    return compressed;
}