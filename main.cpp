#include "prop_logic.h"

using namespace std;

extern int yyparse();

extern Formula parsed_formula;

int main()
{
	yyparse();

	if(parsed_formula.get() != 0)
		cout << parsed_formula << endl;

	return 0;
}
