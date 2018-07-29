#pragma once
#include "Minisql.h"
#include <iostream>
class BufferManager{
public:
    BufferManager();
    virtual ~BufferManager();
    void delete_fileNode(const char * fileName);
    fileNode* get_File(string fileName,bool if_pin = false);
    void set_dirty(blockNode & block);
    void set_pin(blockNode & block,bool pin);
    void set_pin(fileNode & file,bool pin);
    void set_usingSize(blockNode & block,std::size_t usage);
    char* get_content(blockNode& block);
    std::size_t get_UsingSize(blockNode &block);//get the block using size from using_size
    std::size_t get_usingSize(blockNode &block);//get the block using size from address
	int get_BlockSize(); //Get the size of the block that others can use.Others cannot use the block head
	int get_BlockSize(blockNode& block);
    blockNode* get_NextBlock(fileNode * file,blockNode* block);
    blockNode* get_BlockHead(fileNode* file);
    blockNode* get_BlockByOffset(fileNode* file, int offestNumber);

private:
    fileNode *fileHead;
    blockNode *block_pool;
    int used_block; // the number of block that have been used, which means the block is in the list.
    void init_block(blockNode &block);
    void init_file(fileNode &file);
    blockNode* get_Block(fileNode * file, blockNode* position, bool if_pin = false);
    void writtenBack_All();
    void writtenBack(string fileName,blockNode* block);
    void clean_dirty(blockNode &block);
};