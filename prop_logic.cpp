#include "prop_logic.h"

using namespace std;

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

/* False */
Type False::getType() const {
	return T_FALSE;
}

void False::print(ostream &ostr) const
{
	ostr << "FALSE";
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

/* Pomocne funkcije */

std::ostream& operator<<(ostream &ostr, const Formula &f)
{
	f->print(ostr);
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

