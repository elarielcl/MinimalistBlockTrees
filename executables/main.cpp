#include <pointer_based/BlockTree.h>
#include <compressed/CBitBlockTree.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>


int main() {

    std::string input;
    std::ifstream t("../../tests/data/dna.par");
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


    CBitBlockTree* cbt = new CBitBlockTree(bt, input[0]);
    cbt->access(0);
    std::cout << cbt->rank_1(10) << std::endl;

    std::ofstream ot("dna.par.bt");
    cbt->serialize(ot);
    ot.close();
    std::ifstream it("dna.par.bt");
    CBitBlockTree* lcbt = new CBitBlockTree(it);
    lcbt->access(0);
    std::cout << lcbt->rank_1(10) << std::endl;

    delete bt;
    delete cbt;
    delete lcbt;
    return 0;
}

