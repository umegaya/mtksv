#include "config.h"
#include "csv/csv.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

namespace mtk {
static std::string s_empty = "";
bool Config::Strip(std::string &target, const std::string &pattern) {
	size_t tl = target.length(), pl = pattern.length();
	if (tl <= pl) {
		return false;
	}
	if (target.substr(0, pl) == pattern && target.substr(tl - pl) == pattern) {
		target = target.substr(pl, tl - pl);
		return true;
	} 
	return false;
}
bool Config::Strip(std::string &target, const std::vector<std::string> &patterns) {
	for (auto &p : patterns) {
		if (Strip(target, p)) {
			return true;
		}
	}
	return false;
}
void Config::Add(const std::string &key, const std::string &val) {
	static 	std::vector<std::string> quotes{"'", "\""};
	auto it = values_.find(key);
	if (it == values_.end()) {
		values_[key] = std::vector<std::string>();
	} 
	std::string tmp = val;
	Strip(tmp, quotes);
	values_[key].insert(values_[key].begin(), tmp);
}
const std::vector<std::string> &Config::Source(const std::string &key) const {
	static std::vector<std::string> empty;
	auto it = values_.find(key);
	if (it == values_.end()) {
		return empty;
	}
	return it->second;
}
bool Config::Has(const std::string &key) const {
	return values_.find(key) != values_.end();
}
bool Config::FileValue(const std::string &key, std::string &value) {
	if (!Has(key)) {
		return false;
	} else {
		std::string s = Value<std::string>(key, s_empty);
		struct stat st;
		if (stat(s.c_str(), &st) != 0) {
			return false;
		}
		FILE *fp = fopen(s.c_str(), "r");
		if (fp == nullptr) {
			return false;
		}
		char buffer[st.st_size];
		auto sz = fread(buffer, 1, st.st_size, fp);
		value = std::string(buffer, sz);
		return true;
	}
}
bool Config::ConfigFileValue(const std::string &key, const std::string &keycol, const std::string &valcol) {
	if (!Has(key)) {
		return false;
	} else {
		Parse(Value<std::string>(key, s_empty).c_str(), keycol.c_str(), valcol.c_str());
		return true;
	}
}
void Config::Parse(int argc, char *argv[]) {
	std::string key;
	for (int i = 0; i < argc; i++) {
		std::string arg = std::string(argv[i]);
		if (key.length() > 0) {
			Add(key, arg);
			key.clear();
		} else if (arg.find("--") == 0) {
			std::string body = arg.substr(2);
			size_t pos = body.find("=");
			if (pos != std::string::npos) {
				Add(body.substr(0, pos), body.substr(pos + 1));
			} else {
				key = body;
			}
		} else if (arg.find("-") == 0) {
			key = arg.substr(1);
		}
	}
}
void Config::Parse(const char *csvpath, const char *keycol, const char *valcol) {
	io::CSVReader<2, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '"'>> csv(csvpath);
	csv.read_header(io::ignore_extra_column,keycol,valcol);
	std::string key, val;
	while (csv.read_row(key, val)) {
		Add(key, val);
	}
}
}
