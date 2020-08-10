# Description
This repository contains an implementation for the BlockTree data structure described [here](https://ieeexplore.ieee.org/document/7149265). Details of the implementation follow the experimental studies conducted in my MSc. thesis available [here](https://users.dcc.uchile.cl/~gnavarro/mem/algoritmos/tesisManuel.pdf). This repositorie contains only the cannonical BlockTree version. If you want to play with the software used in this experimental study, it is available in the repo [https://github.com/elarielcl/BlockTrees](https://github.com/elarielcl/BlockTrees)
# Installation Guide
First clone the repo:
```
 git clone https://github.com/elarielcl/MinimalistBlockTrees.git
 ```
 
This project is a CMake project. To build this project with some runnables you should do

```
cd ../..
mkdir build
cd build
cmake ..
make
```

You can add an executable by writing your file in the `executables` directory and add its name to the `executables/CMakeLists.txt` file, this adds the necessary libraries for you:
```
set(project_EXECUTABLES
        <new_executable>
        main)
...
```

 ## Usage Example
 Let's suppose we want to build a BlockTree, so we do:
 ```
 ...
 std::string input = "AACCCTGCTGCTGATCGGATCGTAGC";
 int r = 2; //The arity of the BlockTree
 int ll = 16; // The length of the strings represented by the leaves of the BlockTree
 
 BlockTree* bt = new BlockTree(input, r, mll); // This creates the BlockTree object, a pointer-based implementation
 bt->process_back_pointers(); // This method builds the BackPointers in the BlockTree
 bt->clean_unnecessary_expansions(); // This method removes the expansion of InternalBlocks that are unnecesary (this is also called pruning)
  ...
 ```
  in case you want to build the a more compact version but without theoretical guarantiee instead you can replace the last two lines with
 ```
 bt->process_back_pointers_heuristic();
 ```
 .. now you have a proper BlockTree answering access queries(`bt->access(i)`), if you want to give it ``bt->rank(c,i) & bt->select(c,j)`` support you should do:
 ```
 for (int c: characters)
     bt->add_rank_select_support(c);
 ```
 The above is a pointer-based implementation of BlockTrees, if you want to have a more compact representation (using bitvectors to represent the tree) you should do:
 ```
 ...
 CBlockTree *cbt = new CBlockTree(bt); // Builds a more compact BlockTree representation
 cbt->access(i);
 cbt->select(c,i);
 cbt->size(); // It obtains the size (in bytes) of this compact representation
 cbt->serialize(out); // It serializes it components to an output stream
 ...
 CBlockTree *loaded_cbt = new CBlockTree(in); // It loads a CBlockTree from an input stream
 ...
 ```
 If your input is a bitmap you can build an (even) more compact representation specialized for bitmaps:
 ```
 ...
 CBitBlockTree *cbbt = new CBitBlockTree(bt, '('); // It assumes that the BlockTree represents a bitmap whose "1" 
                                                   //is the second argument of the contruction, the rest are "0"s
 cbbt->access(i);
 cbbt->rank_1(i);
 cbbt->select_0(i);
 ...
 ...
 ```
 
 ... and never forget to delet your trash ;)
 ```
 ...
 delete bt;
 delete cbt;
 delete loaded_cbt;
 delete cbbt;
 ...
 ```
 
 # Contact
 Any error, improvement or suggestion you can write me to `elarielcl` at Gmail. 
