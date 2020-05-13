#include "pointer_based/blocks/LeafBlock.h"

LeafBlock::LeafBlock(Block* parent, int64_t start_index, int64_t end_index, std::string& source):
        Block(parent, start_index, end_index, source) {
}

LeafBlock::~LeafBlock() {

}

int64_t LeafBlock::size() {
    int64_t source_end_index = source_.size() - 1;
    return (end_index_ <= source_end_index ? end_index_ : source_end_index)-start_index_+1;
}

int LeafBlock::add_rank_select_support(int c) {
    ranks_[c] = rank(c, size()-1);
    return ranks_[c];
}


int LeafBlock::rank(int c, int i) {
    int r = 0;
    for (int j = 0; j<=i; ++j) {
        if (source_[start_index_+j] == c) ++r;
    }
    return r;
}


int LeafBlock::select(int c, int j) {
    for (int i = 0; i < size(); ++i) {
        if (((int)(source_[start_index_+i])) == c) --j;
        if (!j) return i;
    }
    return -1;
}


int LeafBlock::access(int i) {
    return source_[start_index_+i];
}