#ifndef _PROP_LOGIC_H_
#define _PROP_LOGIC_H_

#include <iostream>
#include <memory>
#include <string>
#include <set>
#include <map>

class BaseFormula;

typedef std::shared_ptr<BaseFormula> Formula;
typedef std::set<std::string> AtomSet;
enum Type { T_ATOM, T_TRUE, T_FALSE, T_IFF, T_IMP, T_NOT, T_AND, T_OR };

/* Deklaracije klasa */

class Valuation
{
	public:
		Valuation(const AtomSet&);
		bool getValue(const std::string&) const;
		void setValue(std::string&, bool);
		bool next();
		void print(std::ostream&) const;
	private:
		std::map<std::string, bool> _vars;
};

class BaseFormula : public std::enable_shared_from_this<BaseFormula>
{
	public:
		virtual Type getType() const = 0;
		virtual void getAtoms(AtomSet&) const = 0;
		virtual Formula simplify() = 0;
		virtual Formula pushNegation() = 0;
		virtual bool equals(const Formula&) const = 0;
		virtual void print(std::ostream&) const = 0;
		bool isEquivalent(const Formula&) const;
		void printTruthTable() const;
		bool isTautology() const;
		bool isSat(Valuation&) const;
		virtual bool eval(const Valuation&) const = 0;
};

class AtomicFormula : public BaseFormula
{
	public:
		Formula simplify();
		Formula pushNegation();
};

class LogicConstant : public AtomicFormula
{
	public:
		void getAtoms(AtomSet&) const;
		bool equals(const Formula&) const;
};

class True : public LogicConstant
{
	public:
		Type getType() const;
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

class False : public LogicConstant
{
	public:
		Type getType() const;
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

class Atom : public AtomicFormula
{
	public:
		Atom(const std::string &id);
		Type getType() const;
		void getAtoms(AtomSet &as) const;
		bool equals(const Formula&) const;
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;

	private:
		std::string _id;
};

class UnaryConnective : public BaseFormula
{
	public:
		UnaryConnective(const Formula &op);
		const Formula& getOp() const;
		void getAtoms(AtomSet &as) const;
		bool equals(const Formula&) const;

	protected:
		Formula _op;
};

class Not : public UnaryConnective
{
	public:
		using UnaryConnective::UnaryConnective;
		Type getType() const;
		Formula simplify();
		Formula pushNegation();
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

class BinaryConnective : public BaseFormula
{
	public:
		BinaryConnective(const Formula &op1, const Formula &op2);
		const Formula& getOp1() const;
		const Formula& getOp2() const;
		void getAtoms(AtomSet &as) const;
		bool equals(const Formula&) const;

	protected:
		Formula _op1, _op2;
};

class And : public BinaryConnective
{
	public:
		using BinaryConnective::BinaryConnective;
		Type getType() const ;
		Formula simplify();
		Formula pushNegation();
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

class Or : public BinaryConnective
{
	public:
		using BinaryConnective::BinaryConnective;
		Type getType() const;
		Formula simplify();
		Formula pushNegation();
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

class Imp : public BinaryConnective
{
	public:
		using BinaryConnective::BinaryConnective;
		Type getType() const;
		Formula simplify();
		Formula pushNegation();
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

class Iff : public BinaryConnective
{
	public:
		using BinaryConnective::BinaryConnective;
		Type getType() const;
		Formula simplify();
		Formula pushNegation();
		void print(std::ostream&) const;
		bool eval(const Valuation&) const;
};

std::ostream& operator<<(std::ostream&, const Formula&);

std::ostream& operator<<(std::ostream&, const Valuation&);

std::string getUniqueId(const AtomSet&);

extern Formula parsed_formula;

#endif
