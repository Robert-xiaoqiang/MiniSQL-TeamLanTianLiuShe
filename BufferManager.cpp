#include "BufferManager.h"

//Construction Function: allocate memories for the pools
//                       and init the variable values
BufferManager::BufferManager():fileHead(nullptr),used_block(0)
{
    fileHead = new fileNode;
	init_file(*fileHead);
	if (block_pool = new blockNode[MAX_BLOCK_NUM]);
	else
	{
		cout << "Can not allocate enough memory for block pool!" << endl;
		exit(0);
	}
	for (int i = 0; i < MAX_BLOCK_NUM; i++)
	{
		if (block_pool[i].address = new char[BLOCK_SIZE]);
		else
		{
			cout << "Can not allocate enough memory for block pool!" << endl;
			exit(0);
		}
		init_block(block_pool[i]);
	}


}

//Destruction Function: free memories of the pools
//                      and write back the buffer content
BufferManager::~BufferManager()
{
    writtenBack_All();
    
    fileNode *fptr;
    for(fptr = fileHead; fptr != nullptr; fptr = fptr->nextFile)//traversal the file node
    {
        if(fptr->preFile)
        {
            delete fptr->preFile;
        }
    }
    delete fptr;
    
	for (int i = 0; i < MAX_BLOCK_NUM; i++)
	{
		delete[] block_pool[i].address;
	}
    delete [] block_pool;
}

/*
* init the fileNode values
* @param fileNode&  the file your want to init
* @return void
*/
void BufferManager::init_file(fileNode &file)
{
    file.nextFile = nullptr;
    file.preFile = nullptr;
    file.blockHead = nullptr;
    file.pin = false;
}

/*
* init the blockNode values
* @param blockNode&  the block your want to init
* @return void
*/
void BufferManager::init_block(blockNode &block)
{
    memset(block.address, 0, sizeof(char) * BLOCK_SIZE);//init block
    block.using_size = 0;
    block.dirty = false;
    block.nextBlock = nullptr;
    block.preBlock = nullptr;
    block.offsetNum = -1;
    block.pin = false;
    block.reference = 0;
    block.ifbottom = false;
}


/*
 *  Get a fileNode
 *  If the file is already in the list, return this fileNode
 *  If the file is not in the list, add it to the file list
 * @param const char* the file name
 * @param bool if true, the file will be locked.
 * @return fileNode*
 */
fileNode* BufferManager::get_File(string fileName, bool if_pin)
{
    blockNode *bptr = nullptr;
    fileNode *fptr = nullptr;
    if(fileHead != nullptr)
    {
        for(fileNode* fptr = fileHead; fptr != nullptr; fptr = fptr->nextFile)
        {
            if(fileName == fptr->fileName) //the fileNode is already in the list
            {
                fptr->pin = if_pin;
                return fptr;
            }
            else
                continue;    //if the fileNode is not in the list
        }
    }

    //if the fileNode is not in the list, insert the file node into the ending of list
    fileNode *ftmp = new fileNode;
    init_file(*ftmp);
    fptr = fileHead;
    while(fptr->nextFile)//find the last node of list
        fptr = fptr->nextFile;
	if (fptr == fileHead)//if the file list only have a filehead
	{
		if (fileHead->fileName == "")
		{
			fileHead = ftmp;
			ftmp->nextFile = nullptr;
		}
		else
		{
			ftmp->preFile = fileHead;
			fileHead->nextFile = ftmp;
		}

	}
	else//if the file has two or more filenode
	{
		fptr->nextFile = ftmp;//insert
		ftmp->preFile = fptr;
	}

    if(fileName.size() > MAX_FILE_NAME)
    {
        cout << "The file name is too long, the maximum length of filename is " << MAX_FILE_NAME << endl;
		exit(0);
    }
    ftmp->fileName = fileName;
    set_pin(*ftmp, if_pin);
    return ftmp;
}




/*
 *  Get a block node
 *  If the block is already in the list, return this blockNode
 *  If the block is not in the list, replace some fileNode, using LRU replacement, if required, to make more space.
 *  Only if the block is dirty, it will be written back to disk when been reaplaced.
 * @param fileNode * the file you want to add the block into.
 * @param blockNode  the position that the Node will added to.
 * @param bool  if true, the block will be locked.
 * @return blockNode*
 */
blockNode* BufferManager::get_Block(fileNode *file, blockNode *position, bool if_pin)
{
    string filename = file->fileName;
    blockNode *bptr = nullptr;
    fileNode *fptr = fileHead;
    while(fptr != nullptr)//find the file in the list
    {
        if(fptr->fileName == filename)
        {
            break;
        }
        fptr = fptr->nextFile;
    }

    bptr = fptr->blockHead;
    while(bptr)//find the block in the list of this file
    {
        if(bptr->offsetNum == position->offsetNum + 1)//if the block is in the list
        {
            set_pin(*bptr, if_pin);
            bptr->reference ++;
            return bptr;
        }
        bptr = bptr->nextBlock;
    }

    //if the block is not in the list
    if(used_block < MAX_BLOCK_NUM) // there are empty blocks in the block pool
    {
        for(int i = 0; i < MAX_BLOCK_NUM; i ++)
        {
            if(block_pool[i].offsetNum == -1)
            {
                bptr = &block_pool[i];
                used_block ++;
                bptr->reference ++;
                break;
            }
            else
                continue;
        }
    }

    //if there are not enough node for block, we need use LRU to replace one block
    //and write back the replaced block node into disk
    else
    {
        int min_reference = block_pool[0].reference;//using LUR to find the block node with the minimum reference
        int max_reference = block_pool[0].reference;
        for(int i = 0; i < used_block; i++)
        {
            if(block_pool[i].reference < min_reference)
            {
                min_reference = block_pool[i].reference;
                bptr = &block_pool[i];
            }
            if(block_pool[i].reference > max_reference)
                max_reference = block_pool[i].reference;
        }
        if(bptr->preBlock) bptr->preBlock->nextBlock = bptr->nextBlock;
        if(bptr->nextBlock) bptr->nextBlock->preBlock = bptr->preBlock;
        if(file->blockHead == bptr) file->blockHead = bptr->nextBlock;
        writtenBack(bptr->fileName, bptr);
        init_block(*bptr);
        bptr->reference = max_reference;
    }
    
    //add the block into the block list
    if(position && !position->nextBlock)//if position is the tail node
    {
        bptr->preBlock = position;
        position->nextBlock = bptr;
        bptr->offsetNum = position->offsetNum + 1;
    }
    else if (position && position->nextBlock)//if position is not the tail node
    {
        bptr->preBlock = position;
        bptr->nextBlock = position->nextBlock;
        position->nextBlock->preBlock = bptr;
        position->nextBlock = bptr;
        bptr->offsetNum = position->offsetNum + 1;
    }
    else // if position is nullptr, then the block will be the head of the list
    {
        bptr->offsetNum = 0;
		if(file->blockHead)
		{
			file->blockHead->preBlock = bptr;
			bptr->nextBlock = file->blockHead;
		}
        file->blockHead = bptr;
    }

    set_pin(*bptr, if_pin);
    if(filename.size() > MAX_FILE_NAME)
    {
		cout << "The file name is too long, the maximum length of filename is " << MAX_FILE_NAME << endl;
        exit(0);
    }
    bptr->fileName = filename;
    
    //read the file content to the block
    FILE * fp = fopen(filename.c_str(), "ab+");//appendix read and write binary file
    if(fp != nullptr)//open the file successfully
    {
        if(fseek(fp, bptr->offsetNum * BLOCK_SIZE, 0) == 0)//seek offset pointer in the file success   
        {
            if(fread(bptr->address, 1, BLOCK_SIZE, fp) == 0)//read 1 byte block_size times to block content ending
                bptr->ifbottom = true;
            bptr->using_size = get_usingSize(*bptr);
			//memcpy(bptr->address, &bptr->using_size, sizeof(size_t));
			*bptr->address = bptr->using_size;
        }
        else // seek failed
        {
            cout << "There are some problems seeking the file "<< filename << " in reading!" << endl;
            exit(0);
        }
        fclose(fp);
    }
    else //open the file failed
    {
        cout << "There are some problems opening the file "<< filename << " in reading!" << endl;
        exit(0);
    }
    return bptr;
}


/*
 * Flush the block node to the disk, if the block is not dirty, do nothing.
 * @param const char*  the file name
 * @param blockNode&  the block your want to flush
 * @return void
 */
void BufferManager::writtenBack(string fileName, blockNode *block)
{
    if(block->dirty == false) return; // this block is not been modified, so it do not need to written back to files
    else // written back to the file
    {
        FILE * fp = fopen(fileName.c_str(), "rb+");
        if(fp != nullptr)//open successfully
        {
            if(fseek(fp, block->offsetNum * BLOCK_SIZE, 0) == 0)//seek successfully
            {
                if(fwrite(block->address, block->using_size + sizeof(size_t), 1, fp) != 1)//write failed
                {
                    cout << "There are some problems writting the file "<< fileName << " in write back to disk!" << endl;
                    exit(0);
                }
            }
            else//seek failed
            {
                cout << "There are some problems seeking the file "<< fileName << " in write back to disk!" << endl;
                exit(0);
            }
            fclose(fp);
        }
        else
        {
            cout << "There are some problems opening the file "<< fileName << " in write back to disk!" << endl;
            exit(0);
        }
    }
}


/*
 * Flush all block node in the list to the disk
 * @return void
 */
void BufferManager::writtenBack_All()
{
    blockNode *bptr = nullptr;
    fileNode *fptr = nullptr;
    if(fileHead != nullptr)
    {
        for(fptr = fileHead; fptr != nullptr; fptr = fptr->nextFile)
        {
            if(fptr->blockHead)
            {
                for(bptr = fptr->blockHead; bptr != nullptr; bptr = bptr->nextBlock)
                {
                    if(bptr->preBlock)
                        init_block(*(bptr->preBlock));
                    writtenBack(bptr->fileName, bptr);
					if (bptr->nextBlock == nullptr)
						init_block(*bptr);
                }
            }
        }
    }
}



/*
 * Get the next block node of the node inputed
 * the next block means the block belongs to the same file and offsetNum is continuous
 * @param fileNode* the file you want to add a block
 * @param blockNode&  the block your want to be added to
 * @return blockNode*
 */
blockNode* BufferManager::get_NextBlock(fileNode *file, blockNode *block)
{
    if(block->nextBlock == nullptr)//block is the last node of this file
    {
        blockNode *bptr = get_Block(file, block);
		if (block->ifbottom)
		{
			block->ifbottom = false;
			bptr->ifbottom = true;
		}
        return bptr;
    }
    else
    {
        if(block->offsetNum == block->nextBlock->offsetNum - 1)//the block's next block in list is right its next block 
        {
            return block->nextBlock;
        }
        else //the block list is not in the right order
        {
            return get_Block(file, block);
        }
    }
}


/*
 * Get the head block of the file
 * @param fileNode* the filenode input
 * @return blockNode*
 */
blockNode* BufferManager::get_BlockHead(fileNode *file)
{
    blockNode* bptr = nullptr;
    if(file->blockHead)
    {
        if(file->blockHead->offsetNum == 0) //the block head of the file node is just the first block of this file
            return file->blockHead;
        else
        {
            bptr = get_Block(file, nullptr);
            return bptr;
        }
    }
    else// If the file have no block head, get a new block node for it
    {
        bptr = get_Block(file, nullptr);
        return bptr;
    }
}


/*
 * Get the block of the file by offset number
 * @param fileNode* the file node input
 * @param int offsetNumber input number
 * @return blockNode*
 */
blockNode* BufferManager::get_BlockByOffset(fileNode *file, int offsetNumber)
{
    blockNode* bptr = nullptr;
    if(offsetNumber == 0) return get_BlockHead(file);//if offset is 0, then the blockHead is right the need
    else//if offset is not 0, find the block in the list from head one by one
    {
        bptr = get_BlockHead(file);
        while(offsetNumber > 0)
        {
            bptr = get_NextBlock(file, bptr);
            offsetNumber --;
        }
        return bptr;
    }
}


/*
 * delete the file node and its block node
 * @param const char * fileName 
 */
void BufferManager::delete_fileNode(const char *fileName)
{
    fileNode* fptr = get_File(fileName);
    if(fptr == nullptr) return;
    blockNode* bptr = get_BlockHead(fptr);

    for(; bptr != nullptr; bptr = bptr->nextBlock)
    {
        if(bptr->preBlock)
        {
            init_block(*(bptr->preBlock));
            used_block--;
        }
		if (bptr->nextBlock == nullptr)
		{
			init_block(*bptr);
			used_block--;
		}
    }

    if(fptr->preFile) fptr->preFile->nextFile = fptr->nextFile;
    if(fptr->nextFile) fptr->nextFile->preFile = fptr->preFile;
	if (fileHead == fptr)
	{
		if (fptr->nextFile)
			fileHead = fptr->nextFile;
		else
			init_file(*fptr);
	}
    init_file(*fptr);
}

/*
 * Set the pin of a block node
 * @param blockNode& the block node input
 * @param bool pin the pin will be set
 */
void BufferManager::set_pin(blockNode &block, bool pin)
{
    block.pin = pin;
}


/*
 * Set the pin of a file node
 * @param fileNode& the file node input
 * @param bool pin the pin will be set
 */
void BufferManager::set_pin(fileNode &file, bool pin)
{
    file.pin = pin;
}


/*
 * Set the dirty of a block node which means that the node has been modified or not
 * @param blockNode& the block node input
 */
void BufferManager::set_dirty(blockNode &block)
{
    block.dirty = true;
}

/*
 * Set the dirty of a block node is false
 * @param blockNode& the block node input
 */
void BufferManager::clean_dirty(blockNode &block)
{
    block.dirty = false;
}

void BufferManager::set_usingSize(blockNode &block, size_t usage)
{
    block.using_size = usage;
    memcpy(block.address, (char*)&usage, sizeof(size_t));
}

/*
*Get the using size of the block input
* @para blockNode& the block node input
* @return size_t
*/
size_t BufferManager::get_UsingSize(blockNode &block)
{
    return block.using_size;
}

/*
*Get the using size of the block input
* @para blockNode& the block node input
* @return size_t
*/
size_t BufferManager::get_usingSize(blockNode &block)
{
    return *(size_t*)block.address;
}

/*
 * Get the content of the block except the block head
 * @param blockNode& the block node input
 * @return char*
 */
char* BufferManager::get_content(blockNode& block)
{
    return block.address + sizeof(size_t);
}


int BufferManager::get_BlockSize()
{
    return BLOCK_SIZE - sizeof(size_t);
}

int BufferManager::get_BlockSize(blockNode &block)
{
	return BLOCK_SIZE - sizeof(size_t) - get_usingSize(block);
}