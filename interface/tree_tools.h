#ifndef TREE_TOOLS_H
#define TREE_TOOLS_H


#include "analysis_types.h"


// Declare global variables
#define SIMPLE_DATA_DIRECTIVE(type, name, default_value) type name = default_value;
#define VECTOR_DATA_DIRECTIVE(type, name) std::vector<type> name;
SIMPLE_DATA_DIRECTIVES
VECTOR_DATA_DIRECTIVES
#undef SIMPLE_DATA_DIRECTIVE
#undef VECTOR_DATA_DIRECTIVE


// Clear branches
void clear_branches(){
#define SIMPLE_DATA_DIRECTIVE(type, name, default_value) name = default_value;
#define VECTOR_DATA_DIRECTIVE(type, name) name.clear();
  SIMPLE_DATA_DIRECTIVES
  VECTOR_DATA_DIRECTIVES
#undef SIMPLE_DATA_DIRECTIVE
#undef VECTOR_DATA_DIRECTIVE
}


#endif
