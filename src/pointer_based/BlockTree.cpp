#include "pointer_based/BlockTree.h"

#include <pointer_based/blocks/InternalBlock.h>
#include <pointer_based/blocks/LeafBlock.h>

#include <stack>


BlockTree::BlockTree(std::string& input, int r, int leaf_length): r_(r), input_(input), leaf_length_(leaf_length) {

    if (input_.size() <= leaf_length_ || input_.size()<r)
        root_block_ = new LeafBlock(nullptr, 0, input_.size() - 1, input_);
    else {
        int number_of_leaves = (input_.size()%leaf_length_ == 0) ? input_.size()/leaf_length_ : input_.size()/leaf_length_+1;
        int height =  0;

        int nl = number_of_leaves-1;
        int64_t block_length = leaf_length_;
        while (nl){
            height++;
            block_length*=r_;
            nl/=r_;
        }

        root_block_ = new InternalBlock(nullptr, 0, block_length - 1, input_);
    }

}


BlockTree::~BlockTree() {
    delete root_block_;
}


void BlockTree::add_rank_select_support(int c) {
    root_block_->add_rank_select_support(c);
}


int BlockTree::rank(int c, int i) {
    return root_block_->rank(c, i);
}


int BlockTree::select(int c, int j) {
    return root_block_->select(c, j);
}


std::vector<std::vector<Block*>> BlockTree::levelwise_iterator() {
    std::vector<std::vector<Block*>> result = {{root_block_}};
    while (!dynamic_cast<LeafBlock*>(result.back()[0])) {
        std::vector<Block*> next_level = {};
        for (Block *b : result.back())
            for (Block *child : b->children(leaf_length_, r_))
                next_level.push_back(child);
        result.push_back(next_level);
    }

    return result;
}


void BlockTree::clean_unnecessary_expansions() {
    root_block_->clean_unnecessary_expansions();
    for (std::vector<Block*> level : levelwise_iterator()) {
        for (int i = 0; i<level.size(); ++i) {
            level[i]->level_index_ = i;
            level[i]->first_occurrence_level_index_ = level[i]->first_block_->level_index_;
        }
    }
}


int BlockTree::access(int i) {
    return root_block_->access(i);
}


std::vector<Block*> BlockTree::next_level(std::vector<Block*>& level) {
    std::vector<Block*> next_level;
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        for (Block *child : b->children(leaf_length_, r_)) { // Do it in order
            child->level_index_ = next_level.size();
            child->first_occurrence_level_index_ = next_level.size();
            next_level.push_back(child);
        }
    }
    return next_level;
}


void BlockTree::forward_pair_window_block_scan(std::vector<Block*>& level, int pair_window_size, int N, std::unordered_map<HashString, std::vector<std::pair<Block*, Block*>>>& pair_hashtable) {
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        Block *b = (*it);
        b->right_ = true;
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, pair_window_size, N); // offset is always 0 here
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++) {
            Block* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < pair_window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + pair_window_size - 1);
                std::unordered_map<HashString, std::vector<std::pair<Block*, Block*>>>::const_iterator result = pair_hashtable.find(hS);
                if (result != pair_hashtable.end()) { // Here, It could be that the scanning should have finished with the penultimate, but it never should enter this ''if''
                                                        // when We're on the penultimate block and the window exceeds the last block because if that is a first occurrence should have been occured before in a pair of blocks
                                                        // maybe use a condition more like rk's condition below could work fine too
                                                        // Same logic: for when passing a window of size 2l + 2 over 2 block of length l
                    for (std::pair<Block*,Block*> p: result->second) {
                        if (current->start_index_ + offset < p.first->start_index_) {
                            p.first->left_ = true;
                            p.second->right_ = true;
                        }
                    }
                    pair_hashtable.erase(hS);
                }
                if (current->start_index_+offset+pair_window_size < input_.size()) rk.next();
            }
        }
        (*(it-1))->left_ = true;
    }
}


void BlockTree::forward_window_block_scan(std::vector<Block*>& level, int window_size, int N, std::unordered_map<HashString, std::vector<Block*>>& hashtable) {
    int i = 0;
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        Block *b = (*it);
        int offset = 0;
        RabinKarp rk(input_, (*it)->start_index_ + offset, window_size, N);
        for (; it != level.end() && ((*it) == b || (*(it-1))->end_index_ == (*it)->start_index_ - 1); it++, i++) {
            Block* current = *(it);
            bool last_block = ((it+1) == level.end() ||  current->end_index_ != (*(it+1))->start_index_ - 1);
            for (offset = 0; offset < current->length(); ++offset) {
                if (last_block && current->length() - offset < window_size)  break;
                HashString hS(rk.hash(), input_, current->start_index_ + offset, current->start_index_ + offset + window_size - 1);
                std::unordered_map<HashString, std::vector<Block *>>::const_iterator result = hashtable.find(hS);
                if (result != hashtable.end()) {
                    std::vector<Block*> blocks = result->second;
                    for (Block* b : blocks) {
                        b->first_occurrence_level_index_ = i;
                        b->first_block_ = current;
                        b->offset_ = offset;
                        if (offset + window_size > b->first_block_->length()) b->second_block_ = (*(it+1));
                        else b->second_block_ = nullptr;
                    }
                    hashtable.erase(hS);
                }
                if (current->start_index_+offset+window_size < input_.size()) rk.next();
            }
        }
    }
}


void BlockTree::block_scan(std::vector<Block *>& level, int N , std::unordered_map<HashString, std::vector<Block*>>& hashtable) {
    for (Block* b : level) {
        RabinKarp rk(input_, b->start_index_, b->length(), N);
        HashString hS(rk.hash(),  input_, b->start_index_, b->end_index_);

        std::unordered_map<HashString, std::vector<Block*>>::const_iterator result = hashtable.find(hS);

        if (result == hashtable.end())
            hashtable[hS] = {b};
        else
            hashtable[hS].push_back(b);
    }
}


void BlockTree::process_level(std::vector<Block*>& level) {

    int N = 6700417; //Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<Block*>> hashtable;
    block_scan(level, N, hashtable);

    // Pairs of blocks scan
    std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>> pair_hashtable;
    for (std::vector<Block *>::iterator it = level.begin(); it != level.end();) {
        for (++it; (it != level.end() && (*(it-1))->end_index_ == (*it)->start_index_ - 1); ++it) {
            Block* current = (*(it - 1));
            Block* next = (*it);
            RabinKarp rk(input_, current->start_index_, current->length() + next->length(), N);
            HashString hS(rk.hash(), input_, current->start_index_, current->start_index_ + current->length() + next->length()-1); // Second parameter is next->end_index

            std::unordered_map<HashString, std::vector<std::pair<Block *,Block*>>>::const_iterator result = pair_hashtable.find(hS);

            if (result == pair_hashtable.end())
                pair_hashtable[hS] = {{current, next}};
            else
                pair_hashtable[hS].push_back({current, next});
        }
    }


    // Window block scan
    //Establishes first occurrences of blocks
    forward_window_block_scan(level, level_length, N, hashtable);



    // Window Pair of blocks scans
    if (level.size() > 1)
        forward_pair_window_block_scan(level, level_length*2, N, pair_hashtable);




    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->left_ && b->right_ && b->first_occurrence_level_index_ < b->level_index_) {
            // This doesn't have the bug of the dangling reference fixed with first_occurrence_level_index, because it shouldn't happen that
            // A block points back to a BackBlock
            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, input_,
                                          level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            bb->left_ = true;
            bb->right_ = true;
            b->parent_->replace_child(b, bb);
            delete b;
            level[i] = bb;
        }
    }

}


void BlockTree::process_back_pointers() {
    std::vector<Block*> current_level = {root_block_};
    std::stack<Block*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||  current_level[0]->length() <= leaf_length_) break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}


void BlockTree::process_level_heuristic(std::vector<Block*>& level) {

    int N = 6700417; // Large prime
    int level_length = level.front()->length();

    // Block scan
    std::unordered_map<HashString, std::vector<Block*>> hashtable;
    block_scan(level, N, hashtable);

    // Window block scan
    // This is almost the same as forward_window_block_scan, as well as the BackBlock creation
    forward_window_block_scan(level, level_length, N, hashtable);



    // BackBlock creation
    for (int i = 0; i < level.size(); ++i) {
        Block* b = level[i];
        if (b->first_occurrence_level_index_ < b->level_index_) {

            BackBlock* bb = new BackBlock(b->parent_, b->start_index_, b->end_index_, input_,
                                          level[b->first_occurrence_level_index_], (b->second_block_ ==
                                                                                                      nullptr) ? nullptr : level[b->first_occurrence_level_index_ +1], b->offset_);
            bb->level_index_ = b->level_index_;
            bb->first_occurrence_level_index_ = b->first_occurrence_level_index_;
            b->parent_->replace_child(b, bb);
            delete b;
            level[i] = bb;
        }
    }

}


void BlockTree::process_back_pointers_heuristic() {
    std::vector<Block *> current_level = {root_block_};
    std::stack<Block*> none_blocks;
    while ((current_level = next_level(current_level)).size() != 0) {
        if (current_level[0]->length() < r_ ||
            current_level[0]->length() <= leaf_length_)
            break;
        while (current_level.size() != 0 && current_level.back()->end_index_ >= input_.size()) {
            none_blocks.push(current_level.back());
            current_level.pop_back();
        }
        process_level_heuristic(current_level);
        while (!none_blocks.empty()) {
            current_level.push_back(none_blocks.top());
            none_blocks.pop();
        }
    }
}