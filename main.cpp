// $Id: main.cpp,v 1.2 2016-08-18 15:13:48-07 - - $

#include <string>
#include <fstream>
using namespace std;

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "string_set.h"
#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"
#include "symboltable.h"
#include "intermlang.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

bool check_programName(const char * program_name) {
        int strlength = strlen(program_name);
        if (strlength <= 3) return false;
        size_t i = strlength - 1;
        if (program_name[i] == 'c') {
                --i;
                if (program_name[i] == 'o') {
                        --i;
                        if (program_name[i] == '.') {
                                return true;
                        }
                        else {
                                return false;
                        }
                }
                else {
                        return false;
                }
        }
        return false;
}

void chomp_buffer(char * buffer, char delim) {
        int len = strlen(buffer);
        if (len == 0) return;
        char * last = buffer + len - 1;
        if (*last == delim) *last = '\0';
}

void cpplines(FILE * pipe) {
        char strdelim[] = " \t\n,.-+=~!?:;#@<>&*^()|[]{}/\'\""; 
        int linenr = 1;
        for (;;) {
                char buffer[LINESIZE];
                char * fgets_rc = fgets(buffer, LINESIZE, pipe);
                if (fgets_rc == NULL) break;
                chomp_buffer(buffer, '\n');

                if (strlen(buffer) >= 1) {
                        if (buffer[0] == '#') {
                                continue;
                        }
                }

                if (strlen(buffer) >= 2) {
                        if (buffer[0] == '/' && buffer[1] == '/') {
                                continue;
                        }
                }

                char * savepos = NULL;
                char * bufptr = buffer;
                for (int tokenct = 1; ; ++tokenct) {
                        char * token = 
                                strtok_r(bufptr, strdelim, &savepos);
                        bufptr = NULL;
                        if (token == NULL) break;
                        auto check = string_set::set.find(token);
                        if (check == string_set::set.end()) {
                                string_set::intern (token);
                        }
                }
                ++linenr;
        }
}

int main (int argc, char** argv) {
        // int pass_to_cpp = 0;
        // string pass_cpp_string;

        yy_flex_debug = 0;
        yydebug = 0;
        opterr = 0;

        int option;
        char * augu_program_name;

        while ((option = getopt(argc, argv, "ly@:D:")) != -1) {
                switch(option) {
                        case 'l':
                                yy_flex_debug = 1;
                                break;
                        case 'y':
                                yydebug = 1; 
                                break;
                        case '@':
                                set_debugflags(optarg);
                                break;
                        case 'D':
                                // pass_to_cpp = 1;
                                // pass_cpp_string = optarg;
                                break;
                        case '?':
                                fprintf(stderr, 
                 "usage: %s [-ly] [-@ flag] [-D string] program.oc", 
                                        argv[0]);
                                exit(EXIT_FAILURE);
                                break;
                }
        }

        if (optind >= argc) {
                fprintf(stderr, "Expect an argument after option");
                exit(EXIT_FAILURE);
        }
        else {
                augu_program_name = argv[optind];    
        }

        if (!check_programName(augu_program_name)) {
                fprintf(stderr, 
"Invalid file name. File name must be [programfilename].oc");
                exit(EXIT_FAILURE);
        }

        char * program_name = augu_program_name;
        string command = CPP + " " + program_name;
        
        int length = strlen(program_name);
        program_name[length - 3] = '\0';
        string tmp_program_name(program_name);
        string string_set_outfilename = tmp_program_name + ".str";

        yyin = popen(command.c_str(),"r");
        freopen(string_set_outfilename.c_str(), "w", stdout);

        if (yyin == NULL) {
                fprintf(stderr, "%s %s: %s", 
                        program_name, command.c_str(), strerror(errno));
                exit(EXIT_FAILURE);
        } else { 
                cpplines(yyin);
        }

        string_set::dump (stdout);

        fclose(stdout);
        int pclose_rc = pclose(yyin);
        if(pclose_rc != 0) exit(EXIT_FAILURE);


       
        string token_set_outfilename = tmp_program_name + ".tok";

        freopen(token_set_outfilename.c_str(), "w", stdout);
        
        yyin = popen(command.c_str(),"r");

        if (yy_flex_debug || yydebug) {
                fprintf (stderr, "Dumping parser::root:\n");
                if (parser::root != nullptr) 
                        parser::root->dump_tree (stderr);
                fprintf (stderr, "Dumping string_set:\n");
                string_set::dump (stderr);
        } 

        if (yyin == NULL) {
                fprintf(stderr, "%s %s: %s", 
                        program_name, command.c_str(), strerror(errno));
                exit(EXIT_FAILURE);
        } else {
                string file_num = tmp_program_name.substr(0,2);
                fprintf(stdout, "# %s \"%s\"\n", file_num.c_str(), 
               (tmp_program_name + ".oc").c_str());

                int yyparse_rc = yyparse();
                if (yyparse_rc != 0) {
                        fprintf(stderr, 
                        "%s %s: %s ", 
                        program_name, 
                        command.c_str(), strerror(errno));
                        exit(EXIT_FAILURE);
                }
        }
        
        fclose(stdout);

        string symbol_table_outfilename = tmp_program_name + ".sym";

        freopen(symbol_table_outfilename.c_str(), "w", stdout);

        bool st_rc = ASTTraversal(parser::root, stdout);

        if (!st_rc) {
                fprintf(stderr, "%s %s: %s", program_name, 
                        command.c_str(), strerror(errno));         
        }   

        fclose(stdout);

        string parser_tree_outfilename = tmp_program_name + ".ast";

        freopen(parser_tree_outfilename.c_str(), "w", stdout);

        astree::print(stdout, parser::root, 1);

        pclose_rc = pclose(yyin);
        if (pclose_rc != 0) {
                fprintf(stderr, "%s %s: %s", program_name, 
                        command.c_str(), strerror(errno));
                exit(EXIT_FAILURE);
        }

        fclose(stdout);
        
        string interm_lang_outfilename = tmp_program_name + ".oil";

        freopen(interm_lang_outfilename.c_str(), "w", stdout);

        genIntermlang(parser::root, stdout);

        fclose(stdout);

        return EXIT_SUCCESS;
}

