#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstring>

template <typename T>
void pushBytes(std::vector<std::byte>& vec, const T& value) {
    const std::byte* begin = reinterpret_cast<const std::byte*>(&value);
    const std::byte* end = begin + sizeof(T);
    vec.insert(vec.end(), begin, end);
}

void pushString(std::vector<std::byte>& vec, const std::string& str);

template <typename T>
void readBytes(const std::vector<std::byte>& vec, size_t& offset, T& value) {
    if (offset + sizeof(T) > vec.size())
        throw std::runtime_error("Unexpected end of file");
    std::memcpy(&value, vec.data() + offset, sizeof(T));
    offset += sizeof(T);
}

void readString(const std::vector<std::byte>& vec, size_t& offset, std::string& str, size_t length);