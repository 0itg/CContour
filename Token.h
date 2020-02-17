#pragma once
#include <functional>
#include <complex>
#include <array>

template<typename T>
class Parser;
template<typename T>
class SymbolError;
template<typename T>
class SymbolNum;

// Precedence ranks which can also serve as identifiers
enum precedence_list
{
	sym_num  = 0,
	sym_func = 1,
	sym_pow  = 2,
	sym_mul  = 3,
	sym_div  = 3,
	sym_neg  = 3,
	sym_add  = 4,
	sym_sub  = 4,
	sym_lparen = -1,
	sym_rparen = 100,
	sym_comma = -1
};

template<typename T>
class Symbol
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() = 0;
	virtual std::string GetToken() = 0;
	virtual bool IsLeftAssoc() { return true; }
	virtual bool IsDyad() { return false; }
	virtual bool IsMonad() { return false; }
	virtual bool IsPunctuation() { return false; }

	Symbol() {};
	virtual ~Symbol() {};
	Symbol(Parser<T>* par) : parent(par) {};

	virtual SNumPtr eval() { return std::make_unique<SymbolError<T>>(); }
	virtual void SetVal(const T& v) {};
	void SetParent(Parser<T>* p) { parent = p; }

	bool leftAssoc = true;
protected:
	//T val = 0;
	Parser<T>* parent = nullptr;
};

template<typename T>
class Dyad : public Symbol<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual SNumPtr Apply(SNumPtr t1, SNumPtr t2) = 0;
	virtual SNumPtr eval()
	{
		try
		{
			if (this->parent->itr < this->parent->GetMinItr() + 1)
				throw std::invalid_argument("Error: Mismatched operations\n");
		}
		catch(std::invalid_argument)
		{
			throw;
		}
		SNumPtr s1 = (*(--this->parent->itr))->eval();
		try
		{
			if (this->parent->itr < this->parent->GetMinItr() + 1)
				throw std::invalid_argument("Error: Mismatched operations\n");
		}
		catch (std::invalid_argument)
		{
			throw;
		}
		SNumPtr s2 = (*(--this->parent->itr))->eval();
		return Apply(std::move(s1), std::move(s2));
	}
	bool IsDyad() { return true; }
};

template<int... Is>
struct seq {};
template<int N, int... Is>
struct int_seq : int_seq<N - 1, N - 1, Is...> {};
template<int... Is>
struct int_seq<0, Is...> : seq<Is...> {};

template<typename T, typename... Ts, int... Is>
T callByVector(std::function<T(Ts...)> f, std::array<T,
	sizeof...(Ts)>& arguments, seq<Is...>)
{
	return f(arguments[Is]...);
}

template<typename T, typename... Ts>
T callByVector(std::function<T(Ts...)> f,
	std::array<T, sizeof...(Ts)>& arguments)
{
	return callByVector(f, arguments, int_seq<sizeof...(Ts)>());
}

template<typename T, typename... Ts>
class SymbolFunc : public Symbol<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	SymbolFunc() {};
	SymbolFunc(const std::function<T(Ts...)>& g, const std::string& s) :
		f(g), name(s) {};

	std::function <T(Ts...)> f;
	virtual SNumPtr Apply(std::array<T, sizeof...(Ts)>& args)
	{
		return std::make_unique<SymbolNum<T>>(callByVector(f, args));
	}
	virtual SNumPtr eval()
	{
		std::array<T, sizeof...(Ts)> s;
		for (size_t i = 0; i < sizeof...(Ts); i++)
		{
			try
			{
				if (this->parent->itr < this->parent->GetMinItr() + 1)
					throw std::invalid_argument("Error: Mismatched operations\n");
			}
			catch (std::invalid_argument)
			{
				throw;
			}
			s[i] = (*(--this->parent->itr))->eval()->getVal();
		}
		return Apply(s);
	}
	virtual int GetPrecedence() { return sym_func; }
	virtual std::string GetToken() { return name; }
private:
	std::string name = "f";	
};

template<typename T>
class Monad : public Symbol<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual SNumPtr Apply(SNumPtr t1) = 0;
	virtual SNumPtr eval()
	{
		try
		{
			if (this->parent->itr < this->parent->GetMinItr() + 1)
				throw std::invalid_argument("Error: Mismatched operations\n");
		}
		catch (std::invalid_argument)
		{
			throw;
		}
		return Apply(std::move((*(--this->parent->itr))->eval()));
	}
	bool IsMonad() { return true; }
};

// Used for parsing strings. Should never make it to the output queue
template<typename T>
class SymbolLParen : public Symbol<T>
{
public:
	virtual int GetPrecedence() { return sym_lparen; }
	virtual std::string GetToken() { return "("; }
	virtual bool IsPunctuation() { return true; }
};

// Used for parsing strings. Should never make it to the output queue
template<typename T>
class SymbolRParen : public Symbol<T>
{
public:
	virtual int GetPrecedence() { return sym_rparen; }
	virtual std::string GetToken() { return ")"; }
	virtual bool IsPunctuation() { return true; }
};

// Used for intermediate calculations and parsed numbers
template<typename T>
class SymbolNum : public Symbol<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	SymbolNum() : Symbol<T>() {}
	SymbolNum(const T& v) : val(v) {}
	SymbolNum(const SymbolNum<T>&& S) { val = S.val; }
	SymbolNum(const SymbolNum<T>* ptr) { val = ptr->val; }

	virtual int GetPrecedence() { return sym_num; }
	virtual std::string GetToken() { return ""; }
	virtual T getVal() { return val; }
	virtual void SetVal(const T& v) { }
	virtual SNumPtr eval() { return std::make_unique<SymbolNum<T>>(this); }
protected:
	T val;
};

template<typename T>
class SymbolVar : public SymbolNum<T>
{
public:
	std::string name;
	SymbolVar(std::string s, T v) : name(s), SymbolNum<T>(v) {}
	virtual int GetPrecedence() { return sym_num; }
	virtual std::string GetToken() { return name; }
	virtual void SetVal(const T& v) { this->val = v; }
};

template<typename T>
class SymbolConst : public SymbolNum<T>
{
public:
	std::string name;
	SymbolConst(std::string s, T v) : name(s), SymbolNum<T>(v) {}
	virtual int GetPrecedence() { return sym_num; }
	virtual std::string GetToken() { return name; }
};

template<typename T>
class SymbolComma : public Monad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	SymbolComma() {};
	virtual int GetPrecedence() { return sym_comma; }
	virtual std::string GetToken() { return ","; }
	virtual bool IsPunctuation() { return true; }
	virtual SNumPtr Apply(SNumPtr t1) { return this->eval(); }
};

template<typename T>
class SymbolError : public SymbolNum<T>
{
public:
	SymbolError() {};
	virtual int GetPrecedence() { return -100; }
	virtual std::string GetToken() { return "error"; }
};

template<typename T>
class SymbolAdd : public Dyad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() { return sym_add; }
	virtual std::string GetToken() { return "+"; }
	virtual SNumPtr Apply(SNumPtr t1, SNumPtr t2)
	{
		return std::make_unique<SymbolNum<T>>(t2->getVal() + t1->getVal());
	}
};

template<typename T>
class SymbolSub : public Dyad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() { return sym_sub; }
	virtual std::string GetToken() { return "-"; }
	virtual SNumPtr Apply(SNumPtr t1, SNumPtr t2)
	{
		return std::make_unique<SymbolNum<T>>(t2->getVal() - t1->getVal());
	}
};

template<typename T>
class SymbolMul : public Dyad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() { return sym_mul; }
	virtual std::string GetToken() { return "*"; }
	virtual SNumPtr Apply(SNumPtr t1, SNumPtr t2)
	{
		return std::make_unique<SymbolNum<T>>(t2->getVal() * t1->getVal());
	}
};

template<typename T>
class SymbolDiv : public Dyad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() { return sym_div; }
	virtual std::string GetToken() { return "/"; }
	virtual SNumPtr Apply(SNumPtr t1, SNumPtr t2)
	{
		return std::make_unique<SymbolNum<T>>(t2->getVal() / t1->getVal());
	}
};

template<typename T>
class SymbolPow : public Dyad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() { return sym_pow; }
	virtual bool IsLeftAssoc() { return false; }
	virtual std::string GetToken() { return "^"; }
	virtual SNumPtr Apply(SNumPtr t1, SNumPtr t2)
	{
		return std::make_unique<SymbolNum<T>>(pow(t2->getVal(), t1->getVal()));
	}
};

template<typename T>
class SymbolNeg : public Monad<T>
{
	typedef std::unique_ptr<SymbolNum<T>> SNumPtr;
public:
	virtual int GetPrecedence() { return sym_neg; }
	virtual std::string GetToken() { return "~"; }
	virtual SNumPtr Apply(SNumPtr t1)
	{
		return std::make_unique<SymbolNum<T>>(-t1->getVal());
	}
};