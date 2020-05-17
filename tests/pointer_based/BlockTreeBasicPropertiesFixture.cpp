#include <pointer_based/blocks/LeafBlock.h>
#include <pointer_based/blocks/InternalBlock.h>
#include <unordered_set>
#include <fstream>
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


class BlockTreeBasicPropertiesFixture : public ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>> {
protected:
    virtual void TearDown() {
        delete block_tree_;
        delete block_tree_rs_;
    }

    virtual void SetUp() {
        CreateBlockTreeFunc* create_blocktree = ::testing::get<3>(GetParam());
        r_ = ::testing::get<0>(GetParam());
        leaf_length_ = ::testing::get<1>(GetParam());

        std::ifstream t(::testing::get<2>(GetParam()));
        std::stringstream buffer;
        buffer << t.rdbuf();
        input_= buffer.str();
        block_tree_ = (*create_blocktree)(r_ , leaf_length_, input_);


        block_tree_rs_ = (*create_blocktree)(r_ , leaf_length_, input_);

        std::unordered_set<int> characters;
        for (char c: input_)
            characters.insert(c);
        for (int c: characters) {
            characters_[c] = {};
            block_tree_rs_->add_rank_select_support(c);
        }

        for (int i = 0; i<input_.size(); ++i)
            characters_[input_[i]].push_back(i);
    }

public:
    BlockTree* block_tree_;
    BlockTree* block_tree_rs_;

    std::string input_;
    int r_;
    int leaf_length_;
    std::unordered_map<int,std::vector<int>> characters_; // Characters in the input and its select results

    BlockTreeBasicPropertiesFixture() : ::testing::TestWithParam<::testing::tuple<int, int, std::string, CreateBlockTreeFunc*>>() {
    }

    virtual ~BlockTreeBasicPropertiesFixture() {
    }
};

INSTANTIATE_TEST_CASE_P(AllVariantsTest,
                        BlockTreeBasicPropertiesFixture,
                        Combine(Values(2),
                                Values(4),
                                Values("../../../tests/data/as", "../../../tests/data/dna", "../../../tests/data/dna.par", "../../../tests/data/einstein"),
                                Values(&block_tree, &block_tree_without_cleanning, &heuristic_block_tree)));

// This test checks if the parameters given to the tree
// are the same inside the components of the BlockTree
TEST_P(BlockTreeBasicPropertiesFixture, parameters_check) {
    EXPECT_EQ(block_tree_->r_, r_);
    EXPECT_EQ(block_tree_->input_, input_);
    EXPECT_EQ(block_tree_->leaf_length_, leaf_length_);
    std::vector< std::vector<Block*> > levels = block_tree_->levelwise_iterator();

    for (std::vector<Block*> level : levels)
        for (Block* b : level) {
            EXPECT_EQ(b->source_, input_);
        }
}

// This test checks if the internal nodes of the PBlockTree
// have r_ children except the last of each level
TEST_P(BlockTreeBasicPropertiesFixture, almost_always_r_children_property_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size()-1; ++i) {
            Block* b =  level[i];
            if (dynamic_cast<InternalBlock*>(b))
                EXPECT_EQ(r_, b->children_.size());
        }
        if (dynamic_cast<InternalBlock*>(level.back()))
            EXPECT_GE(r_, level.back()->children_.size());
    }
}


// This test checks if the internal and back blocks' lengths
// are > ll and if the leaves' lengths are = ll
TEST_P(BlockTreeBasicPropertiesFixture, leaf_length_property_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b : level) {
            if (dynamic_cast<LeafBlock*>(b))
                EXPECT_EQ(b->length(), leaf_length_);
            else {
                EXPECT_GT(b->length(), leaf_length_);
            }
        }
    }
}

// This test checks if the BlockTree represents its input
// string and if the access method is correct
TEST_P(BlockTreeBasicPropertiesFixture, input_integrity_or_access_check) {
    for (int i = 0; i < input_.size(); ++i)
        EXPECT_EQ(block_tree_->access(i), input_[i]);
}

// This test checks the rank method for every character
// and position in the input
TEST_P(BlockTreeBasicPropertiesFixture, ranks_check) {
    for (auto pair : characters_) {
        int c = pair.first;
        int r = 0;
        for (int i = 0; i < input_.size(); ++i) {
            if (input_[i] == c) ++r;
            EXPECT_EQ(block_tree_rs_->rank(c, i), r);
        }
    }
}


// This test checks the select method for every character
// and rank
TEST_P(BlockTreeBasicPropertiesFixture, selects_check) {
    for (auto pair : characters_) {
        int c  = pair.first;
        for (int j = 1; j<=pair.second.size(); ++j)
            EXPECT_EQ(block_tree_rs_->select(c, j), pair.second[j-1]);
    }
}


// This test checks the property that all the leaves are in the
// last level of the PBlockTree
TEST_P(BlockTreeBasicPropertiesFixture, all_leaves_last_level_property_check) {
    auto iterator = block_tree_->levelwise_iterator();
    for (int i = 0; i < iterator.size()-1; ++i) {
        std::vector<Block*> level = iterator[i];
        for (Block* b : level)
            EXPECT_FALSE(dynamic_cast<LeafBlock*>(b));
    }
    for (Block* b : iterator.back())
        EXPECT_TRUE(dynamic_cast<LeafBlock*>(b));
}


// This test checks that in a level all the blocks has the same "length"
TEST_P(BlockTreeBasicPropertiesFixture, same_level_length_property_check) {
    std::vector< std::vector<Block*> > levels = block_tree_->levelwise_iterator();

    for (std::vector<Block*> level : levels) {
        int l = level.front()->length();
        for (Block* b : level)
            EXPECT_TRUE(l == b->length());
    }
}


// This test checks if the indices are assigned correctly
TEST_P(BlockTreeBasicPropertiesFixture, correct_children_indices_property_check) {
    int padded_length = 1;
    while (padded_length < input_.size()) padded_length *= r_;

    EXPECT_EQ(block_tree_->root_block_->start_index_, 0);
    EXPECT_EQ(block_tree_->root_block_->end_index_, padded_length-1);

    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size()-1; ++i) {
            Block* b = level[i];
            if (dynamic_cast<InternalBlock*>(b)) {
                EXPECT_EQ(b->start_index_, b->children_.front()->start_index_);
                EXPECT_EQ(b->end_index_, b->children_.back()->end_index_);

                int l = b->children_[0]->length();
                int last_end = b->start_index_-1;
                for (Block* child : b->children_) {
                    EXPECT_EQ(l, child->length());
                    EXPECT_EQ(last_end+1, child->start_index_);
                    last_end = child->end_index_;
                }

            }
        }
        Block* b = level.back();
        if (dynamic_cast<InternalBlock*>(b)) {
            EXPECT_EQ(b->start_index_, b->children_.front()->start_index_);
            EXPECT_GE(b->end_index_, b->children_.back()->end_index_);

            int l = b->children_[0]->length();
            int last_end = b->start_index_-1;
            for (Block* child : b->children_) {
                EXPECT_EQ(l, child->length());
                EXPECT_EQ(last_end+1, child->start_index_);
                last_end = child->end_index_;
            }

        }
    }
}


// This test checks if the field parent is correct
TEST_P(BlockTreeBasicPropertiesFixture, parent_and_child_number_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<InternalBlock*>(b)) {
                for (int i = 0; i < b->children_.size(); ++i) {
                    Block* child = b->children_[i];
                    EXPECT_EQ(child->parent_, b);
                }
            }
        }
    }
}


// This test checks if the pointed string (by a back pointer)
// is correct
TEST_P(BlockTreeBasicPropertiesFixture, back_pointer_right_representation_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                std::string data = b->first_block_->represented_string().substr(b->offset_, b->length());
                if (b->second_block_ != nullptr)
                    data += b->second_block_->represented_string().substr(0, b->length()-data.length());
                EXPECT_EQ(b->represented_string(), data);
            }
        }
    }
}



// This test checks if the first pointed block (by a back pointer)
// occurrs before on its level
TEST_P(BlockTreeBasicPropertiesFixture, back_pointer_points_back_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        int max_level_length = level.front()->length();
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                EXPECT_LT(b->first_block_->end_index_, b->start_index_);
                EXPECT_TRUE(b->first_block_->length() == max_level_length || b->first_block_->length() == max_level_length-1);
            }
        }
    }
}

// This test checks if the block pointed by a back block are consecutive
// Also, it checks that the fields level_index and first_occurrence_level_index
// are right
TEST_P(BlockTreeBasicPropertiesFixture, consecutive_blocks_back_pointer_and_indices_fields_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (int i = 0; i < level.size(); ++i) {
            Block* b = level[i];
            EXPECT_EQ(i, b->level_index_);
            EXPECT_LE(b->first_occurrence_level_index_, b->level_index_);
            if (dynamic_cast<BackBlock*>(b)) {
                EXPECT_EQ(b->first_block_, level[b->first_occurrence_level_index_]);
                if (b->second_block_ != nullptr) EXPECT_EQ(b->second_block_, level[b->first_occurrence_level_index_+1]);
            }

        }
    }
}

// This test checks if the pointed string (by a back pointer)
// fits on the pointed blocks
TEST_P(BlockTreeBasicPropertiesFixture, back_pointer_fit_check) {
    for (std::vector<Block*> level : block_tree_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                if (b->second_block_ == nullptr) {
                    EXPECT_TRUE(!b->offset_);
                } else {
                    EXPECT_LT(b->offset_+b->length()-1-b->first_block_->length(), b->second_block_->length());
                }
            }
        }
    }
}

// This test checks if the ranks field is correct
TEST_P(BlockTreeBasicPropertiesFixture, ranks_field_check) {
    for (std::vector<Block*> level : block_tree_rs_->levelwise_iterator()) {
        for (Block *b: level) {
            std::unordered_map<int, int> ranks;
            for (auto pair: b->ranks_)
                ranks[pair.first] = 0;
            for (int i = b->start_index_; i <= b->end_index_ && i < input_.size(); ++i)
                ranks[input_[i]] = ranks[input_[i]] + 1;
            for (auto pair : ranks)
                EXPECT_EQ(ranks[pair.first], b->ranks_[pair.first]);
        }
    }
}

/// This test checks if the second_rank field is correct
TEST_P(BlockTreeBasicPropertiesFixture, second_ranks_field_check) {
    for (std::vector<Block*> level : block_tree_rs_->levelwise_iterator()) {
        for (Block* b: level) {
            if (dynamic_cast<BackBlock*>(b)) {
                std::unordered_map<int,int> first_ranks;
                std::unordered_map<int,int> second_ranks;
                for (auto pair: b->second_ranks_)
                    second_ranks[pair.second] = 0;
                int i = b->first_block_->start_index_;
                for (; i < b->first_block_->start_index_ +  b->offset_; ++i)
                    first_ranks[input_[i]] = first_ranks[input_[i]] + 1;
                for (; i <= b->first_block_->end_index_ && i < b->first_block_->start_index_ + b->offset_ + b->length(); ++i)
                    second_ranks[input_[i]] = second_ranks[input_[i]] + 1;

                for (auto pair: second_ranks)
                    EXPECT_EQ(second_ranks[pair.first], b->second_ranks_[pair.first]);
            }
        }
    }
}
