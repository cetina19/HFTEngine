#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <nlohmann/json.hpp>

using std::string;

class Order {
public:
    Order(const string& symbol,
          const string& qty,
          const string& side,
          const string& type,
          const string& time_in_force)
        : symbol(symbol), qty(qty), side(side), type(type), time_in_force(time_in_force) {}

    string toJSON() const {
        nlohmann::json order_json = {
            {"action", "order"},
            {"symbol", symbol},
            {"qty", qty},
            {"side", side},
            {"type", type},
            {"time_in_force", time_in_force}
        };
        return order_json.dump();
    }

    string symbol;
    string qty;
    string side;
    string type;
    string time_in_force;
};

#endif