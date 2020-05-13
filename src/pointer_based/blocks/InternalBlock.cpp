#include "pointer_based/blocks/InternalBlock.h"
#include "pointer_based/blocks/LeafBlock.h"
#include "pointer_based/blocks/BackBlock.h"

InternalBlock::InternalBlock(Block* parent, int64_t start_index, int64_t end_index, std::string& source):
        Block(parent, start_index, end_index, source) {
}


InternalBlock::~InternalBlock() {
    for (int i = children_.size()-1 ; i>=0; i--)
        delete children_[i];
}


int InternalBlock::add_rank_select_support(int c) {
    int r = 0;
    for (Block* child: children_) {
        r += child->add_rank_select_support(c);
    }
    ranks_[c] = r;
    return ranks_[c];
}


int InternalBlock::rank(int c, int i) {
    int cumulative_length = 0;
    int r = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return r + child->rank(c, i-(cumulative_length-child->length()));
        r += child->ranks_[c];
    }
    return 0;
}


int InternalBlock::select(int c, int j) {
    int cumulative_length = 0;
    int r = 0;
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if ((it+1) == children_.end()) return (*it)->select(c, j-r) + cumulative_length;
        if (r + (*it)->ranks_[c] >= j) return (*it)->select(c, j-r) + cumulative_length;
        r += (*it)->ranks_[c];
        cumulative_length += (*it)->length();
    }
    return -1;
}


std::vector<Block*>& InternalBlock::children(int leaf_length, int r) {
    if (children_.size() == 0) {
        int next_length = length()/r;
        if (next_length <= leaf_length) {
            for (int i = 0; i < r; ++i) {
                int init = start_index_ + i * next_length;
                int end = start_index_ + (i + 1) * next_length - 1;
                if (init < source_.size()) {
                    Block *child = new LeafBlock(this, init, end, source_);
                    children_.push_back(child);
                }
            }
        } else {
            for (int i = 0; i < r; ++i) {
                int init = start_index_ + i * next_length;
                int end = start_index_ + (i + 1) * next_length - 1;
                if (init < source_.size()) {
                    Block *child = new InternalBlock(this, init, end, source_);
                    children_.push_back(child);
                }
            }
        }
    }
    return children_;
}

void InternalBlock::clean_unnecessary_expansions() {
    for (std::vector<Block *>::reverse_iterator rit = children_.rbegin(); rit != children_.rend(); ++rit) {
        Block *b = (*rit);
        b->clean_unnecessary_expansions();
    }

    bool all_children_leaves = true;
    for (Block* child : children_)
        all_children_leaves = all_children_leaves && child->is_leaf();

    if (all_children_leaves && pointing_to_me_ == 0 && first_block_->start_index_ < start_index_ && second_block_!=this) {
        BackBlock* bb = new BackBlock(parent_, start_index_, end_index_, source_,
                                      first_block_, second_block_, offset_);
        bb->level_index_ = level_index_;
        bb->first_occurrence_level_index_ = first_occurrence_level_index_;
        bb->left_ = true;
        bb->right_ = true;
        parent_->replace_child(this, bb);
        delete this;
    } else { //To avoid dangling references
        first_block_ = this;
        second_block_ = nullptr;
    }

}


bool InternalBlock::is_leaf() {
    return false;
}


int InternalBlock::access(int i) {
    int cumulative_length = 0;
    for (Block* child: children_) {
        cumulative_length += child->length();
        if (i < cumulative_length) return child->access(i-(cumulative_length-child->length()));
    }
    return -1;
}