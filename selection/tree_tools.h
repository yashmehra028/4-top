#include "analysis_types.h"

void declare_variables(){
	#define SIMPLE_DATA_DIRECTIVE(type, name, default_value) type name;
	#define VECTOR_DATA_DIRECTIVE(type, name) std::vec<type> name;
	SIMPLE_DATA_DIRECTIVES
	VECTOR_DATA_DIRECTIVES
	#undef SIMPLE_DATA_DIRECTIVE
	#undef VECTOR_DATA_DIRECTIVE
}

void clear_branches(){
	#define SIMPLE_DATA_DIRECTIVE(type, name, default_value) name = default_value;
	#define VECTOR_DATA_DIRECTIVE(type, name) name.clear();
	SIMPLE_DATA_DIRECTIVES
        VECTOR_DATA_DIRECTIVES
	#undef SIMPLE_DATA_DIRECTIVE
	#undef VECTOR_DATA_DIRECTIVE
}
