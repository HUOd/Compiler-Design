#ifndef _INTERMLANG_H_
#define _INTERMLANG_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
using namespace std;

#include "astree.h"
#include "lyutils.h"

void genIntermlang(astree* node, FILE* outfile);

#endif 
