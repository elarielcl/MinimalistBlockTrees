#include "pointer_based/blocks/BackBlock.h"


BackBlock::BackBlock(Block* parent, int64_t start_index, int64_t end_index, std::string& source, Block* first_block,
                     Block* second_block, int offset) :
        Block(parent, start_index, end_index, source) {
    first_block_ = first_block;
    if (second_block != nullptr)
        if (second_block->start_index_ == start_index && second_block->end_index_ == end_index) second_block_ = this;
        else second_block_ = second_block;
    offset_ = offset;
    if (first_block_ != nullptr) first_block_->pointing_to_me_ = first_block_->pointing_to_me_ + 1;
    if (second_block_ != nullptr) second_block_->pointing_to_me_ = second_block_->pointing_to_me_ + 1;
}

BackBlock::~BackBlock() {
    if (first_block_ != nullptr) {
        first_block_->pointing_to_me_ = first_block_->pointing_to_me_ - 1;
    }
    if (second_block_ != nullptr) {
        second_block_->pointing_to_me_ = second_block_->pointing_to_me_ - 1;
    }
}

int BackBlock::add_rank_select_support(int c) {
    int first_rank = first_block_->rank(c, offset_-1);
    int second_rank = (second_block_ == nullptr) ? first_block_->rank(c, offset_ + length() - 1) - first_rank : first_block_->rank(c, first_block_->length() - 1) - first_rank;
    second_ranks_[c] = second_rank;
    ranks_[c] = (second_block_ == nullptr) ? second_rank : second_rank + second_block_->rank(c, offset_+length()-1-first_block_->length());
    return ranks_[c];
}


int BackBlock::rank(int c, int i) {
    if (i + offset_ >= first_block_->length()) return second_ranks_[c] + second_block_->rank(c, offset_+i-first_block_->length()); //Loop if it's itself
    return first_block_->rank(c, i+offset_) - (first_block_->ranks_[c] - second_ranks_[c]);
}


int BackBlock::select(int c, int j) {
    if (j > second_ranks_[c]) return second_block_->select(c, j-second_ranks_[c]) + first_block_->length() - offset_;
    return first_block_->select(c, j+first_block_->ranks_[c] - second_ranks_[c]) - offset_;
}


int BackBlock::access(int i) {
    if (i + offset_ >= first_block_->length()) return second_block_->access(offset_+i-first_block_->length());
    return first_block_->access(i+offset_);
}


int BackBlock::info_access(int i, int& number_of_leaves) {
    number_of_leaves += 1;
    if (i + offset_ >= first_block_->length()) return second_block_->info_access(offset_+i-first_block_->length(), number_of_leaves);
    return first_block_->info_access(i+offset_, number_of_leaves);
}


int BackBlock::number_of_nodes() {
    return 1;
}


int BackBlock::number_of_leaves() {
    return 1;
}


int BackBlock::number_of_back_nodes() {
    return 1;
}


int BackBlock::longest_back_nodes_chain_length() {
    int longest_previous_block_chain_length = -1;
    if (first_block_ != nullptr) {
        int longest_first_block_chain_length = first_block_->longest_back_nodes_chain_length();
        if (longest_first_block_chain_length > longest_previous_block_chain_length)
            longest_previous_block_chain_length = longest_first_block_chain_length;
    }
    if (second_block_ != nullptr) {
        int longest_second_block_chain_length = second_block_->longest_back_nodes_chain_length();
        if (longest_second_block_chain_length > longest_previous_block_chain_length)
            longest_previous_block_chain_length = longest_second_block_chain_length;
    }

    if (longest_previous_block_chain_length == -1) return -1;

    return 1 + longest_previous_block_chain_length;
}