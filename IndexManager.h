#ifndef INDEXMANAGER_H_
#define INDEXMANAGER_H_

#include <map>
#include <string>
#include <vector>
#include "BPTree.h"
#include "Attribute.h"

using namespace std;
#define  D  100

class IndexManager {
public:
	using IntMap = map<string, BPTree<int, D> *>;
	using FloatMap = map<string, BPTree<float, D> *>;
	using CharMap = map<string, BPTree<string, D> *>;
	
	const static int TYPE_FLOAT = -2;
	const static int TYPE_INT = -1;
	// others TYPE_CHAR

	IndexManager();
	virtual ~IndexManager();
	
	// true/flase -> successful/fail
	bool create(const string& fileName, attr_type type);
	bool drop(const string& fileName, attr_type type);
	bool insert(const string& fileName, const Value& v, int offset);
	bool erase(const string& fileName, const Value& v);

	//  -1/offset -> successful/fail
	int search(const string& fileName, const Value& v);
	
private:
	// rum-time memory index!
	IntMap intMap;
	FloatMap floatMap;
	CharMap  charMap;

};

#endif