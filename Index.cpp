#include "Index.h"


Index::Index(std::string new_name, std::string new_table_name, std::string new_attr_name, attr_type new_type):
	name(new_name), table_name(new_table_name), attr_name(new_attr_name), type(new_type)
{
}


Index::~Index()
{
}
