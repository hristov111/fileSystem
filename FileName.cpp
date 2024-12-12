#include <iostream>
#include <list>
#include <cstring>
#include <vector>
#include <fstream>
#include <list>
#include <string>
#include <functional> 

using namespace std;
class HashTable {
private:
	static const int TABLE_SIZE = 100;
	std::list<std::pair<string, uint64_t>> table[TABLE_SIZE];

	// custom hash function
	int hashFunction(const std::string& key) const {
		int hash = 0;
		for (char c : key) {
			hash = (hash * 31 + c) % TABLE_SIZE;
		}
		return hash;
	}

public:
	// Insert a key value pair
	void insert(const string& key, const uint64_t& value) {
		int index = hashFunction(key);

		for (auto& kv : table[index]) {
			if (kv.first == key) {
				kv.second = value;
				return;
			}
			// key already exists
		}

		table[index].emplace_back(key, value);
	}

	// retrieve a value by key
	uint64_t get(const std::string& key) const {
		int index = hashFunction(key);

		for (const auto& kv : table[index]) {
			if (kv.first == key) { // Key found
				return kv.second;
			}
		}

		throw std::runtime_error("Key not found in the hash table");
	}

	bool exists(const string& key) const {
		int index = hashFunction(key);

		for (const auto& kv : table[index]) {
			if (kv.first == key) { // Key exists
				return true;
			}
		}
		return false;
	}

	bool remove(const string& key) {
		int index = hashFunction(key);

		for (auto it = table[index].begin(); it != table[index].end(); it++) {
			if (it->first == key) { // Key found
				table[index].erase(it);
				return true;
			}
		}
		return false; // key not found
	}

	void iterate(std::function<void(const std::string&, uint64_t)> func) const {
		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				func(kv.first, kv.second);
			}
		}
	}
};

HashTable blockHashTable;
HashTable metadataHashtable;

// deduplication ?
// Deduplication is a technique used to 
// eliminate duplicate copies of data in 
// storage systems. Instead of storing the 
// same data multiple times, deduplication ensures
// that identical blocks of data are stored only once 
// and reused wherever necessary. This can save a significant 
//  
// that share similar content (e.g., backups, images, or duplicate fmount of storage space, especially when dealing with large filesiles).

//  metaData strucutre

uint64_t currentDirectoryOffset = 0;

struct Block {
	uint64_t content_offset; // offset of the block content
	uint64_t block_offset; // offset of the block metadata
	uint64_t size; // size of the block
	string hashBl;
	uint32_t checksum; // Resilliency checksum
	
	void serialize(std::ofstream& out) const {
		out.write(reinterpret_cast<const char*>(&content_offset), sizeof(content_offset));
		out.write(reinterpret_cast<const char*>(&block_offset), sizeof(block_offset));
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));

		// serialize the hash string
		size_t hashLength = hashBl.size();
		out.write(reinterpret_cast<const char*>(&hashLength), sizeof(hashLength));
		//hashBl.data() gives a pointer to the raw character array inside the string.
		out.write(hashBl.data(), hashLength);
	}
	static Block* deserialize(std::ifstream& in) {
		Block* block = new Block();

		in.read(reinterpret_cast<char*>(&block->content_offset), sizeof(block->content_offset));
		in.read(reinterpret_cast<char*>(&block->block_offset), sizeof(block->block_offset));

		in.read(reinterpret_cast<char*>(&block->size), sizeof(block->size));
		in.read(reinterpret_cast<char*>(&block->checksum), sizeof(block->checksum));

		// Deserialize the hash string
		size_t hashLength;		
		in.read(reinterpret_cast<char*>(&hashLength), sizeof(hashLength)); 

		block->hashBl.resize(hashLength);
		in.read(&block->hashBl[0], hashLength);

		return block;

	}
	void writeToContainer(std::ofstream& container, const char* buffer, size_t buffersize,uint64_t offset) {
		block_offset = offset;
		size = buffersize;
		container.seekp(offset);

		// computer checksum (somple XOR checksum)
		checksum = 0;
		for (size_t i = 0; i < buffersize; ++i) {
			checksum ^= buffer[i];
		}
		// Serialize the block data (because of checksum)
		serialize(container);
		// write the block's data to thec ontainer
		content_offset = container.tellp();

		container.write(buffer, buffersize);
	}
	string hashBlock(const char* block, size_t size) {
		string hash;
		for (size_t i = 0; i < size; ++i) {
			hash += to_string((block[i] + i) % 256);
		}
		return hash;
	}

};
struct Metadata {
	char name[100]; // file or directory name
	bool isDirectory; // True if it's a directory
	uint64_t offset; // offset in the container
	uint64_t size; // size of the file (0 for directories)
	vector<string> keys; // keys to the hashtable for the blocks
	vector<uint64_t> children; // offsets of child files or directories 
	uint64_t parent; // Parent directory's index in the metadata (0 for root)
	bool isDeleted;

	void serialize(std::ofstream& out) {
		out.write(reinterpret_cast<const char*>(&isDirectory), sizeof(isDirectory));
		out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(&parent), sizeof(parent));
		out.write(reinterpret_cast<const char*>(&isDeleted), sizeof(isDeleted));


		// Write= the size of the vector
		size_t blockKeyCount = keys.size();
		out.write(reinterpret_cast<const char*>(&blockKeyCount), sizeof(blockKeyCount));

		// Write each string in the vectopr
		for (const auto& key : keys) {
			size_t keyLength = key.size();
			out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
			out.write(key.data(), keyLength);
		}
		size_t children_count = children.size();
		out.write(reinterpret_cast<const char*>(&children_count), sizeof(children_count));

		for (const auto& child : children) {
			out.write(reinterpret_cast<const char*>(&child), sizeof(child));
		}




	}

	static Metadata* deserialize(std::ifstream& in) {
		Metadata* metadata = new Metadata();

		// Read fixed size fields
		in.read(metadata->name, sizeof(metadata->name));
		in.read(reinterpret_cast<char*>(&metadata->isDirectory), sizeof(metadata->isDirectory));
		in.read(reinterpret_cast<char*>(&metadata->offset), sizeof(metadata->offset));
		in.read(reinterpret_cast<char*>(&metadata->size), sizeof(metadata->size));
		in.read(reinterpret_cast<char*>(&metadata->parent), sizeof(metadata->parent));
		in.read(reinterpret_cast<char*>(&metadata->isDeleted), sizeof(metadata->isDeleted));


		size_t blockKeycount;
		in.read(reinterpret_cast<char*>(&blockKeycount), sizeof(blockKeycount));

		// Read each string in the vector
		metadata->keys.resize(blockKeycount);
		for (size_t i = 0; i < blockKeycount; ++i) {
			size_t keyLength;
			in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
			metadata->keys[i].resize(keyLength);
			in.read(&metadata->keys[i][0], keyLength);
		}



		return metadata;


	}

};

void cpin(string& sourcePath, string& destName, size_t blockSize);
void ls(const char* command);

vector<string> split(const char* str, char delimeter) {
	vector<string> res;


	const char* start = str; // Pointer to the beginning of the substring
	const char* end = nullptr; // Pointer top the position of the delimeter

	while ((end = strchr(start, delimeter)) != nullptr) { // strchr - find the first occurence of the delimeter \ in the input string
		// calculate the length of the substring
		size_t length = end - start;

		// Create a substring and add it to the result
		res.emplace_back(start, length);

		// Move the start pointer to the character after the delimiter
		start = end + 1;


	}
	if (*start != '\0') { // if not empty
		res.emplace_back(start);
	}

	return res;

}


uint64_t findDestName(const char* command, string&FileName, bool isFile = true) {
	std::ifstream src("container.bin", std::ios::binary); // opens the file in binary mode , file is read byte by byte
	if (!src) {
		std::cerr << "Error: Cannot open source file. \n";
		return -1;
	}
	vector<string> directories = split(command, '\\');
	if (isFile) {
		FileName = directories.back();
		directories.pop_back();
	}
	// we start with parent offset > 
	vector<uint64_t> offsets;

	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	for (const auto& dir : directories) {
		if (!metadataHashtable.exists(dir)) {
			cerr << "Error: The path specified doesnt exist!\n";
			delete parentMeta;
			delete childMeta;
			return -1;
		}
		uint64_t offset = metadataHashtable.get(dir);
		src.seekg(offset, ios::beg);
		childMeta = Metadata::deserialize(src);
		if (parentMeta != nullptr) {
			bool isPart = false;
			for (const auto& off : parentMeta->children) {
				if (off == childMeta->offset) {
					isPart = true;
					break;
				}
			}
			if (!isPart) {
				cerr << "Error: " << dir << " is not a valid child of its parent\n";
				delete parentMeta;
				delete childMeta;
				return -1;

			}
		}
		
		offsets.push_back(offset);
		delete parentMeta; // free prvious parent
		parentMeta = childMeta; // current child becomes the new parent
		childMeta = nullptr; // reset childmeta
	}
	delete parentMeta;
	return offsets[offsets.size() - 1];
}

void InitializeContainer(string& containerPath);


int main(int argc, char* argv[]) {
	/*if (argc < 2) {
		std::cerr << "Ussage: <command> [arguments]\n";
		return 1;
	}*/
	string fileSystem = "container.bin";
	string outfile = "C:\\Users\\vboxuser\\Desktop\\aaa.txt";
	string fileName = "bbb.txt";
	InitializeContainer(fileSystem);
	string command = "cpin";
	if (command == "cpin") {
		cpin(outfile, fileName, 4096);
	}
	//else if (command == "ls") {
	//	ls();
	//}
	//else if (command == "rm") {
	//	deleteFile("bbb.txt");
	//}

	return 0;
}
vector<uint64_t> freeBlocks;
vector<uint64_t> freeMeta;

void InsertionSort(vector<uint64_t>& array) {
	size_t len = array.size();
	for (int i = 1; i < len; ++i) {
		int key = array[i];
		int j = i - 1;

		while (j >= 0 && array[j] > key) {
			array[j + 1] = array[j];
			--j;
		}
		array[j + 1] = key;
	}
}

std::vector<uint64_t> allocatedContiguosBlocks(size_t requiredBlocks, size_t blockSize) {
	std::vector<uint64_t> allocatedBlocks;
	InsertionSort(freeBlocks);
	size_t contiguouscount = 1;

	for (size_t i = 1; i < freeBlocks.size(); ++i) {
		if (freeBlocks[i] == freeBlocks[i - 1] + blockSize) {
			++contiguouscount;
			if (contiguouscount == requiredBlocks) {
				allocatedBlocks.assign(freeBlocks.begin() + i - requiredBlocks +1, freeBlocks.begin() +i +1);
				freeBlocks.erase(freeBlocks.begin() + i - requiredBlocks +1, freeBlocks.begin() + i+1);
				return allocatedBlocks;
			}
		}
		else {
			contiguouscount = 1; // reset if not contiguous
		}
	}
	return allocatedBlocks; // Empty if no contiguous space is found

}

void deleteFile(string& fileName) {
	string actual_fileName;
	uint64_t parent_offset = findDestName(fileName.c_str(), actual_fileName);
	if (parent_offset == -1) {
		return;
	}
	fileName = actual_fileName;

	if(!metadataHashtable.exists(fileName)){
		std::cerr << "Error: file " << fileName << " not found in the container" << endl;
		return;
	}
	ifstream container("container.bin", std::ios::binary);
	if (!container) {
		std::cerr << "Error: cannot open the conatiner" << endl;
		return;
	}
	ofstream containerWrite("container.bin", std::ios::binary);
	if (!container) {
		std::cerr << "Error: cannot open the conatiner" << endl;
		return;
	}

	// Read an process	the container file
	uint64_t offset = metadataHashtable.get(fileName);

	// remove the meta from parent children
	container.seekg(parent_offset, ios::beg);
	Metadata* parentMeta = Metadata::deserialize(container);
	parentMeta->children.erase(remove(parentMeta->children.begin(), parentMeta->children.end(), offset), parentMeta->children.end());
	
	// Write back the changed meta for the parent
	containerWrite.seekp(parent_offset, ios::beg);
	parentMeta->serialize(containerWrite);


	// find the metadata offset for the file 
	container.seekg(offset, ios::beg);
	// deserialize the meta to get all variables
	Metadata* fileMeta = Metadata::deserialize(container);
	// mark it as deleted so other files can overwrite it
	fileMeta->isDeleted = true;

	// delete all blocks assosiated with the meta file 
	for (string key: fileMeta->keys) {
		// firstly add all the blocks to a list of reusable blocks
		freeBlocks.push_back(blockHashTable.get(key)); // getting the offset of the actual block, that will lead to the content of the block
		// remove the block from hashtable
		blockHashTable.remove(key); // remove the key from the block hash table 
	}
	// clear the list
	fileMeta->keys.clear(); 
	// Update the metadata in the container to rteflect the deletion
	// go to the location of the fileMeta so we can overwrite it as deleted
	containerWrite.seekp(offset, ios::beg);
	fileMeta->serialize(containerWrite);
	// add it to the free meta file
	freeMeta.push_back(offset);

	string s(fileMeta->name);
	metadataHashtable.remove(s);
	delete fileMeta;
}

void InitializeContainer(string& containerPath) {
	ofstream container(containerPath, ios::binary | ios::trunc);
	if (!container) {
		cerr << "Error: Cannot create container.\n";
		return;
	}
	// Create the root directory metadata
	Metadata rootDir;
	strncpy_s(rootDir.name, "/", sizeof(rootDir.name));
	rootDir.isDirectory = true;
	rootDir.size = 0;
	rootDir.parent = 0; // Root has no parent
	rootDir.isDeleted = false;

	rootDir.offset = 0;

	// Write the root directory metadata to the container
	rootDir.serialize(container);
	metadataHashtable.insert("/", 0);

	cout << "Container initialized with root directory.\n";


}

void ls(const char* command) {
	// we need to find the offset of the dir that we want to ls 
	string fileName;
	uint64_t last_offset = findDestName(command, fileName, false);
	if (last_offset == -1) {
		return;
	}
	std::ifstream container("container.bin", std::ios::binary);
	if (!container) {
		std::cerr << "Error: Cannot open container.\n";
		return;
	}
	container.seekg(last_offset, ios::beg);
	Metadata* parentMeta = Metadata::deserialize(container);
	for (const auto& off : parentMeta->children) {
		container.seekg(off, ios::beg);
		Metadata* currentChild = Metadata::deserialize(container);
		cout << currentChild->name << " " << currentChild->size << endl;
		delete currentChild;
	}
	delete parentMeta;
	
}

void printBlockAndContent(const string& filePath, uint64_t offset) {
	ifstream file(filePath, ios::binary);
	if (!file) {
		std::cerr << "Error: Cannot open file.\n";
		return;
	}

	// seek the offset
	file.seekg(offset, ios::beg);
	if (!file) {
		std::cerr << "Error: Seek failed. Offset may be invalid.\n";
		return;
	}
	Block* block = Block::deserialize(file);
	file.seekg(block->content_offset, ios::beg);
	vector<char> buffer(block->size);

	file.read(buffer.data(), block->size);

	cout << "Block content: ";
	for (char c : buffer) {
		cout << c;
	}

	file.close();
	delete block;

	
}

void printMeta(const string& filePath, uint64_t offset) {
	ifstream file(filePath, ios::binary);
	if (!file) {
		std::cerr << "Error: Cannot open file.\n";
		return;
	}
	file.seekg(offset, ios::beg);
	Metadata* meta = Metadata::deserialize(file);
	if (meta->isDirectory) {
		for (const auto& child_offset : meta->children) {
			file.seekg(child_offset, ios::beg);
			Metadata* child_meta = Metadata::deserialize(file);
			cout << child_meta->name << " " << child_meta->size;
			delete child_meta;
		}
	}
	else {
		for (const auto& child_block_key : meta->keys) {
			uint64_t of = metadataHashtable.get(child_block_key);
			printBlockAndContent(filePath,of);

		}
	}
	delete meta;
}

// TASK: WHEN A  FILE GET DELETED EVERY BLOCK HAS ITS OWN BLOCK META, SO I NEED TO CONSIDER THIS 
void cpout(const string& containerPath, string fileName, const string outputPath) {
	string actualFile;
	uint64_t parent_offset = findDestName(fileName.c_str(), actualFile);
	if (parent_offset == -1) {
		return;
	}
	fileName = actualFile;


	ifstream container(containerPath, ios::binary);
	if (!container) {
		std::cerr << "Failed: cannot open container.\n";
		return;
	}

	if (!metadataHashtable.exists(fileName)) {
		cerr << "Error: File not found in the container.\n";
		return;
	}
	uint64_t metadataOffset = metadataHashtable.get(fileName);

	// seek to the metadata offset and deserialize the meta
	container.seekg(metadataOffset, ios::beg);
	Metadata* fileMeta = Metadata::deserialize(container);

	ofstream outputFile(outputPath, ios::binary);
	if (!outputFile) {
		cerr << "Error: Cannot create output file.\n";
		return;
	}
	char buffer[4096];
	for (const auto& blockKey : fileMeta->keys) {
		if (!blockHashTable.exists(blockKey)) {
			cerr << "Error: block not found for key " << blockKey << ".\n";
			return;
		}
		uint64_t blockOffset = blockHashTable.get(blockKey);

		// seek to the block offset and read the block struct to get the content offset
		container.seekg(blockOffset, ios::beg);
		Block* block = Block::deserialize(container);
		container.seekg(block->content_offset, ios::beg);
		container.read(buffer, block->size); // Adjust size if block sizes vary
		if (!container) {
			cerr << "Error: Failed to read block data.\n"; 
			return;
		}

		outputFile.write(buffer, container.gcount());
		delete block;
		block = nullptr;

	}
	delete fileMeta;
	fileMeta = nullptr;
	cout << "File successfully copied to " << outputPath << ".\n";

}

void cpin(string& sourcePath, string& destName, size_t blockSize) {
	string fileName;
	uint64_t dir_offset = findDestName(destName.c_str(), fileName);
	if (dir_offset == -1) {
		return;
	}
	destName = fileName;
	std::ifstream src(sourcePath, std::ios::binary); // opens the file in binary mode , file is read byte by byte
	if (!src) {
		std::cerr << "Error: Cannot open source file. \n";
		return;
	}


	std::ofstream container("container.bin", std::ios::binary | std::ios::app);
	if (!container) {
		std::cerr << "Error: Cannot open conatiner.\n";
		return;
	}

	// Metadata for the file
	Metadata fileMeta;
	strncpy_s(fileMeta.name, destName.c_str(), sizeof(fileMeta.name));
	fileMeta.isDirectory = false;
	fileMeta.isDeleted = false;
	fileMeta.size = 0;
	fileMeta.parent = dir_offset;


	// Get the file size 
	src.seekg(0, ios::end);
	size_t filesize = src.tellg();
	src.seekg(0, ios::beg);
	

	// Calculate the required number of blocks
	size_t requiredBlocks = (filesize + blockSize - 1) / blockSize; // ceiling divisiopn

	// Step 1: Try to allocated from free blocks
	vector<uint64_t> allocatedBlocks = allocatedContiguosBlocks(requiredBlocks, blockSize);

	// Step 2 : If no free blocks are available, append to the end of the file
	if (allocatedBlocks.empty()) {
		// Get the current end of the container
		container.seekp(0, ios::end);
		uint64_t currentOffset = container.tellp();

		// Allocate new blocks at the end of the container
		for (size_t i = 0; i < requiredBlocks; ++i) {
			allocatedBlocks.push_back(currentOffset);
			currentOffset += blockSize;
		}


	}
	// copy file content in chunks
	char buffer[4096];
	size_t remainingSize = filesize;
	for(size_t i =0;i< allocatedBlocks.size();++i){

		size_t chunk_size = std::min(blockSize, remainingSize);
		src.read(buffer, chunk_size);
		size_t bytesRead = src.gcount();


		Block block;
		block.hashBl = block.hashBlock(buffer, bytesRead);
		block.size = bytesRead;
		// block doesnt exitsts
		if (!blockHashTable.exists(block.hashBl)) {
			block.writeToContainer(container, buffer, bytesRead, allocatedBlocks[i]);

			// add the block hjash to tghe hashtable
			blockHashTable.insert(block.hashBl, block.block_offset);
		}
		
		fileMeta.size += bytesRead;
		fileMeta.keys.push_back(block.hashBl);
		remainingSize -= bytesRead;
	}
	string str(fileMeta.name);
	//fileMeta.offset = container.tellp();
	uint64_t metaOff;
	if (!freeMeta.empty()) {
		metaOff = freeMeta.back();
		freeMeta.pop_back();
	}
	else {
		metaOff = container.tellp();
	}
	fileMeta.offset = metaOff;
	container.seekp(metaOff, ios::beg);
	fileMeta.serialize(container);
	metadataHashtable.insert(str, fileMeta.offset);

	printMeta(sourcePath,fileMeta.offset);

	// Write metadata to the container 
	cout << "File copied to container.\n";
}

void md(string& containerPath, string& directoryName) {
	string directory;
	uint64_t parent_offset = findDestName(directoryName.c_str(), directory);
	if (parent_offset == -1) {
		return;
	}
	directoryName = directory;

	ifstream container(containerPath, ios::binary | ios::in);
	if (!container) {
		cerr << "Error: Cannot open container.\n";
		return;
	}
	ofstream containerWrite(containerPath, ios::binary | ios::in);
	if (!containerWrite) {
		cerr << "Error: Cannot open container.\n";
		return;
	}

	container.seekg(currentDirectoryOffset, ios::beg);
	Metadata* currentMeta = Metadata::deserialize(container);

	// Check if a child with the same name already exist
	for (const auto& childOffset: currentMeta->children) {
		container.seekg(childOffset, ios::beg);
		Metadata* childMeta = Metadata::deserialize(container);
		if (string(childMeta->name) == directoryName && !childMeta->isDeleted) {
			cerr << "Error: Didrectory with thes ame name already exists.\n";
			delete childMeta;
			return;
		}
		delete childMeta;
	}

	// Create new directory metadata
	Metadata* newDirMeta = new Metadata();
	strncpy_s(newDirMeta->name, directoryName.c_str(), sizeof(newDirMeta->name));
	newDirMeta->isDirectory = true;
	newDirMeta->size = 0;
	newDirMeta->parent = parent_offset;
	newDirMeta->isDeleted = false;

	// Determine the offset for the new directory metadata
	containerWrite.seekp(0, ios::end);
	uint64_t newdirOffset = containerWrite.tellp();
	newDirMeta->offset = newdirOffset;

	// Write new directory metadata with the new child
	containerWrite.seekp(newdirOffset, ios::beg);
	newDirMeta->serialize(containerWrite);
	metadataHashtable.insert(string(newDirMeta->name), newdirOffset);


	// Update current direcotry metadata wioth the new child
	currentMeta->children.push_back(newdirOffset);
	containerWrite.seekp(currentDirectoryOffset, ios::beg);
	currentMeta->serialize(containerWrite);

	cout << "Directory " << directoryName << " created successfully\n";

}

// 2 options - cd Home\user\soemwhere or cd Home
void cd(string& containerPath, string& directoryName) {
	string directory;
	uint64_t parent_offset = findDestName(directoryName.c_str(), directory);
	if (parent_offset == -1) {
		return;
	}
	directoryName = directory;
	ifstream container(containerPath, ios::binary);
	if (!container) {
		cerr << "Error: Cannot open container.\n";
		return;
	}
	container.seekg(parent_offset, ios::beg);
	Metadata* currentDirMeta = Metadata::deserialize(container);
	if (!currentDirMeta->isDirectory || currentDirMeta->isDeleted) {
		cerr << "Error: There is a corruptuion with the direcotries.\n";
		return;
	}
	currentDirectoryOffset = parent_offset;
	
	delete currentDirMeta;
}

void cdDots(string& containerPath) {
	ifstream container(containerPath, ios::binary);
	if (!container) {
		cerr << "Error: Cannot open container.\n";
		return;
	}

	container.seekg(currentDirectoryOffset, ios::beg);
	Metadata* currentMeta = Metadata::deserialize(container);
	if (!currentMeta->isDirectory || currentMeta->isDeleted) {
		cerr << "Error: There is a corruptuion with the direcotries.\n";
		return;
	}
	if (currentMeta->parent == 0) {
		cout << "Already in home.\n";
		return;
	}
	currentDirectoryOffset = currentMeta->parent;

}

void cdRoot(string& containerPath) {
	ifstream container(containerPath, ios::binary);
	if (!container) {
		cerr << "Error: Cannot open container.\n";
		return;
	}
	currentDirectoryOffset = 0;
}

void DeleteDir(ifstream& containerRead, ofstream& containerWrite,Metadata* currentMeta) {
	for (const auto& off : currentMeta->children) {
		containerRead.seekg(off, ios::beg);
		Metadata* currentFile = Metadata::deserialize(containerRead);
		if (currentFile->isDirectory && !currentFile->isDeleted) {
			DeleteDir(containerRead, containerWrite, currentFile);
			currentFile->isDeleted = true;
			currentFile->children.clear();
			freeMeta.push_back(currentFile->offset);
			containerWrite.seekp(currentFile->offset, ios::beg);
			currentFile->serialize(containerWrite);
		}
		else {
			string name(currentFile->name);
			deleteFile(name);
		}
		delete currentFile;

	}
}

// 2 options rd Folderaa or rd Home\Folderaaa
void rd(string& containerPath, string& dirName) {
	// delete a folder in current directory, if there are other files or directories in that, thy are also deleted
	string dir;
	uint64_t parent_offset = findDestName(dirName.c_str(), dir);
	if (parent_offset == -1) {
		return;
	}
	dirName = dir;

	ifstream containerRead(containerPath, ios::binary);
	if (!containerRead) {
		cerr << "Error: Cannot open container.\n";
		return;
	}
	ofstream containerWrite(containerPath, ios::binary);
	if (!containerWrite) {
		cerr << "Error: Cannot open container.\n";
		return;
	}

	containerRead.seekg(parent_offset, ios::beg);
	Metadata* currentMeta = Metadata::deserialize(containerRead);
	if (!currentMeta->isDirectory || currentMeta->isDeleted) {
		cerr << "Error: There is a corruptuion with the direcotries.\n";
		return;
	}
	uint64_t offset_remove;
	for (const auto& offset : currentMeta->children) {
		containerRead.seekg(offset, ios::beg);
		Metadata* childMeta = Metadata::deserialize(containerRead);
		if (string(childMeta->name) == dirName && childMeta->isDirectory && !childMeta->isDeleted) {
			DeleteDir(containerRead, containerWrite, currentMeta);
			offset_remove = childMeta->offset;
			childMeta->isDeleted = true;
			freeMeta.push_back(childMeta->offset);
			containerWrite.seekp(childMeta->offset, ios::beg);
			childMeta->serialize(containerWrite);
			break;
		}
		delete childMeta;
	}
	currentMeta->children.erase(remove(currentMeta->children.begin(), currentMeta->children.end(), offset_remove), currentMeta->children.end());
	containerWrite.seekp(currentDirectoryOffset, ios::beg);
	currentMeta->serialize(containerWrite);

	delete currentMeta;


}