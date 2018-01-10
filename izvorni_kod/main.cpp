#include "prop_logic.h"
#include "colors.h"

using namespace std;

extern int yyparse();
extern Formula parsed_formula;

int main()
{
	yyparse();

	Formula a = parsed_formula;

	if (a.get() != nullptr)
	{
		cout << FRED("Formula before transformation: ") << a << endl;

		Formula b = a->tseitinTransformation();
		cout << FGRN("Formula after transformation: ") << b << endl;

		Formula c = b->nnf();
		cout << FYEL("Formula after nnf: ") << c << endl;

		LiteralListList d = c->flatCNF();
		cout << FCYN("Flat formula format: ") << d << endl;
	}

	return 0;
}
