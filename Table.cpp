//#include "stdafx.h"
#include "Table.h"


Table::Table(std::string new_name, std::string new_key, int new_attr_num, int new_rec_num):
	name(new_name), key(new_key), attr_num(new_attr_num), rec_num(new_rec_num)
{
}


Table::~Table()
{
}
