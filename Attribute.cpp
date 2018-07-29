#include "Attribute.h"

Attribute::Attribute(std::string new_name, attr_type new_type, bool new_unique) :
		name(new_name), type(new_type), unique(new_unique)
{

}


Attribute::~Attribute()
{

}
