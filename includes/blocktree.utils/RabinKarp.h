#ifndef BLOCKTREE_RABINKARP_H
#define BLOCKTREE_RABINKARP_H

#include <string>

class RabinKarp {
    uint64_t kp_;
    uint64_t init_;
    uint64_t rm_;

public:
    uint64_t sigma_;
    uint64_t hash_;
    uint64_t size_;
    std::string& s_;


    RabinKarp(std::string& s, int init, int size, int range, int sigma = 257);

    uint64_t  hash();
    void next();
};

#endif //BLOCKTREE_RABINKARP_H
