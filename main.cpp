#include "prop_logic.h"

using namespace std;

extern int yyparse();

extern Formula parsed_formula;

int main()
{
	yyparse();
	Formula a = parsed_formula;
	
	if(a.get() != 0) {	
		cout << "Formula before transformation: " << a << endl;

		Formula b = a->tseitinTransformation();

		cout << "Formula after transformation: " << b << endl;
	}

	return 0;
}
