#include <algorithm>
#include <exception>
#include "IndexManager.h"
#include "Index.h"
#include "API.h"

extern API api;

template <>
inline void BPTree<string>::readBlock(blockNode *bi)
{
	unsigned char* keyBegin = reinterpret_cast<unsigned char *>(bm.get_content(*bi));
	unsigned char* offsetBegin = keyBegin + sizeofKey;
	int *offset;
	while(offsetBegin + sizeof(int) <= reinterpret_cast<unsigned char *>(bm.get_content(*bi)) + bm.get_UsingSize(*bi)) {
		// there are available position in the block
		// reinterpret memory
		// a naive parse method!

		char buf[300];
		memcpy(buf, reinterpret_cast<char *>(keyBegin), sizeofKey);
		buf[sizeofKey] = 0;
		string key = buf; // assignment

		offset = reinterpret_cast<int *>(offsetBegin);
		insert(key, *offset);
		keyBegin += sizeofKey + sizeof(int);
		offsetBegin += sizeofKey + sizeof(int);
	}

}

template <>
inline void BPTree<string>::dtorToFile()
{
	blockNode* bi = bm.get_BlockHead(fileHandler);
	BPTNodeLeaf<string, 4> *cur = dynamic_cast<BPTNodeLeaf<string, 4> *>(head);

	size_t unitSize = sizeofKey + sizeof(int);
	bm.set_usingSize(*bi, 0);
	bm.set_dirty(*bi);
	while(cur != nullptr)
	{
		for(int i = 0; i < cur->keysSize(); i++) {
			string keyTemp(cur->getKey(i));
			int offsetTemp(cur->getOffset(i));
			unsigned char *key = reinterpret_cast<unsigned char *>(&keyTemp);
			unsigned char *offset = reinterpret_cast<unsigned char *>(&offsetTemp);

			if(8192 - bm.get_UsingSize(*bi) - sizeof(size_t) < unitSize) {
				// next block!!! ??? !!!
				bi = bm.get_NextBlock(fileHandler, bi);
				bm.set_usingSize(*bi, 0);
			}
			bm.set_dirty(*bi);
			string tag = "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >";
			memcpy(bm.get_content(*bi) + bm.get_UsingSize(*bi), keyTemp.c_str(), sizeofKey);
			bm.set_usingSize(*bi, bm.get_UsingSize(*bi) + sizeofKey);

			memcpy(bm.get_content(*bi) + bm.get_UsingSize(*bi), offset, sizeof(int));
			bm.set_usingSize(*bi, bm.get_UsingSize(*bi) + sizeof(int));
		}

		cur = cur->getNextSubling();
	}
}


IndexManager::IndexManager()
{
	vector<Index> allIndex;
	api.allIndexAddressInfoGet(&allIndex);
	for_each(allIndex.begin(), allIndex.end(), [=](const Index& i) { create(i.get_name(), i.get_type());});
}

// write to disk && delete memory
// implemented by B+Tree 
// here is trival
IndexManager::~IndexManager()
{
	try {
		for_each(intMap.begin(), intMap.end(),
			[](const auto& i) { delete i.second; });
		for_each(floatMap.begin(), floatMap.end(),
			[](const auto& i) { delete i.second; });
		for_each(charMap.begin(), charMap.end(),
			[](const auto& i) { delete i.second; });
	} catch(const exception& e) {
		cerr << "-----------------" << endl;
		cerr << "exception in IndexManager::~IndexManager()" << endl
			<< e.what() << endl;
		cerr << "-----------------" << endl;
	}
}

// fileName == indexName
bool IndexManager::create(const string& fileName, attr_type type)
{
	bool ret = true;
	try {
		if(type == TYPE_INT) {
			BPTree<int, D> *b = new BPTree<int, D>(fileName, sizeof(int));
			intMap.insert(make_pair(fileName, b));
		} else if(type == TYPE_FLOAT) {
			BPTree<float, D> *b = new BPTree<float, D>(fileName, sizeof(float));
			floatMap.insert(make_pair(fileName, b));
		} else {
			BPTree<string, D> *b = new BPTree<string, D>(fileName, sizeof(char) * type);
			charMap.insert(make_pair(fileName, b));
		}
	} catch(const exception& e) {
		cerr << "-----------------" << endl;
		cerr << "exception in IndexManager::create()" << endl
			<< e.what() << endl;
		cerr << "fileName -> " << fileName << endl;
		cerr << "-----------------" << endl;
		ret = false;
	}

	return ret;
}

bool IndexManager::drop(const string& fileName, attr_type type)
{
	bool ret = true;
	try {
		if(type == TYPE_INT) {
			auto res = intMap.find(fileName);
			if(res == intMap.end()) {
				cout << "Error:in drop index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				delete res->second;
				intMap.erase(fileName);
			}
		} else if(type == TYPE_FLOAT) {
			auto res = floatMap.find(fileName);
			if(res == floatMap.end()) {
				cout << "Error:in drop index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				
				delete res->second;
				floatMap.erase(fileName);
			}
		} else {
			auto res = charMap.find(fileName);
			if(res == charMap.end()) {
				cout << "Error:in drop index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				
				delete res->second;
				charMap.erase(fileName);
			}
		}
	} catch(const exception& e) {
		cerr << "-----------------" << endl;
		cerr << "exception in IndexManager::drop()" << endl
			<< e.what() << endl;
		cerr << "fileName -> " << fileName << endl;
		cerr << "-----------------" << endl;
		ret = false;
	}
	return ret;
}

// @return 
// offset of recodes in db file
// -1 for search failure / don't exist v
int IndexManager::search(const string& fileName, const Value& v)
{
	int ret = -1;
	try {
		if(v.type == TYPE_INT) {
			auto res = intMap.find(fileName);
			if(res == intMap.end()) {
				cout << "Error:in search index, no index " << fileName << " exits" << endl;
				ret = -1;
			} else if(res->second->find(v.i, ret)) {
				ret = ret; // exist
			} else {
				ret = -1;  // don't exist 
			}
		} else if(v.type == TYPE_FLOAT) {
			auto res = floatMap.find(fileName);
			if(res == floatMap.end()) {
				cout << "Error:in search index, no index " << fileName << " exits" << endl;
				ret = -1;
			} else if(res->second->find(v.f, ret)) {
				ret = ret; // exist
			} else {
				ret = -1;  // don't exist 
			}
		} else {
			auto res = charMap.find(fileName);
			if(res == charMap.end()) {
				cout << "Error:in search index, no index " << fileName << " exits" << endl;
				ret = -1;
			} else if(res->second->find(v.str, ret)) {
				ret = ret; // exist
			} else {
				ret = -1;  // don't exist 
			}
		}
	} catch(const exception& e) {
		cerr << "-----------------" << endl;
		cerr << "exception in IndexManager::search()" << endl
			<< e.what() << endl;
		cerr << "fileName -> " << fileName << endl;
		cerr << "-----------------" << endl;
		ret = false;
	}

	return ret;
}

bool IndexManager::insert(const string& fileName, const Value& v, int offset)
{
	bool ret = true;
	try {
		if(v.type == TYPE_INT) {
			auto res = intMap.find(fileName);
			if(res == intMap.end()) {
				cout << "Error:in insert index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				res->second->insert(v.i, offset);
			}
		} else if(v.type == TYPE_FLOAT) {
			auto res = floatMap.find(fileName);
			if(res == floatMap.end()) {
				cout << "Error:in insert index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				res->second->insert(v.f, offset);
			}
		} else {
			auto res = charMap.find(fileName);
			if(res == charMap.end()) {
				cout << "Error:in insert index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				res->second->insert(v.str, offset);
			}
		}
	} catch(const exception& e) {
		cerr << "-----------------" << endl;
		cerr << "exception in IndexManager::insert()" << endl
			<< e.what() << endl;
		cerr << "fileName -> " << fileName << endl;
		cerr << "-----------------" << endl;
		ret = false;
	}

	return ret;
}

bool IndexManager::erase(const string& fileName, const Value& v)
{
	bool ret = true;
	try {
		if(v.type == TYPE_INT) {
			auto res = intMap.find(fileName);
			if(res == intMap.end()) {
				cout << "Error:in erase index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				res->second->erase(v.i);
			}
		} else if(v.type == TYPE_FLOAT) {
			auto res = floatMap.find(fileName);
			if(res == floatMap.end()) {
				cout << "Error:in erase index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				res->second->erase(v.f);
			}
		} else {
			auto res = charMap.find(fileName);
			if(res == charMap.end()) {
				cout << "Error:in erase index, no index " << fileName << " exits" << endl;
				ret = false;
			} else {
				res->second->erase(v.str);
			}
		}
	} catch(const exception& e) {
		cerr << "-----------------" << endl;
		cerr << "exception in IndexManager::erase()" << endl
			<< e.what() << endl;
		cerr << "fileName -> " << fileName << endl;
		cerr << "-----------------" << endl;
		ret = false;
	}

	return ret;
}