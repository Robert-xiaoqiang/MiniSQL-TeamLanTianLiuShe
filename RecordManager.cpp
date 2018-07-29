#include <iostream>
#include <cstring>
#include <fstream>
#include "RecordManager.h"
#include "API.h"
#define ContentSize 255
#pragma warning(disable:4996)
using namespace std;
/**
* get a index's file name
* indexName: name of index
*/
string RecordManager::indexFileNameGet(string indexName)
{
	string tmp = "";
	return "INDEX_FILE_" + indexName;
}

/**
* get a table's file name
* tableName: name of table
*/
string RecordManager::tableFileNameGet(string tableName)
{
	string tmp = "";
	return tmp + "TABLE_FILE_" + tableName;
}

/**
*
* print value of record
* recordBeginAddress: point to a record
* recordSize: size of the record
* arrayOfAttribute: the attribute list of the record
* arrayOfAttribute: the name list of all attribute you want to print
*/
void RecordManager::recordPrint(char* recordBeginAddress, int recordSize, vector<Attribute>* arrayOfAttribute, vector<string> *arrayOfAttributeName)
{
	int typeSize;
	int type;
	char content[ContentSize];
	string attributeName;
	char *contentBeginAddress = recordBeginAddress;
	int repeatmax = arrayOfAttribute->size();
	//printf("%d", repeatmax);
	for (int i = 0; i < repeatmax; i++) {
		type = (*arrayOfAttribute)[i].get_type();
		typeSize = api->typeSizeGet(type);
		//init content (when content is string , we can get a string easily)
		memset(content, 0, ContentSize);
		memcpy(content, contentBeginAddress, typeSize);
		//printf("%d\n", (*arrayOfAttributeName).size());
		for (int k = 0; k < (*arrayOfAttributeName).size(); k++) {
			if ((*arrayOfAttributeName)[k] == (*arrayOfAttribute)[i].get_name()) {
				contentPrint(content, type);
				//printf("a");
				break;
			}
		}
		contentBeginAddress += typeSize;
	}
}

char* RecordManager::transferString(string x) {
	char* p;
	p = (char*)malloc(sizeof(char)*x.size());
	strcpy(p, x.c_str());
	return p;
}

/**
* insert a record to table
* tableName: name of table
* record: value of record
* recordSize: size of the record
*
* return: the position of block in the file(-1 represent error)
*/
int RecordManager::recordInsert(string tableName, char* record, int recordSize)
{
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode *ptrToBlockNode = buffer.get_BlockHead(ptrToFileNode);
	int ret;
	while (true) {
		if (ptrToBlockNode == NULL) {
			ret = -1;
			break;
		}
		if (buffer.get_usingSize(*ptrToBlockNode) <= buffer.get_BlockSize() - recordSize) {

			char* beginAddress;
			/*get the position where we write the record*/
			beginAddress = buffer.get_content(*ptrToBlockNode) + buffer.get_usingSize(*ptrToBlockNode);
			memcpy(beginAddress, record, recordSize);
			/*change the usingsize of the block and give the block a symbol that the block has changed*/
			buffer.set_usingSize(*ptrToBlockNode, buffer.get_usingSize(*ptrToBlockNode) + recordSize);
			buffer.set_dirty(*ptrToBlockNode);
			/*return the offset of the block*/
			ret = ptrToBlockNode->offsetNum;
			break;
		}
		else {
			ptrToBlockNode = buffer.get_NextBlock(ptrToFileNode, ptrToBlockNode);
		}
	}
	return ret;
}
/**
* create a table
* tableName: name of table
*/
bool RecordManager::tableCreate(string tableName)
{
	string tableFileName = tableFileNameGet(tableName);
	fstream file;
	file.open(tableFileName, ios::out);
	if (!file) {
		printf("create file faied");
		return false;
	}
	file.close();
	return true;
}

/**
* drop a table
* tableName: name of table
*/
bool RecordManager::tableDrop(string tableName)
{
	string tableFileName = tableFileNameGet(tableName);//change tablename to indicate it's another kind of file
	char* name = transferString(tableFileName);
	buffer.delete_fileNode(name);//BufferManager buffer; 
	if (remove(name)) {
		printf("cant find the table_file");
		return false;
	}
	return true;
}

/**
* create a index
* indexName: name of index
*/
bool RecordManager::indexCreate(string indexName)
{
	string indexFileName = indexFileNameGet(indexName);
	fstream file;
	file.open(indexFileName, ios::out);
	if (!file) {
		return false;
	}
	file.close();
	return true;
}

/**
* drop a index
* indexName: name of index
*/
/*ряееЁЩ*/
bool RecordManager::indexDrop(string indexName)
{
	string indexFileName = indexFileNameGet(indexName);
	char* name;
	name = transferString(indexFileName);
	buffer.delete_fileNode(name);
	if (remove(name)) {
		return false;
	}
	return true;
}

/**
* print all record of a table meet requirement
* tableName: name of table
* arrayOfAttributeName: the attribute list
* arrayOfCondition: the conditions list
*
* return: the number of the record meet requirements(-1 represent error)
*/
int RecordManager::recordAllShow(string tableName, vector<string>* arrayOfAttributeName, vector<Condition>* arrayOfCondition)
{
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode *ptrToBlockNode = buffer.get_BlockHead(ptrToFileNode);
	int count = 0;
	int ret = -1;
	while (true) {
		if (ptrToBlockNode == NULL) {
			ret = -1;
			break;
		}
		/*if the block is last block of the file*/
		if (ptrToBlockNode->ifbottom) {
			int recordBlockNum = recordBlockShow(tableName, arrayOfAttributeName, arrayOfCondition, ptrToBlockNode);
			count += recordBlockNum;
			ret = count;
			break;
		}
		/*if the block is not the last block of the file*/
		else {
			int recordBlockNum = recordBlockShow(tableName, arrayOfAttributeName, arrayOfCondition, ptrToBlockNode);
			count += recordBlockNum;
		}
		ptrToBlockNode = buffer.get_NextBlock(ptrToFileNode, ptrToBlockNode);
	}
	return ret;
}

/**
*
* print record of a table in a block
* tableName: name of table
* arrayOfAttributeName: the attribute list
* arrayOfCondition: the conditions list
* blockOffset: the block's offsetNum
* return int: the number of the record meet requirements in the block(-1 represent error)
*/
int RecordManager::recordBlockShow(string tableName, vector<string>* arrayOfAttributeName, vector<Condition>* arrayOfCondition, int blockOffset)
{
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode* block = buffer.get_BlockByOffset(ptrToFileNode, blockOffset);
	int ret = -1;
	if (block == NULL) {
		ret = -1;
	}
	else {
		ret = recordBlockShow(tableName, arrayOfAttributeName, arrayOfCondition, block);
	}
	return ret;
}

/**
* print record of a table in a block
* tableName: name of table
* arrayOfAttributeName: the attribute list
* arrayOfCondition: the conditions list
* block: search record in the block
* return int: the number of the record meet requirements in the block(-1 represent error)
*/
int RecordManager::recordBlockShow(string tableName, vector<string>* arrayOfAttributeName, vector<Condition>* arrayOfCondition, blockNode* block)
{
	//if block is null, return -1
	printf("\n\n\n1: %d\n\n\n", (*arrayOfAttributeName).size());

	if (block == NULL) {
		return -1;
	}
	int count = 0;
	//get the position of the record starting and initialize it the position of the block starting
	char* recordBeginAddress = buffer.get_content(*block);
	//get the attribute of the table
	vector<Attribute> arrayOfAttribute;
	api->attributeGet(tableName, &arrayOfAttribute);
	//get the using size of the block
	size_t usingSize = buffer.get_usingSize(*block);
	//get the position of the block starting
	char* blockBegin = buffer.get_content(*block);
	int recordSize = api->recordSizeGet(tableName);

	while (recordBeginAddress - blockBegin  < usingSize) {
		//if the recordBeginAddress point to a record
		if (recordConditionFit(recordBeginAddress, recordSize, &arrayOfAttribute, arrayOfCondition)) {
			count++;
			recordPrint(recordBeginAddress, recordSize, &arrayOfAttribute, arrayOfAttributeName);
			cout << endl;
		}
		recordBeginAddress += recordSize;
	}
	return count;
}

/**
* find the number of all record of a table meet requirement
* tableName: name of table
* arrayOfCondition: the conditions list
* return int: the number of the record meet requirements(-1 represent error)
*/
int RecordManager::recordAllFind(string tableName, vector<Condition>* arrayOfCondition)
{
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode *ptrToBlockNode = buffer.get_BlockHead(ptrToFileNode);
	int recordBlockNum;
	int count = 0;
	int ret = -1;
	while (true) {
		if (ptrToBlockNode == NULL) {
			ret = -1;
			break;
		}
		/*if the block is the last block of the file*/
		if (ptrToBlockNode->ifbottom) {
			recordBlockNum = recordBlockFind(tableName, arrayOfCondition, ptrToBlockNode);
			count += recordBlockNum;
			ret = count;
			break;
		}
		else {
			recordBlockNum = recordBlockFind(tableName, arrayOfCondition, ptrToBlockNode);
			count += recordBlockNum;
			ptrToBlockNode = buffer.get_NextBlock(ptrToFileNode, ptrToBlockNode);
		}
	}
	return ret;
}
/**
* find the number of record of a table in a block
* tableName: name of table
* block: search record in the block
* arrayOfCondition: the conditions list
* return int: the number of the record meet requirements in the block(-1 represent error)
*/
int RecordManager::recordBlockFind(string tableName, vector<Condition>* arrayOfCondition, blockNode* block)
{
	//if block is null, return -1
	if (block == NULL) {
		return -1;
	}
	int count = 0;
	char* recordBeginAddress = buffer.get_content(*block);
	vector<Attribute> arrayOfAttribute;
	int recordSize = api->recordSizeGet(tableName);
	api->attributeGet(tableName, &arrayOfAttribute);
	while (recordBeginAddress - buffer.get_content(*block)  < buffer.get_usingSize(*block)) {
		//if the recordBeginAddress point to a record
		if (recordConditionFit(recordBeginAddress, recordSize, &arrayOfAttribute, arrayOfCondition)) {
			count++;
		}
		recordBeginAddress += recordSize;
	}
	return count;
}

/**
* delete all record of a table meet requirement
* tableName: name of table
* arrayOfCondition: the conditions list
* return int: the number of the record meet requirements(-1 represent error)
*/
int RecordManager::recordAllDelete(string tableName, vector<Condition>* arrayOfCondition)
{
	int count = 0;
	int ret=-1;
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode *ptrToBlockNode = buffer.get_BlockHead(ptrToFileNode);
	do {
		if (ptrToBlockNode == NULL) {
			ret = -1;
			break;
		}
		if (ptrToBlockNode->ifbottom) {
			int recordBlockNum = recordBlockDelete(tableName, arrayOfCondition, ptrToBlockNode);
			count += recordBlockNum;
			ret = count;
			break;
		}
		else {
			int recordBlockNum = recordBlockDelete(tableName, arrayOfCondition, ptrToBlockNode);
			count += recordBlockNum;
			ptrToBlockNode = buffer.get_NextBlock(ptrToFileNode, ptrToBlockNode);
		}
	} while (true);
	return ret;
}

/**
* delete record of a table in a block
* tableName: name of table
* arrayOfCondition: the conditions list
* blockOffset: the block's offsetNum
* return int: the number of the record meet requirements in the block(-1 represent error)
*/
int RecordManager::recordBlockDelete(string tableName, vector<Condition>* arrayOfCondition, int blockOffset)
{
	int ret;
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode* block = buffer.get_BlockByOffset(ptrToFileNode, blockOffset);
	if (block == NULL) {
		ret = -1;
	}
	else {
		ret = recordBlockDelete(tableName, arrayOfCondition, block);
	}
	return ret;
}

/**
* delete record of a table in a block
* tableName: name of table
* arrayOfCondition: the conditions list
* block: search record in the block
* return int: the number of the record meet requirements in the block(-1 represent error)
*/
/*ряееЁЩ*/
int RecordManager::recordBlockDelete(string tableName, vector<Condition>* arrayOfCondition, blockNode* block)
{
	//if block is null, return -1
	if (block == NULL) {
		return -1;
	}
	char* recordBeginAddress = buffer.get_content(*block);
	vector<Attribute> arrayOfAttribute;
	int recordSize = api->recordSizeGet(tableName);
	api->attributeGet(tableName, &arrayOfAttribute);
	int count = 0;
	while (recordBeginAddress - buffer.get_content(*block) < buffer.get_usingSize(*block)) {
		//if the recordBeginAddress point to a record
		if (recordConditionFit(recordBeginAddress, recordSize, &arrayOfAttribute, arrayOfCondition)) {
			api->recordIndexDelete(recordBeginAddress, recordSize, &arrayOfAttribute, block->offsetNum, tableName);
			count++;
			int i = 0;
			while (i + recordSize + recordBeginAddress - buffer.get_content(*block) < buffer.get_usingSize(*block)) {
				recordBeginAddress[i] = recordBeginAddress[i + recordSize];
				i++;
			}
			memset(recordBeginAddress + i, 0, recordSize);
			buffer.set_usingSize(*block, buffer.get_usingSize(*block) - recordSize);
			buffer.set_dirty(*block);
		}
		else {
			recordBeginAddress += recordSize;
		}
	}
	return count;
}

/**
*
* insert the index of all record of the table
* tableName: name of table
* indexName: name of index
* return int: the number of the record meet requirements(-1 represent error)
*/
/*ееЁЩ*/
int RecordManager::indexRecordAllAlreadyInsert(string tableName, string indexName, string attributeName) {
	fileNode *ptrToFileNode = buffer.get_File(transferString(tableFileNameGet(tableName)));
	blockNode *ptrToBlockNode = buffer.get_BlockHead(ptrToFileNode);
	int count = 0;
	int ret = -1;
	while(true) {
		if(ptrToBlockNode == NULL) {
			ret = -1;
			break;
		}
		if(ptrToBlockNode->ifbottom) {
			int recordBlockNum = indexRecordBlockAlreadyInsert(tableName, indexName, ptrToBlockNode, attributeName);
			count += recordBlockNum;
			ret = count;
			break;
		} else {
			int recordBlockNum = indexRecordBlockAlreadyInsert(tableName, indexName, ptrToBlockNode, attributeName);
			count += recordBlockNum;
			ptrToBlockNode = buffer.get_NextBlock(ptrToFileNode, ptrToBlockNode);
		}
	}
	return ret;
}



/**
* insert the index of a record of a table in a block
* tableName: name of table
* indexName: name of index
* block: search record in the block
* return int: the number of the record meet requirements in the block(-1 represent error)
*/
/*ееЁЩ*/
int RecordManager::indexRecordBlockAlreadyInsert(string tableName, string indexName, blockNode* block, string attributeName)
{
	//if block is null, return -1
	if(block == NULL) {
		return -1;
	}
	char* recordBeginAddress = buffer.get_content(*block);
	vector<Attribute> arrayOfAttribute;
	int recordSize = api->recordSizeGet(tableName);
	api->attributeGet(tableName, &arrayOfAttribute);
	int type;
	int typeSize;
	int count = 0;
	char* contentBeginAddress;

	while(recordBeginAddress - buffer.get_content(*block) < buffer.get_usingSize(*block)) {
		//if the recordBeginAddress point to a record
		contentBeginAddress = recordBeginAddress;
		for(int i = 0; i < arrayOfAttribute.size(); i++) {
			type = arrayOfAttribute[i].get_type();
			typeSize = api->typeSizeGet(type);
			//find the index  of the record, and insert it to index tree

			//if (api->isIndexTrue(tableName,arrayOfAttribute[i].get_name(),indexName) && ) {
			if(arrayOfAttribute[i].get_name() == attributeName) {
				count++;
				int offset = block->offsetNum;
				api->indexInsert(indexName, contentBeginAddress, type, offset);
			}
			contentBeginAddress += typeSize;
		}
		recordBeginAddress += recordSize;
		
	}
	return count;
}


/**
* judge if the record meet the requirement
* recordBeginAddress: point to a record
* recordSize: size of the record
* arrayOfAttribute: the attribute list of the record
* arrayOfCondition: the conditions
* bool: if the record fit the condition
*/
bool RecordManager::recordConditionFit(char* recordBeginAddress, int recordSize, vector<Attribute>* arrayOfAttribute, vector<Condition>* arrayOfCondition)
{
	if (arrayOfCondition == NULL) {
		return true;
	}
	int typeSize;
	int type;
	char content[ContentSize];
	string attributeName;
	char *contentBeginAddress = recordBeginAddress;
	for (int i = 0; i < arrayOfAttribute->size(); i++) {
		type = (*arrayOfAttribute)[i].get_type();
		typeSize = api->typeSizeGet(type);

		//init content (when content is string , we can get a string easily)
		memset(content, 0, ContentSize);
		memcpy(content, contentBeginAddress, typeSize);
		attributeName = (*arrayOfAttribute)[i].get_name();

		for (int j = 0; j < (*arrayOfCondition).size(); j++) {
			if ((*arrayOfCondition)[j].attributeName == attributeName) {
				//if this attribute need to deal about the condition
				if (!contentConditionFit(content, type, &(*arrayOfCondition)[j])) {
					//if this record is not fit the conditon
					return false;
				}
			}
		}
		contentBeginAddress += typeSize;
	}
	return true;
}

/**
*
* print value of content
* @param content: point to content
* @param type: type of content
*/
void RecordManager::contentPrint(char * content, int type)
{

	if (type == Attribute::TYPE_FLOAT) {
		//if the content is a float
		float tmp = *((float *)content);   //get content value by point
		//cout << tmp << " ";
		printf("| %-15.5f", tmp);
	}else if (type == Attribute::TYPE_INT) {
		//if the content is a int
		int tmp = *((int *)content);   //get content value by point
		//cout << tmp << " ";
		printf("| %-15d", tmp);
	}
	else {
		//if the content is a string
		string tmp = content;
		printf("| %-15s", transferString(tmp));
	}
}

/**
*
* judge if the content meet the requirement
* content: point to content
* type: type of content
* condition: condition
* bool: the content if meet
*/
bool RecordManager::contentConditionFit(char* content, int type, Condition* condition)
{
	bool ret = true;
	if (type == Attribute::TYPE_FLOAT) {
		//if the content is a float
		float tmp = *((float *)content);   //get content value by point
		ret = condition->ifRight(tmp);
	}else if (type == Attribute::TYPE_INT) {
		//if the content is a int
		int tmp = *((int *)content);   //get content value by point
		ret = condition->ifRight(tmp);
	}
	else {
		//if the content is a string
		ret = condition->ifRight(content);
	}
	return ret;
}
