#include "dotenv.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

using std::cout;
using std::endl;
using std::cerr;

namespace dotenv {
    static inline string trim(const string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            start++;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }

        return s.substr(start, end - start);
    }

    static std::vector<string> parse_list(const string& s) {
        std::vector<string> list;
        string trimmed = trim(s);

        if (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']') {
            trimmed = trimmed.substr(1, trimmed.size() - 2);
        }

        stringstream ss(trimmed);
        string item;
        while (std::getline(ss, item, ',')) {
            item = trim(item);

            if (!item.empty() && (item.front() == '\"' || item.front() == '\'')) {
                char quote = item.front();
                if (item.back() == quote && item.size() >= 2) {
                    item = item.substr(1, item.size() - 2);
                }
            }

            if (!item.empty()) {
                list.push_back(item);
            }
        }

        return list;
    }


    EnvSingleton& EnvSingleton::get_instance() {
        static EnvSingleton instance;
        return instance;
    }

    bool EnvSingleton::load_env(const string& filepath) {
        std::lock_guard<std::mutex> lock(mtx_);

        EnvMap temp_env_map;
        std::ifstream env_file(filepath);
        if (!env_file.is_open()) {
            cerr << "Failed to open " << filepath << endl;
            return false;
        }

        string line;
        int line_number = 0;
        while (std::getline(env_file, line)) {
            line_number++;

            line = trim(line);

            if (line.empty() || line[0] == '#') {
                continue;
            }
            size_t delimiter_pos = line.find('=');
            if (delimiter_pos == string::npos) {
                cerr << "Invalid line in " << filepath << " at line " << line_number << ": " << line << endl;
                continue;
            }

            string key = trim(line.substr(0, delimiter_pos));
            string value = trim(line.substr(delimiter_pos + 1));

            if (!value.empty() && (value.front() == '\"' || value.front() == '\'')) {
                char quote = value.front();
                if (value.back() == quote && value.size() >= 2) {
                    value = value.substr(1, value.size() - 2);
                }
            }

            if (key.find("LIST") != string::npos) {
                std::vector<string> list = parse_list(value);
                temp_env_map[key] = list;
                cout << "Loaded list variable [" << key << "]: ";
                for(const auto& item : list) {
                    cout << item << " ";
            }
            cout << endl;
        } else {
            temp_env_map[key] = value;
            if(!key.find("KEY"))
                cout << "Loaded string variable [" << key << "]: " << value << endl;
        }
    }

    env_file.close();

    env_map_ = std::move(temp_env_map);
    cout << "Environment variables loaded successfully from " << filepath << endl;
    return true;
}

std::optional<dotenv::EnvValue> EnvSingleton::get(const string& key) const {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = env_map_.find(key);
    if (it != env_map_.end()) {
        return it->second;
    }
    return std::nullopt;
}

}
