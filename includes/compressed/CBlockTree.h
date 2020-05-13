#ifndef BLOCKTREE_PCBLOCKTREE_H
#define BLOCKTREE_PCBLOCKTREE_H

#include "pointer_based/BlockTree.h"

#include <sdsl/bit_vectors.hpp>
#include <sdsl/vectors.hpp>

#include <unordered_map>


class CBlockTree {
public:
    int r_; // Arity
    int first_level_length_;
    int number_of_levels_;

    std::vector<sdsl::bit_vector*> bt_bv_; // 1 when is Internal Block
    std::vector<sdsl::rank_support_v<1>*> bt_bv_rank_;
    std::vector<sdsl::int_vector<>*> bt_offsets_;
    sdsl::int_vector<>* leaf_string_;


    sdsl::int_vector<>* alphabet_;
    std::unordered_map<char,int> mapping_;


    std::unordered_map<int,sdsl::int_vector<>*> bt_first_level_prefix_ranks_;

    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_ranks_;
    std::unordered_map<int,std::vector<sdsl::int_vector<>*>> bt_second_ranks_;


    CBlockTree(BlockTree*);
    virtual ~CBlockTree();

    int access(int);
    int rank(int, int);
    int select(int, int);

    int size();
};


#endif //BLOCKTREE_PCBLOCKTREE_H
