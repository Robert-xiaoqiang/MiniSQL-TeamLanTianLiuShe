#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#include <string>
#include <vector>
#include "BufferManager.h"
#include "Attribute.h"
#include "Table.h"
#include "Index.h"

#define TRUE 1
#define FALSE -1

class CatalogManager
{
private:
	BufferManager buffer;
	bool create_table_file(const std::string& name, const std::vector<Attribute>& attributes);
	bool add_table_to_list(const std::string& name, const std::string& key, int attr_num);
	bool drop_table_file(const std::string& name);
	bool delete_table_from_list(const std::string& name);

public:
	CatalogManager();
	~CatalogManager();

	bool create_table(const std::string& name, const std::string& key, const std::vector<Attribute>& attributes);
	bool drop_table(const std::string& name);

	bool create_index(const std::string& name, const std::string& table_name, const std::string& attr_name, attr_type type);
	bool drop_index(const std::string& name);

	bool add_record(const std::string& table_name);
	bool delete_record(const std::string& table_name, int delete_num);

	int is_unique(const std::string& table_name, const std::string& attr_name);	//judge whether an attribute is unique 
	int is_table_exist(const std::string& name); //judge whether a table exists
	int is_attr_exist(const std::string& table_name, const std::string& attr_name); //judge whether an attribute in a specific table exists
	int is_index_exist(const std::string& name); //judge whether an index exists
	int is_index_true(const std::string& table_name, const std::string& attr_name, const std::string& index_name);

	bool get_key(const std::string& table_name, std::string& key); //get the primary key of a specific table
	int get_record_num(const std::string& table_name); //get the number of records in a specific table
	int get_record_size(const std::string& table_name); //get the size of every record in a specific table
	bool get_attribute(const std::string& table_name, vector<Attribute>& attributes); //get all attributes of a specific table
	bool get_index_name(const std::string& table_name, std::vector<std::string>& index_names); //get all indexes' names of a specific table
	attr_type get_index_type(const std::string& name);
	bool get_all_index(const std::string& table_name, std::vector<Index>& indexes);
	bool get_all_index(std::vector<Index>& indexes); //get all indexes
	int get_type_size(attr_type type);
	std::string get_index(const std::string& table_name, const std::string& attr_name);
	bool get_record(const std::string& table_name, vector<string>& record_content, char* record);
};

bool new_file(const std::string& name);


#endif