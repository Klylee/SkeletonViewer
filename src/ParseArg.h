#pragma once

#include <string>
#include <unordered_map>

inline std::string getArg(
    const std::unordered_map<std::string, std::string> &args,
    const std::string &key,
    const std::string &defaultValue = "")
{
    auto it = args.find(key);
    if (it != args.end())
        return it->second;
    return defaultValue;
}

template <typename T>
T parse(const std::string &s);

// 特化：int
template <>
inline int parse<int>(const std::string &s)
{
    return std::stoi(s);
}

// 特化：double
template <>
inline double parse<double>(const std::string &s)
{
    return std::stod(s);
}

// 特化：float
template <>
inline float parse<float>(const std::string &s)
{
    return std::stof(s);
}

// 特化：bool
template <>
inline bool parse<bool>(const std::string &s)
{
    return (s == "1" || s == "true" || s == "True");
}

// 通用封装：带默认值
template <typename T>
T getArgAs(
    const std::unordered_map<std::string, std::string> &args,
    const std::string &key,
    const T &defaultValue = T())
{
    auto it = args.find(key);
    if (it != args.end())
        return parse<T>(it->second);
    return defaultValue;
}
