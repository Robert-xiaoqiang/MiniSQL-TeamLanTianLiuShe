#ifndef TABLE_H
#define TABLE_H

#include <string>

#define SUCCEED 1
#define FAIL 0

class Table
{
private:
	std::string name;
	std::string key;
	int attr_num;
	int rec_num;
public:
	Table(std::string new_name, std::string new_key, int new_attr_num, int new_rec_num);
	~Table();
	int get_attr_num() const { return attr_num; };
	int get_rec_num() const { return rec_num; };
	const std::string& get_name() const { return name; };
	const std::string& get_key() const { return key; };
	void add_rec() { rec_num++; return; };
	bool reduce_rec(int num) { if (rec_num < num) return FAIL; rec_num -= num; return SUCCEED; };
};

#endif

