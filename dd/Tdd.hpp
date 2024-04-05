#pragma once


#include "Definitions.hpp"
#include "Edge.hpp"
#include "Node.hpp"
#include "../nlohmann/json.hpp"
using json = nlohmann::json;

#include <map>

namespace dd {

    struct Index {
        std::string key; //
        short idx;

        // std::strong_ordering operator<=>(const Index& other) const {
        //     if (auto cmp = idx <=> other.idx; cmp != 0) return cmp;
        //     return key <=> other.key;
        // }

        friend bool operator<(const Index& l, const Index& r) {
            if (l.idx < r.idx) return true;
            else if (l.idx == r.idx) return l.key < r.key;
            return false;
        }

        friend bool operator>(const Index& l, const Index& r) {
            return r < l;
        }

        friend bool operator<=(const Index& l, const Index& r) {
            return !(l > r);
        }

        friend bool operator>=(const Index& l, const Index& r) {
            return !(l < r);
        }

        friend bool operator==(const Index& l, const Index& r) {
            return l.idx == r.idx && l.key == r.key;
        }

        friend bool operator!=(const Index& l, const Index& r) {
            return !(l == r);
        }

    };

    struct GateDef {
        std::string name;
        std::vector<dd::fp> params;
    };


    struct TDD {

        Edge<mNode> e;
        std::vector<Index> index_set;
        std::vector<std::string> key_2_index;
        std::vector<GateDef> gates;
        float pred_size;
    };


    struct key_2_new_key_node {
        short level;
        float new_key;
        std::map<float, key_2_new_key_node*> next;
        key_2_new_key_node* father;
    };

    void to_json(json& j, const GateDef& g) {
        j = json{
            {"name", g.name},
            {"params", g.params}
        };
    }

}