#include "byte_util.h"

void pushString(std::vector<std::byte>& vec, const std::string& str) {
    const std::byte* begin = reinterpret_cast<const std::byte*>(str.data());
    vec.insert(vec.end(), begin, begin + str.size());
}

void readString(const std::vector<std::byte>& vec, size_t& offset, std::string& str, size_t length) {
    if (offset + length > vec.size())
        throw std::runtime_error("Unexpected end of file");
    str.assign(reinterpret_cast<const char*>(vec.data() + offset), length);
    offset += length;
}