#ifndef INDEX_H
#define INDEX_H

#include <string>
#include "Attribute.h"

class Index
{
private:
	std::string name;
	std::string table_name;
	std::string attr_name;
	attr_type type;
public:
	Index(std::string new_name, std::string new_table_name, std::string new_attr_name, attr_type new_type);
	~Index();
	const std::string& get_name() const { return name; };
	const std::string& get_table() const { return table_name; };
	const std::string& get_attr() const { return attr_name; };
	attr_type get_type() const { return type; };
	void set_name(std::string n) {
		name = n;
	}
};

#endif

