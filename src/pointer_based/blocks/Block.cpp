#include "pointer_based/blocks/Block.h"


Block::Block(Block* parent, int64_t start_index, int64_t end_index, std::string& source):
        parent_(parent), start_index_(start_index), end_index_(end_index), source_(source), left_(false), right_(false), first_block_(
        this), second_block_(nullptr), pointing_to_me_(0), level_index_(0), first_occurrence_level_index_(0) {

}


Block::~Block() {

}


int Block::add_rank_select_support(int c) {
    return 0;
}


int Block::rank(int c, int i) {
    return 0;
}


int Block::select(int c, int j) {
    return -1;
}


int64_t Block::length() {
    return end_index_-start_index_+1;
}


std::string Block::represented_string() {
    return source_.substr(start_index_, length());
}


std::vector<Block*>& Block::children(int leaf_length, int r) {
    return children_;
}


void Block::clean_unnecessary_expansions() {

}


bool Block::is_leaf() {
    return true;
}


int Block::access(int i) {
    return -1;
}

void Block::replace_child(Block* old_child, Block* new_child) {
    for (int i = 0; i < children_.size(); ++i) {
        if (children_[i] == old_child) {
            children_[i] = new_child;
            return;
        }
    }
}