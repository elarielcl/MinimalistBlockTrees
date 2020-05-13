#ifndef BLOCKTREE_HASHSTRING_H
#define BLOCKTREE_HASHSTRING_H

#include <string>

class HashString {
public:
    size_t hash_;
    std::string& s_;
    int init_;
    int end_;

    HashString(size_t, std::string&, int, int);
    ~HashString();

    bool operator==(const HashString&) const;
};

namespace std {
    template <> struct hash<HashString> {
        std::size_t operator()(const HashString& hS) const {
            return hS.hash_;
        }
    };
}
#endif //BLOCKTREE_HASHSTRING_H
