#pragma once
#include <map>
#include <string>
#include <vector>

namespace mtk {
class Config {
private:
    std::map<std::string, std::vector<std::string>> values_;

    bool Strip(std::string &target, const std::string &pattern);
    bool Strip(std::string &target, const std::vector<std::string> &patterns);
    void Add(const std::string &key, const std::string &val);
    const std::vector<std::string> &Value(const std::string &key) const;
    bool Has(const std::string &key) const;
public:
    Config() : values_() {}
    void Parse(int argc, char *argv[]);
    void Parse(const char *csvpath, const char *keycol, const char *valcol);
public:
    template <class T> inline T Value(const std::string &key, const T &def) {
        const auto &v = Source(key);
        if (v.size() <= 0) { return def; }
        return ToValue<T>(v);
    }
    bool FileValue(const std::string &key, std::string &value);
    bool ConfigFileValue(const std::string &key, const std::string &keycol, const std::string &valcol);
protected:
    template <class T> static T
    ToValue(const std::vector<std::string> &s) { return (T)s[0]; }
    const std::vector<std::string> &Source(const std::string &key) const;
};
//specialization
template <> inline int32_t Config::ToValue<int32_t>(const std::vector<std::string> &s) { return (int32_t)std::stoi(s[0]); }
template <> inline int64_t Config::ToValue<int64_t>(const std::vector<std::string> &s) { return (int64_t)std::stoll(s[0]); }
template <> inline uint32_t  Config::ToValue<uint32_t>(const std::vector<std::string> &s) { return (uint32_t)std::stoul(s[0]); }
template <> inline uint64_t Config::ToValue<uint64_t>(const std::vector<std::string> &s) { return (uint64_t)std::stoull(s[0]); }
template <> inline float Config::ToValue<float>(const std::vector<std::string> &s) { return std::stof(s[0]); }
template <> inline double Config::ToValue<double>(const std::vector<std::string> &s) { return std::stod(s[0]); }
template <> inline std::string Config::ToValue<std::string>(const std::vector<std::string> &s) { return s[0]; }
template <> inline std::vector<std::string> Config::ToValue<std::vector<std::string>>(const std::vector<std::string> &s) { return s; }
}
