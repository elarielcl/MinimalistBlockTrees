#include <pointer_based/BlockTree.h>
#include <compressed/CBlockTree.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>


int main() {

    std::string input;
    std::ifstream t("/home/elarielcl/test/BlockTrees/einstein");
    std::stringstream buffer;
    buffer << t.rdbuf();
    input = buffer.str();
    std::cout << input.length() << std::endl;

    std::unordered_set<int> characters;
    for (char c: input) {
        characters.insert(c);
    }

    BlockTree* bt = new BlockTree(input, 2, 32);
    bt->process_back_pointers();
    bt->clean_unnecessary_expansions();
    for (char c: characters)
        bt->add_rank_select_support(c);


    CBlockTree* cbt = new CBlockTree(bt);
    cbt->access(0);
    std::cout << cbt->rank(input[100],100) << std::endl;

    delete bt;
    delete cbt;
    return 0;
}

