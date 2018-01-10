#include "prop_logic.h"

using namespace std;


//-----------------------------------------------------------------------------
// BaseFormula
//-----------------------------------------------------------------------------

// returns true if a type of the first arg is T_NOT, T_ATOM, T_TRUE or T_FALSE
bool BaseFormula::isNATF(const Formula &f) const
{
	Type t = f->getType();

	return t == T_TRUE || t == T_FALSE || t == T_ATOM || t == T_NOT;
}

bool BaseFormula::isEquivalent(const Formula &f) const
{
	AtomSet as;
	getAtoms(as);
	f->getAtoms(as);
	Valuation v(as);

	do
	{
		if(eval(v) != f->eval(v))
			return false;
	} while(v.next());

	return true;
}

void BaseFormula::printTruthTable() const
{
	AtomSet as;
	getAtoms(as);
	Valuation v(as);

	do
	{
		cout << v << " | " << eval(v) << endl;
	} while(v.next());
}

bool BaseFormula::isTautology() const
{
	AtomSet as;
	getAtoms(as);
	Valuation v(as);

	do
	{
		if(eval(v) == false)
			return false;
	} while(v.next());

	return true;
}

bool BaseFormula::isSat(Valuation &v) const
{
	AtomSet as;
	getAtoms(as);
	v = Valuation(as);

	do
	{
		if(eval(v) == true)
			return true;
	} while(v.next());

	return false;
}

Formula BaseFormula::tseitinTransformation()
{
	AtomSet as;
	Formula simpl = simplify()->pushNegation();
	simpl->getAtoms(as);
	Formula tmp = nullptr;
	Formula res = _tseitin(simpl, as, tmp);

	if(tmp.get() == nullptr)
		return res;
	else
		return make_shared<And>(res, tmp);
}

Formula BaseFormula::_tseitin(const Formula &f, AtomSet &as, Formula &tmp) const
{
	if(isNATF(f))
		return f;

	// apply transformation on subformulas
	Formula ts1 = _tseitin(((BinaryConnective*) f.get())->getOp1(), as, tmp);
	Formula ts2 = _tseitin(((BinaryConnective*) f.get())->getOp2(), as, tmp);

	// make new atom
	string id = getUniqueId(as);
	Formula atom = make_shared<Atom>(id);
	as.insert(id);

	Formula conn;
	switch(f->getType())
	{
		case T_AND:
			conn = make_shared<And>(ts1, ts2);
			break;
		case T_OR:
			conn = make_shared<Or>(ts1, ts2);
			break;
		case T_IMP:
			conn = make_shared<Imp>(ts1, ts2);
			break;
		case T_IFF:
			conn = make_shared<Iff>(ts1, ts2);
			break;
	}

	if(tmp.get() == nullptr)
		tmp = make_shared<Iff>(atom, conn);
	else
		tmp = make_shared<And>(tmp, make_shared<Iff>(atom, conn));

	return atom;
}

//-----------------------------------------------------------------------------
// List functions
//-----------------------------------------------------------------------------
template <typename T> T concatenateLists(const T &l1, const T &l2)
{
	T l = l1;
	copy(l2.begin(), l2.end(), back_inserter(l));

	return l;
}

LiteralListList makePairs(const LiteralListList &ll1, const LiteralListList &ll2)
{
	LiteralListList ll;

	for (auto &l1 : ll1)
		for (auto &l2 : ll2)
			ll.push_back(concatenateLists(l1, l2));

	return ll;
}

//-----------------------------------------------------------------------------
// AtomicFormula
//-----------------------------------------------------------------------------
Formula AtomicFormula::simplify()
{
	return shared_from_this();
}

Formula AtomicFormula::pushNegation()
{
	return shared_from_this();
}

Formula AtomicFormula::nnf()
{
	return shared_from_this();
}

//-----------------------------------------------------------------------------
// LogicConstant
//-----------------------------------------------------------------------------
void LogicConstant::getAtoms(AtomSet &as) const
{}

bool LogicConstant::equals(const Formula &f) const
{
	return getType() == f->getType();
}

//-----------------------------------------------------------------------------
// True
//-----------------------------------------------------------------------------
Type True::getType() const
{
	return T_TRUE;
}

void True::print(ostream &ostr) const
{
	ostr << "TRUE";
}

bool True::eval(const Valuation &v) const
{
	return true;
}

LiteralListList True::flatCNF()
{
	return { };
}

//-----------------------------------------------------------------------------
// False
//-----------------------------------------------------------------------------
Type False::getType() const
{
	return T_FALSE;
}

void False::print(ostream &ostr) const
{
	ostr << "FALSE";
}

bool False::eval(const Valuation &v) const
{
	return false;
}

LiteralListList False::flatCNF()
{
	return {{}};
}

//-----------------------------------------------------------------------------
// Atom
//-----------------------------------------------------------------------------
Atom::Atom(const string &id)
	: _id(id) {}

Type Atom::getType() const
{
	return T_ATOM;
}

void Atom::getAtoms(AtomSet &as) const
{
	as.insert(_id);
}

bool Atom::equals(const Formula &f) const
{
	return getType() == f->getType() && _id == ((Atom*) f.get())->_id;
}

void Atom::print(ostream &ostr) const
{
	ostr << _id;
}

bool Atom::eval(const Valuation &v) const
{
	return v.getValue(_id);
}

string Atom::getId() const
{
	return _id;
}

LiteralListList Atom::flatCNF()
{
	return { { shared_from_this() } };
}

//-----------------------------------------------------------------------------
// UnaryConnective
//-----------------------------------------------------------------------------
UnaryConnective::UnaryConnective(const Formula &op)
	: _op(op) {}

const Formula& UnaryConnective::getOp() const
{
	return _op;
}

void UnaryConnective::getAtoms(AtomSet &as) const
{
	_op->getAtoms(as);
}

bool UnaryConnective::equals(const Formula &f) const
{
	return getType() == f->getType() && _op->equals(((UnaryConnective*) f.get())->_op);
}

//-----------------------------------------------------------------------------
// Not
//-----------------------------------------------------------------------------
Type Not::getType() const
{
	return T_NOT;
}

Formula Not::simplify()
{
	Formula simp = _op->simplify();

	if(simp->getType() == T_FALSE)
		return make_shared<True>();
	else if(simp->getType() == T_TRUE)
		return make_shared<False>();
	else
		return make_shared<Not>(simp);
}

Formula Not::pushNegation()
{
	if(_op->getType() == T_NOT)
	{
		return ((Not*) _op.get())->_op->pushNegation();
	}
	else if(_op->getType() == T_AND)
	{
		And *tmp = (And*) _op.get();

		return make_shared<Or>(make_shared<Not>(tmp->getOp1())->pushNegation(), make_shared<Not>(tmp->getOp2()->pushNegation()));
	}
	else if(_op->getType() == T_OR)
	{
		Or *tmp = (Or*) _op.get();

		return make_shared<And>(make_shared<Not>(tmp->getOp1())->pushNegation(), make_shared<Not>(tmp->getOp2())->pushNegation());
	}
	else if(_op->getType() == T_IMP)
	{
		Imp *tmp = (Imp*) _op.get();

		return make_shared<And>(tmp->getOp1()->pushNegation(), make_shared<Not>(tmp->getOp2())->pushNegation());
	}
	else if(_op->getType() == T_IFF)
	{
		Iff *tmp = (Iff*) _op.get();

		return make_shared<Iff>(make_shared<Not>(tmp->getOp1())->pushNegation(), tmp->getOp2()->pushNegation());
	}
	else
	{
		return shared_from_this();
	}
}

void Not::print(ostream &ostr) const
{
	if(!isNATF(_op))
		ostr << "¬(" << _op << ")";
	else
		ostr << "¬" << _op;
}

bool Not::eval(const Valuation &v) const
{
	return !_op->eval(v);
}

LiteralListList Not::flatCNF()
{
	return { { shared_from_this() } };
}

Formula Not::nnf()
{
	if(_op->getType() == T_NOT)
	{
		Not *notOp = (Not*)_op.get();

		return notOp->getOperand()->nnf();
	}
	else if(_op->getType() == T_AND)
	{
		And *andOp = (And*)_op.get();

		return make_shared<Or>(make_shared<Not>(andOp->getOperand1())->nnf(), make_shared<Not>(andOp->getOperand2())->nnf());
	}
	else if(_op->getType() == T_OR)
	{
		Or *orOp = (Or*)_op.get();

		return make_shared<And>(make_shared<Not>(orOp->getOperand1())->nnf(), make_shared<Not>(orOp->getOperand2())->nnf());
	}
	else if(_op->getType() == T_IMP)
	{
		Imp *impOp = (Imp*)_op.get();

		return make_shared<And>(impOp->getOperand1()->nnf(), make_shared<Not>(impOp->getOperand2())->nnf());

	}
	else if(_op->getType() == T_IFF)
	{
		Iff *iffOp = (Iff*)_op.get();

		return make_shared<Or>(make_shared<And>(iffOp->getOperand1()->nnf(), make_shared<Not>(iffOp->getOperand2())->nnf()),
			make_shared<And>(iffOp->getOperand2()->nnf(), make_shared<Not>(iffOp->getOperand1())->nnf()));
	}
	else
	{
		return shared_from_this();
	}
}

//-----------------------------------------------------------------------------
// BinaryConnective
//-----------------------------------------------------------------------------
BinaryConnective::BinaryConnective(const Formula &op1, const Formula &op2)
	: _op1(op1), _op2(op2) {}

const Formula& BinaryConnective::getOp1() const
{
	return _op1;
}

const Formula& BinaryConnective::getOp2() const
{
	return _op2;
}

void BinaryConnective::getAtoms(AtomSet &as) const
{
	_op1->getAtoms(as);
	_op2->getAtoms(as);
}

bool BinaryConnective::equals(const Formula &f) const
{
	return getType() == f->getType() && _op1->equals(((BinaryConnective*) f.get())->_op1) && _op2->equals(((BinaryConnective*) f.get())->_op2);
}

void BinaryConnective::print(ostream &ostr) const
{
	if(!(_op1->getType() == getType() || isNATF(_op1)))
		ostr << "(" << _op1 << ")";
	else
		ostr << _op1;

	switch(getType())
	{
		case T_AND:
			ostr << " /\\ ";
			break;
		case T_OR:
			ostr << " \\/ ";
			break;
		case T_IMP:
			ostr << " => ";
			break;
		case T_IFF:
			ostr << " <=> ";
			break;
	}

	if(!(_op2->getType() == getType() || isNATF(_op2)))
		ostr << "(" << _op2 << ")";
	else
		ostr << _op2;
}

//-----------------------------------------------------------------------------
// And
//-----------------------------------------------------------------------------
Type And::getType() const
{
	return T_AND;
}

Formula And::simplify()
{
	Formula simp1 = _op1->simplify();
	Formula simp2 = _op2->simplify();

	if(simp1->getType() == T_TRUE)
		return simp2;
	else if(simp2->getType() == T_TRUE)
		return simp1;
	else if(simp1->getType() == T_FALSE || simp2->getType() == T_FALSE)
		return make_shared<False>();
	else
		return make_shared<And>(simp1, simp2);
}

Formula And::pushNegation()
{
	return make_shared<And>(_op1->pushNegation(), _op2->pushNegation());
}

bool And::eval(const Valuation &v) const
{
	return _op1->eval(v) && _op2->eval(v);
}

LiteralListList And::flatCNF()
{
	LiteralListList cl1 = _op1->flatCNF();
	LiteralListList cl2 = _op2->flatCNF();

	return concatenateLists(cl1, cl2);
}

Formula And::nnf()
{
	return make_shared<And>(_op1->nnf(), _op2->nnf());
}
//-----------------------------------------------------------------------------
// Or
//-----------------------------------------------------------------------------
Type Or::getType() const
{
	return T_OR;
}

Formula Or::simplify()
{
	Formula simp1 = _op1->simplify();
	Formula simp2 = _op2->simplify();

	if(simp1->getType() == T_TRUE || simp2->getType() == T_TRUE)
		return make_shared<True>();
	else if(simp1->getType() == T_FALSE)
		return simp2;
	else if(simp2->getType() == T_FALSE)
		return simp1;
	else
		return make_shared<Or>(simp1, simp2);
}

Formula Or::pushNegation()
{
	return make_shared<Or>(_op1->pushNegation(), _op2->pushNegation());
}

bool Or::eval(const Valuation &v) const
{
	return _op1->eval(v) || _op2->eval(v);
}

LiteralListList Or::flatCNF()
{
  LiteralListList cl1 = _op1->flatCNF();
  LiteralListList cl2 = _op2->flatCNF();

  return makePairs(cl1, cl2);
}

Formula Or::nnf()
{
	return make_shared<Or>(_op1->nnf(), _op2->nnf());
}

//-----------------------------------------------------------------------------
// Imp
//-----------------------------------------------------------------------------
Type Imp::getType() const
{
	return T_IMP;
}

Formula Imp::simplify()
{
	Formula simp1 = _op1->simplify();
	Formula simp2 = _op2->simplify();

	if(simp2->getType() == T_TRUE || simp1->getType() == T_FALSE)
		return make_shared<True>();
	else if(simp1->getType() == T_TRUE)
		return simp2;
	else if(simp2->getType() == T_FALSE)
		return make_shared<Not>(simp1);
	else
		return make_shared<Imp>(simp1, simp2);
}

Formula Imp::pushNegation()
{
	return make_shared<Or>(make_shared<Not>(_op1)->pushNegation(), _op2->pushNegation());
}

bool Imp::eval(const Valuation &v) const
{
	return !_op1->eval(v) || _op2->eval(v);
}

LiteralListList Imp::flatCNF()
{
	assert(!"HAHA1");
}

Formula Imp::nnf()
{
	return make_shared<Or>(make_shared<Not>(_op1)->nnf(), _op2->nnf());
}

//-----------------------------------------------------------------------------
// Iff
//-----------------------------------------------------------------------------
Type Iff::getType() const
{
	return T_IFF;
}

Formula Iff::simplify()
{
	Formula simp1 = _op1->simplify();
	Formula simp2 = _op2->simplify();

	if(simp1->getType() == T_FALSE && simp2->getType() == T_FALSE)
		return make_shared<True>();
	else if(simp1->getType() == T_TRUE)
		return simp2;
	else if(simp2->getType() == T_TRUE)
		return simp1;
	else if(simp1->getType() == T_FALSE)
		return make_shared<Not>(simp2);
	else if(simp2->getType() == T_FALSE)
		return make_shared<Not>(simp1);
	else
		return make_shared<Iff>(simp1, simp2);
}

Formula Iff::pushNegation()
{
	return make_shared<Iff>(_op1->pushNegation(), _op2->pushNegation());
}

bool Iff::eval(const Valuation &v) const
{
	return _op1->eval(v) == _op2->eval(v);
}

LiteralListList Iff::flatCNF()
{
	assert(!"HAHA2");
}

Formula Iff::nnf()
{
	return make_shared<And>(make_shared<Or>(make_shared<Not>(_op1)->nnf(), _op2->nnf()), make_shared<Or>(make_shared<Not>(_op2)->nnf(), _op1->nnf()));
}

//-----------------------------------------------------------------------------
// Valuation
//-----------------------------------------------------------------------------
Valuation::Valuation(const AtomSet &as)
{
	for(const string &i : as)
		_vars.insert(make_pair(i, false));
}

bool Valuation::getValue(const string &str) const
{
	return _vars.find(str)->second;
}

void Valuation::setValue(string &str, bool val)
{
	_vars[str] = val;
}

bool Valuation::next()
{
	for(auto i = _vars.rbegin(), k = _vars.rend(); i != k; i++)
	{
		i->second = !i->second;
		if(i->second != false)
			return true;
	}
	return false;
}

void Valuation::print(ostream &ostr) const
{
	for(auto b = _vars.cbegin(), e = _vars.cend(); b != e; b++)
	{
		ostr << b->second << " ";
	}
}

//-----------------------------------------------------------------------------
// Support functions
//-----------------------------------------------------------------------------
ostream& operator<<(ostream &ostr, const Formula &f)
{
	f->print(ostr);
	return ostr;
}

ostream& operator<<(ostream &ostr, const Valuation &v)
{
	v.print(ostr);
	return ostr;
}

string getUniqueId(const AtomSet &as)
{
	static unsigned id = 0;

	do
	{
		++id;
	} while (as.find("s" + to_string(id)) != as.cend());

	return "s" + to_string(id);
}

ostream& operator<<(ostream &ostr, const LiteralListList &l)
{
	ostr << "[ ";
	for(auto & ll :  l)
	{
		ostr << "[ ";
		for(auto & f : ll)
	{
		ostr << f << " ";
	}
		ostr << "] ";
	}
	ostr << " ]";

	return ostr;
}
