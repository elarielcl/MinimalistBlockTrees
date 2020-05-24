#include <compressed/CBlockTree.h>
#include <unordered_set>

CBlockTree::CBlockTree(BlockTree * bt) : r_(bt->r_) {
    std::vector<Block*> first_level = {bt->root_block_};
    bool is_first_level = false;
    while (!is_first_level) {
        for (Block* b: first_level) {
            is_first_level = is_first_level || b->is_leaf();
        }
        if (is_first_level) break;
        first_level = bt->next_level(first_level);
    }

    std::unordered_map<int,int> first_level_prefix_ranks_;
    std::unordered_map<int,sdsl::int_vector<>*> first_level_prefix_ranks;
    std::unordered_map<int,sdsl::int_vector<>*> first_level_ranks;

    for (auto pair: first_level[0]->ranks_) { //auto &
        first_level_prefix_ranks[pair.first] = new sdsl::int_vector<>(first_level.size());
        first_level_ranks[pair.first] = new sdsl::int_vector<>(first_level.size());
    }
    for (int i = 0; i < first_level.size(); ++i) {
        for (auto pair: first_level_prefix_ranks_)
            (*first_level_prefix_ranks[pair.first])[i] = pair.second;

        for (auto pair: first_level[i]->ranks_) {
            (*first_level_ranks[pair.first])[i] = first_level[i]->ranks_[pair.first];
            first_level_prefix_ranks_[pair.first] = first_level_prefix_ranks_[pair.first] + first_level[i]->ranks_[pair.first];
        }
    }
    for (auto pair : first_level_prefix_ranks) {
        sdsl::util::bit_compress(*(pair.second));
        bt_first_level_prefix_ranks_[pair.first] = pair.second;
    }
    for (auto pair : first_level_ranks) {
        sdsl::util::bit_compress(*(pair.second));
        bt_ranks_[pair.first].push_back(pair.second);
    }

    first_level_length_ = first_level[0]->length();
    number_of_levels_ = 0;

    std::vector<Block*> current_level = first_level;
    std::vector<Block*> next_level = bt->next_level(first_level);

    while (next_level.size() != 0) {
        sdsl::bit_vector* current_level_bv = new sdsl::bit_vector(current_level.size() ,0);

        std::unordered_map<int,sdsl::int_vector<>*> next_level_ranks;

        for (auto pair: next_level[0]->ranks_) {
            next_level_ranks[pair.first] = new sdsl::int_vector<>(next_level.size());
        }

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
                (*next_level_ranks[pair.first])[i] = pair.second;
            }
        }


        sdsl::int_vector<>* current_level_offsets = new sdsl::int_vector<>(number_of_leaves);

        std::unordered_map<int,sdsl::int_vector<>*> current_level_second_ranks;
        for (auto pair: current_level[0]->ranks_) {
            current_level_second_ranks[pair.first] = new sdsl::int_vector<>(number_of_leaves);
        }

        int j = 0;
        for (int i = 0; i < current_level.size(); ++i) {
            if (!(*current_level_bv)[i]) {
                for (auto pair: current_level[i]->second_ranks_) {
                    (*current_level_second_ranks[pair.first])[j] = pair.second;
                }

                (*current_level_offsets)[j++] = current_level[i]->first_block_->level_index_ * current_length + current_level[i]->offset_;
            }
        }

        sdsl::util::bit_compress(*current_level_offsets);
        bt_offsets_.push_back(current_level_offsets);

        for (auto pair : next_level_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_ranks_[pair.first]).push_back(pair.second);
        }

        for (auto pair : current_level_second_ranks) {
            sdsl::util::bit_compress(*(pair.second));
            (bt_second_ranks_[pair.first]).push_back(pair.second);
        }

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

    std::unordered_set<char> alphabet;
    for (char c : leaf_string) {
        alphabet.insert(c);
    }
    alphabet_ = new sdsl::int_vector<>(alphabet.size());
    int counter = 0;
    for (char c : alphabet){
        mapping_[c] = counter;
        (*alphabet_)[counter++] = c;
    }
    sdsl::util::bit_compress(*alphabet_);


    leaf_string_ = new sdsl::int_vector<>(leaf_string.size());

    for (int i = 0; i<(*leaf_string_).size(); ++i) {
        (*leaf_string_)[i] = mapping_[leaf_string[i]];
    }

    sdsl::util::bit_compress(*leaf_string_);
}

CBlockTree::CBlockTree(std::istream& in) {
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

    leaf_string_ = new sdsl::int_vector<>();
    (*leaf_string_).load(in);

    alphabet_ = new sdsl::int_vector<>();
    (*alphabet_).load(in);

    int c = 0;
    for (int character : (*alphabet_)) {
        mapping_[character] = c++;
    }

    for (int character : (*alphabet_)) {
        bt_first_level_prefix_ranks_[character] = new sdsl::int_vector<>();
        (*bt_first_level_prefix_ranks_[character]).load(in);
    }

    for (int character : (*alphabet_)) {
        for (int i = 0; i < number_of_levels_; ++i) {
            bt_ranks_[character].push_back(new sdsl::int_vector<>());
            (*bt_ranks_[character][i]).load(in);
        }
    }

    for (int character : (*alphabet_)) {
        for (int i = 0; i < number_of_levels_-1; ++i) {
            bt_second_ranks_[character].push_back(new sdsl::int_vector<>());
            (*bt_second_ranks_[character][i]).load(in);
        }
    }

}

CBlockTree::~CBlockTree() {

    for (sdsl::bit_vector* bv : bt_bv_) {
        delete bv;
    }

    for (sdsl::rank_support_v<1>* rank : bt_bv_rank_) {
        delete rank;
    }

    for (sdsl::int_vector<>* offsets : bt_offsets_) {
        delete offsets;
    }

    delete leaf_string_;
    delete alphabet_;

    for (auto pair : bt_first_level_prefix_ranks_) {
        delete pair.second;
    }

    for (auto pair : bt_ranks_) {
        for (sdsl::int_vector<>* ranks : pair.second) {
            delete ranks;
        }
    }

    for (auto pair : bt_second_ranks_) {
        for (sdsl::int_vector<> *ranks : pair.second) {
            delete ranks;
        }
    }
}


int CBlockTree::access(int i) {

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
    return (*alphabet_)[(*leaf_string_)[i+current_block*current_length]];
}


int CBlockTree::rank(int c, int i) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];

    int current_block = i/first_level_length_;
    int current_length = first_level_length_;
    i = i-current_block*current_length;
    int level = 0;

    int r = (*bt_first_level_prefix_ranks_[c])[current_block];
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

    i  += current_block*current_length;
    int d = mapping_[c];
    for (int j = current_block*current_length; j <= i; ++j) {
        if ((*leaf_string_)[j] == d) ++r;
    }

    return r;
}


int CBlockTree::select(int c, int k) {

    auto& ranks = bt_ranks_[c];
    auto& second_ranks = bt_second_ranks_[c];
    auto& first_level_prefix_ranks = bt_first_level_prefix_ranks_[c];


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
            while ( child < last_possible_child && k > r) { //Border conditions?
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

    int d = mapping_[c];
    for (int j = current_block*current_length;  ; ++j) {
        if ((*leaf_string_)[j] == d) --k;
        if (!k) return s + j - current_block*current_length;
    }

    return -1;
}

int CBlockTree::get_partial_size() {
    int fields = sizeof(int) * 3;

    int leaf_string_size = sdsl::size_in_bytes(*leaf_string_);

    int alphabet_size = sdsl::size_in_bytes(*alphabet_);
    int mapping_size = sizeof(int) * 256;

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

    return  fields + mapping_size + alphabet_size + bt_bv_size+ bt_bv_rank_size+ bt_offsets_size + leaf_string_size;
}


int CBlockTree::size() {

    int bt_ranks_total_size = (bt_ranks_.size()+1) * sizeof(void*);
    for (auto pair: bt_ranks_) {
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_ranks_total_size += size;
    }


    int bt_prefix_ranks_first_level_size = 0;
    for (auto pair: bt_first_level_prefix_ranks_) {
        bt_prefix_ranks_first_level_size += sdsl::size_in_bytes(*(pair.second));
    }

    int bt_second_ranks_total_size = (bt_second_ranks_.size()+1) * sizeof(void*);
    for (auto pair: bt_second_ranks_) {
        int size = 0;
        for (sdsl::int_vector<>* ranks: pair.second) {
            size += sdsl::size_in_bytes(*ranks);
        }
        bt_second_ranks_total_size += size;
    }

    int partial_total_size = get_partial_size();
    int rank_size = bt_second_ranks_total_size + bt_ranks_total_size + bt_prefix_ranks_first_level_size;
    return rank_size + partial_total_size;
}


void CBlockTree::serialize(std::ostream& out) {

    out.write((char *) &r_, sizeof(int));
    out.write((char *) &first_level_length_, sizeof(int));
    out.write((char *) &number_of_levels_, sizeof(int));

    for (sdsl::bit_vector* bv : bt_bv_) {
        (*bv).serialize(out);
    }

    for (sdsl::int_vector<>* offsets : bt_offsets_) {
        (*offsets).serialize(out);
    }

    (*leaf_string_).serialize(out);

    (*alphabet_).serialize(out);

    for (int character: (*alphabet_)) {
        (*bt_first_level_prefix_ranks_[character]).serialize(out);
    }

    for (int character: (*alphabet_)) {
        for (sdsl::int_vector<>* ranks: bt_ranks_[character]) {
            (*ranks).serialize(out);
        }
    }

    for (int character: (*alphabet_)) {
        for (sdsl::int_vector<>* second_ranks: bt_second_ranks_[character]) {
            (*second_ranks).serialize(out);
        }
    }
}