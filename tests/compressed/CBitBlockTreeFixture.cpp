#include <pointer_based/blocks/LeafBlock.h>
#include <unordered_set>
#include <compressed/CBitBlockTree.h>
#include "gtest/gtest.h"

#include "pointer_based/BlockTree.h"

using ::testing::Combine;
using ::testing::Values;

typedef BlockTree* CreateBlockTreeFunc(int, int, std::string);

BlockTree* block_tree(int r, int max_leaf_length, std::string input) {

    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    block_tree_->clean_unnecessary_expansions();
    return block_tree_;
}

BlockTree* block_tree_without_cleanning(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers();
    return block_tree_;
}


BlockTree* heuristic_bit_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_heuristic();
    return block_tree_;
}


class CBitBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>> {
protected:
    virtual void TearDown() {
        delete block_tree_;
        delete block_tree_rs_;
        delete c_bit_block_tree_;
        delete c_bit_block_tree_rs_;
    }

    virtual void SetUp() {
        CreateBlockTreeFunc* create_blocktree = ::testing::get<3>(GetParam());
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        one_symbol = input_[0];
        block_tree_ = (*create_blocktree)(r_ , max_leaf_length_, input_);
        c_bit_block_tree_ = new CBitBlockTree(block_tree_, one_symbol);

        block_tree_rs_ = (*create_blocktree)(r_ , max_leaf_length_, input_);

        std::unordered_set<int> characters;
        for (char c: input_)
            characters.insert(c);
        for (int c: characters) {
            block_tree_rs_->add_rank_select_support(c);
        }

        for (int i = 0; i<input_.size(); ++i) {
            if (input_[i] == one_symbol) {
                selects_1.push_back(i);
            } else {
                selects_0.push_back(i);
            }
        }

        c_bit_block_tree_rs_ = new CBitBlockTree(block_tree_rs_, input_[0]);
    }

public:
    BlockTree* block_tree_;
    BlockTree* block_tree_rs_;

    CBitBlockTree* c_bit_block_tree_;
    CBitBlockTree* c_bit_block_tree_rs_;

    std::string input_;
    int one_symbol;
    int r_;
    int max_leaf_length_;
    std::vector<int> selects_1;
    std::vector<int> selects_0;

    CBitBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>>() {
    }

    virtual ~CBitBlockTreeFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(PCBitBlockTreeTest,
                        CBitBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../../tests/data/dna.par"),
                                Values(&block_tree, &block_tree_without_cleanning, &heuristic_bit_block_tree)));


// This test checks if the fields and
// number_of_levels_ are correct
TEST_P(CBitBlockTreeFixture, general_fields_check) {
    EXPECT_EQ(c_bit_block_tree_->r_, r_);
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i;
    for (i = 0; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }
    EXPECT_EQ(iterator.size()-i, c_bit_block_tree_->number_of_levels_);

}

// This test checks if the CBitBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_bv field is checked
TEST_P(CBitBlockTreeFixture, bt_bv_field_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i;
    for (i = 0; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }

    for (int j = 0; j < c_bit_block_tree_->number_of_levels_-1; ++j) {
        level = iterator[i + j];
        auto level_bt_bv = *(c_bit_block_tree_->bt_bv_[j]);
        EXPECT_EQ(level.size(), level_bt_bv.size());
        for (int k = 0; k < level.size(); ++k) {
            Block *b = level[k];
            if (dynamic_cast<BackBlock *>(b)) EXPECT_FALSE(level_bt_bv[k]);
            else
                EXPECT_TRUE(level_bt_bv[k]);
        }
    }
}


// This test checks if the CBitBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_offsets field is checked
TEST_P(CBitBlockTreeFixture, bt_offsets_field_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }

    for (int j = 0; j < c_bit_block_tree_->number_of_levels_-1; ++j) {
        level = iterator[i+j];
        auto level_bt_offsets = *(c_bit_block_tree_->bt_offsets_[j]);

        int max_size_level = level.front()->length();
        int l = 0;
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                EXPECT_EQ(level_bt_offsets[l], max_size_level*b->first_block_->level_index_ + b->offset_);
                ++l;
            }
        }
        EXPECT_EQ(l, level_bt_offsets.size());
    }
}



// This test checks if the CBitBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_leaf_string field is checked
TEST_P(CBitBlockTreeFixture, bt_leaf_string_field_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::string leaf_bv = "";
    for (Block* b : iterator.back()) {
        for (char c : b->represented_string()) {
            if (c == one_symbol) {
                leaf_bv += "1";
            } else {
                leaf_bv += "0";
            }
        }
    }

    std::string leaf_c_bv = "";
    for (int i : (*c_bit_block_tree_->leaf_bv_)) {
        leaf_c_bv += i ? "1" : "0";
    }

    EXPECT_EQ(leaf_c_bv, leaf_bv);
}




// This test checks if the CBitBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_second_ranks field are checked
TEST_P(CBitBlockTreeFixture, bt_second_ranks_field_check) {
    auto iterator = block_tree_rs_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }

    for (int j = 0; j < c_bit_block_tree_rs_->number_of_levels_-1; ++j) {
        level = iterator[i+j];

        auto level_bt_second_ranks = *(c_bit_block_tree_rs_->bt_second_ranks_[j]);

        int l = 0;
        for (Block *b: level) {
            if (dynamic_cast<BackBlock *>(b)) {
                EXPECT_EQ(level_bt_second_ranks[l], b->second_ranks_[one_symbol]) ;
                ++l;
            }
        }
        EXPECT_EQ(l, level_bt_second_ranks.size());
    }
}


// This test checks if the CBitBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_ranks_ is checked
TEST_P(CBitBlockTreeFixture, bt_bv_ranks_prefix_check) {
    auto iterator = block_tree_rs_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }



    level = iterator[i];
    auto level_bt_ranks = *(c_bit_block_tree_rs_->bt_ranks_[0]);
    EXPECT_EQ(level.size(), level_bt_ranks.size());

    for (int k = 0; k < level.size(); ++k) {
        Block* b = level[k];
        EXPECT_EQ(b->ranks_[one_symbol], level_bt_ranks[k]);
    }


    for (int j = 1; j < c_bit_block_tree_->number_of_levels_; ++j) {
        level = iterator[i + j];
        auto level_bt_ranks = *(c_bit_block_tree_rs_->bt_ranks_[j]);
        EXPECT_EQ(level.size(), level_bt_ranks.size());

        for (int k = 0; k < level.size(); ++k) {
            Block* b = level[k];
            EXPECT_EQ(b->ranks_[one_symbol], level_bt_ranks[k]);
        }
    }
}


// This test checks if the CBitBlockTree has the same
// structure that its correspondent BlockTree
// in particular the first level for bt_prefix_ranks_,
// is checked
TEST_P(CBitBlockTreeFixture, bt_bv_first_level_prefix_ranks_check) {
    auto iterator = block_tree_rs_->levelwise_iterator();
    std::vector<Block*> level;
    bool contains_back_block = false;
    int i = 0;
    for (; i < iterator.size(); ++i) {
        level = iterator[i];
        for (Block *b : level) {
            if (dynamic_cast<BackBlock*>(b)) contains_back_block = true;
        }
        if (contains_back_block) break;
    }

    level = iterator[i];
    auto first_level_bt_prefix_ranks = *(c_bit_block_tree_rs_->bt_first_level_prefix_ranks_);
    int r  = 0;

    EXPECT_EQ(first_level_bt_prefix_ranks.size(), level.size());
    for (int k = 0; k < level.size(); ++k) {
        EXPECT_EQ(r, first_level_bt_prefix_ranks[k]);
        r += level[k]->ranks_[one_symbol];
    }

}




// This test checks if the CBitBlockTree represents its input
// string and if the access method is correct
TEST_P(CBitBlockTreeFixture, input_integrity_or_access_check) {
    for (int i = 0; i < input_.size(); ++i) {
        if (input_[i] == one_symbol) {
            EXPECT_EQ(c_bit_block_tree_->access(i), 1);
        } else {
            EXPECT_EQ(c_bit_block_tree_->access(i), 0);
        }
    }
}


// This test checks the rank_1 method for every character
// and position in the input
TEST_P(CBitBlockTreeFixture, ranks_1_check) {
    int r = 0;
    for (int i = 0; i < input_.size(); ++i) {
        if (input_[i] == one_symbol) ++r;
        EXPECT_EQ(c_bit_block_tree_rs_->rank_1(i), r);
    }
}


// This test checks the rank_0 method for every character
// and position in the input
TEST_P(CBitBlockTreeFixture, ranks_0_check) {
    int r = 0;
    for (int i = 0; i < input_.size(); ++i) {
        if (input_[i] != one_symbol) ++r;
        EXPECT_EQ(c_bit_block_tree_rs_->rank_0(i), r);
    }
}


// This test checks the select_1 method for every character
// and rank
TEST_P(CBitBlockTreeFixture, selects_1_check) {
    for (int j = 1; j<=selects_1.size(); ++j) {
        EXPECT_EQ(c_bit_block_tree_rs_->select_1(j), selects_1[j-1]);
    }
}


// This test checks the select_0 method for every character
// and rank
TEST_P(CBitBlockTreeFixture, selects_0_check) {
    for (int j = 1; j<=selects_0.size(); ++j) {
        EXPECT_EQ(c_bit_block_tree_rs_->select_0(j), selects_0[j-1]);
    }
}