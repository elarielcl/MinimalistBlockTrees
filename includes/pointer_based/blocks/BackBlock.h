#ifndef BLOCKTREE_PBACKBLOCK_H
#define BLOCKTREE_PBACKBLOCK_H

#include "Block.h"

class BackBlock : public Block {
public:

    BackBlock(Block*, int64_t, int64_t, std::string&, Block*, Block*, int);
    ~BackBlock();

    int access(int);
    int add_rank_select_support(int);

    int rank(int, int);
    int select(int, int);
    int info_access(int, int&);

    int number_of_nodes();
    int number_of_leaves();
    int number_of_back_nodes();
};

#endif //BLOCKTREE_PBACKBLOCK_H
