#include "intermlang.h"

using var_table = unordered_map<const string*, string>; 
unordered_map<const string*, var_table*> struct_table;
int nextblock = 1;
var_table* global_var = new var_table();
vector<var_table*> var_table_stack(1, global_var);
map<int, var_table*> _func_map;
unordered_set<astree*> traversedBlock;
unordered_map<astree*, string> traversednode;
unordered_set<astree*> node_traversed;
unordered_set<astree*> signinstat;

int reg_num = 1;

FILE* outfile = nullptr;

string find_ident_name(astree* node) {
    int blocknum = nextblock - 1;
    var_table* func_table = nullptr;
    string var_name;
    while(blocknum > 0) {
        if (_func_map.find(blocknum) != _func_map.end()) {
            func_table = _func_map.find(blocknum)->second;
            if (func_table->find(node->lexinfo) == func_table->end()) {
                func_table = nullptr;
                break;
            }
        }
        func_table = var_table_stack[blocknum];
        if (func_table->find(node->lexinfo) == func_table->end()) {
            blocknum--;
        }
        else break;
    }
    if (func_table == nullptr) {
        if (global_var->find(node->lexinfo) == global_var->end()) {
            fprintf(stderr, "ERROR, Can't find variable!");
            return "";
        }
        else {
            var_name = global_var->find(node->lexinfo)->second;
        }
    }
    else {
        var_name = func_table->find(node->lexinfo)->second;
    }
    return var_name;
}

string format_local_var(astree* node) {
    string var_name = "_";
    var_name += to_string(node->block_nr);
    var_name += "_";
    var_name += *(node->lexinfo);
    return var_name;
}

string format_reg(astree* node) {
    string regt = "";
    if (node->attributes[static_cast<size_t>(attr::ARRAY)] == 1) {
        regt += "a";
    }
    else if (node->attributes[static_cast<size_t>(attr::INT)] == 1) {
        regt += "i";
    }
    else if (node->attributes[static_cast<size_t>(attr::STRING)] == 1) {
        regt += "s";
    }
    else if (node->attributes[static_cast<size_t>(attr::TYPEID)] == 1) {
        regt += "p";
    }

    regt += to_string(reg_num);
    reg_num++;
    return regt;
}

string format_pointer_get(astree* node) {
    string format = "";
    string struct_name = find_ident_name(node->children[0]);
    var_table* field_table = struct_table.find(node->children[0]->struct_name)->second;
    string field_name = field_table->find(node->children[1]->lexinfo)->second;
    format += struct_name;
    format += "->";
    format += field_name;
    return format;
}

string format_new(astree* node) {
    string format = "";
    if (node->children.size() == 1) {
        format += "xcalloc(1, sizeof (struct ";
        format += *(node->children[0]->lexinfo);
        format += "))";
    }
    else if (node->children.size() == 2) {
        format += "xcalloc(";
        astree* opnd = node->children[1];
        if (opnd->attributes[static_cast<size_t>(attr::CONST)] == 1) {
            format += *(opnd->lexinfo);
        }
        else {
            int blocknum = nextblock - 1;
            var_table* func_table = nullptr;
            string var_name;
            while(blocknum > 0) {
                if (_func_map.find(blocknum) != _func_map.end()) {
                    func_table = _func_map.find(blocknum)->second;
                    if (func_table->find(opnd->lexinfo) == func_table->end()) {
                        func_table = nullptr;
                        break;
                    }
                }
                func_table = var_table_stack[blocknum];
                if (func_table->find(opnd->lexinfo) == func_table->end()) {
                    blocknum--;
                }
                else break;
            }
            if (func_table == nullptr) {
                var_name = global_var->find(opnd->lexinfo)->second;
            }
            else {
                var_name = func_table->find(opnd->lexinfo)->second;
            }
            format += var_name;
        }
        format += ", sizeof (char))";
    }
    else if (node->children.size() == 3) {
        format += "xcalloc(";
        astree* opnd = node->children[2];
        if (opnd->attributes[static_cast<size_t>(attr::CONST)] == 1) {
            format += *(opnd->lexinfo);
        }
        else {
            int blocknum = nextblock - 1;
            var_table* func_table = nullptr;
            string var_name;
            while(blocknum > 0) {
                if (_func_map.find(blocknum) != _func_map.end()) {
                    func_table = _func_map.find(blocknum)->second;
                    if (func_table->find(opnd->lexinfo) == func_table->end()) {
                        func_table = nullptr;
                        break;
                    }
                }
                func_table = var_table_stack[blocknum];
                if (func_table->find(opnd->lexinfo) == func_table->end()) {
                    blocknum--;
                }
                else break;
            }
            if (func_table == nullptr) {
                var_name = global_var->find(opnd->lexinfo)->second;
            }
            else {
                var_name = func_table->find(opnd->lexinfo)->second;
            }
            format += var_name;
        }

        format += ", sizeof (";

        astree* basetype = node->children[0];
        string type;
        if (basetype->symbol == TOK_TYPEID) {
            type += "struct ";
            type += *(basetype->lexinfo);
            type += "*";
        }
        else if (basetype->symbol == TOK_KW_STRING){
            type += "char*";
        }
        else {
            type += *(basetype->lexinfo);
        }

        format += type;
        format += "))";
    }
    return format;
}

string format_field_list(astree* node) {
    string list = "";
    if (node->attributes[static_cast<size_t>(attr::STRUCT)]) {
        list += *(node->struct_name);
        list += "_pointer";
    }
    if (node->attributes[static_cast<size_t>(attr::INT)]) {
        list += "_int";
    }
    if (node->attributes[static_cast<size_t>(attr::STRING)]) {
        list += "_string";
    }
    if (node->attributes[static_cast<size_t>(attr::ARRAY)]) {
        list += "_array";
    }
    list += "_";
    list += *(node->lexinfo);
    return list;
}

void print_struct(astree* node) {
    const string* struct_name = node->children[0]->lexinfo;
    if (node->children.size() == 1) {
        struct_table.emplace(struct_name, nullptr);
        fprintf(outfile, "struct %s ;\n", (*struct_name).c_str());
    }
    else if (node->children.size() == 2) {
        var_table* field_table = new var_table();
        struct_table.emplace(struct_name, field_table);
        fprintf(outfile, "struct %s { \n", (*struct_name).c_str());
        astree* struct_fields = node->children[1];
        for (size_t i = 0; i < struct_fields->children.size(); ++i) {
            astree* field =  struct_fields->children[i];
            if (field->symbol == TOK_ARRAY) {
                if (field->children[0]->symbol == TOK_TYPEID) {
                    fprintf(outfile, "\tstruct %s*", (*(field->children[0]->lexinfo)).c_str());
                }
                else if (field->children[0]->symbol == TOK_KW_STRING){
                    fprintf(outfile, "\tchar*");
                }
                else {
                    fprintf(outfile, "\t%s", (*(field->children[0]->lexinfo)).c_str());
                }
                fprintf(outfile, "* ");
                
                string list = format_field_list(field->children[1]);
                list = *(struct_name) + "_" + list;
                field_table->emplace(field->children[1]->lexinfo, list);
                fprintf(outfile, "%s;\n", list.c_str());
            }
            else if (field->symbol == TOK_TYPEID) {
                fprintf(outfile, "\tstruct %s * ", (*(field->lexinfo)).c_str());
                string list = format_field_list(field->children[0]);
                list = *(struct_name) + "_" + list;
                field_table->emplace(field->children[0]->lexinfo, list);
                fprintf(outfile, "%s;\n", list.c_str());
            }
            else if (field->symbol == TOK_KW_STRING) {
                fprintf(outfile, "\tchar* ");
                string list = format_field_list(field->children[0]);
                list = *(struct_name) + "_" + list;
                field_table->emplace(field->children[0]->lexinfo, list);
                fprintf(outfile, "%s;\n", list.c_str());
            }
            else {
                fprintf(outfile, "\t%s ", (*(field->lexinfo)).c_str());
                string list = format_field_list(field->children[0]);
                list = *(struct_name) + "_" + list;
                field_table->emplace(field->children[0]->lexinfo, list);
                fprintf(outfile, "%s;\n", list.c_str());
            }
        }
        fprintf(outfile, "};\n\n");
    }
}

void print_function(astree* node) {
    astree* func_type_node = node->children[0];
    if (func_type_node->symbol == TOK_ARRAY) {
        if (func_type_node->children[0]->symbol == TOK_TYPEID) {
            fprintf(outfile, "struct %s*", (*(func_type_node->children[0]->lexinfo)).c_str());
        }
        else if (func_type_node->children[0]->symbol == TOK_KW_STRING){
            fprintf(outfile, "char*");
        }
        else {
            fprintf(outfile, "%s", (*(func_type_node->children[0]->lexinfo)).c_str());
        }
        fprintf(outfile, "* ");
        fprintf(outfile, "%s (\n", (*(func_type_node->children[1]->lexinfo)).c_str());
    }
    else if (func_type_node->symbol == TOK_TYPEID) {
        fprintf(outfile, "struct %s* ", (*(func_type_node->lexinfo)).c_str());
        fprintf(outfile, "%s (\n", (*(func_type_node->children[0]->lexinfo)).c_str());
    }
    else if (func_type_node->symbol == TOK_KW_STRING) {
        fprintf(outfile, "char* ");
        fprintf(outfile, "%s (\n", (*(func_type_node->children[0]->lexinfo)).c_str());
    }
    else {
        fprintf(outfile, "%s ", (*(func_type_node->lexinfo)).c_str());
        fprintf(outfile, "%s (\n", (*(func_type_node->children[0]->lexinfo)).c_str());
    }

    astree* func_params = node->children[1];
    var_table* localVarTable = new var_table();
    ++nextblock;
    var_table_stack.push_back(localVarTable);

    for (size_t i = 0; i < func_params->children.size(); ++i) {
        astree* param = func_params->children[i];
        if (i < func_params->children.size() - 1) {
            if (param->symbol == TOK_ARRAY) {
                if (param->children[0]->symbol == TOK_TYPEID) {
                    fprintf(outfile, "\tstruct %s*", (*(param->children[0]->lexinfo)).c_str());
                }
                else if (param->children[0]->symbol == TOK_KW_STRING){
                    fprintf(outfile, "\tchar*");
                }
                else {
                    fprintf(outfile, "\t%s", (*(param->children[0]->lexinfo)).c_str());
                }
                fprintf(outfile, "* ");
                string var_name = format_local_var(param->children[1]);
                fprintf(outfile, "%s,\n", var_name.c_str());
                localVarTable->emplace(param->children[1]->lexinfo, var_name);
            }
            else if (param->symbol == TOK_TYPEID) {
                fprintf(outfile, "\tstruct %s* ", (*(param->lexinfo)).c_str());
                string var_name = format_local_var(param->children[0]);
                fprintf(outfile, "%s,\n", var_name.c_str());
                localVarTable->emplace(param->children[0]->lexinfo, var_name);
            }
            else if (func_type_node->symbol == TOK_KW_STRING) {
                fprintf(outfile, "\tchar* ");
                string var_name = format_local_var(param->children[0]);
                fprintf(outfile, "%s,\n", var_name.c_str());
                localVarTable->emplace(param->children[0]->lexinfo, var_name);
            }
            else {
                fprintf(outfile, "\t%s ", (*(param->lexinfo)).c_str());
                string var_name = format_local_var(param->children[0]);
                fprintf(outfile, "%s,\n", var_name.c_str());
                localVarTable->emplace(param->children[0]->lexinfo, var_name);
            }
        }
        else if (i == func_params->children.size() - 1) {
            if (param->symbol == TOK_ARRAY) {
                if (param->children[0]->symbol == TOK_TYPEID) {
                    fprintf(outfile, "\tstruct %s*", (*(param->children[0]->lexinfo)).c_str());
                }
                else if (param->children[0]->symbol == TOK_KW_STRING){
                    fprintf(outfile, "\tchar*");
                }
                else {
                    fprintf(outfile, "\t%s", (*(param->children[0]->lexinfo)).c_str());
                }
                fprintf(outfile, "* ");
                string var_name = format_local_var(param->children[1]);
                fprintf(outfile, "%s\n", var_name.c_str());
                localVarTable->emplace(param->children[1]->lexinfo, var_name);
            }
            else if (param->symbol == TOK_TYPEID) {
                fprintf(outfile, "\tstruct %s* ", (*(param->lexinfo)).c_str());
                string var_name = format_local_var(param->children[0]);
                fprintf(outfile, "%s\n", var_name.c_str());
                localVarTable->emplace(param->children[0]->lexinfo, var_name);
            }
            else if (func_type_node->symbol == TOK_KW_STRING) {
                fprintf(outfile, "\tchar* ");
                string var_name = format_local_var(param->children[0]);
                fprintf(outfile, "%s\n", var_name.c_str());
                localVarTable->emplace(param->children[0]->lexinfo, var_name);
            }
            else {
                fprintf(outfile, "\t%s ", (*(param->lexinfo)).c_str());
                string var_name = format_local_var(param->children[0]);
                fprintf(outfile, "%s\n", var_name.c_str());
                localVarTable->emplace(param->children[0]->lexinfo, var_name);
            }
        }
    }
    fprintf(outfile, "\t)");
    if (node->symbol == TOK_PROTO) fprintf(outfile, ";\n\n");
    else {
        fprintf(outfile, "\n");
        _func_map[nextblock - 1] = localVarTable;
    }
}

void traverse_block(astree* node) {

    if (node_traversed.find(node) != node_traversed.end()) return;
    if (node->symbol == TOK_BLOCK && traversedBlock.find(node) == traversedBlock.end()) return;

    for (size_t i = 0; i < node->children.size(); ++i) {
        traverse_block(node->children[i]);
    }

    switch(node->symbol) {
        case TOK_VARDECL : {
            var_table* local_table = var_table_stack.back();
            astree* left = node->children[0];
            astree* right = node->children[1];

            string format = "";
            
            if (left->symbol == TOK_ARRAY) {
                if (left->children[0]->symbol == TOK_TYPEID) {
                    format += "struct ";
                    format += *(left->children[0]->lexinfo);
                    format += "*";
                }
                else if (left->children[0]->symbol == TOK_KW_STRING){
                    format += "char*";
                }
                else {
                    format += *(left->children[0]->lexinfo);
                }
                format += "* "; 
                string reg_name = format_reg(left->children[1]);
                format += reg_name;
                local_table->emplace(left->children[1]->lexinfo, reg_name);
            }
            else if (left->symbol == TOK_TYPEID) {
                format += "struct ";
                format += *(left->lexinfo);
                format += "* ";
                string reg_name = format_reg(left->children[0]);
                format += reg_name;
                local_table->emplace(left->children[0]->lexinfo, reg_name);
            }
            else if (left->symbol == TOK_KW_STRING) {
                format += "char* ";
                string reg_name = format_reg(left->children[0]);
                format += reg_name; 
                local_table->emplace(left->children[0]->lexinfo, reg_name);
            }
            else {
                format += *(left->lexinfo);
                format += " ";
                string reg_name = format_reg(left->children[0]);
                format += reg_name;
                local_table->emplace(left->children[0]->lexinfo, reg_name);
            }

            fprintf(outfile, "\t%s = ", format.c_str());

            if (right->children.size() == 0) {
                if (right->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    fprintf(outfile, "%s;\n", (*(right->lexinfo)).c_str());
                }
                else if (right->attributes[static_cast<size_t>(attr::NULLX)] == 1){
                    fprintf(outfile, "0;\n");
                }
                else {
                    string ident_name = find_ident_name(right);
                    if (left->symbol == TOK_ARRAY) {
                        fprintf(outfile, "&%s;\n", ident_name.c_str());
                    }
                    else {
                        fprintf(outfile, "%s;\n", ident_name.c_str());
                    }
                }
            }
            else {
                string right_part = traversednode.find(right)->second;
                fprintf(outfile, "%s;\n", right_part.c_str());
            }
            break;
        }
        case TOK_KW_NEW : {
            string format = format_new(node);
            traversednode.emplace(node, format);
            break;
        }
        case TOK_POINTER_GET : {
            astree* left = node->children[0];
            astree* right = node->children[1];
            
            string format = "";
            if (left->children.size() == 0 && right->children.size() == 0) {
                format = format_pointer_get(node);
                traversednode.emplace(node, format);
            }
            else if (left->children.size() > 0) {
                format = traversednode.find(left)->second;
                format += "->";
                var_table* field_table = struct_table.find(left->struct_name)->second;
                string field = field_table->find(right->lexinfo)->second;
                format += field;
                traversednode.emplace(node, format);
            }
            break;
        }
        case TOK_INDEX : {
            astree* left = node->children[0];
            astree* right = node->children[1];
            string format = "";

            string leftformat = "";

            if (left->children.size() == 0) {
                leftformat = find_ident_name(left);
            }
            else {
                leftformat = traversednode.find(left)->second;
            }

            format += leftformat;
            format += "[";
            if (right->children.size() == 0) {
                if (right->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    format += *(right->lexinfo);
                }
                else {
                    string rightformat = find_ident_name(right);
                    format += rightformat;
                }
            }
            else {
                string rightformat = traversednode.find(right)->second;
                format += rightformat;
            }
            format += "]";
            traversednode.emplace(node, format);
            break;
        }
        case TOK_CALL : {
            astree* name = node->children[0];
            astree* params = node->children[1];
            if (node->attributes[static_cast<size_t>(attr::VOID)] == 1) {
                fprintf(outfile, "\t%s (", (*(name->lexinfo)).c_str());
                for (size_t i = 0; i < params->children.size(); ++i) {
                    astree* param = params->children[i];
                    if (i == 0) {
                        if(*(param->lexinfo) == "__FUNC__") {
                            fprintf(outfile, "%s", (*(param->lexinfo)).c_str());
                        }
                        else if (param->children.size() == 0) {
                            if (param->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                                fprintf(outfile, "%s", (*(param->lexinfo)).c_str());
                            }
                            else {
                                string rightformat = find_ident_name(param);
                                fprintf(outfile, "%s", rightformat.c_str());
                            }
                        }
                        else {
                            string format = traversednode.find(param)->second;
                            fprintf(outfile, "%s", format.c_str());
                        }
                    }
                    else {
                        if(*(param->lexinfo) == "__FUNC__") {
                            fprintf(outfile, ", %s", (*(param->lexinfo)).c_str());
                        }
                        else if (param->children.size() == 0) {
                            if (param->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                                fprintf(outfile, ", %s", (*(param->lexinfo)).c_str());
                            }
                            else {
                                string rightformat = find_ident_name(param);
                                fprintf(outfile, ", %s", rightformat.c_str());
                            }
                        }
                        else {
                            string format = traversednode.find(param)->second;
                            fprintf(outfile, ", %s", format.c_str());
                        }
                    }
                }
                fprintf(outfile, ");\n");
            }
            else {
                string format = "";
                format += *(name->lexinfo);
                format += " (";
                for (size_t i = 0; i < params->children.size(); ++i) {
                    astree* param = params->children[i];
                    if (i == 0) {
                        if (param->children.size() == 0) {
                            if (param->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                                format += *(param->lexinfo);
                            }
                            else {
                                string rightformat = find_ident_name(param);
                                format += rightformat;
                            }
                        }
                        else {
                            string rightformat = traversednode.find(param)->second;
                            format += rightformat;
                        }
                    }
                    else {
                        if (param->children.size() == 0) {
                            if (param->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                                format += ", ";
                                format += *(param->lexinfo);
                            }
                            else {
                                string rightformat = find_ident_name(param);
                                format += ", ";
                                format += rightformat;
                            }
                        }
                        else {
                            string rightformat = traversednode.find(param)->second;
                            format += ", ";
                            format += rightformat;
                        }
                    }
                }
                format += ")";

                string regt = format_reg(node);
                string leftformat = "";
                if (node->symbol == TOK_ARRAY) {
                    if (node->children[0]->symbol == TOK_TYPEID) {
                        leftformat += "struct ";
                        leftformat += *(node->struct_name);
                        leftformat += "*";
                    }
                    else if (node->children[0]->symbol == TOK_KW_STRING){
                        leftformat += "char*";
                    }
                    else if (node->attributes[static_cast<size_t>(attr::INT)] == 1){
                        leftformat += "int ";
                    }
                    leftformat += "* "; 
                    leftformat += regt;
                }
                else if (node->symbol == TOK_TYPEID) {
                    leftformat += "struct ";
                    leftformat += *(node->struct_name);
                    leftformat += "* ";
                    leftformat += regt;
                }
                else if (node->symbol == TOK_KW_STRING) {
                    leftformat += "char* ";
                    leftformat += regt;
                }
                else if (node->attributes[static_cast<size_t>(attr::INT)] == 1) {
                    leftformat += "int ";
                    leftformat += regt;
                }

                fprintf(outfile, "\t%s = %s;\n", leftformat.c_str(), format.c_str());

                traversednode.emplace(node, regt);
            }
            break; 
        }
        case '!' : 
        case TOK_KW_NOT : {
            string format = "";
            format += "!";
            if (node->children[0]->children.size() == 0) {
                string var_name = find_ident_name(node->children[0]);
                format += var_name;
            }
            else {
                string var_name = traversednode.find(node->children[0])->second;
                format += var_name;
            }
            traversednode.emplace(node, format);
            break;
        }
        case TOK_NEG :
        case TOK_POS : {
            string format = "";
            if (node->symbol == TOK_NEG) {
                format += "-";
            }
            else if (node->symbol == TOK_POS) {
                format += "+";
            }

            if (node->children[0]->children.size() == 0) {
                if (node->children[0]->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    format += *(node->children[0]->lexinfo);
                }
                else {
                    string var_name = find_ident_name(node->children[0]);
                    format += var_name;
                }
            }
            else {
                string var_name = traversednode.find(node->children[0])->second;
                format += var_name;
            }
            traversednode.emplace(node, format);
            break;
        }

        case '+' :
        case '-' :
        case '*' :
        case '/' :
        case '%' :
        case '>' :
        case '<' :
        case TOK_DOUBLE_EQ :
        case TOK_NOT_EQ :
        case TOK_LESS_TH_EQ :
        case TOK_LARGE_TH_EQ: {
            astree* left = node->children[0];
            astree* right = node->children[1];
            string format = "";

            if (left->children.size() == 0) {
                if (left->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    format += *(left->lexinfo);
                }
                else {
                    string var_name = find_ident_name(left);
                    format += var_name;
                }
            }
            else {
                string name = traversednode.find(left)->second;
                format += name;
            }

            format += " ";
            format += *(node->lexinfo);
            format += " ";

            if (right->children.size() == 0) {
                if (right->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    format += *(right->lexinfo);
                }
                else if (right->attributes[static_cast<size_t>(attr::NULLX)] == 1) {
                    format += "NULL";
                }
                else {
                    string var_name = find_ident_name(right);
                    format += var_name;
                }
            }
            else {
                string name = traversednode.find(right)->second;
                format += name;
            }
            traversednode.emplace(node, format);
            break;
        }

        case '=' : {
            astree* left = node->children[0];
            astree* right = node->children[1];
            string leftformat = "";
            string rightformat = "";

            if (left->children.size() == 0) {
                if (left->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    leftformat += *(left->lexinfo);
                }
                else {
                    string var_name = find_ident_name(left);
                    leftformat += var_name;
                }
            }
            else {
                string name = traversednode.find(left)->second;
                leftformat += name;
            }

            if (right->children.size() == 0) {
                if (right->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    rightformat += *(right->lexinfo);
                }
                else if (right->attributes[static_cast<size_t>(attr::NULLX)] == 1) {
                    rightformat += "NULL";
                }
                else {
                    string var_name = find_ident_name(right);
                    rightformat += var_name;
                }
            }
            else {
                string name = traversednode.find(right)->second;
                rightformat += name;
            }
            fprintf(outfile, "\t%s = %s;\n", leftformat.c_str(), rightformat.c_str());
            break;
        }

        case TOK_KW_RETURN : {
            if (node->children.size() == 0) {
                fprintf(outfile, "\treturn;\n");
            }
            else {
                astree* return_type = node->children[0];
                string format = "";
                if (return_type->children.size() == 0) {
                    if (return_type->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                        format += *(return_type->lexinfo);
                    }
                    else {
                        string var_name = find_ident_name(return_type);
                        format += var_name;
                    }
                }
                else {
                    string name = traversednode.find(return_type)->second;
                    format += name;
                }
                fprintf(outfile, "\treturn %s;\n", format.c_str());
            }
            break;
        }

        case TOK_KW_WHILE : {
            string coordinate = "";
            coordinate += "_";
            coordinate += to_string(node->lloc.filenr);
            coordinate += "_";
            coordinate += to_string(node->lloc.linenr);
            coordinate += "_";
            coordinate += to_string(node->lloc.offset);
            string whilename = "while";
            whilename += coordinate;
            string breakname = "break";
            breakname += coordinate;

            fprintf(outfile, "%s:;\n", whilename.c_str());

            string opnd = "i";
            opnd += to_string(reg_num);
            reg_num++;

            astree* opnd_type = node->children[0];
            string real_opnd = "";
            if (opnd_type->children.size() == 0) {
                if (opnd_type->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    real_opnd += *(opnd_type->lexinfo);
                }
                else {
                    string var_name = find_ident_name(opnd_type);
                    real_opnd += var_name;
                }
            }
            else {
                string name = traversednode.find(opnd_type)->second;
                real_opnd += name;
            }
            
            fprintf(outfile, "\tchar %s = %s;\n", opnd.c_str(), real_opnd.c_str());

            fprintf(outfile, "\tif(!%s) goto %s;\n", opnd.c_str(), breakname.c_str());

            if (node->children[1]->symbol != TOK_BLOCK) {
                traverse_block(node->children[1]);
            }
            else {
                nextblock++;
                var_table* newtable = new var_table();
                var_table_stack.push_back(newtable);
                traversedBlock.emplace(node->children[1]);
                traverse_block(node->children[1]);
                var_table_stack.pop_back();
                nextblock = var_table_stack.size();
            }

            fprintf(outfile, "\tgoto %s;\n", whilename.c_str());

            fprintf(outfile, "%s:;\n", breakname.c_str());
            break;
        }

        case TOK_KW_IF : {
            string coordinate = "";
            coordinate += "_";
            coordinate += to_string(node->lloc.filenr);
            coordinate += "_";
            coordinate += to_string(node->lloc.linenr);
            coordinate += "_";
            coordinate += to_string(node->lloc.offset);
            string finame = "fi";
            finame += coordinate;
            string elsename = "else";
            elsename += coordinate;

            string opnd = "i";
            opnd += to_string(reg_num);
            reg_num++;

            astree* opnd_type = node->children[0];
            string real_opnd = "";
            if (opnd_type->children.size() == 0) {
                if (opnd_type->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                    real_opnd += *(opnd_type->lexinfo);
                }
                else if (opnd_type->attributes[static_cast<size_t>(attr::NULLX)] == 1) {
                    real_opnd += "NULL";
                }
                else {
                    string var_name = find_ident_name(opnd_type);
                    real_opnd += var_name;
                }
            }
            else {
                string name = traversednode.find(opnd_type)->second;
                real_opnd += name;
            }

            fprintf(outfile, "\tchar %s = %s;\n", opnd.c_str(), real_opnd.c_str());

            if (node->children.size() == 3) {
                fprintf(outfile, "\tif(!%s) goto %s;\n", opnd.c_str(), elsename.c_str());
                if (node->children[1]->symbol != TOK_BLOCK) {
                    traverse_block(node->children[1]);
                }
                else {
                    nextblock++;
                    var_table* newtable = new var_table();
                    var_table_stack.push_back(newtable);
                    traversedBlock.emplace(node->children[1]);
                    traverse_block(node->children[1]);
                    var_table_stack.pop_back();
                    nextblock = var_table_stack.size();
                }

                fprintf(outfile, "\tgoto %s;\n", finame.c_str());

                fprintf(outfile, "\t%s:;\n", elsename.c_str());

                astree* third_child = node->children[2]->children[0];

                if (third_child->symbol != TOK_BLOCK) {
                    traverse_block(third_child);
                }
                else {
                    nextblock++;
                    var_table* newtable = new var_table();
                    var_table_stack.push_back(newtable);
                    traversedBlock.emplace(third_child);
                    traverse_block(third_child);
                    var_table_stack.pop_back();
                    nextblock = var_table_stack.size();
                }

                fprintf(outfile, "\t%s:;\n", finame.c_str());
            }

            if (node->children.size() == 2) {
                fprintf(outfile, "\tif(!%s) goto %s;\n", opnd.c_str(), finame.c_str());
                if (node->children[1]->symbol != TOK_BLOCK) {
                    traverse_block(node->children[1]);
                }
                else {
                    nextblock++;
                    var_table* newtable = new var_table();
                    var_table_stack.push_back(newtable);
                    traversedBlock.emplace(node->children[1]);
                    traverse_block(node->children[1]);
                    var_table_stack.pop_back();
                    nextblock = var_table_stack.size();
                }

                fprintf(outfile, "\t%s:;\n", finame.c_str());
            }
            break;
        } 
        default: break;
    }
    node_traversed.emplace(node);
}

void genIntermlang(astree* node, FILE* out_file) {
    if (outfile == nullptr) {
        outfile = out_file;
        fprintf(outfile, "#define __OCLIB_C__\n");
        fprintf(outfile, "#include \"oclib.h\"\n\n");
    }

    switch(node->symbol) {
        case TOK_KW_STRUCT : {
            print_struct(node);
            break;
        }
        case TOK_FUNCTION : {
            print_function(node);
            break;
        }
        case TOK_PROTO : {
            print_function(node);
            break;
        }
        case TOK_BLOCK : {
            if (traversedBlock.find(node) == traversedBlock.end()) {
                nextblock++;
                var_table* newtable = new var_table();
                var_table_stack.push_back(newtable);
                fprintf(outfile, "\t{\n");
                traversedBlock.emplace(node);
                traverse_block(node);
                var_table_stack.pop_back();
                nextblock = var_table_stack.size();
                fprintf(outfile, "\t}\n");
            }
            break;
        }
        case TOK_VARDECL : {
            if (nextblock == 1) {
                var_table* globalTable = var_table_stack.back();
                astree* left = node->children[0];
                
                if (left->symbol == TOK_ARRAY) {
                    if (left->children[0]->symbol == TOK_TYPEID) {
                        fprintf(outfile, "struct %s*", (*(left->children[0]->lexinfo)).c_str());
                    }
                    else if (left->children[0]->symbol == TOK_KW_STRING){
                        fprintf(outfile, "char*");
                    }
                    else {
                        fprintf(outfile, "%s", (*(left->children[0]->lexinfo)).c_str());
                    }
                    fprintf(outfile, "* ");
                    string reg_name = format_reg(left->children[1]);
                    fprintf(outfile, "%s ", reg_name.c_str());
                    globalTable->emplace(left->children[1]->lexinfo, reg_name);
                }
                else if (left->symbol == TOK_TYPEID) {
                    fprintf(outfile, "struct %s* ", (*(left->lexinfo)).c_str());
                    string reg_name = format_reg(left->children[0]);
                    fprintf(outfile, "%s ", reg_name.c_str());
                    globalTable->emplace(left->children[0]->lexinfo, reg_name);
                }
                else if (left->symbol == TOK_KW_STRING) {
                    fprintf(outfile, "char* ");
                    string reg_name = format_reg(left->children[0]);
                    fprintf(outfile, "%s ", reg_name.c_str());
                    globalTable->emplace(left->children[0]->lexinfo, reg_name);
                }
                else {
                    fprintf(outfile, "%s ", (*(left->lexinfo)).c_str());
                    string reg_name = format_reg(left->children[0]);
                    fprintf(outfile, "%s ", reg_name.c_str());
                    globalTable->emplace(left->children[0]->lexinfo, reg_name);
                }

                fprintf(outfile, "= ");
                astree* right = node->children[1];

                if (right->children.size() == 0) {
                    if (right->attributes[static_cast<size_t>(attr::CONST)] == 1) {
                        fprintf(outfile, "%s;\n", (*(right->lexinfo)).c_str());
                    }
                    else if (right->attributes[static_cast<size_t>(attr::NULLX)] == 1) {
                        fprintf(outfile, "NULL;\n");
                    }
                    else {
                        string var_name = globalTable->find(right->lexinfo)->second;
                        fprintf(outfile, "%s;\n", var_name.c_str());
                    }
                }
                else {
                    if (right->symbol == TOK_POINTER_GET) {
                        string format = format_pointer_get(right);
                        fprintf(outfile, "%s;\n", format.c_str());
                    }
                    if (right->symbol == TOK_KW_NEW) {
                        string format = format_pointer_get(right);
                        fprintf(outfile, "%s;\n", format.c_str());
                    }
                }
            }
            break;
        } 

        default : break;
    }

    for (size_t i = 0; i < node->children.size(); ++i) {
        genIntermlang(node->children[i], out_file);
    }

    if (node->symbol == TOK_FUNCTION 
        || node->symbol == TOK_PROTO) {
        var_table_stack.pop_back();
        nextblock = var_table_stack.size();
    }

    // if (node->symbol == TOK_BLOCK) {
    //     var_table_stack.pop_back();
    //     nextblock = var_table_stack.size();
    // }
}