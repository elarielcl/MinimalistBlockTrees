#ifndef BLOCKTREE_PLEAVEBLOCK_H
#define BLOCKTREE_PLEAVEBLOCK_H

#include "Block.h"

class LeafBlock : public Block {
public:

    LeafBlock(Block*, int64_t, int64_t, std::string&);
    ~LeafBlock();

    int add_rank_select_support(int);
    int64_t size();

    int rank(int, int);
    int select(int, int);

    int access(int);
};

#endif //BLOCKTREE_PLEAVEBLOCK_H
