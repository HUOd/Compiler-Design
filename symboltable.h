#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <map>
#include <set>
using namespace std;

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

// enum class attr{
//     VOID, INT, NULLX, STRING, STRUCT, ARRAY, FUNCTION, VARIABLE, FIELD,
//     TYPEID, PARAM, LOCAL, LVAL, CONST, VREG, VADDR, BITSET_SIZE
// };

// using attr_bitset = bitset<(size_t)attr::BITSET_SIZE>;

struct symbol;

using symbol_table = unordered_map<const string*, symbol*>;
using symbol_entry = symbol_table::value_type;

struct symbol {
    attr_bitset attributes;
    size_t sequence = 0;
    symbol_table* fields = nullptr;
    location lloc;
    size_t block_nr = 0;
    vector<symbol *> * parameters;
    const string* struct_name;

    void setAttr(astree* node);
}; 

bool ASTTraversal(astree * root, FILE* outfile);
void arrange_list(string& list, symbol* table);

#endif 