#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>

typedef int attr_type; //-2 stands for float, -1 stands for int and other number represents the number of char. 

class Attribute
{
private:
	std::string name;
	attr_type type;
	bool unique;
public:
	Attribute(std::string new_name, attr_type new_type, bool new_unique);
	~Attribute();
	const std::string& get_name() const { return name; };
	attr_type get_type() const { return type; };
	bool is_unique() const { return unique; };
	void set_unique(bool unique) { this->unique = unique; }
	int static const TYPE_FLOAT = -2;
	int static const TYPE_INT = -1;
};

// public completely  !
// naive struct Value !
struct Value {
	Value(int i, float f, const std::string& str, attr_type type) :
		i(i),
		f(f),
		str(str),
		type(type)
	{

	}
	virtual ~Value() 
	{
	
	}

	bool operator<(const Value& right) const
	{
		if(type == -2) {
			return i < right.i;
		} else if(type == -1) {
			return f < right.f;
		} else {
			return str < right.str;
		}
	}

	void reset() {
		str.clear();
		i = 0;
		f = 0.0f;
	}

	std::string to_str() const {
		if(type == -2) {
			return std::to_string(i); // 10 base int
		} else if(type == -1) {
			return std::to_string(f); // 10 base float
		} else {
			return str;
		}
	}

	attr_type type;
	int i;
	float f;
	std::string str;
};


#endif
