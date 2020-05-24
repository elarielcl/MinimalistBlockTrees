#include <compressed/CBitBlockTree.h>
#include <unordered_set>

CBitBlockTree::CBitBlockTree(BlockTree * bt, int one_symbol) : r_(bt->r_) {
    std::vector<Block*> first_level = {bt->root_block_};
    bool is_first_level = false;
    while (!is_first_level) {
        for (Block* b: first_level) {
            is_first_level = is_first_level || b->is_leaf();
        }
        if (is_first_level) break;
        first_level = bt->next_level(first_level);
    }

    int first_level_prefix_ranks_ = 0;
    sdsl::int_vector<>* first_level_prefix_ranks = new sdsl::int_vector<>(first_level.size());;
    sdsl::int_vector<>* first_level_ranks = new sdsl::int_vector<>(first_level.size());;


    for (int i = 0; i < first_level.size(); ++i) {
        (*first_level_prefix_ranks)[i] = first_level_prefix_ranks_;

        for (auto pair: first_level[i]->ranks_) {
            if (pair.first == one_symbol) {
                (*first_level_ranks)[i] = first_level[i]->ranks_[pair.first];
                first_level_prefix_ranks_ = first_level_prefix_ranks_ + first_level[i]->ranks_[pair.first];
            }
        }
    }

    sdsl::util::bit_compress(*(first_level_prefix_ranks));
    bt_first_level_prefix_ranks_ = first_level_prefix_ranks;

    sdsl::util::bit_compress(*(first_level_ranks));
    bt_ranks_.push_back(first_level_ranks);


    first_level_length_ = first_level[0]->length();
    number_of_levels_ = 0;

    std::vector<Block*> current_level = first_level;
    std::vector<Block*> next_level = bt->next_level(first_level);

    while (next_level.size() != 0) {
        sdsl::bit_vector* current_level_bv = new sdsl::bit_vector(current_level.size() ,0);

        sdsl::int_vector<>* next_level_ranks = new sdsl::int_vector<>(next_level.size());

        int number_of_leaves = 0;
        int current_length = current_level.front()->length();
        for (int i = 0; i < current_level.size(); ++i) {
            current_level[i]->level_index_ = i;

            if (current_level[i]->is_leaf()) {
                (*current_level_bv)[i] = 0;
                ++number_of_leaves;
            }
            else {
                (*current_level_bv)[i] = 1;
            }
        }

        for (int i = 0; i < next_level.size(); ++i) {
            for (auto pair: next_level[i]->ranks_) {
                if (pair.first == one_symbol) {
                    (*next_level_ranks)[i] = pair.second;
                }
            }
        }


        sdsl::int_vector<>* current_level_offsets = new sdsl::int_vector<>(number_of_leaves);

        sdsl::int_vector<>* current_level_second_ranks = new sdsl::int_vector<>(number_of_leaves);

        int j = 0;
        for (int i = 0; i < current_level.size(); ++i) {
            if (!(*current_level_bv)[i]) {
                for (auto pair: current_level[i]->second_ranks_) {
                    if (pair.first == one_symbol) {
                        (*current_level_second_ranks)[j] = pair.second;
                    }
                }

                (*current_level_offsets)[j++] = current_level[i]->first_block_->level_index_ * current_length + current_level[i]->offset_;
            }
        }

        sdsl::util::bit_compress(*current_level_offsets);
        bt_offsets_.push_back(current_level_offsets);

        sdsl::util::bit_compress(*(next_level_ranks));
        bt_ranks_.push_back(next_level_ranks);

        sdsl::util::bit_compress(*(current_level_second_ranks));
        bt_second_ranks_.push_back(current_level_second_ranks);


        bt_bv_.push_back(current_level_bv);
        sdsl::rank_support_v<1>* current_level_bv_rank = new sdsl::rank_support_v<1>(current_level_bv);
        bt_bv_rank_.push_back(current_level_bv_rank);

        current_level = next_level;
        next_level = bt->next_level(current_level);
        ++number_of_levels_;
    }

    ++number_of_levels_;

    std::vector<Block*> last_level = current_level;

    std::string leaf_string = "";
    for (Block* b: last_level) {
        leaf_string += b->represented_string();
    }


    leaf_bv_ = new sdsl::bit_vector (leaf_string.size());

    for (int i = 0; i<(*leaf_bv_).size(); ++i) {
        if (leaf_string[i] == one_symbol) {
            (*leaf_bv_)[i] = 1;
        } else {
            (*leaf_bv_)[i] = 0;
        }
    }
}

CBitBlockTree::CBitBlockTree(std::istream& in) {
    in.read((char *) &r_, sizeof(int));
    in.read((char *) &first_level_length_, sizeof(int));
    in.read((char *) &number_of_levels_, sizeof(int));

    for (int i = 0; i < number_of_levels_-1; ++i) {
        bt_bv_.push_back(new sdsl::bit_vector());
        (*bt_bv_[i]).load(in);
    }

    for (sdsl::bit_vector* bv : bt_bv_) {
        bt_bv_rank_.push_back(new sdsl::rank_support_v<1>(bv));
    }

    for (int i = 0; i < number_of_levels_-1; ++i) {
        bt_offsets_.push_back(new sdsl::int_vector<>());
        (*bt_offsets_[i]).load(in);
    }

    leaf_bv_ = new sdsl::bit_vector ();
    (*leaf_bv_).load(in);

    bt_first_level_prefix_ranks_ = new sdsl::int_vector<>();
    (*bt_first_level_prefix_ranks_).load(in);

    for (int i = 0; i < number_of_levels_; ++i) {
        bt_ranks_.push_back(new sdsl::int_vector<>());
        (*bt_ranks_[i]).load(in);
    }

    for (int i = 0; i < number_of_levels_-1; ++i) {
        bt_second_ranks_.push_back(new sdsl::int_vector<>());
        (*bt_second_ranks_[i]).load(in);
    }
}

CBitBlockTree::~CBitBlockTree() {

    for (sdsl::bit_vector* bv : bt_bv_) {
        delete bv;
    }

    for (sdsl::rank_support_v<1>* rank : bt_bv_rank_) {
        delete rank;
    }

    for (sdsl::int_vector<>* offsets : bt_offsets_) {
        delete offsets;
    }

    delete leaf_bv_;

    if (bt_first_level_prefix_ranks_)
        delete bt_first_level_prefix_ranks_;

    for (auto ranks : bt_ranks_) {
        delete ranks;
    }

    for (auto ranks : bt_second_ranks_) {
        delete ranks;
    }
}


int CBitBlockTree::access(int i) {

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i -= current_block*first_level_length_;
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;
            current_block = (*bt_bv_rank_[level])(current_block)*r_ + child_number;
            ++level;
        } else { // Case BackBlock
            int encoded_offset = (*bt_offsets_[level])[current_block-(*bt_bv_rank_[level])(current_block+1)];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            }
        }
    }
    return (*leaf_bv_)[i+current_block*current_length];
}


int CBitBlockTree::rank_1(int i) {

    auto& ranks = bt_ranks_;
    auto& second_ranks = bt_second_ranks_;

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i = i-current_block*current_length;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_)[current_block];
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            current_length /= r_;
            int child_number = i/current_length;
            i -= child_number*current_length;

            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            for (int child = firstChild; child < firstChild + child_number; ++child)
                r += (*ranks[level+1])[child];
            current_block = firstChild + child_number;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;
            i += encoded_offset%current_length;
            r += (*second_ranks[level])[index];
            if (i >= current_length) {
                ++current_block;
                i -= current_length;
            } else {
                r -= (*ranks[level])[current_block];
            }

        }
    }

    auto& leaf_bv = *leaf_bv_;
    int chunk = (current_block*current_length)/64;
    uint64_t chunk_info = *(leaf_bv.m_data + chunk);

    i  += current_block*current_length;
    for (int j = current_block*current_length; j <= i; ++j) {
        int value = (chunk_info >> (j%64))%2;
        if (value == 1) ++r;
        if ((j + 1)%64 == 0) {
            ++chunk;
            chunk_info = *(leaf_bv.m_data + chunk);
        }
    }

    return r;
}


int CBitBlockTree::rank_0(int i) {
    return i-rank_1(i)+1;
}


int CBitBlockTree::select_1(int k) {

    auto& ranks = bt_ranks_;
    auto& second_ranks = bt_second_ranks_;
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_;


    int current_block = (k-1)/first_level_length_;

    int end_block = (*first_level_prefix_ranks).size()-1;
    while (current_block != end_block) {
        int m = current_block + (end_block-current_block)/2;
        int f = (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((*first_level_prefix_ranks)[m+1] < k) {
                    current_block = m+1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m-1;
        }
    }


    int current_length = first_level_length_;
    int s = current_block*current_length;
    k -= (*first_level_prefix_ranks)[current_block];
    int level = 0;
    while (level < number_of_levels_-1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block)*r_;
            int child = firstChild;
            int r = (*ranks[level+1])[child];
            int last_possible_child = (firstChild + r_-1 > (*ranks[level+1]).size()-1) ?  (*ranks[level+1]).size()-1 : firstChild + r_-1;
            while ( child < last_possible_child && k > r) {
                ++child;
                r+= (*ranks[level+1])[child];
            }
            k -= r - (*ranks[level+1])[child];
            current_length /= r_;
            s += (child-firstChild)*current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block -  (*bt_bv_rank_[level])(current_block+1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset/current_length;

            k -= (*second_ranks[level])[index];
            s -= encoded_offset%current_length;
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                k += (*ranks[level])[current_block];
            }

        }
    }

    auto& leaf_bv = *leaf_bv_;
    int chunk = (current_block*current_length)/64;
    uint64_t chunk_info = *(leaf_bv.m_data + chunk);

    for (int j = current_block*current_length; ; ++j) {
        int value = (chunk_info >> (j%64))%2;
        if (value == 1) --k;
        if (!k) return s + j - current_block*current_length;
        if ((j + 1)%64 == 0) {
            ++chunk;
            chunk_info = *(leaf_bv.m_data + chunk);
        }
    }

    return -1;
}


int CBitBlockTree::select_0(int k) {

    auto &ranks = bt_ranks_;
    auto &second_ranks = bt_second_ranks_;
    auto &first_level_prefix_ranks = bt_first_level_prefix_ranks_;


    int current_block = (k - 1) / first_level_length_;

    int end_block = (*first_level_prefix_ranks).size() - 1;
    while (current_block != end_block) {
        int m = current_block + (end_block - current_block) / 2;
        int f = first_level_length_ * m - (*first_level_prefix_ranks)[m];
        if (f < k) {
            if (end_block - current_block == 1) {
                if ((first_level_length_ * (m + 1) - (*first_level_prefix_ranks)[m + 1]) < k) {
                    current_block = m + 1;
                }
                break;
            }
            current_block = m;
        } else {
            end_block = m - 1;
        }
    }


    int current_length = first_level_length_;
    int s = current_block * current_length;
    k -= s - (*first_level_prefix_ranks)[current_block];
    int level = 0;
    while (level < number_of_levels_ - 1) {
        if ((*bt_bv_[level])[current_block]) { // Case InternalBlock
            int firstChild = (*bt_bv_rank_[level])(current_block) * r_;
            int child = firstChild;
            int child_length = current_length / r_;
            int r = child_length - (*ranks[level + 1])[child];
            int last_possible_child = (firstChild + r_ - 1 > (*ranks[level + 1]).size() - 1) ?
                                      (*ranks[level + 1]).size() - 1 : firstChild + r_ - 1;
            while (child < last_possible_child && k > r) {
                ++child;
                r += child_length - (*ranks[level + 1])[child];
            }
            k -= r - (child_length - (*ranks[level + 1])[child]);
            current_length = child_length;
            s += (child - firstChild) * current_length;
            current_block = child;
            ++level;
        } else { // Case BackBlock
            int index = current_block - (*bt_bv_rank_[level])(current_block + 1);
            int encoded_offset = (*bt_offsets_[level])[index];
            current_block = encoded_offset / current_length;

            k -= (current_length - encoded_offset % current_length) - (*second_ranks[level])[index];
            s -= encoded_offset % current_length;
            if (k > 0) {
                s += current_length;
                ++current_block;
            } else {
                k += current_length - (*ranks[level])[current_block];
            }

        }
    }


    auto& leaf_bv = *leaf_bv_;
    int chunk = (current_block*current_length)/64;
    uint64_t chunk_info = *(leaf_bv.m_data + chunk);

    for (int j = current_block*current_length; ; ++j) {
        int value = (chunk_info >> (j%64))%2;
        if (value == 0) --k;
        if (!k) return s + j - current_block*current_length;
        if ((j + 1)%64 == 0) {
            ++chunk;
            chunk_info = *(leaf_bv.m_data + chunk);
        }
    }

    return -1;
}


int CBitBlockTree::get_partial_size() {

    int fields = sizeof(int) * 3;

    int leaf_bv_size = sdsl::size_in_bytes(*leaf_bv_);

    int bt_bv_size = sizeof(void*);
    for (sdsl::bit_vector* bv : bt_bv_) {
        bt_bv_size += sdsl::size_in_bytes(*bv);
    }


    int bt_bv_rank_size = sizeof(void*);
    for (sdsl::rank_support_v<1>* bvr : bt_bv_rank_) {
        bt_bv_rank_size += sdsl::size_in_bytes(*bvr);
    }

    int bt_offsets_size = sizeof(void*);
    for (sdsl::int_vector<>* offsets: bt_offsets_) {
        bt_offsets_size += sdsl::size_in_bytes(*offsets);
    }

    return  fields + bt_bv_size+ bt_bv_rank_size+ bt_offsets_size + leaf_bv_size;
}


int CBitBlockTree::size() {

    int bt_ranks_total_size = sizeof(void*);
    for (auto ranks: bt_ranks_) {
        bt_ranks_total_size += sdsl::size_in_bytes(*ranks);
    }


    int bt_prefix_ranks_first_level_size = sdsl::size_in_bytes(*(bt_first_level_prefix_ranks_));

    int bt_second_ranks_total_size = sizeof(void*);
    for (auto ranks: bt_second_ranks_) {
        bt_second_ranks_total_size += sdsl::size_in_bytes(*ranks);
    }

    int partial_total_size = get_partial_size();
    int rank_size = bt_second_ranks_total_size + bt_ranks_total_size + bt_prefix_ranks_first_level_size;
    return rank_size + partial_total_size;
}

void CBitBlockTree::serialize(std::ostream& out) {

    out.write((char *) &r_, sizeof(int));
    out.write((char *) &first_level_length_, sizeof(int));
    out.write((char *) &number_of_levels_, sizeof(int));

    for (sdsl::bit_vector* bv : bt_bv_) {
        (*bv).serialize(out);
    }

    for (sdsl::int_vector<>* offsets : bt_offsets_) {
        (*offsets).serialize(out);
    }

    (*leaf_bv_).serialize(out);

    (*bt_first_level_prefix_ranks_).serialize(out);

    for (sdsl::int_vector<>* ranks: bt_ranks_) {
        (*ranks).serialize(out);
    }

    for (sdsl::int_vector<>* second_ranks: bt_second_ranks_) {
        (*second_ranks).serialize(out);
    }
}