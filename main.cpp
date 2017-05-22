#include "prop_logic.h"

using namespace std;

extern int yyparse();

extern Formula parsed_formula;

int main()
{
	//yyparse();
	//Formula a = parsed_formula;
	
	//if(a.get() != 0)	
	//	cout << a << endl;
	
	Valuation v(AtomSet { "a", "b1", "c", "d" });
	do {
		cout << v << endl;
	} while(v.next());

	return 0;
}
