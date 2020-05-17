#include <pointer_based/blocks/LeafBlock.h>
#include <unordered_set>
#include <compressed/CBlockTree.h>
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


BlockTree* heuristic_block_tree(int r, int max_leaf_length, std::string input) {
    BlockTree* block_tree_ = new BlockTree(input, r, max_leaf_length);
    block_tree_->process_back_pointers_heuristic();
    return block_tree_;
}


class CBlockTreeFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>> {
protected:
    virtual void TearDown() {
        delete block_tree_;
        delete block_tree_rs_;
        delete c_block_tree_;
        delete c_block_tree_rs_;
    }

    virtual void SetUp() {
        CreateBlockTreeFunc* create_blocktree = ::testing::get<3>(GetParam());
        r_ = ::testing::get<0>(GetParam());
        max_leaf_length_ = ::testing::get<1>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = (*create_blocktree)(r_ , max_leaf_length_, input_);
        c_block_tree_ = new CBlockTree(block_tree_);

        block_tree_rs_ = (*create_blocktree)(r_ , max_leaf_length_, input_);

        std::unordered_set<int> characters;
        for (char c: input_)
            characters.insert(c);
        for (int c: characters) {
            characters_[c] = {};
            block_tree_rs_->add_rank_select_support(c);
        }

        for (int i = 0; i<input_.size(); ++i)
            characters_[input_[i]].push_back(i);

        c_block_tree_rs_ = new CBlockTree(block_tree_rs_);
    }

public:
    BlockTree* block_tree_;
    BlockTree* block_tree_rs_;

    CBlockTree* c_block_tree_;
    CBlockTree* c_block_tree_rs_;

    std::string input_;
    int r_;
    int max_leaf_length_;
    std::unordered_map<int,std::vector<int>> characters_; // Characters in the input and its select results

    CBlockTreeFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>>() {
    }

    virtual ~CBlockTreeFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(PCBlockTreeTest,
                        CBlockTreeFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../../tests/data/as", "../../../tests/data/dna", "../../../tests/data/dna.par", "../../../tests/data/einstein"),
                                Values(&block_tree, &block_tree_without_cleanning, &heuristic_block_tree)));


// This test checks if the fields and
// number_of_levels_ are correct
TEST_P(CBlockTreeFixture, general_fields_check) {
    EXPECT_EQ(c_block_tree_->r_, r_);
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
    EXPECT_EQ(iterator.size()-i, c_block_tree_->number_of_levels_);

}

// This test checks if the CBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_bv field is checked
TEST_P(CBlockTreeFixture, bt_bv_field_check) {
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

    for (int j = 0; j < c_block_tree_->number_of_levels_-1; ++j) {
        level = iterator[i + j];
        auto level_bt_bv = *(c_block_tree_->bt_bv_[j]);
        EXPECT_EQ(level.size(), level_bt_bv.size());
        for (int k = 0; k < level.size(); ++k) {
            Block *b = level[k];
            if (dynamic_cast<BackBlock *>(b)) EXPECT_FALSE(level_bt_bv[k]);
            else
                EXPECT_TRUE(level_bt_bv[k]);
        }
    }
}


// This test checks if the CBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_offsets field is checked
TEST_P(CBlockTreeFixture, bt_offsets_field_check) {
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

    for (int j = 0; j < c_block_tree_->number_of_levels_-1; ++j) {
        level = iterator[i+j];
        auto level_bt_offsets = *(c_block_tree_->bt_offsets_[j]);

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



// This test checks if the CBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_leaf_string field is checked
TEST_P(CBlockTreeFixture, bt_leaf_string_field_check) {
    auto iterator = block_tree_->levelwise_iterator();
    std::string leaf_string = "";
    for (Block* b : iterator.back()) {
        leaf_string += b->represented_string();
    }

    std::string leaf_c_string = "";
    for (int i : (*c_block_tree_->leaf_string_)) {
        leaf_c_string += (char)(*c_block_tree_->alphabet_)[i];
    }

    EXPECT_EQ(leaf_c_string, leaf_string);
}




// This test checks if the CBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_second_ranks field are checked
TEST_P(CBlockTreeFixture, bt_second_ranks_field_check) {
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

    for (int j = 0; j < c_block_tree_->number_of_levels_-1; ++j) {
        level = iterator[i+j];
        for (auto pair : characters_) {
            int c = pair.first;
            auto level_bt_second_ranks = *(c_block_tree_rs_->bt_second_ranks_[c][j]);

            int l = 0;
            for (Block *b: level) {
                if (dynamic_cast<BackBlock *>(b)) {
                    EXPECT_EQ(level_bt_second_ranks[l], b->second_ranks_[c]) ;
                    ++l;
                }
            }
            EXPECT_EQ(l, level_bt_second_ranks.size());
        }
    }
}

// This test checks if the CBlockTree has the same
// structure that its correspondent BlockTree
// in particular the bt_ranks_ is checked
TEST_P(CBlockTreeFixture, bt_bv_ranks_prefix_cumulated_check) {
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


    for (auto pair : characters_) {
        int c = pair.first;
        level = iterator[i];
        auto level_bt_ranks = *(c_block_tree_rs_->bt_ranks_[c][0]);
        EXPECT_EQ(level.size(), level_bt_ranks.size());

        for (int k = 0; k < level.size(); ++k) {
            Block* b = level[k];
            EXPECT_EQ(b->ranks_[c], level_bt_ranks[k]);
        }
    }


    for (int j = 1; j < c_block_tree_->number_of_levels_; ++j) {
        for (auto pair : characters_) {
            int c = pair.first;
            level = iterator[i + j];
            auto level_bt_ranks = *(c_block_tree_rs_->bt_ranks_[c][j]);
            EXPECT_EQ(level.size(), level_bt_ranks.size());

            for (int k = 0; k < level.size(); ++k) {
                Block* b = level[k];
                EXPECT_EQ(b->ranks_[c], level_bt_ranks[k]);
            }
        }
    }
}


// This test checks if the CBlockTree has the same
// structure that its correspondent BlockTree
// in particular the first level for bt_prefix_ranks_,
// is checked
TEST_P(CBlockTreeFixture, bt_bv_first_level_prefix_cumulated_ranks_check) {
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


    for (auto pair : characters_) {
        int c = pair.first;
        level = iterator[i];
        auto first_level_bt_prefix_ranks = *(c_block_tree_rs_->bt_first_level_prefix_ranks_[c]);
        int r  = 0;

        EXPECT_EQ(first_level_bt_prefix_ranks.size(), level.size());
        for (int k = 0; k < level.size(); ++k) {
            EXPECT_EQ(r, first_level_bt_prefix_ranks[k]);
            r += level[k]->ranks_[c];
        }
    }

}


// This test checks if the mapping and alphabet fields are correct
TEST_P(CBlockTreeFixture, bt_mapping_alphabet_field_check) {
    EXPECT_EQ(c_block_tree_rs_->mapping_.size(), characters_.size());
    EXPECT_EQ(c_block_tree_rs_->alphabet_->size(), characters_.size());
    for (int i = 0; i < c_block_tree_->alphabet_->size(); ++i) {
        EXPECT_EQ(c_block_tree_rs_->mapping_[(*c_block_tree_->alphabet_)[i]], i);
    }
}


// This test checks if the CBlockTree represents its input
// string and if the access method is correct
TEST_P(CBlockTreeFixture, input_integrity_or_access_check) {
    for (int i = 0; i < input_.size(); ++i)
        EXPECT_EQ(c_block_tree_->access(i), input_[i]);
}

// This test checks the rank method for every character
// and position in the input
TEST_P(CBlockTreeFixture, ranks_check) {
    for (auto pair : characters_) {
        int c = pair.first;
        int r = 0;
        for (int i = 0; i < input_.size(); ++i) {
            if (input_[i] == c) ++r;
            EXPECT_EQ(c_block_tree_rs_->rank(c, i), r);
        }
    }
}


// This test checks the select method for every character
// and rank
TEST_P(CBlockTreeFixture, selects_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        for (int j = 1; j<=pair.second.size(); ++j)
            EXPECT_EQ(c_block_tree_rs_->select(c, j), pair.second[j-1]);
    }
}