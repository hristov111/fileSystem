#include <iostream>
#include <list>
#include <cstring>
#include <vector>
#include <fstream>
#include <list>
#include <string>
#include <functional> 

using namespace std;
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
	vector<string> block_keys; // keys to the hashtable for the blocks
	uint64_t parent; // Parent directory's index in the metadata

	void serialize(std::ofstream& out) {
		out.write(reinterpret_cast<const char*>(&isDirectory), sizeof(isDirectory));
		out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(&parent), sizeof(parent));

		// Write= the size of the vector
		size_t blockKeyCount = block_keys.size();
		out.write(reinterpret_cast<const char*>(&blockKeyCount), sizeof(blockKeyCount));

		// Write each string in the vectopr
		for (const auto& key : block_keys) {
			size_t keyLength = key.size();
			out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
			out.write(key.data(), keyLength);
		}





	}

	static Metadata* deserialize(std::ifstream& in) {
		Metadata* metadata = new Metadata();

		// Read fixed size fields
		in.read(metadata->name, sizeof(metadata->name));
		in.read(reinterpret_cast<char*>(&metadata->isDirectory), sizeof(metadata->isDirectory));



	}

};
void saveBlocksToContainer(const std::vector<Block*>& blocks, const std::string& containerfile) {
	ofstream container(containerfile, ios::binary | ios::app);
	if (!container) {
		std::cerr << "Error: cannot open container file for writing.\n";
		return;
	}

	for (const Block* block : blocks) {
		block->serialize(container);
	}
	container.close();
	std::cout << "Blocks serialized and saved\n";
}

struct Directory {
	uint64_t parent;
	vector<uint64_t> children;
};
void cpin(const char* sourcePath, const char* destName, size_t blockSize);
void ls();



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

		table[index].emplace_back(key,value);
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


int main(int argc, char* argv[]) {
	/*if (argc < 2) {
		std::cerr << "Ussage: <command> [arguments]\n";
		return 1;
	}*/

	string command = "cpin";
	if (command == "cpin") {
		cpin("C:\\Users\\vboxuser\\Desktop\\aaa.txt", "bbb.txt", 4096);
	}

	return 0;
}
vector<uint64_t> freeBlocks;

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

void deleteFile(const string& fileName) {
	if(!metadataHashtable.exists(fileName)){
		std::cerr << "Error: file " << fileName << " not found in the container" << endl;
		return;
	}
	ifstream container("container.bin", std::ios::binary);
	if (!container) {
		std::cerr << "Error: cannot open the conatiner" << endl;
		return;
	}

	// Read an process	the container file
	uint64_t offset = metadataHashtable.get(fileName);
	Metadata fileMeta;


	container.seekg(offset);
	container.read(reinterpret_cast<char*>(&fileMeta), sizeof(Metadata));

	for (string key: fileMeta.block_keys) {
		freeBlocks.push_back(blockHashTable.get(key));
		blockHashTable.remove(key);
	}
	fileMeta.block_keys.clear();
	string s(fileMeta.name);
	metadataHashtable.remove(s);
}


void ls() {
	std::ifstream container("container.bin", std::ios::binary);
	if (!container) {
		std::cerr << "Error: Cannot open container.\n";
		return;
	}
	metadataHashtable.iterate([&container](const string& fileName, uint64_t offset) {
		// seek to the metdatada offset in the container file
		container.seekg(offset);

		// Read metadata
		Metadata fileMeta;
		container.read(reinterpret_cast<char*>(&fileMeta), sizeof(Metadata));

		// Print metadata information
		std::cout << fileMeta.name << "\t" << fileMeta.size << "B\n";
	});
}

Block* readBlockMeta(const string& filePath, uint64_t offset, size_t dataSize) {
	ifstream file(filePath, ios::binary);
	if (!file) {
		std::cerr << "Error: Cannot open file.\n";
		return {};
	}

	// seek the offset
	file.seekg(offset, ios::beg);
	if (!file) {
		std::cerr << "Error: Seek failed. Offset may be invalid.\n";
		return {};
	}

	 return Block::deserialize(file);

	
}

void cpin(const char* sourcePath, const char* destName, size_t blockSize) {
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
	strncpy_s(fileMeta.name, destName, sizeof(fileMeta.name));
	fileMeta.isDirectory = false;
	fileMeta.size = 0;


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
		fileMeta.block_keys.push_back(block.hashBl);
		remainingSize -= bytesRead;
	}
	string str(fileMeta.name);
	fileMeta.offset = container.tellp();
	metadataHashtable.insert(str, fileMeta.offset);

	// Write metadata to the container 
	container.write(reinterpret_cast<char*>(&fileMeta), sizeof(fileMeta));
	cout << "File copied to container.\n";
}