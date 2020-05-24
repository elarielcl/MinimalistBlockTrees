#ifndef BLOCKTREE_PCBLOCKTREE_H
#define BLOCKTREE_PCBLOCKTREE_H

#include "pointer_based/BlockTree.h"

#include <sdsl/bit_vectors.hpp>
#include <sdsl/vectors.hpp>

#include <unordered_map>


class CBitBlockTree {
public:
    int r_; // Arity
    int first_level_length_;
    int number_of_levels_;

    std::vector<sdsl::bit_vector*> bt_bv_; // 1 when is Internal Block
    std::vector<sdsl::rank_support_v<1>*> bt_bv_rank_;
    std::vector<sdsl::int_vector<>*> bt_offsets_;
    sdsl::bit_vector* leaf_bv_;


    sdsl::int_vector<>* bt_first_level_prefix_ranks_;

    std::vector<sdsl::int_vector<>*> bt_ranks_;
    std::vector<sdsl::int_vector<>*> bt_second_ranks_;


    CBitBlockTree(BlockTree*, int);
    CBitBlockTree(std::istream&);
    virtual ~CBitBlockTree();

    int access(int);
    int rank_0(int);
    int rank_1(int);
    int select_0(int);
    int select_1(int);

    int size();
    int get_partial_size();
    void serialize(std::ostream&);
};


#endif //BLOCKTREE_PCBLOCKTREE_H
