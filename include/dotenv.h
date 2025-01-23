#ifndef DOTENV_H
#define DOTENV_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <vector>
#include <optional>
#include <mutex>

using std::string;
using std::stringstream;
using std::vector;
using std::mutex;
using std::cout;
using std::cin;
using std::cerr;

namespace dotenv {

    using EnvValue = std::variant<string, vector<string>>;

    using EnvMap = std::unordered_map<string, EnvValue>;

    class EnvSingleton {
    public:
        EnvSingleton(const EnvSingleton&) = delete;
        EnvSingleton& operator=(const EnvSingleton&) = delete;

        static EnvSingleton& get_instance();

        bool load_env(const string& filepath = "../.env");
        std::optional<EnvValue> get(const string& key) const;

    private:
        EnvSingleton() = default;

        EnvMap env_map_;

        mutable mutex mtx_;
    };

}

#endif
