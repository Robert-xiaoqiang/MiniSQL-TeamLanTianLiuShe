#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H
#include <string>
#include <vector>
#include "Condition.h"
#include "Attribute.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include "Minisql.h"
using namespace std;
class API;
class RecordManager {
public:
	RecordManager() {}
	BufferManager buffer;
	API *api;

	bool tableCreate(string tableName);
	bool tableDrop(string tableName);

	bool indexDrop(string indexName);
	bool indexCreate(string indexName);

	int recordInsert(string tableName, char* record, int recordSize);

	int recordAllShow(string tableName, vector<string>* arrayOfAttributeName, vector<Condition>* arrayOfCondition);
	int recordBlockShow(string tableName, vector<string>* arrayOfAttributeName, vector<Condition>* arrayOfCondition, int blockOffset);

	int recordAllFind(string tableName, vector<Condition>* arrayOfCondition);

	int recordAllDelete(string tableName, vector<Condition>* arrayOfCondition);
	int recordBlockDelete(string tableName, vector<Condition>* arrayOfCondition, int blockOffset);

	int indexRecordAllAlreadyInsert(string tableName, string indexName, string attributeName);

	string tableFileNameGet(string tableName);
	string indexFileNameGet(string indexName);
private:
	int recordBlockShow(string tableName, vector<string>* arrayOfAttributeName, vector<Condition>* arrayOfCondition, blockNode* block);
	int recordBlockFind(string tableName, vector<Condition>* arrayOfCondition, blockNode* block);
	int recordBlockDelete(string tableName, vector<Condition>* arrayOfCondition, blockNode* block);
	int indexRecordBlockAlreadyInsert(string tableName, string indexName, blockNode* block, string attributeName);

	bool recordConditionFit(char* recordBegin, int recordSize, vector<Attribute>* arrayOfAttribute, vector<Condition>* arrayOfCondition);
	void recordPrint(char* recordBegin, int recordSize, vector<Attribute>* arrayOfAttribute, vector<string> *arrayOfAttributeName);
	bool contentConditionFit(char* content, int type, Condition* condition);
	void contentPrint(char * content, int type);

	char* transferString(string x);
};
#endif
