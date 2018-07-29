//#include "stdafx.h"
#include "CatalogManager.h"
#include <fstream>
#include <cstring>
#include <sstream>
#include <string>
using namespace std;

CatalogManager::CatalogManager()
{
}

CatalogManager::~CatalogManager()
{
}

bool CatalogManager::create_table(const std::string& name, const std::string& key, const std::vector<Attribute>& attributes)
{
	bool feedback = add_table_to_list(name, key, attributes.size());
	if (!feedback) return FAIL;
	feedback = create_table_file(name, attributes);
	return feedback;
}

bool CatalogManager::create_table_file(const std::string& name, const std::vector<Attribute>& attributes)
{
	string file_name = name + ".dat";
	bool feedback = new_file(file_name);
	if (!feedback) return FAIL;

	fileNode *file = buffer.get_File(file_name.c_str());
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* write;
	for (int i = 0; i < attributes.size(); i++)
	{
		while (1)
		{
			if (buffer.get_usingSize(*block) + sizeof(Attribute) <= buffer.get_BlockSize()) break;
			block = buffer.get_NextBlock(file, block);
			if (!block) return FAIL;
		}

		write = buffer.get_content(*block) + buffer.get_usingSize(*block);
		memcpy(write, &(attributes[i]), sizeof(Attribute));
		buffer.set_usingSize(*block, buffer.get_usingSize(*block) + sizeof(Attribute));
		buffer.set_dirty(*block);
	}
	return SUCCEED;
}

bool CatalogManager::add_table_to_list(const std::string& name, const std::string& key, int attr_num)
{
	fileNode *file = buffer.get_File("Table.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	Table new_table(name, key, attr_num, 0);
	while (1)
	{
		if (buffer.get_usingSize(*block) + sizeof(Table) <= buffer.get_BlockSize()) break;
		block = buffer.get_NextBlock(file, block);
		if (!block) return FAIL;
	}

	char* write = buffer.get_content(*block) + buffer.get_usingSize(*block);
	memcpy(write, &new_table, sizeof(Table));
	buffer.set_usingSize(*block, buffer.get_usingSize(*block) + sizeof(Table));
	buffer.set_dirty(*block);
	return SUCCEED;
}

bool new_file(const std::string& name)
{
	fstream file;
	file.open(name, ios::out);
	if (!file) return FAIL;
	else return SUCCEED;
}

bool CatalogManager::drop_table(const std::string& name)
{
	bool feedback = delete_table_from_list(name);
	if (!feedback) return FAIL;

	vector<string> table_index;
	feedback = get_index_name(name, table_index);
	if (!feedback) return FAIL;

	int index_num = table_index.size();
	for (int i = 0; i < index_num; i++)
	{
		feedback = drop_index(table_index[i]);
		if (!feedback) return FAIL;
	}
	feedback = drop_table_file(name);
	return feedback;
}

bool CatalogManager::get_index_name(const std::string& table_name, std::vector<std::string>& index_names)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Index* index_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return SUCCEED;
		index_info = (Index*)read;
		if (index_info->get_table() == table_name) index_names.push_back(index_info->get_name());

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) break;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
	return SUCCEED;
}

bool CatalogManager::get_all_index(const std::string& table_name, std::vector<Index>& indexes)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Index* index_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return SUCCEED;
		index_info = (Index*)read;
		if (index_info->get_table() == table_name) indexes.push_back(*index_info);

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) break;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
	return SUCCEED;
}

bool CatalogManager::get_all_index(std::vector<Index>& indexes)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Index* index_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return SUCCEED;
		index_info = (Index*)read;
		indexes.push_back(*index_info);

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) break;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
	return SUCCEED;
}


bool CatalogManager::get_attribute(const std::string& table_name, vector<Attribute>& attributes)
{
	string filename = table_name + ".dat";
	fileNode *file = buffer.get_File(filename.c_str());
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Attribute* attr_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return SUCCEED;
		attr_info = (Attribute*)read;
		attributes.push_back(*attr_info);

		if ((read + sizeof(Attribute) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Attribute);
		else
		{
			if (block->ifbottom) break;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
	return SUCCEED;
}

int CatalogManager::get_record_size(const std::string& table_name)
{
	vector<Attribute> attributes;
	get_attribute(table_name, attributes);

	int record_size = 0;
	int i = 0, size = attributes.size();
	for(; i < size; i++) record_size += get_type_size(attributes[i].get_type());

	return record_size;
}

bool CatalogManager::delete_table_from_list(const std::string& name)
{
	fileNode *file = buffer.get_File("Table.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Table* table_info;
	char* last_table;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FALSE;
		table_info = (Table*)read;
		if (table_info->get_name() == name)
		{
			last_table = read;
			while (1)
			{
				if (last_table + sizeof(Table) - buffer.get_content(*block) >= buffer.get_usingSize(*block)) break;
				else last_table += sizeof(Table);
			}

			memcpy(read, last_table, sizeof(Table));
			buffer.set_usingSize(*block, buffer.get_usingSize(*block) - sizeof(Table));
			buffer.set_dirty(*block);
			return SUCCEED;
		}

		if ((read + sizeof(Table) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Table);
		else
		{
			if (block->ifbottom) return FAIL;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

bool CatalogManager::drop_table_file(const std::string& name)
{
	string filename = name + ".dat";
	buffer.delete_fileNode(filename.c_str());
	int feedback = remove(filename.c_str());
	if (!feedback) return SUCCEED;
	else return FAIL;
}

bool CatalogManager::create_index(const std::string& name, const std::string& table_name, const std::string& attr_name, attr_type type)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	Index new_index(name, table_name, attr_name, type);
	while (1)
	{
		if (buffer.get_usingSize(*block) + sizeof(Index) <= buffer.get_BlockSize()) break;
		block = buffer.get_NextBlock(file, block);
		if (!block) return FAIL;
	}

	char* write = buffer.get_content(*block) + buffer.get_usingSize(*block);
	memcpy(write, &new_index, sizeof(Index));
	buffer.set_usingSize(*block, buffer.get_usingSize(*block) + sizeof(Index));
	buffer.set_dirty(*block);
	return SUCCEED;
}

bool CatalogManager::drop_index(const std::string& name)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Index* index_info;
	char* last_index;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FAIL;
		index_info = (Index*)read;
		if (index_info->get_name() == name)
		{
			last_index = read;
			while (1)
			{
				if (last_index + sizeof(Index) - buffer.get_content(*block) >= buffer.get_usingSize(*block)) break;
				else last_index += sizeof(Index);
			}

			memcpy(read, last_index, sizeof(Index));
			buffer.set_usingSize(*block, buffer.get_usingSize(*block) - sizeof(Index));
			buffer.set_dirty(*block);
			return SUCCEED;
		}

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) return FAIL;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

bool CatalogManager::add_record(const std::string& table_name)
{
	fileNode *file = buffer.get_File("Table.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Table* table_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FAIL;
		table_info = (Table*)read;
		if (table_info->get_name() == table_name)
		{
			table_info->add_rec();
			return SUCCEED;
		}

		if ((read + sizeof(Table) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Table);
		else
		{
			if (block->ifbottom) return FAIL;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

bool CatalogManager::delete_record(const std::string& table_name, int delete_num)
{
	fileNode *file = buffer.get_File("Table.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Table* table_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FAIL;
		table_info = (Table*)read;
		if (table_info->get_name() == table_name) return table_info->reduce_rec(delete_num);

		if ((read + sizeof(Table) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Table);
		else
		{
			if (block->ifbottom) return FAIL;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

int CatalogManager::get_record_num(const std::string& table_name)
{
	fileNode *file = buffer.get_File("Table.dat");
	blockNode *block = buffer.get_BlockHead(file);

	int record_num = -1;
	char* read = buffer.get_content(*block);
	Table* table_info;
	while(1)
	{
		if(!buffer.get_usingSize(*block)) break;
		table_info = (Table*)read;
		if(table_info->get_name() == table_name)
		{
			record_num = table_info->get_rec_num();
			return record_num;
		}

		if((read + sizeof(Table) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Table);
		else
		{
			if(block->ifbottom) break;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
	return record_num;
}

int CatalogManager::is_unique(const std::string& table_name, const std::string& attr_name)
{
	string file_name = table_name + ".dat";
	fileNode *file = buffer.get_File(file_name.c_str());
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Attribute* attr_info;
	while (1)
	{
		attr_info = (Attribute*)read;
		if (attr_info->get_name() == attr_name)
		{
			if (attr_info->is_unique()) return TRUE;
			else return FALSE;
		}

		if ((read + sizeof(Attribute) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Attribute);
		else
		{
			if (block->ifbottom) return FAIL;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

int CatalogManager::is_table_exist(const std::string& name)
{
	fileNode *file = buffer.get_File("Table.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Table* table_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FALSE;
		table_info = (Table*)read;
		if (table_info->get_name() == name) return TRUE;

		if ((read + sizeof(Table) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Table);
		else
		{
			if (block->ifbottom) return FALSE;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}
attr_type CatalogManager::get_index_type(const std::string& name)
{
	fileNode *file = buffer.get_File("Index.dat");
	blockNode *block = buffer.get_BlockHead(file);

	char* read = buffer.get_content(*block);
	Index* index_info;
	attr_type type = 0;
	while(1)
	{
		if(!buffer.get_usingSize(*block)) return type;
		index_info = (Index*)read;
		if(index_info->get_name() == name) return index_info->get_type();

		if((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if(block->ifbottom) return type;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

std::string CatalogManager::get_index(const std::string& table_name, const std::string& attr_name)
{
	fileNode *file = buffer.get_File("Index.dat");
	blockNode *block = buffer.get_BlockHead(file);

	string index_name = "";
	char* read = buffer.get_content(*block);
	Index* index_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) break;
		index_info = (Index*)read;
		if (index_info->get_table() == table_name && index_info->get_attr() == attr_name)
		{
			index_name = index_info->get_name();
			break;
		}

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) break;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
	return index_name;
}

int CatalogManager::is_index_true(const std::string& table_name, const std::string& attr_name, const std::string& index_name)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Index* index_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FALSE;
		index_info = (Index*)read;
		if (index_info->get_table() == table_name && index_info->get_attr() == attr_name)
		{
			if (index_info->get_name() == index_name) return TRUE;
			else return FALSE;
		}

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) return FALSE;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

int CatalogManager::is_index_exist(const std::string& name)
{
	fileNode *file = buffer.get_File("Index.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Index* index_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FALSE;
		index_info = (Index*)read;
		if (index_info->get_name() == name) return TRUE;

		if ((read + sizeof(Index) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Index);
		else
		{
			if (block->ifbottom) return FALSE;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

int CatalogManager::is_attr_exist(const std::string& table_name, const std::string& attr_name)
{
	string file_name = table_name + ".dat";
	fileNode *file = buffer.get_File(file_name.c_str());
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Attribute* attr_info;
	while (1)
	{
		attr_info = (Attribute*)read;
		if (attr_info->get_name() == attr_name) return TRUE;

		if ((read + sizeof(Attribute) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Attribute);
		else
		{
			if (block->ifbottom) return FALSE;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

bool CatalogManager::get_key(const std::string& table_name, std::string& key)
{
	fileNode *file = buffer.get_File("Table.dat");
	if (!file) return FAIL;
	blockNode *block = buffer.get_BlockHead(file);
	if (!block) return FAIL;

	char* read = buffer.get_content(*block);
	Table* table_info;
	while (1)
	{
		if (!buffer.get_usingSize(*block)) return FAIL;
		table_info = (Table*)read;
		if (table_info->get_name() == table_name)
		{
			key = table_info->get_key();
			return SUCCEED;
		}

		if ((read + sizeof(Table) - buffer.get_content(*block)) < buffer.get_usingSize(*block)) read += sizeof(Table);
		else
		{
			if (block->ifbottom) return FAIL;
			block = buffer.get_NextBlock(file, block);
			read = buffer.get_content(*block);
		}
	}
}

int CatalogManager::get_type_size(attr_type type)
{
	if (type == Attribute::TYPE_INT) return sizeof(int);
	else if (type == Attribute::TYPE_FLOAT) return sizeof(float);
	else return type * sizeof(char);
}

bool CatalogManager::get_record(const std::string& table_name, vector<string>& record_content, char* record)
{
	vector<Attribute> attributes;
	bool feedback = get_attribute(table_name, attributes);
	if (feedback == FAIL) return FAIL;

	char * read = record;
	for (int i = 0; i < attributes.size(); i++)
	{
		string content = record_content[i];
		attr_type type = attributes[i].get_type();
		int type_size = get_type_size(type);
		stringstream ss;
		ss << content;
		if (type == Attribute::TYPE_INT)
		{
			int int_t;
			ss >> int_t;
			memcpy(read, ((char*)&int_t), type_size);
		}
		else if (type == Attribute::TYPE_FLOAT)
		{
			float float_t;
			ss >> float_t;
			memcpy(read, ((char*)&float_t), type_size);
		}
		else memcpy(read, content.c_str(), type_size);

		read += type_size;
	}
	return SUCCEED;
}