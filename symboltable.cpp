#include "symboltable.h" 

int next_block = 1;
symbol_table* global_names = new symbol_table();
vector<symbol_table* > symbol_table_stack(1, global_names);
map<int, symbol*> func_map;
unordered_map<astree*, symbol*> nodesb_table;

bool return_compatible(symbol* func, symbol* return_sym) {
    if (func->attributes[static_cast<size_t>(attr::VOID)] != return_sym->attributes[static_cast<size_t>(attr::VOID)]
        || func->attributes[static_cast<size_t>(attr::INT)] != return_sym->attributes[static_cast<size_t>(attr::INT)]
        || func->attributes[static_cast<size_t>(attr::STRING)] != return_sym->attributes[static_cast<size_t>(attr::STRING)]) {
            return false;
    }

    if (func->attributes[static_cast<size_t>(attr::STRUCT)] != return_sym->attributes[static_cast<size_t>(attr::STRUCT)]) {
        return false;
    }
    else {
        if (func->struct_name != return_sym->struct_name) {
            return false;
        }
    }

    if (func->attributes[static_cast<size_t>(attr::ARRAY)] != return_sym->attributes[static_cast<size_t>(attr::ARRAY)]) {
        return false;
    }

    return true;
}

void symbol::setAttr(astree* node) {
    
    switch(node->symbol) {
        case TOK_KW_VOID : {
            this->attributes.set(static_cast<size_t>(attr::VOID));
            break;
        }
        case TOK_KW_NULL : {
            this->attributes.set(static_cast<size_t>(attr::NULLX));
            break;
        }
        case TOK_KW_INT : {
            this->attributes.set(static_cast<size_t>(attr::INT));
            break;
        }
        case TOK_KW_CHAR : {
            this->attributes.set(static_cast<size_t>(attr::INT));
            break;
        }
        case TOK_KW_STRING : {
            this->attributes.set(static_cast<size_t>(attr::STRING));
            break;
        }

        case TOK_TYPEID : {
            if (global_names->find(node->lexinfo) == global_names->end()) {
                global_names->emplace(node->lexinfo, nullptr);
            }
            else if (global_names->find(node->lexinfo) != global_names->end()
                        && global_names->find(node->lexinfo)->second != nullptr){
                this->attributes = global_names->find(node->lexinfo)->second->attributes;
                global_names->emplace(node->lexinfo, this);
            }
            this->struct_name = node->lexinfo;
            this->attributes.set(static_cast<size_t>(attr::STRUCT));
            this->attributes.set(static_cast<size_t>(attr::TYPEID));
            break;
        }

        case TOK_INTCON : {
            this->attributes.set(static_cast<size_t>(attr::INT));
            this->attributes.set(static_cast<size_t>(attr::CONST));
            break;
        }
        case TOK_CHARCON : {
            this->attributes.set(static_cast<size_t>(attr::INT));
            this->attributes.set(static_cast<size_t>(attr::CONST));
            break;
        }
        case TOK_STRINGCON : {
            this->attributes.set(static_cast<size_t>(attr::STRING));
            this->attributes.set(static_cast<size_t>(attr::CONST));
            break;
        }

        case TOK_KW_RETURN : {
            this->attributes.set(static_cast<size_t>(attr::VOID));
            int blocknum = next_block - 1;

            symbol* func = nullptr;
            while(blocknum > 0) {
                if (func_map.find(blocknum) != func_map.end()) {
                    func = func_map.find(blocknum)->second;
                    break;
                }
                blocknum--;
            }

            if (func == nullptr) {
                fprintf(stderr, "(%zd.%zd.%zd) return out of a block!\n", 
                    node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
            }
            else {
                if (!return_compatible(func, this)) {
                        fprintf(stderr, "(%zd.%zd.%zd) return type not match!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                }
            }

            break;
        }

        case TOK_IDENT : {
            const string* var_name = node->lexinfo;
            if (*(var_name) == "__FUNC__") this->attributes.set(static_cast<size_t>(attr::STRING));
            else {

                int blocknum = next_block - 1;
                int found = 0;
                symbol_table * localtable;
                while(blocknum > 0) {
                    localtable = symbol_table_stack[blocknum];
                    if (localtable->find(var_name) != localtable->end()) {
                        this->attributes = localtable->find(var_name)->second->attributes;
                        if (this->attributes[static_cast<size_t>(attr::TYPEID)] == 1) {
                            this->struct_name = localtable->find(var_name)->second->struct_name;
                        }
                        found = 1;
                    }
                    if (func_map.find(blocknum) != func_map.end()) break;
                    --blocknum; 
                }
                if (found == 0) {
                    if (global_names->find(var_name) != global_names->end()) {
                        this->attributes = global_names->find(var_name)->second->attributes;
                        if (this->attributes[static_cast<size_t>(attr::TYPEID)] == 1) {
                            this->struct_name = global_names->find(var_name)->second->struct_name;
                        }
                    }
                    else {
                        fprintf(stderr, "(%zd.%zd.%zd) The variable has not been defined! \n", 
                                 node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    }
                }
            }
            this->attributes.set(static_cast<size_t>(attr::VARIABLE));
            this->attributes.set(static_cast<size_t>(attr::LVAL));
            break;
        }

        default : break;
    }
    this->lloc = node->lloc;
}

bool compatible_check(symbol* left, symbol* right) {

    if (right->attributes[static_cast<size_t>(attr::NULLX)] == 1) return true;

    if (left->attributes[static_cast<size_t>(attr::INT)] != right->attributes[static_cast<size_t>(attr::INT)]
        || left->attributes[static_cast<size_t>(attr::STRING)] != right->attributes[static_cast<size_t>(attr::STRING)]
        || left->attributes[static_cast<size_t>(attr::STRUCT)] != right->attributes[static_cast<size_t>(attr::STRUCT)]
        || left->attributes[static_cast<size_t>(attr::VOID)] != right->attributes[static_cast<size_t>(attr::VOID)]
        || left->attributes[static_cast<size_t>(attr::ARRAY)] != right->attributes[static_cast<size_t>(attr::ARRAY)]) {
            return false;
        }
    
    if (left->attributes[static_cast<size_t>(attr::STRUCT)] == 1 
        && right->attributes[static_cast<size_t>(attr::STRUCT)] == 1) {
            if (left->struct_name != right->struct_name) {
                return false;
            }
        }

    return true;
}

void arrange_list(string& list, symbol* table) {
    if (table->attributes[static_cast<size_t>(attr::VOID)] == 1) {
        list += "void ";
    }
    if (table->attributes[static_cast<size_t>(attr::STRUCT)] == 1) {
        list += "struct "; 
        list += *(table->struct_name);
        list += " ";
    }
    if (table->attributes[static_cast<size_t>(attr::INT)] == 1) {
        list += "int ";
    }
    if (table->attributes[static_cast<size_t>(attr::STRING)] == 1) {
        list += "string ";
    }
    if (table->attributes[static_cast<size_t>(attr::ARRAY)] == 1) {
        list += "array ";
    }
    if (table->attributes[static_cast<size_t>(attr::FUNCTION)] == 1) {
        list += "function ";
    }
    if (table->attributes[static_cast<size_t>(attr::VARIABLE)] == 1) {
        list += "variable ";
    }
    if (table->attributes[static_cast<size_t>(attr::FIELD)] == 1) {
        list += "field ";
    }
    if (table->attributes[static_cast<size_t>(attr::TYPEID)] == 1) {
        list += "typeid ";
    }
    if (table->attributes[static_cast<size_t>(attr::PARAM)] == 1) {
        list += "param ";
    }
    if (table->attributes[static_cast<size_t>(attr::LVAL)] == 1) {
        list += "lval ";
    }
    if (table->attributes[static_cast<size_t>(attr::LOCAL)] == 1) {
        list += "local ";
    }
}

void print_parentsymboltable(symbol* parsym, const string* name, FILE* outfile) {
    string list = "";
    arrange_list(list, parsym);
    fprintf(outfile, "%s (%zd.%zd.%zd) {%zd} %s \n", 
            (*name).c_str(), 
            parsym->lloc.filenr, parsym->lloc.linenr, parsym->lloc.offset, 
            parsym->block_nr,
            list.c_str());
}

void print_childsymboltable(symbol* chisym, const string* name, FILE* outfile) {
    string list = "";
    arrange_list(list, chisym);
    fprintf(outfile, "\t %s (%zd.%zd.%zd) {%zd} %s %zd \n", 
            (*name).c_str(), 
            chisym->lloc.filenr, chisym->lloc.linenr, chisym->lloc.offset, 
            chisym->block_nr,
            list.c_str(),
            chisym->sequence);
}

bool ASTtypecheck(astree * node) {
    
    for (size_t i = 0; i < node->children.size(); ++i) {
        bool check_true = ASTtypecheck(node->children[i]);
        if (!check_true) return false;
    }

    if (nodesb_table.find(node) != nodesb_table.end()) return true;

    if (node->children.size() == 0) {
        symbol* nodeSymbol = new symbol();
        nodeSymbol->setAttr(node);
        node->block_nr = next_block - 1;
        node->attributes = nodeSymbol->attributes;
        nodeSymbol->lloc = node->lloc;
        if (nodeSymbol->struct_name != nullptr) {
            node->struct_name = nodeSymbol->struct_name;
        }
        nodesb_table.emplace(node, nodeSymbol);
        return true;
    }
    symbol* nodeSymbol = new symbol();
    nodeSymbol->block_nr = next_block - 1;
    switch(node->symbol) {

        case TOK_INDEX : {
            astree* left = node->children[0];
            astree* right = node->children[1];
            symbol* left_sym = nodesb_table[left];
            symbol* right_sym = nodesb_table[right];

            if (left_sym->attributes[static_cast<size_t>(attr::STRING)] == 1 
                && left_sym->attributes[static_cast<size_t>(attr::ARRAY)] == 1) {
                if (right_sym->attributes[static_cast<size_t>(attr::INT)] != 1) {
                    fprintf(stderr, "(%zd.%zd.%zd) Can only use INT type for indexing!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                } 
                else {
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::STRING));
                    //nodeSymbol->attributes[static_cast<size_t>(attr::VARIABLE] = 0;
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::LVAL));
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::VADDR));
                }
            }
            else if (left_sym->attributes[static_cast<size_t>(attr::ARRAY)] == 1) {
                if (right_sym->attributes[static_cast<size_t>(attr::INT)] != 1) {
                    fprintf(stderr, "(%zd.%zd.%zd) Can only use INT type for indexing!\n",
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                } 
                else {
                    nodeSymbol->attributes = left_sym->attributes;
                    nodeSymbol->attributes[static_cast<size_t>(attr::ARRAY)] = 0;
                    //nodeSymbol->attributes[static_cast<size_t>(attr::VARIABLE] = 0;
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::LVAL));
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::VADDR));
                }
            }
            else if (left_sym->attributes[static_cast<size_t>(attr::ARRAY)] == 0
                    && left_sym->attributes[static_cast<size_t>(attr::STRING)] == 1) {
                if (right_sym->attributes[static_cast<size_t>(attr::INT)] != 1) {
                    fprintf(stderr, "(%zd.%zd.%zd) Can only use INT type for indexing!\n",
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                } 
                else {
                    //nodeSymbol->attributes[static_cast<size_t>(attr::VARIABLE] = 0;
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::INT));
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::LVAL));
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::VADDR));
                }
            }
            else {
                fprintf(stderr, "(%zd.%zd.%zd) This Type can't use indexing! \n",
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }
            break;
        }

        case TOK_CALL : {
            astree* left = node->children[0];
            astree* right = node->children[1];

            if (global_names->find(left->lexinfo) == global_names->end() 
               || (global_names->find(left->lexinfo) != global_names->end()
                   && global_names->find(left->lexinfo)->second->attributes[static_cast<size_t>(attr::FUNCTION)] == 0)) {
                       fprintf(stderr, "(%zd.%zd.%zd) Can't call a function not declared!\n",
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                       return false;
                   }
            
            symbol* func_sym = global_names->find(left->lexinfo)->second;

            if (func_sym->parameters->size() != right->children.size()) {
                fprintf(stderr, "(%zd.%zd.%zd) Parameter length mismatched!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                       return false;
            }

            for (size_t i = 0; i < right->children.size(); ++i) {
                symbol* argument = nodesb_table[right->children[i]];
                if (!compatible_check(func_sym->parameters->at(i), argument)) {
                    fprintf(stderr, "(%zd.%zd.%zd) Type assignment did not match!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                }
            }

            nodeSymbol->attributes = func_sym->attributes;
            nodeSymbol->struct_name = func_sym->struct_name;
            nodeSymbol->attributes[static_cast<size_t>(attr::FUNCTION)] = 0;
            nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
            break;
        }

        case TOK_KW_NEW : {
            astree* left = node->children[0];

            if (node->children.size() == 1) {
                if (left->symbol == TOK_TYPEID) {
                    if (global_names->find(left->lexinfo) == global_names->end()) {
                        fprintf(stderr, "(%zd.%zd.%zd) The struct has not been defined! \n", 
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                        return false;
                    }
                    else {
                        nodeSymbol->attributes = nodesb_table[left]->attributes;
                        nodeSymbol->struct_name = nodesb_table[left]->struct_name;
                        nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
                    }
                }
            }
            else if (node->children.size() == 2) {
                if (left->symbol == TOK_KW_STRING) {
                    if (!nodesb_table[node->children[1]]->attributes[static_cast<size_t>(attr::INT)]) {
                        fprintf(stderr, "(%zd.%zd.%zd) The string length should be int type!\n", 
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                        return false;
                    }
                    nodeSymbol->attributes = nodesb_table[left]->attributes;
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
                }
            }
            else if (node->children.size() == 3) {
                if (node->children[1]->symbol == TOK_ARRAY) {
                    if (nodesb_table[node->children[2]]->attributes[static_cast<size_t>(attr::INT)] == 0) {
                        fprintf(stderr, "(%zd.%zd.%zd) The array length should be int type!\n",
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                        return false;
                    }
                    nodeSymbol->attributes = nodesb_table[left]->attributes;
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::ARRAY));
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
                }
            }
            break;
        }

        case TOK_KW_RETURN : {
            if (node->children.size() > 0) {
                astree* return_content = node->children[0];
                symbol* return_type = nodesb_table[return_content];
                nodeSymbol->attributes = return_type->attributes;

                int blocknum = next_block - 1;

                symbol* func = nullptr;
                while(blocknum > 0) {
                    if (func_map.find(blocknum) != func_map.end()) {
                        func = func_map.find(blocknum)->second;  
                        break;
                    }
                    blocknum--;
                }

                if (func == nullptr) {
                    fprintf(stderr, "(%zd.%zd.%zd) return out of a block!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                }
                else {
                    if (!return_compatible(func, return_type)) {
                         fprintf(stderr, "(%zd.%zd.%zd) return type not match!\n", 
                         node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    }
                }
            }
            break;
        }

        case '+' :
        case '-' : {
                astree* left = node->children[0];
                astree* right = node->children[1];
                symbol* left_sym = nodesb_table[left];
                symbol* right_sym = nodesb_table[right];
                if (left_sym->attributes[static_cast<size_t>(attr::INT)] == 1 && right_sym->attributes[static_cast<size_t>(attr::INT)] == 1) {
                        nodeSymbol->attributes = left_sym->attributes;
                        nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
                    } 
                    else {
                    fprintf(stderr, "(%zd.%zd.%zd) Only int type could do such culculation!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                    }
            break;
        }

        case TOK_NEG :
        case TOK_POS : {
            astree* right = node->children[0];
            symbol* right_sym = nodesb_table[right];
            if (right_sym->attributes[static_cast<size_t>(attr::INT)] == 0) {
                fprintf(stderr, "(%zd.%zd.%zd) Only int type could do such assignment!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }
            nodeSymbol->attributes = right_sym->attributes;
            nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
            break;
        }

        case '!' :
        case TOK_KW_NOT : {
            astree* left = node->children[0];
            symbol* left_sym = nodesb_table[left];
            if (left_sym->attributes[static_cast<size_t>(attr::INT)] == 0) {
                fprintf(stderr, "(%zd.%zd.%zd) Only int type could do such assignment!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }
            nodeSymbol->attributes = left_sym->attributes;
            nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
            break;
        }

        case '*' :
        case '/' :
        case '%' : {
            astree* left = node->children[0];
            astree* right = node->children[1];
            symbol* left_sym = nodesb_table[left];
            symbol* right_sym = nodesb_table[right];
            if (left_sym->attributes[static_cast<size_t>(attr::INT)] == 1 
                && right_sym->attributes[static_cast<size_t>(attr::INT)] == 1) {
                    nodeSymbol->attributes = left_sym->attributes;
                    nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
                }
            else {
                fprintf(stderr, "(%zd.%zd.%zd) Only int type could do such culculation!\n",
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }
            break;
        }

        case '<' :
        case '>' :
        case TOK_DOUBLE_EQ :
        case TOK_NOT_EQ :
        case TOK_LESS_TH_EQ :
        case TOK_LARGE_TH_EQ: {
            astree* left = node->children[0];
            astree* right = node->children[1];
            symbol* left_sym = nodesb_table[left];
            symbol* right_sym = nodesb_table[right];

            if (compatible_check(left_sym, right_sym)) {
                nodeSymbol->attributes.set(static_cast<size_t>(attr::INT));
                nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
            }
            else {
                fprintf(stderr, "(%zd.%zd.%zd) Not compatible, not able to compare!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }
            break;
        }

        case '=' : {
            astree* left = node->children[0];
            astree* right = node->children[1];
            symbol* left_sym = nodesb_table[left];
            symbol* right_sym = nodesb_table[right];

            if (compatible_check(left_sym, right_sym)) {
                if (left_sym->attributes[static_cast<size_t>(attr::LVAL)] == 0) {
                    fprintf(stderr, "(%zd.%zd.%zd) Can only assign to a LVAL variable \n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                }
                nodeSymbol->attributes = left_sym->attributes;
                nodeSymbol->attributes.set(static_cast<size_t>(attr::VREG));
            }
            else {
                fprintf(stderr, "(%zd.%zd.%zd) Not compatible, not able to assign!\n", 
                        node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }
            break;
        }

        case TOK_KW_INT : {
            astree* var = node->children[0];
            nodesb_table[var]->attributes.set(static_cast<size_t>(attr::INT));
            nodeSymbol->attributes.set(static_cast<size_t>(attr::INT));
            break;
        }

        case TOK_KW_CHAR : {
            astree* var = node->children[0];
            nodesb_table[var]->attributes.set(static_cast<size_t>(attr::INT));
            nodeSymbol->attributes.set(static_cast<size_t>(attr::INT));
            break;
        }

        case TOK_KW_STRING : {
            astree* var = node->children[0];
            nodesb_table[var]->attributes.set(static_cast<size_t>(attr::STRING));
            nodeSymbol->attributes.set(static_cast<size_t>(attr::STRING));
            break;
        }

        case TOK_KW_VOID : {
            astree* var = node->children[0];
            nodesb_table[var]->attributes.set(static_cast<size_t>(attr::VOID));
            nodeSymbol->attributes.set(static_cast<size_t>(attr::VOID));
            break;
        }

        case TOK_TYPEID : {
            astree* var = node->children[0];
            symbol* var_sym = nodesb_table[var];
            nodeSymbol->setAttr(node);
            var_sym->fields = nodeSymbol->fields;
            var_sym->struct_name = nodeSymbol->struct_name;
            var_sym->attributes.set(static_cast<size_t>(attr::STRUCT));
            break;
        }

        case TOK_ARRAY : {
            astree* var = node->children[1];
            astree* type = node->children[0];
            symbol* var_sym = nodesb_table[var];
            symbol* type_sym = nodesb_table[type];
            var_sym->attributes.set(static_cast<size_t>(attr::ARRAY));
            for (size_t i = 0; i < static_cast<size_t>(attr::BITSET_SIZE); ++i) {
                if (type_sym->attributes[i] == 1) {
                    var_sym->attributes.set(i);
                }
            }

            break;
        }
        default : break;
    }
        nodeSymbol->lloc = node->lloc;
        node->block_nr = next_block - 1;
        node->attributes = nodeSymbol->attributes;
        if (nodeSymbol->struct_name != nullptr) {
            node->struct_name = nodeSymbol->struct_name;
        }
        nodesb_table.emplace(node, nodeSymbol);

    return true;
}

bool ASTTraversal(astree * node, FILE* outfile) {

    symbol* nodeSymbol = new symbol();
    nodeSymbol->lloc = node->lloc;
    switch(node->symbol) {

        case TOK_FUNCTION : {
            astree* func_type_node = node->children[0];
            const string* func_name;
            symbol* func_attr_info = new symbol();
            if (func_type_node->symbol == TOK_ARRAY) {
                func_attr_info->setAttr(func_type_node->children[0]);
                nodeSymbol->attributes = func_attr_info->attributes;
                func_name = func_type_node->children[1]->lexinfo;
                func_attr_info->attributes.set(static_cast<size_t>(attr::ARRAY));
                nodesb_table.emplace(func_type_node->children[0], func_attr_info);
                nodesb_table.emplace(func_type_node->children[1], func_attr_info);
                nodesb_table.emplace(func_type_node, func_attr_info);
                func_attr_info->attributes.set(static_cast<size_t>(attr::FUNCTION));
                func_attr_info->attributes[static_cast<size_t>(attr::VARIABLE)] = 0;
                func_type_node->children[1]->attributes = func_attr_info->attributes;
            }
            else if (func_type_node->symbol == TOK_TYPEID) {
                func_attr_info->setAttr(func_type_node);
                if (global_names->find(func_type_node->lexinfo) == global_names->end()) {
                    fprintf(stderr, "(%zd.%zd.%zd) This type has not been declared!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                }
                func_attr_info->struct_name = global_names->find(func_type_node->lexinfo)->second->struct_name;
                func_attr_info->fields = global_names->find(func_type_node->lexinfo)->second->fields; 
                func_name = func_type_node->children[0]->lexinfo;
                nodesb_table.emplace(func_type_node->children[0], func_attr_info);
                nodesb_table.emplace(func_type_node, func_attr_info);
                func_attr_info->attributes.set(static_cast<size_t>(attr::FUNCTION));
                func_attr_info->attributes[static_cast<size_t>(attr::VARIABLE)] = 0;
                func_type_node->children[0]->attributes = func_attr_info->attributes;
                func_type_node->children[0]->struct_name = func_attr_info->struct_name;
            }
            else {
                func_attr_info->setAttr(func_type_node);
                nodeSymbol->attributes = func_attr_info->attributes;
                func_name = func_type_node->children[0]->lexinfo;
                nodesb_table.emplace(func_type_node->children[0], func_attr_info);
                nodesb_table.emplace(func_type_node, func_attr_info);
                func_attr_info->attributes.set(static_cast<size_t>(attr::FUNCTION));
                func_attr_info->attributes[static_cast<size_t>(attr::VARIABLE)] = 0;
                func_type_node->children[0]->attributes = func_attr_info->attributes;
            }
            print_parentsymboltable(func_attr_info, func_name, outfile);
            
            symbol_table* localSymTable = new symbol_table();
            symbol_table_stack.push_back(localSymTable);
            next_block = symbol_table_stack.size();
            func_map[next_block - 1] = func_attr_info;

            astree* func_params = node->children[1];

            int sequence_count = 0;
            func_attr_info->parameters = new vector<symbol*>();
            for (size_t i = 0; i < func_params->children.size(); ++i) {
                astree * param = func_params->children[i];
                symbol* param_info = new symbol();
                const string* param_name;
                if (param->symbol == TOK_ARRAY) {
                    param_info->setAttr(param->children[0]);
                    localSymTable->emplace(param->children[1]->lexinfo, param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::ARRAY));
                    param_name = param->children[1]->lexinfo;
                    nodesb_table.emplace(param->children[0], param_info);
                    nodesb_table.emplace(param->children[1], param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::PARAM));
                    param_info->attributes.set(static_cast<size_t>(attr::VARIABLE));
                    param_info->attributes.set(static_cast<size_t>(attr::LVAL));
                    param_info->sequence = sequence_count;
                    param_info->block_nr = next_block - 1;
                    param->children[1]->attributes = param_info->attributes;
                }
                else if (param->symbol == TOK_TYPEID) {
                    param_info->setAttr(param);
                    if (global_names->find(param->lexinfo) == global_names->end()) {
                        fprintf(stderr, "(%zd.%zd.%zd) This type has not been declared!\n", 
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    }
                    param_info->struct_name = global_names->find(param->lexinfo)->second->struct_name;
                    param_info->fields = global_names->find(param->lexinfo)->second->fields; 
                    param_name = param->children[0]->lexinfo;
                    nodesb_table.emplace(param->children[0], param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::PARAM));
                    param_info->attributes.set(static_cast<size_t>(attr::VARIABLE));
                    param_info->attributes.set(static_cast<size_t>(attr::LVAL));
                    param_info->sequence = sequence_count;
                    param_info->block_nr = next_block - 1;
                    param->children[0]->attributes = param_info->attributes;
                    param->children[0]->struct_name = param_info->struct_name;
                }
                else {
                    param_info->setAttr(param);
                    localSymTable->emplace(param->children[0]->lexinfo, param_info);
                    param_name = param->children[0]->lexinfo;
                    nodesb_table.emplace(param->children[0], param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::PARAM));
                    param_info->attributes.set(static_cast<size_t>(attr::VARIABLE));
                    param_info->attributes.set(static_cast<size_t>(attr::LVAL));
                    param_info->sequence = sequence_count;
                    param_info->block_nr = next_block - 1;
                    param->children[0]->attributes = param_info->attributes;
                }
                sequence_count++;
                func_attr_info->parameters->push_back(param_info);
                print_childsymboltable(param_info, param_name, outfile);
                nodesb_table.emplace(param, param_info);
            }
            
            if (global_names->find(func_name) != global_names->end()
                && global_names->find(func_name)->second->attributes[static_cast<size_t>(attr::FUNCTION)]) {
                    symbol* prefunction = global_names->find(func_name)->second;
                    vector<symbol*>* preparams = prefunction->parameters;
                    if (preparams->size() == func_attr_info->parameters->size()) {
                        for (size_t i = 0; i < preparams->size(); ++i) {
                            if (compatible_check(preparams->at(i), func_attr_info->parameters->at(i))) continue;
                            else {
                                fprintf(stderr, "(%zd.%zd.%zd) The parameters mismatched with the previous declaration\n", 
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                                return false;
                            }
                        }
                    }
                    else {
                        fprintf(stderr, "(%zd.%zd.%zd) The parameters mismatched with the previous declaration\n", 
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                        return false;
                    }
                }
            
            global_names->emplace(func_name, func_attr_info);

            func_attr_info->lloc = node->lloc;
            break;
        }

        case TOK_PROTO : {
            astree* func_type_node = node->children[0];
            const string* func_name;
            symbol* func_attr_info = new symbol();
            if (func_type_node->symbol == TOK_ARRAY) {
                func_attr_info->setAttr(func_type_node->children[0]);
                nodeSymbol->attributes = func_attr_info->attributes;
                func_name = func_type_node->children[1]->lexinfo;
                func_attr_info->attributes.set(static_cast<size_t>(attr::ARRAY));
                nodesb_table.emplace(func_type_node->children[0], func_attr_info);
                nodesb_table.emplace(func_type_node->children[1], func_attr_info);
                nodesb_table.emplace(func_type_node, func_attr_info);
                func_attr_info->attributes.set(static_cast<size_t>(attr::FUNCTION));
                func_attr_info->attributes[static_cast<size_t>(attr::VARIABLE)] = 0;
                func_type_node->children[1]->attributes = func_attr_info->attributes;
            }
            else if (func_type_node->symbol == TOK_TYPEID) {
                func_attr_info->setAttr(func_type_node);
                if (global_names->find(func_type_node->lexinfo) == global_names->end()) {
                    fprintf(stderr, "(%zd.%zd.%zd) This type has not been declared!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                }
                func_attr_info->struct_name = global_names->find(func_type_node->lexinfo)->second->struct_name;
                func_attr_info->fields = global_names->find(func_type_node->lexinfo)->second->fields; 
                func_name = func_type_node->children[0]->lexinfo;
                nodesb_table.emplace(func_type_node->children[0], func_attr_info);
                nodesb_table.emplace(func_type_node, func_attr_info);
                func_attr_info->attributes.set(static_cast<size_t>(attr::FUNCTION));
                func_attr_info->attributes[static_cast<size_t>(attr::VARIABLE)] = 0;
                func_type_node->children[0]->attributes = func_attr_info->attributes;
                func_type_node->children[0]->struct_name = func_attr_info->struct_name;
            }
            else {
                func_attr_info->setAttr(func_type_node);
                nodeSymbol->attributes = func_attr_info->attributes;
                func_name = func_type_node->children[0]->lexinfo;
                nodesb_table.emplace(func_type_node->children[0], func_attr_info);
                nodesb_table.emplace(func_type_node, func_attr_info);
                func_attr_info->attributes.set(static_cast<size_t>(attr::FUNCTION));
                func_attr_info->attributes[static_cast<size_t>(attr::VARIABLE)] = 0;
                func_type_node->children[0]->attributes = func_attr_info->attributes;
            }
            print_parentsymboltable(func_attr_info, func_name, outfile);

            if (global_names->find(func_name) != global_names->end()
                && global_names->find(func_name)->second->attributes[static_cast<size_t>(attr::FUNCTION)]) {
                    fprintf(stderr, "(%zd.%zd.%zd) The function has already been declared!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                }

            next_block++;
            symbol_table* localSymTable = new symbol_table();
            symbol_table_stack.push_back(localSymTable);

            astree* func_params = node->children[1];
            
            int sequence_count = 0;
            func_attr_info->parameters = new vector<symbol*>();
            for (size_t i = 0; i < func_params->children.size(); ++i) {
                astree * param = func_params->children[i];
                symbol* param_info = new symbol();
                const string* param_name;
                if (param->symbol == TOK_ARRAY) {
                    param_info->setAttr(param->children[0]);
                    localSymTable->emplace(param->children[1]->lexinfo, param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::ARRAY));
                    param_name = param->children[1]->lexinfo;
                    nodesb_table.emplace(param->children[0], param_info);
                    nodesb_table.emplace(param->children[1], param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::PARAM));
                    param_info->attributes.set(static_cast<size_t>(attr::VARIABLE));
                    param_info->attributes.set(static_cast<size_t>(attr::LVAL));
                    param_info->sequence = sequence_count;
                    param_info->block_nr = next_block - 1;
                    param->children[1]->attributes = param_info->attributes;
                }
                else if (param->symbol == TOK_TYPEID) {
                    param_info->setAttr(param);
                    if (global_names->find(param->lexinfo) == global_names->end()) {
                        fprintf(stderr, "(%zd.%zd.%zd) This type has not been declared!\n", 
                                node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    }
                    param_info->struct_name = global_names->find(param->lexinfo)->second->struct_name;
                    param_info->fields = global_names->find(param->lexinfo)->second->fields; 
                    param_name = param->children[0]->lexinfo;
                    nodesb_table.emplace(param->children[0], param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::PARAM));
                    param_info->attributes.set(static_cast<size_t>(attr::VARIABLE));
                    param_info->attributes.set(static_cast<size_t>(attr::LVAL));
                    param_info->sequence = sequence_count;
                    param_info->block_nr = next_block - 1;
                    param->children[0]->attributes = param_info->attributes;
                    param->children[0]->struct_name = param_info->struct_name;
                }
                else {
                    param_info->setAttr(param);
                    localSymTable->emplace(param->children[0]->lexinfo, param_info);
                    param_name = param->children[0]->lexinfo;
                    nodesb_table.emplace(param->children[0], param_info);
                    param_info->attributes.set(static_cast<size_t>(attr::PARAM));
                    param_info->attributes.set(static_cast<size_t>(attr::VARIABLE));
                    param_info->attributes.set(static_cast<size_t>(attr::LVAL));
                    param_info->sequence = sequence_count;
                    param_info->block_nr = next_block - 1;
                    param->children[0]->attributes = param_info->attributes;
                }
                sequence_count++;
                func_attr_info->parameters->push_back(param_info);
                print_childsymboltable(param_info, param_name, outfile);
                nodesb_table.emplace(param, param_info);
            }

            global_names->emplace(func_name, func_attr_info);
            func_attr_info->lloc = node->lloc;
            break;
        }

        case TOK_KW_STRUCT : {

            const string* structName = node->children[0]->lexinfo;
            
            symbol* struct_attr_info = new symbol();
            struct_attr_info->setAttr(node->children[0]);
            print_parentsymboltable(struct_attr_info, structName, outfile);
            nodesb_table.emplace(node, struct_attr_info);
            node->children[0]->attributes = struct_attr_info->attributes;
            node->children[0]->struct_name = structName;

            if (node->children.size() > 1) {
                if (global_names->find(structName) != global_names->end() 
                && global_names->find(structName)->second != nullptr 
                && global_names->find(structName)->second->attributes[static_cast<size_t>(attr::STRUCT)] == 1) {
                    fprintf(stderr, "(%zd.%zd.%zd) This Structure has been declared!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                }
                
                astree* struct_fields = node->children[1];
                struct_attr_info->fields = new symbol_table();
                int sequence_count = 0;
                for (size_t i = 0; i < struct_fields->children.size(); ++i) {
                    astree* field = struct_fields->children[i];
                    symbol* field_info = new symbol();
                    const string* field_name;
                    if (field->symbol == TOK_ARRAY) {
                        field_info->setAttr(field->children[0]);
                        field_info->attributes.set(static_cast<size_t>(attr::ARRAY));
                        struct_attr_info->fields->emplace(field->children[1]->lexinfo, field_info);
                        field_name = field->children[1]->lexinfo;
                        nodesb_table.emplace(field->children[0], field_info);
                        nodesb_table.emplace(field->children[1], field_info);
                        field_info->attributes.set(static_cast<size_t>(attr::FIELD));
                        field->children[1]->attributes = field_info->attributes;
                    }
                    else if (field->symbol == TOK_TYPEID) {
                        field_info->setAttr(field);
                        if (global_names->find(field->lexinfo) == global_names->end()) {
                            fprintf(stderr, "(%zd.%zd.%zd) This type has not been declared!\n", 
                                    node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                        }
                        if (global_names->find(field->lexinfo)->second != nullptr) {
                            field_info->struct_name = global_names->find(field->lexinfo)->second->struct_name;
                            field_info->fields = global_names->find(field->lexinfo)->second->fields; 
                        }
                        struct_attr_info->fields->emplace(field->children[0]->lexinfo, field_info);
                        field_name = field->children[0]->lexinfo;
                        nodesb_table.emplace(field->children[0], field_info);
                        field_info->attributes.set(static_cast<size_t>(attr::FIELD));
                        field->children[0]->attributes = field_info->attributes;
                        field->children[0]->struct_name = field_info->struct_name;
                    }
                    else {
                        field_info->setAttr(field);
                        struct_attr_info->fields->emplace(field->children[0]->lexinfo, field_info);
                        field_name = field->children[0]->lexinfo;
                        nodesb_table.emplace(field->children[0], field_info);
                        field_info->attributes.set(static_cast<size_t>(attr::FIELD));
                        field->children[0]->attributes = field_info->attributes;
                    }
                    field_info->sequence = sequence_count;
                    print_childsymboltable(field_info, field_name, outfile);
                    nodesb_table.emplace(field, field_info);
                }
            }

            if (global_names->find(structName) != global_names->end() 
                && global_names->find(structName)->second == nullptr ) {
                    global_names->find(structName)->second = struct_attr_info;
                }
            else if (global_names->find(structName) == global_names->end()){
                global_names->emplace(structName, struct_attr_info);
            }
            struct_attr_info->struct_name = structName;
            
            break;
        } 

        case TOK_BLOCK : {
            next_block++;
            symbol_table* localSymTable = new symbol_table();
            symbol_table_stack.push_back(localSymTable);
            break;
        }

        case TOK_VARDECL : {
            symbol_table* localSymTable = symbol_table_stack.back();

            astree* left = node->children[0];
            symbol* left_sym = new symbol();

            const string* var_name;

            if(left->symbol == TOK_ARRAY) {
                left_sym->setAttr(left->children[0]);
                left_sym->attributes.set(static_cast<size_t>(attr::ARRAY));
                var_name = left->children[1]->lexinfo;
                left_sym->attributes.set(static_cast<size_t>(attr::VARIABLE));
                left_sym->attributes.set(static_cast<size_t>(attr::LOCAL));
                left_sym->attributes.set(static_cast<size_t>(attr::LVAL));
                left->children[1]->attributes = left_sym->attributes;
                left->children[1]->block_nr = next_block - 1;
                nodesb_table.emplace(left->children[0], left_sym);
                nodesb_table.emplace(left->children[1], left_sym);
            }
            else if (left->symbol == TOK_TYPEID) {
                left_sym->setAttr(left);
                if (global_names->find(left->lexinfo) == global_names->end()) {
                    fprintf(stderr, "(%zd.%zd.%zd) This type has not been declared!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                }
                left_sym->struct_name = global_names->find(left->lexinfo)->second->struct_name;
                left_sym->fields = global_names->find(left->lexinfo)->second->fields; 
                var_name = left->children[0]->lexinfo;
                left_sym->attributes.set(static_cast<size_t>(attr::VARIABLE));
                left_sym->attributes.set(static_cast<size_t>(attr::LOCAL));
                left_sym->attributes.set(static_cast<size_t>(attr::LVAL));
                left->children[0]->attributes = left_sym->attributes;
                left->children[0]->struct_name = left_sym->struct_name;
                left->children[0]->block_nr = next_block - 1;
                nodesb_table.emplace(left->children[0], left_sym);
            }
            else {
                left_sym->setAttr(left);
                var_name = left->children[0]->lexinfo;
                left_sym->attributes.set(static_cast<size_t>(attr::VARIABLE));
                left_sym->attributes.set(static_cast<size_t>(attr::LOCAL));
                left_sym->attributes.set(static_cast<size_t>(attr::LVAL));
                left->children[0]->attributes = left_sym->attributes;
                left->children[0]->block_nr = next_block - 1;
                nodesb_table.emplace(left->children[0], left_sym);
            }

            if (localSymTable->find(var_name) != localSymTable->end()) {
                fprintf(stderr, "(%zd.%zd.%zd) This variable has been declared!\n", 
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                return false;
            }

            int sequence_count = 0;
            for (auto it = localSymTable->begin(); it != localSymTable->end(); ++it) {
                if (it->second->attributes[static_cast<size_t>(attr::PARAM)] == 0) {
                    ++sequence_count;
                }
            }

            left_sym->sequence = sequence_count;
            left_sym->block_nr = next_block - 1;
            if (next_block == 1) {
                left_sym->attributes[static_cast<size_t>(attr::LOCAL)] = 0;
                print_parentsymboltable(left_sym, var_name, outfile);
            }
            else print_childsymboltable(left_sym, var_name, outfile);
            nodesb_table.emplace(left, left_sym);
            localSymTable->emplace(var_name, left_sym);
            break;
        }

        case TOK_POINTER_GET : {
            if (nodesb_table.find(node) != nodesb_table.end()) return true;
            astree* left = node->children[0];
            astree* right = node->children[1];
            if (left->symbol == TOK_POINTER_GET) {
                astree* lleft = left->children[0];
                astree* lright = left->children[1];
                symbol* left_sym = new symbol();
                int blocknum = next_block - 1;
                int found = 0;
                symbol_table * localSymTable;
                symbol* struct_id = nullptr;
                while(blocknum > 0) {
                    localSymTable = symbol_table_stack[blocknum];
                    if (localSymTable->find(lleft->lexinfo) != localSymTable->end()) {
                        struct_id = localSymTable->find(lleft->lexinfo)->second;
                        found = 1;
                    }
                    if (func_map.find(blocknum) != func_map.end()) break;
                    --blocknum; 
                }
                if (found == 0) {
                    if (global_names->find(lleft->lexinfo) != global_names->end()) {
                        struct_id = global_names->find(lleft->lexinfo)->second;
                    }
                    else {
                        fprintf(stderr, "(%zd.%zd.%zd) The struct variable has not been declared! \n", 
                                    node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    }
                }

                if (struct_id->fields == nullptr || struct_id->fields->find(lright->lexinfo) == struct_id->fields->end()) {
                    fprintf(stderr, "(%zd.%zd.%zd) The Struct has no such field! \n",
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                } 

                left_sym->attributes = struct_id->fields->find(lright->lexinfo)->second->attributes;
                if (left_sym->attributes[static_cast<size_t>(attr::TYPEID)] == 1) {
                    left_sym->struct_name = struct_id->fields->find(lright->lexinfo)->second->struct_name;
                    left_sym->fields = struct_id->fields->find(lright->lexinfo)->second->fields;
                }
                left_sym->attributes[static_cast<size_t>(attr::FIELD)] = 0;
                left_sym->attributes[static_cast<size_t>(attr::VADDR)] = 1;
                left_sym->attributes[static_cast<size_t>(attr::LVAL)] = 1;
                nodesb_table.emplace(left, left_sym);
                nodesb_table.emplace(lleft, left_sym);
                nodesb_table.emplace(lright, left_sym);
                lleft->attributes = struct_id->attributes;
                lleft->struct_name = struct_id->struct_name;
                lright->attributes = left_sym->attributes;
                lright->struct_name = left_sym->struct_name;
                left->attributes = left_sym->attributes;
                left->struct_name = left_sym->struct_name;

                if (left_sym->fields == nullptr || left_sym->fields->find(right->lexinfo) == left_sym->fields->end()) {
                    fprintf(stderr, "(%zd.%zd.%zd) The Struct has no such field! \n",
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                } 
                nodeSymbol->attributes = left_sym->fields->find(right->lexinfo)->second->attributes;
                if (nodeSymbol->attributes[static_cast<size_t>(attr::TYPEID)] == 1) {
                    nodeSymbol->struct_name = left_sym->fields->find(right->lexinfo)->second->struct_name;
                    nodeSymbol->fields = left_sym->fields->find(right->lexinfo)->second->fields;
                }
                nodeSymbol->attributes[static_cast<size_t>(attr::FIELD)] = 0;
                nodeSymbol->attributes[static_cast<size_t>(attr::VADDR)] = 1;
                nodeSymbol->attributes[static_cast<size_t>(attr::LVAL)] = 1;
                nodesb_table.emplace(node, nodeSymbol);
                nodesb_table.emplace(left, nodeSymbol);
                nodesb_table.emplace(right, nodeSymbol);
                right->attributes = nodeSymbol->attributes;
                right->struct_name = nodeSymbol->struct_name;
            }
            else {
                int blocknum = next_block - 1;
                int found = 0;
                symbol_table * localSymTable;
                symbol* struct_id = nullptr;
                while(blocknum > 0) {
                    localSymTable = symbol_table_stack[blocknum];
                    if (localSymTable->find(left->lexinfo) != localSymTable->end()) {
                        struct_id = localSymTable->find(left->lexinfo)->second;
                        found = 1;
                    }
                    if (func_map.find(blocknum) != func_map.end()) break;
                    --blocknum; 
                }
                if (found == 0) {
                    if (global_names->find(left->lexinfo) != global_names->end()) {
                        struct_id = global_names->find(left->lexinfo)->second;
                    }
                    else {
                        fprintf(stderr, "(%zd.%zd.%zd) The struct variable has not been declared! \n", 
                                    node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    }
                }

                if (struct_id->fields == nullptr || struct_id->fields->find(right->lexinfo) == struct_id->fields->end()) {
                    fprintf(stderr, "(%zd.%zd.%zd) The Struct has no such field! \n",
                            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                    return false;
                } 

                nodeSymbol->attributes = struct_id->fields->find(right->lexinfo)->second->attributes;
                if (nodeSymbol->attributes[static_cast<size_t>(attr::TYPEID)] == 1 
                    && struct_id->fields->find(right->lexinfo)->second != nullptr) {
                    nodeSymbol->struct_name = struct_id->fields->find(right->lexinfo)->second->struct_name;
                }
                nodeSymbol->attributes[static_cast<size_t>(attr::FIELD)] = 0;
                nodeSymbol->attributes[static_cast<size_t>(attr::VADDR)] = 1;
                nodeSymbol->attributes[static_cast<size_t>(attr::LVAL)] = 1;
                nodesb_table.emplace(node, nodeSymbol);
                nodesb_table.emplace(left, nodeSymbol);
                nodesb_table.emplace(right, nodeSymbol);
                left->attributes = struct_id->attributes;
                left->struct_name = struct_id->struct_name;
                right->attributes = nodeSymbol->attributes;
                right->struct_name = nodeSymbol->struct_name;
            }
            break;
        }

        default : break;    
    }

    for (size_t i = 0; i < node->children.size(); ++i) {
        bool check_true = ASTTraversal(node->children[i], outfile);
        if (!check_true) return false;
    }

    bool type_check_result = ASTtypecheck(node);

    if (!type_check_result) return false;

    if (node->symbol == TOK_FUNCTION || node->symbol == TOK_PROTO) {
            fprintf(outfile, "\n");
            symbol_table_stack.pop_back();
            next_block = symbol_table_stack.size();
    }
    if (node->symbol == TOK_BLOCK) {
        symbol_table_stack.pop_back();
        next_block = symbol_table_stack.size();
    }

    return true;
}
