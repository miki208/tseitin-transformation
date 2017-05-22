#include "prop_logic.h"

using namespace std;

/* BaseFormula */
bool BaseFormula::isEquivalent(const Formula &f) const
{
	AtomSet as;
	getAtoms(as);
	f->getAtoms(as);
	Valuation v(as);

	do {
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

	do {
		cout << v << " | " << eval(v) << endl;
	} while(v.next());
}

bool BaseFormula::isTautology() const
{
	AtomSet as;
	getAtoms(as);
	Valuation v(as);

	do {
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

	do {
		if(eval(v) == true)
			return true;
	} while(v.next());

	return false;
}

/* AtomicFormula */
Formula AtomicFormula::simplify() {
	return shared_from_this();
}

Formula AtomicFormula::pushNegation() {
	return shared_from_this();
}

/* LogicConstant */
void LogicConstant::getAtoms(AtomSet &as) const {
}

bool LogicConstant::equals(const Formula &f) const {
	return getType() == f->getType();
}

/* True */
Type True::getType() const {
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

/* False */
Type False::getType() const {
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

/* Atom */
Atom::Atom(const string &id) : _id(id) {
}

Type Atom::getType() const {
	return T_ATOM;
}

void Atom::getAtoms(AtomSet &as) const {
	as.insert(_id);	
}

bool Atom::equals(const Formula &f) const {
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

/* UnaryConnective */
UnaryConnective::UnaryConnective(const Formula &op) : _op(op) {
}

const Formula& UnaryConnective::getOp() const {
	return _op;
}

void UnaryConnective::getAtoms(AtomSet &as) const {
	_op->getAtoms(as);
}

bool UnaryConnective::equals(const Formula &f) const {
	return getType() == f->getType() && _op->equals(((UnaryConnective*) f.get())->_op);
}

/* Not */
Type Not::getType() const {
	return T_NOT;
}

Formula Not::simplify() {
	Formula simp = _op->simplify();

	if(simp->getType() == T_FALSE)
		return make_shared<True>();
	else if(simp->getType() == T_TRUE)
		return make_shared<False>();
	else
		return make_shared<Not>(simp);
}

Formula Not::pushNegation() {
	if(_op->getType() == T_NOT)
		return ((Not*) _op.get())->_op->pushNegation();
	else if(_op->getType() == T_AND) {
		And *tmp = (And*) _op.get();

		return make_shared<Or>(make_shared<Not>(tmp->getOp1())->pushNegation(), make_shared<Not>(tmp->getOp2()->pushNegation()));
	} else if(_op->getType() == T_OR) {
		Or *tmp = (Or*) _op.get();

		return make_shared<And>(make_shared<Not>(tmp->getOp1())->pushNegation(), make_shared<Not>(tmp->getOp2())->pushNegation());
	} else if(_op->getType() == T_IMP) {
		Imp *tmp = (Imp*) _op.get();

		return make_shared<And>(tmp->getOp1()->pushNegation(), make_shared<Not>(tmp->getOp2())->pushNegation());
	} else if(_op->getType() == T_IFF) {
		Iff *tmp = (Iff*) _op.get();

		return make_shared<Iff>(make_shared<Not>(tmp->getOp1())->pushNegation(), tmp->getOp2()->pushNegation());
	} else {
		return shared_from_this();
	}
}

void Not::print(ostream &ostr) const
{
	ostr << "¬" << _op;
}

bool Not::eval(const Valuation &v) const
{
	return !_op->eval(v);
}

/* BinaryConnective */
BinaryConnective::BinaryConnective(const Formula &op1, const Formula &op2) : _op1(op1), _op2(op2) {
}

const Formula& BinaryConnective::getOp1() const {
	return _op1;
}

const Formula& BinaryConnective::getOp2() const {
	return _op2;
}

void BinaryConnective::getAtoms(AtomSet &as) const {
	_op1->getAtoms(as);
	_op2->getAtoms(as);
}

bool BinaryConnective::equals(const Formula &f) const {
	return getType() == f->getType() && _op1->equals(((BinaryConnective*) f.get())->_op1) && _op2->equals(((BinaryConnective*) f.get())->_op2);
}

/* And */
Type And::getType() const {
	return T_AND;
}

Formula And::simplify() {
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

Formula And::pushNegation() {
	return make_shared<And>(_op1->pushNegation(), _op2->pushNegation());
}

void And::print(ostream &ostr) const
{
	ostr << "(" << _op1 << " /\\ " << _op2 << ")";
}

bool And::eval(const Valuation &v) const
{
	return _op1->eval(v) && _op2->eval(v);
}

/* Or */
Type Or::getType() const {
	return T_OR;
}

Formula Or::simplify() {
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

Formula Or::pushNegation() {
	return make_shared<Or>(_op1->pushNegation(), _op2->pushNegation());
}

void Or::print(ostream &ostr) const
{
	ostr << "(" << _op1 << " \\/ " << _op2 << ")";
}

bool Or::eval(const Valuation &v) const
{
	return _op1->eval(v) || _op2->eval(v);
}

/* Imp */
Type Imp::getType() const {
	return T_IMP;
}

Formula Imp::simplify() {
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

Formula Imp::pushNegation() {
	return make_shared<Or>(make_shared<Not>(_op1)->pushNegation(), _op2->pushNegation());
}

void Imp::print(ostream &ostr) const
{
	ostr << "(" << _op1 << " => " << _op2 << ")";
}

bool Imp::eval(const Valuation &v) const
{
	return !_op1->eval(v) || _op2->eval(v);
}

/* Iff */
Type Iff::getType() const {
	return T_IFF;
}

Formula Iff::simplify() {
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

Formula Iff::pushNegation() {
	return make_shared<Iff>(_op1->pushNegation(), _op2->pushNegation());
}

void Iff::print(ostream &ostr) const
{
	ostr << "(" << _op1 << " <=> " << _op2 << ")";
}

bool Iff::eval(const Valuation &v) const
{
	return _op1->eval(v) == _op2->eval(v);
}

/* Valuation */
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
	for(auto i = _vars.rbegin(), k = _vars.rend(); i != k; i++) {
		i->second = !i->second;
		if(i->second != false)
			return true;
	}
	return false;
}

void Valuation::print(ostream &ostr) const
{
	for(auto b = _vars.cbegin(), e = _vars.cend(); b != e; b++) {
		ostr << b->second << " ";
	}
}

/* Pomocne funkcije */

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
	
	do {
		++id;
	} while (as.find("s" + to_string(id)) != as.cend());
	
	return "s" + to_string(id);
}

