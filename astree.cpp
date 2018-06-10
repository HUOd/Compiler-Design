// $Id: astree.cpp,v 1.9 2017-10-04 15:59:50-07 - - $

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"
#include "symboltable.h"

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   // vector defaults to empty -- no children
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2, astree* child3) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   if (child3 != nullptr) children.push_back (child3);
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}


void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_tname (symbol),
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (nullptr);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void arrange_list(string& list, astree* table) {
    if (table->attributes[static_cast<size_t>(attr::VOID)] == 1) {
        list += "void ";
    }
    if (table->attributes[static_cast<size_t>(attr::STRUCT)] == 1) {
        list += "struct "; 
        if (table->struct_name != nullptr) 
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

void astree::print (FILE* outfile, astree* tree, int depth) {
//    fprintf (outfile, "; %*s", depth * 3, "");
//    fprintf (outfile, "  %zd     %zd    %d  %s (%s)\n",
//             tree->lloc.linenr, tree->lloc.offset,
//             tree->symbol,
//             parser::get_tname (tree->symbol), 
//             tree->lexinfo->c_str());
//    for (astree* child: tree->children) {
//       astree::print (outfile, child, depth + 1);
//    }

   fprintf (outfile, " %*s", depth * 3, "");
   const char *tname = parser::get_tname (tree->symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;
   string list = "";
   arrange_list(list, tree);

   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd) {%zd} %s\n",
            tname, tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset, 
            tree->block_nr, list.c_str());
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void astree::change_symbol(int symbol_) {
    symbol = symbol_;
}

void astree::print_tok(FILE* outfile, astree* tree) {
    fprintf (outfile, "  %zd     %zd    %d  %s (%s)\n",
            tree->lloc.linenr, tree->lloc.offset,
            tree->symbol,
            parser::get_tname (tree->symbol), 
            tree->lexinfo->c_str());
}

void destroy (astree* tree1, astree* tree2) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}


