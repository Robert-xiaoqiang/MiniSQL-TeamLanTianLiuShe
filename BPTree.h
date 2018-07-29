#ifndef BPTREE_H_
#define BPTREE_H_

#define _DEBUG_
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <typeinfo>
#include "Minisql.h"
#include "BufferManager.h"

using namespace std;

static BufferManager bm;

template <typename T, size_t DEGREE = 4>
class BPTNodeBase {
public:
	// T parameter
	using TYPE_INT = int;
	using TYPE_FLOAT = float;
	using TYPE_CHAR = string;

	BPTNodeBase(BPTNodeBase *parent = nullptr);
	virtual ~BPTNodeBase();

	bool search(const T &key, int &index) const;
	int findFirstLess(const T& key);
	bool isRoot() const;

	int keysSize() const
	{
		return keys.size();
	}

	void setParent(BPTNodeBase *parent)
	{
		this->parent = parent;
	}

	BPTNodeBase<T, DEGREE> *getParent() const
	{
		return parent;
	}

	virtual BPTNodeBase *split(T& pivot) = 0; // pivot upupup! return new BPTNode!
#ifdef _DEBUG_
	virtual void output() const = 0;
#endif
	bool isLeaf;
protected:
	BPTNodeBase *parent;   // nullptr for root
	vector<T> keys;		   // ascending order && distinct
};

template <typename T, size_t DEGREE = 4>
class BPTNodeInternal : public BPTNodeBase<T, DEGREE> {
public:
	BPTNodeInternal(BPTNodeBase<T, DEGREE> *parent = nullptr);
	~BPTNodeInternal();

	virtual BPTNodeInternal *split(T& pivot);// new BPTNode is internal, 
											 // pivot can not be in lower level

	int add(const T &key, BPTNodeBase<T, DEGREE> *fellow);

	void add(BPTNodeBase<T, DEGREE> *first)
	{
		children.push_back(first);
	}

	BPTNodeBase<T, DEGREE> *getChild(int index)
	{
		return children[index];
	}

	BPTNodeBase<T, DEGREE> *getChildLeft(BPTNodeBase<T, DEGREE> *child)
	{
		// child must be child
		auto pos = find_if(children.begin(), children.end(),
			[child](BPTNodeBase<T, DEGREE> *i) { return i == child; });
		return (pos == children.begin()) ? nullptr : *(pos - 1);
	}

	BPTNodeBase<T, DEGREE> *getChildRight(BPTNodeBase<T, DEGREE> *child)
	{
		// child must be child
		auto pos = find_if(children.begin(), children.end(),
			[child](BPTNodeBase<T, DEGREE> *i) { return i == child; });
		return (pos == children.end() - 1) ? nullptr : *(pos + 1);
	}

	int getIndex(BPTNodeBase<T, DEGREE> *child)
	{
		auto pos = find_if(children.begin(), children.end(),
			[child](BPTNodeBase<T, DEGREE> *i) { return i == child; });
		return pos - children.begin();
	}


	void clear()
	{
		this->keys.clear();
		children.clear();
	}
#ifdef _DEBUG_
	void output() const
	{
		for(const auto& i : this->keys) {
			cout << i << ' ';
		}
		cout << endl;
		for(const auto& i : children) {
			i->output();
		}
	}
#endif
	// index of pivot
	// left tree - pivot - right tree
	void coalesce(int index);
	void lRedistribute(int index);
	void rRedistribute(int index);

private:
	vector<BPTNodeBase<T, DEGREE> *> children; // internal or leaf
};

template <typename T, size_t DEGREE = 4>
class BPTNodeLeaf : public BPTNodeBase<T, DEGREE> {
public:
	friend class BPTNodeInternal<T, DEGREE>;

	BPTNodeLeaf(BPTNodeBase<T, DEGREE> *parent = nullptr);
	~BPTNodeLeaf();

	virtual BPTNodeLeaf *split(T& pivot); // new BPTNode is leaf, 
										  //pivot is also in lower level
	int add(const T &key, int offset);    // add key
	void erase(int index)
	{
		assert(size_t(index) >= 0 && size_t(index) < this->keys.size());
		this->keys.erase(this->keys.begin() + index);
		keysOffset.erase(keysOffset.begin() + index);
	}        // erase key by the index of key
	int getOffset(int index)
	{
		assert(index >= 0 && size_t(index) < keysOffset.size());
		return keysOffset[index];
	}
	T getKey(int index)
	{
		assert(index >= 0 && index < this->keysSize());
		return this->keys[index];
	}
	BPTNodeLeaf<T, DEGREE> *getNextSubling() const
	{
		return nextSibling;
	}

#ifdef _DEBUG_
	void output() const
	{
		for(const auto& i : this->keys) {
			cout << i << ' ';
		}
		cout << endl;
	}
#endif
private:
	vector<int> keysOffset;   		 // keysOffset in .db file
	BPTNodeLeaf *nextSibling;
};

/*
D = DEGREE
internal:
keys 	  [ DEGREE / 2 up - 1, DEGREE - 1 ]
pointers  [ DEGREE / 2 up, DEGREE ]

leaf:
keys      [ (DEGREE - 1) / 2 up, DEGREE - 1 ]
pointers  [ (DEGREE - 1) / 2 up + 1, DEGREE ]

root-general:
keys      [ 1, DEGREE - 1 ]
pointers  [ 2, DEGREE ]

root-only:
keys      [ 0, DEGREE - 1 ]
pointerx  [ 1, DEGREE ]

// empty root -> h+- unique conditions! also B-Tree
*/


template <typename T, size_t DEGREE>
BPTNodeBase<T, DEGREE>::BPTNodeBase(BPTNodeBase *parent) :
	parent(parent),
	isLeaf(false)
{
	//empty keys
}

template <typename T, size_t DEGREE>
BPTNodeBase<T, DEGREE>::~BPTNodeBase()
{
	;
}

template <typename T, size_t DEGREE>
bool BPTNodeBase<T, DEGREE>::search(const T& key, int& index) const
{
	bool ret = false;
	index = -1;
	int left = 0, right = keys.size();
	while(left <= right) {
		int middle = (left + right) / 2;
		if(middle >= keys.size()) break;
		if(keys[middle] == key) {
			ret = true;
			index = middle;
			break;
		} else if(keys[middle] < key) {
			left = middle + 1;
		} else {
			right = middle - 1;
		}
	}

	return ret;
}

template <typename T, size_t DEGREE>
bool BPTNodeBase<T, DEGREE>::isRoot() const
{
	return parent == nullptr;
}

template <typename T, size_t DEGREE>
int BPTNodeBase<T, DEGREE>::findFirstLess(const T& key)
{
	auto pos = find_if(keys.begin(), keys.end(),
		[&key](const T& i) { return key < i; });
	int nextLevel = pos - keys.begin();

	return nextLevel;
}


// BPTNodeInternal
template <typename T, size_t DEGREE>
BPTNodeInternal<T, DEGREE>::BPTNodeInternal(BPTNodeBase<T, DEGREE> *parent) :
	BPTNodeBase<T, DEGREE>(parent)
{
	this->isLeaf = false;
	//empty children
	//empty keys
}

template <typename T, size_t DEGREE>
BPTNodeInternal<T, DEGREE>::~BPTNodeInternal()
{
	for(const auto& i : children) {
		if(i != nullptr) delete i;
	}
}


template <typename T, size_t DEGREE>
BPTNodeInternal<T, DEGREE> *BPTNodeInternal<T, DEGREE>::split(T& pivot)
{
	int index = this->keys.size() % 2 ? this->keys.size() / 2 - 1 : this->keys.size() / 2;
	pivot = this->keys[index]; // copy assignment
	assert(size_t(index) >= 0 && size_t(index) < this->keys.size());
	BPTNodeInternal *ret = new BPTNodeInternal(this->parent);

	// index of key!
	copy(this->keys.begin() + index + 1, this->keys.end(), back_inserter(ret->keys));
	copy(this->children.begin() + index + 1, this->children.end(), back_inserter(ret->children));

	for_each(ret->children.begin(), ret->children.end(), [=](const auto& i) { i->setParent(ret); });

	this->keys.erase(this->keys.begin() + index, this->keys.end());
	this->children.erase(this->children.begin() + index + 1, this->children.end());

	return ret;
}

// @NOTE: add position is related to my position in parent
// here find is a naive method!!!

template <typename T, size_t DEGREE>
int BPTNodeInternal<T, DEGREE>::add(const T& key, BPTNodeBase<T, DEGREE> *fellow)
{
	// offset == -1 -> offset info is not necessary for internal BPTNode
	// index successul

	// find first key largeer than key
	auto pos = find_if(this->keys.begin(), this->keys.end(),
		[&key](const T& i) { return i > key; });
	int index = pos - this->keys.begin();

	this->keys.insert(pos, key);
	children.insert(children.begin() + index + 1, fellow);

	return index;
}

// BPTNodeLeaf

template <typename T, size_t DEGREE>
BPTNodeLeaf<T, DEGREE>::BPTNodeLeaf(BPTNodeBase<T, DEGREE> *parent) :
	BPTNodeBase<T, DEGREE>(parent),
	nextSibling(nullptr)
{
	this->isLeaf = true;
	//empty keys
	//empty offsets
}

template <typename T, size_t DEGREE>
BPTNodeLeaf<T, DEGREE>::~BPTNodeLeaf()
{
	;
}


template <typename T, size_t DEGREE>
BPTNodeLeaf<T, DEGREE> *BPTNodeLeaf<T, DEGREE>::split(T& pivot)
{
	int index = this->keys.size() % 2 ? this->keys.size() / 2 - 1 : this->keys.size() / 2;
	pivot = this->keys[index];
	assert(size_t(index) >= 0 && size_t(index) < this->keys.size());
	BPTNodeLeaf *ret = new BPTNodeLeaf(this->parent);

	copy(this->keys.begin() + index, this->keys.end(), back_inserter(ret->keys));
	this->keys.erase(this->keys.begin() + index, this->keys.end());

	copy(keysOffset.begin() + index, keysOffset.end(), back_inserter(ret->keysOffset));
	keysOffset.erase(keysOffset.begin() + index, keysOffset.end());

	auto temp = nextSibling;
	nextSibling = ret;
	ret->nextSibling = temp;

	return ret;
}

template <typename T, size_t DEGREE>
int BPTNodeLeaf<T, DEGREE>::add(const T& key, int offset)
{
	auto pos = find_if(this->keys.begin(), this->keys.end(),
		[&key](const T& i) { return i > key; });
	int index = pos - this->keys.begin();
	this->keys.insert(pos, key);
	keysOffset.insert(keysOffset.begin() + index, offset);

	return index;
}

// index of key
template <typename T, size_t DEGREE>
void BPTNodeInternal<T, DEGREE>::coalesce(int index)
{
	assert(size_t(index) >= 0 && size_t(index) < this->keys.size());
	BPTNodeBase<T, DEGREE> *_left = getChild(index);
	BPTNodeBase<T, DEGREE> *_right = getChild(index + 1);

	if(_left->isLeaf) {  // index key can't be downdown
		BPTNodeLeaf<T, DEGREE> *left = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(_left);
		BPTNodeLeaf<T, DEGREE> *right = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(_right);
		this->keys.erase(this->keys.begin() + index);
		children.erase(children.begin() + index + 1);

		left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
		left->keysOffset.insert(left->keysOffset.end(), right->keysOffset.begin(), right->keysOffset.end());
		left->nextSibling = right->nextSibling;
		delete right;
	} else {			// index key must be downdown
		BPTNodeInternal<T, DEGREE> *left = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(_left);
		BPTNodeInternal<T, DEGREE> *right = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(_right);
		T key = this->keys[index];
		this->keys.erase(this->keys.begin() + index);
		children.erase(children.begin() + index + 1);

		left->keys.push_back(key);
		left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
		left->children.insert(left->children.end(), right->children.begin(), right->children.end());

		//reset parent!
		for_each(left->children.begin(), left->children.end(), [left](auto i) { i->setParent(left); });

		right->clear(); // here is equivalent to "this->keys.erase() + children.erase()"
		delete right;
	}
}

// index of key
// left -> right
template <typename T, size_t DEGREE>
void BPTNodeInternal<T, DEGREE>::lRedistribute(int index)
{
	assert(size_t(index) >= 0 && size_t(index) < this->keys.size());
	BPTNodeBase<T, DEGREE> *_left = getChild(index);
	BPTNodeBase<T, DEGREE> *_right = getChild(index + 1);

	int internalLower = DEGREE % 2 ? DEGREE / 2 : DEGREE / 2 - 1;
	int leafLower = (DEGREE - 1) % 2 ? (DEGREE - 1) / 2 + 1 : (DEGREE - 1) / 2;

	if(_left->isLeaf) {  // index key can't be downdown
		BPTNodeLeaf<T, DEGREE> *left = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(_left);
		BPTNodeLeaf<T, DEGREE> *right = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(_right);
		int from = leafLower;

		right->keys.insert(right->keys.begin(), left->keys.begin() + from, left->keys.end());
		left->keys.erase(left->keys.begin() + from, left->keys.end());

		right->keysOffset.insert(right->keysOffset.begin(), left->keysOffset.begin() + from, left->keysOffset.end());
		left->keysOffset.erase(left->keysOffset.begin() + from, left->keysOffset.end());

		this->keys[index] = right->keys[0];

	} else {			// index key must be downdown
		BPTNodeInternal<T, DEGREE> *left = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(_left);
		BPTNodeInternal<T, DEGREE> *right = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(_right);
		int from = internalLower;

		//right->keys.push_front(this->keys[index]);
		right->keys.insert(right->keys.begin(), this->keys[index]);

		right->keys.insert(right->keys.begin(), left->keys.begin() + from + 1, left->keys.end());
		this->keys[index] = left->keys[from];
		left->keys.erase(left->keys.begin() + from, left->keys.end());

		right->children.insert(right->children.begin(), left->children.begin() + from + 1, left->children.end());
		// reset parent !!! 
		for_each(right->children.begin(), right->children.end(), [=](auto i) { i->setParent(right); });

		left->children.erase(left->children.begin() + from + 1, left->children.end());
	}
}

// index of key
// left <- right
template <typename T, size_t DEGREE>
void BPTNodeInternal<T, DEGREE>::rRedistribute(int index)
{
	assert(size_t(index) >= 0 && size_t(index) < this->keys.size());
	BPTNodeBase<T, DEGREE> *_left = getChild(index);
	BPTNodeBase<T, DEGREE> *_right = getChild(index + 1);

	int internalLower = DEGREE % 2 ? DEGREE / 2 : DEGREE / 2 - 1;
	int leafLower = (DEGREE - 1) % 2 ? (DEGREE - 1) / 2 + 1 : (DEGREE - 1) / 2;

	if(_left->isLeaf) {  // index key can't be downdown
		BPTNodeLeaf<T, DEGREE> *left = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(_left);
		BPTNodeLeaf<T, DEGREE> *right = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(_right);
		int from = leafLower; // end index of keys suppoed to transfer

		left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end() - from);
		right->keys.erase(right->keys.begin(), right->keys.end() - from);

		left->keysOffset.insert(left->keysOffset.end(), right->keysOffset.begin(), right->keysOffset.end() - from);
		right->keysOffset.erase(right->keysOffset.begin(), right->keysOffset.end() - from);

		this->keys[index] = right->keys[0];

	} else {			// index key must be downdown
		BPTNodeInternal<T, DEGREE> *left = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(_left);
		BPTNodeInternal<T, DEGREE> *right = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(_right);
		int from = internalLower;

		left->keys.push_back(this->keys[index]);
		left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end() - from - 1);
		this->keys[index] = right->keys[right->keys.size() - from - 1];
		right->keys.erase(right->keys.begin(), right->keys.end() - from);

		left->children.insert(left->children.end(), right->children.begin(), right->children.end() - from - 1);
		// reset parent !!!
		for_each(left->children.begin(), left->children.end(), [=](auto i) { i->setParent(left); });
		right->children.erase(right->children.begin(), right->children.end() - from - 1);
	}
}


template <typename T, size_t DEGREE = 4>
struct SearchNode {
	BPTNodeLeaf<T, DEGREE> *leafNode;
	int index; // index of key/offset
};

template <typename T, size_t DEGREE = 4>
class BPTree {
public:
	BPTree(const string& fileName, int typeSize);
	~BPTree();
	BPTNodeLeaf<T, DEGREE> *getHead() const
	{
		return dynamic_cast<BPTNodeLeaf<T, DEGREE>*>(head);
	}

	BPTNodeLeaf<T, DEGREE> *getNext(BPTNodeLeaf<T, DEGREE> *cur) const
	{
		return cur == nullptr ? nullptr : cur->getNextSubling();
	}
	bool find(const T& key, SearchNode<T, DEGREE>& res); // equal-condition && return pair<node, offset>
	bool find(const T& key, int& offset);  // equal-condition && return offset
	bool insert(const T &key, int offset); // insert with offset
	bool erase(const T &key);              // remove a record -> remove its index entry

#ifdef _DEBUG_
	void output() const // root-first
	{
		cout << "-------------------------" << endl;
		if(root != nullptr)
			root->output();
	}
#endif

private:
	void dtorToFile();
	void ctorFromFile(); // when using a index! construct it from file!
	void readBlock(blockNode *bi);

	void insertAdjust(BPTNodeBase<T, DEGREE> *pos);
	void eraseAdjust(BPTNodeBase<T, DEGREE> *pos);  // prior to fuse -> share/redistribute
	BPTNodeLeaf<T, DEGREE> *findLeaf(const T& key);

	fileNode *fileHandler;
	string fileName;      // fileName == indexName
	BPTNodeBase<T, DEGREE> *root;
	BPTNodeBase<T, DEGREE> *head; // head of single-list of leaf node
	int sizeofKey;        // for construct a B+Tree from a .inx file
	int keyCount;
	int nodeCount;
};

template <typename T, size_t DEGREE>
BPTree<T, DEGREE>::BPTree(const string& fileName, int typeSize) :
	fileName(fileName),    // must not be null
	root(nullptr),
	head(nullptr),
	sizeofKey(typeSize),
	keyCount(0),
	nodeCount(0),
	fileHandler(nullptr)
{
	root = new BPTNodeLeaf<T, DEGREE>; // parent nullptr
	head = root;
	nodeCount++;
	ctorFromFile();
}

template <typename T, size_t DEGREE>
void BPTree<T, DEGREE>::ctorFromFile()
{
	fileHandler = bm.get_File(fileName.c_str());
	blockNode* bi = bm.get_BlockHead(fileHandler);
	while(bi != nullptr) {
		readBlock(bi);
		if(bi->ifbottom) break;
		bi = bm.get_NextBlock(fileHandler, bi);
	}
}

template <typename T, size_t DEGREE>
void BPTree<T, DEGREE>::readBlock(blockNode *bi)
{
	unsigned char* keyBegin = reinterpret_cast<unsigned char *>(bm.get_content(*bi));
	unsigned char* offsetBegin = keyBegin + sizeofKey;
	T *key;
	int *offset;
	while(offsetBegin + sizeof(int) <= reinterpret_cast<unsigned char *>(bm.get_content(*bi)) + bm.get_UsingSize(*bi)) {
		// there are available position in the block
		// reinterpret memory
		// a naive parse method!
		string tag = "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >";

		key = reinterpret_cast<T *>(keyBegin);
		offset = reinterpret_cast<int *>(offsetBegin);
		insert(*key, *offset);
		keyBegin += sizeofKey + sizeof(int);
		offsetBegin += sizeofKey + sizeof(int);
	}

}

template <typename T, size_t DEGREE>
BPTree<T, DEGREE>::~BPTree()
{
	dtorToFile();
	// why crash? 
	// delete fileHandler;
	bm.delete_fileNode(fileHandler->fileName.c_str());
	delete root;
}

template <typename T, size_t DEGREE>
void BPTree<T, DEGREE>::dtorToFile()
{
	blockNode* bi = bm.get_BlockHead(fileHandler);
	BPTNodeLeaf<T, DEGREE> *cur = dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(head);

	size_t unitSize = sizeofKey + sizeof(int);
	bm.set_usingSize(*bi, 0);
	bm.set_dirty(*bi);
	while(cur != nullptr)
	{
		for(int i = 0; i < cur->keysSize(); i++) {
			T keyTemp(cur->getKey(i));
			int offsetTemp(cur->getOffset(i));
			unsigned char *key = reinterpret_cast<unsigned char *>(&keyTemp);
			unsigned char *offset = reinterpret_cast<unsigned char *>(&offsetTemp);

			if(8192 - bm.get_UsingSize(*bi) - sizeof(size_t) < unitSize) {
				// next block!!! ??? !!!
				bi = bm.get_NextBlock(fileHandler, bi);
				bm.set_usingSize(*bi, 0);
			}
			bm.set_dirty(*bi);

			memcpy(bm.get_content(*bi) + bm.get_UsingSize(*bi), key, sizeofKey);
			bm.set_usingSize(*bi, bm.get_UsingSize(*bi) + sizeofKey);

			memcpy(bm.get_content(*bi) + bm.get_UsingSize(*bi), offset, sizeof(int));
			bm.set_usingSize(*bi, bm.get_UsingSize(*bi) + sizeof(int));
		}
		
		cur = cur->getNextSubling();
	}

}

// @return
// true   / false
// offset / -1
template <typename T, size_t DEGREE>
bool BPTree<T, DEGREE>::find(const T& key, int& offset)
{
	bool ret;
	BPTNodeLeaf<T, DEGREE> *cur = findLeaf(key);
	int index;
	ret = cur->search(key, index);
	if(ret)
		offset = cur->getOffset(index);
	else
		offset = -1;

	return ret;
}

template <typename T, size_t DEGREE>
bool BPTree<T, DEGREE>::find(const T& key, SearchNode<T, DEGREE>& res)
{
	bool ret;
	BPTNodeLeaf<T, DEGREE> *cur = findLeaf(key);
	res.leafNode = cur;
	int index; // lvalue ref
	ret = cur->search(key, index);
	if(ret) {
		offset = cur->getOffset(index);
		res.index = index;
	} else {
		offset = -1;
		res.index = -1;
	}
	return ret;
}

template <typename T, size_t DEGREE>
BPTNodeLeaf<T, DEGREE> *BPTree<T, DEGREE>::findLeaf(const T& key)
{
	BPTNodeBase<T, DEGREE> *cur = root;
	while(!cur->isLeaf) {
		BPTNodeInternal<T, DEGREE> *nonLeaf = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(cur);
		int nextLevel = nonLeaf->findFirstLess(key);
		cur = nonLeaf->getChild(nextLevel);
	}
	return dynamic_cast<BPTNodeLeaf<T, DEGREE> *>(cur);
}

template <typename T, size_t DEGREE>
bool BPTree<T, DEGREE>::insert(const T& key, int offset)
{
	BPTNodeLeaf<T, DEGREE> *pos = findLeaf(key);
	pos->add(key, offset);
	keyCount++;

	if(pos->keysSize() == DEGREE) insertAdjust(pos);

	//output();
	return true;
}

template <typename T, size_t DEGREE>
void BPTree<T, DEGREE>::insertAdjust(BPTNodeBase<T, DEGREE> *pos)
{
	BPTNodeBase<T, DEGREE> *cur = pos;
	T pivot;
	BPTNodeBase<T, DEGREE> *newBPTNode = cur->split(pivot); // leaf or internal

	nodeCount++;
	if(cur->isRoot()) {
		BPTNodeInternal<T, DEGREE> *temp = new BPTNodeInternal<T, DEGREE>;
		nodeCount++;
		temp->add(cur);
		cur->setParent(temp);

		temp->add(pivot, newBPTNode);
		newBPTNode->setParent(temp);

		root = temp;
	} else {
		BPTNodeInternal<T, DEGREE> *parent = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(cur->getParent());
		parent->add(pivot, newBPTNode);
		if(parent->keysSize() == DEGREE) insertAdjust(parent);
	}
}

template <typename T, size_t DEGREE>
bool BPTree<T, DEGREE>::erase(const T& key)
{
	int ret, index;
	BPTNodeLeaf<T, DEGREE> *pos = findLeaf(key);
	ret = pos->search(key, index);
	if(ret) {
		pos->erase(index);
		keyCount--;

		//leafLower
		int lower = (DEGREE - 1) % 2 ? (DEGREE - 1) / 2 + 1 : (DEGREE - 1) / 2;
		if(pos->keysSize() < lower) {  // leaf [ (D - 1) / 2 up, D - 1]
			eraseAdjust(pos);
		}
	}

	return ret;
}

template <typename T, size_t DEGREE>
void BPTree<T, DEGREE>::eraseAdjust(BPTNodeBase<T, DEGREE> *pos) // internal or leaf
{
	BPTNodeBase<T, DEGREE> *cur = pos;

	if(cur->isRoot() && cur->keysSize() == 0) {
		if(cur->isLeaf) {
			;
		} else {
			// must be only a child!
			// must be zero key!
			BPTNodeInternal<T, DEGREE> *temp = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(cur);
			root = temp->getChild(0);
			root->setParent(nullptr);

			temp->clear();   // unique public interface!
			delete temp;
			nodeCount--;
		}
	} else if(!cur->isRoot()) {
		BPTNodeInternal<T, DEGREE> *parent = dynamic_cast<BPTNodeInternal<T, DEGREE> *>(cur->getParent());
		BPTNodeBase<T, DEGREE> *childLeft = parent->getChildLeft(cur);
		BPTNodeBase<T, DEGREE> *childRight = parent->getChildRight(cur);
		int childIndex = parent->getIndex(cur);

		int internalLower = DEGREE % 2 ? DEGREE / 2 : DEGREE / 2 - 1;
		int leafLower = (DEGREE - 1) % 2 ? (DEGREE - 1) / 2 + 1 : (DEGREE - 1) / 2;

		if(cur->isLeaf && cur->keysSize() < leafLower) {
			if(childLeft != nullptr && childLeft->keysSize() + cur->keysSize() < DEGREE) {
				parent->coalesce(childIndex - 1);
				nodeCount--;
			} else if(childRight != nullptr && childRight->keysSize() + cur->keysSize() < DEGREE) {
				parent->coalesce(childIndex);
				nodeCount--;
			} else if(childLeft != nullptr && childLeft->keysSize() > leafLower) {
				parent->lRedistribute(childIndex - 1);
			} else { // must valid
				parent->rRedistribute(childIndex);
			}
		} else if(!cur->isLeaf && cur->keysSize() < internalLower) {
			if(childLeft != nullptr && childLeft->keysSize() + cur->keysSize() + 1 < DEGREE) {
				parent->coalesce(childIndex - 1);
				nodeCount--;
			} else if(childRight != nullptr && childRight->keysSize() + cur->keysSize() + 1 < DEGREE) {
				parent->coalesce(childIndex);
				nodeCount--;
			} else if(childLeft != nullptr && childLeft->keysSize() > leafLower) {
				parent->lRedistribute(childIndex - 1);
			} else { // must valid
				parent->rRedistribute(childIndex);
			}
		}

		if(parent->keysSize() < internalLower) eraseAdjust(parent);
	}
}
#endif