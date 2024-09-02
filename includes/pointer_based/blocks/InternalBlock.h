#ifndef BLOCKTREE_PLAZYINTERNALBLOCK_H
#define BLOCKTREE_PLAZYINTERNALBLOCK_H

#include "Block.h"

class InternalBlock : public Block {
public:

    InternalBlock(Block*, int64_t, int64_t, std::string&);
    ~InternalBlock();

    std::vector<Block*>& children(int, int);
    void clean_unnecessary_expansions();

    bool is_leaf();
    int access(int);
    int info_access(int, int&);
    int add_rank_select_support(int);

    int number_of_nodes();
    int number_of_leaves();
    int number_of_back_nodes();
    int longest_back_nodes_chain_length();

    int rank(int, int);
    int select(int, int);

};

#endif //BLOCKTREE_PLAZYINTERNALBLOCK_H
