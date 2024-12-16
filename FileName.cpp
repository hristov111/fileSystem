#include <iostream>
#include <list>
#include <cstring>
#include <filesystem>
#include <vector>
#include <fstream>
#include <list>
#include <string>
#include <functional> 


using namespace std;
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
		out.clear();
		out.write(name, sizeof(name));
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
		in.clear();
		// Read fixed size fields
		in.read(metadata->name, sizeof(metadata->name));
		in.read(reinterpret_cast<char*>(&metadata->isDirectory), sizeof(metadata->isDirectory));
		in.read(reinterpret_cast<char*>(&metadata->offset), sizeof(metadata->offset));
		in.read(reinterpret_cast<char*>(&metadata->size), sizeof(metadata->size));
		in.read(reinterpret_cast<char*>(&metadata->parent), sizeof(metadata->parent));
		in.read(reinterpret_cast<char*>(&metadata->isDeleted), sizeof(metadata->isDeleted));


		// Read each string in the vector
		  // Deserialize the keys vector
		size_t blockKeyCount;
		in.read(reinterpret_cast<char*>(&blockKeyCount), sizeof(blockKeyCount));
		if (blockKeyCount > 0) {
			metadata->keys.resize(blockKeyCount);
			for (size_t i = 0; i < blockKeyCount; ++i) {
				size_t keyLength;
				in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
				if (keyLength > 0) {
					metadata->keys[i].resize(keyLength);
					in.read(&metadata->keys[i][0], keyLength);
				}
			}
		}

		// Deserialize the children vector
		size_t childrenCount;
		in.read(reinterpret_cast<char*>(&childrenCount), sizeof(childrenCount));
		if (childrenCount > 0) {
			metadata->children.resize(childrenCount);
			for (size_t i = 0; i < childrenCount; ++i) {
				in.read(reinterpret_cast<char*>(&metadata->children[i]), sizeof(metadata->children[i]));
			}
		}

		return metadata;


	}

};

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

	void serialize(ofstream& out) const {
		if (!out) {
			throw runtime_error("Error: Output stream is not valid.");
		}

		size_t totalCount = 0;
		// Count the total number of key-value pairs
		for (int i = 0; i < TABLE_SIZE; ++i) {
			totalCount += table[i].size();
		}

		out.write(reinterpret_cast<const char*>(&totalCount), sizeof(totalCount));

		// Write each key-value pair
		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				size_t keyLength = kv.first.size();
				out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
				out.write(kv.first.data(), keyLength);
				out.write(reinterpret_cast<const char*>(&kv.second), sizeof(kv.second));
			}
		}
	}

	void deserialize(ifstream& in) {
		if (!in) {
			throw runtime_error("Error: Input stream is not valid");
		}

		// Clear the current hash table
		for (int i = 0; i < TABLE_SIZE; ++i) {
			table[i].clear();
		}

		size_t totalCount;
		// Read the total cout ovchar* key-value pairs
		in.read(reinterpret_cast<char*>(&totalCount), sizeof(totalCount));

		// Read each key value pair
		for (size_t i = 0; i < totalCount; ++i) {
			size_t keyLength;
			in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));

			string key(keyLength, '\0');
			in.read(&key[0], keyLength);

			uint64_t value;
			in.read(reinterpret_cast<char*>(&value), sizeof(value));

			//Inser the key-value pair into the hash table

			insert(key, value);
		}

	}
};
class ResourceManager {
private:
	ifstream inputStream;
	ofstream outputStream;
	vector<uint64_t> freeBlocks;
	vector<uint64_t> freeMeta;
	HashTable blockHashTable;
	HashTable metadataHashtable;
	string fileName;
	uint64_t vec1Offset = 0;
	uint64_t vec2Offset = 0;
	uint64_t hash1Offset = 0;
	uint64_t hash2Offset = 0;

	void serializeVector(vector<uint64_t>& vec, ofstream& out) {
		if (!out) {
			throw runtime_error("Error: Output stream is not valid");
		}

		// Write the size of the vector
		size_t size = vec.size();
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));

		// Write the elements of the vector
		for (const auto& element : vec) {
			out.write(reinterpret_cast<const char*>(&element), sizeof(element));
		}
	}

	vector<uint64_t> deserializeVector(ifstream& in) {
		if (!in) {
			throw runtime_error("Error: Input stream is not valid");
		}

		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));

		vector<uint64_t> vec(size);
		for (size_t i = 0; i < size; ++i) {
			in.read(reinterpret_cast<char*>(&vec[i]), sizeof(vec[i]));
		}
		return vec;
	}
	bool InitializeContainer() {
		
		ifstream filecheck(fileName, ios::binary | ios::in);
		if (filecheck) {
			filecheck.seekg(0, ios::end);
			if (filecheck.tellg() > 0) {
				inputStream.open(fileName, ios::binary | ios::in);
				outputStream.open(fileName, ios::binary | ios::in | ios::out);

				if (!inputStream || !outputStream) {
					throw runtime_error("Error: Cannot open container file for reading and writing.\n");
				}

				return false;  // No initialization was required
			}
		}

		// if the file does not exist or is empty, initialize it 
		ofstream container(fileName,ios::binary | ios::out);
		if (!container) {
			throw runtime_error("Error: Cannot create container file");
		}

		// Create the root directory metadata
		Metadata rootDir;
		strncpy_s(rootDir.name, "/", sizeof(rootDir.name));
		rootDir.isDirectory = true;
		rootDir.size = 0;
		rootDir.parent = -1; // Root has no parent
		rootDir.isDeleted = false;

		rootDir.offset = 0;
		container.seekp(rootDir.offset, ios::beg);
		// Write the root directory metadata to the container
		metadataHashtable.insert("/", rootDir.offset);
		rootDir.serialize(container);


		cout << "Container initialized with root directory.\n";

		size_t placeholder = 0; // reserved for the hashtables and vectors offsets
		for (size_t i = 0; i < 4; ++i) {
			container.write(reinterpret_cast<const char*>(&placeholder), sizeof(placeholder));
		}

		container.close();

		inputStream.open(fileName, ios::binary | ios::in);
		outputStream.open(fileName, ios::binary | ios::in | ios::out);

		if (!inputStream || !outputStream) {
			throw runtime_error("Error : Cannot open contaioner file for reading and writing.\n");
		}
		return true;
	}

public:
	explicit ResourceManager(const string& file) : fileName(file) {
		if (InitializeContainer()) {
			return;
		}
		inputStream.clear();
		inputStream.seekg(sizeof(Metadata), ios::beg);

		inputStream.read(reinterpret_cast<char*>(&vec1Offset), sizeof(vec1Offset));
		inputStream.read(reinterpret_cast<char*>(&vec2Offset), sizeof(vec2Offset));

		inputStream.read(reinterpret_cast<char*>(&hash1Offset), sizeof(hash1Offset));

		inputStream.read(reinterpret_cast<char*>(&hash2Offset), sizeof(hash2Offset));

		if (vec1Offset != 0) {
			inputStream.seekg(vec1Offset, ios::beg);
			freeBlocks = deserializeVector(inputStream);
		}
		if (vec2Offset != 0) {
			inputStream.seekg(vec2Offset, ios::beg);
			freeMeta = deserializeVector(inputStream);
		}

		if (hash1Offset != 0) {
			inputStream.seekg(hash1Offset, ios::beg);
			blockHashTable.deserialize(inputStream);
		}

		if (hash2Offset != 0) {
			inputStream.seekg(hash2Offset, ios::beg);
			metadataHashtable.deserialize(inputStream);
		}




	}

	ifstream& getInputStream() { return inputStream; }
	ofstream& getOutputStream() { return outputStream; }

	vector<uint64_t>& getFreeblocks() {
		return freeBlocks;
	}
	vector<uint64_t>& getFreeMeta() {
		return freeMeta;
	}

	HashTable& getBlockHashTable() {
		return blockHashTable;
	}
	HashTable& getMetadataHashTable() {
		return metadataHashtable;
	}

	

	~ResourceManager() {
		// Serialize data when the program end
		outputStream.clear();
		outputStream.seekp(0, ios::end); // Move to the end of the file
		//Write vector 1
		vec1Offset = outputStream.tellp();
		serializeVector(freeBlocks, outputStream);

		// Write vector 2
		vec2Offset = outputStream.tellp();
		serializeVector(freeMeta, outputStream);

		// Write HaSh table 1
		hash1Offset = outputStream.tellp();
		blockHashTable.serialize(outputStream);

		// Write hash table 2
		hash2Offset = outputStream.tellp();
		metadataHashtable.serialize(outputStream);

		outputStream.seekp(sizeof(Metadata), ios::beg);
		outputStream.write(reinterpret_cast<const char*>(&vec1Offset), sizeof(vec1Offset));
		outputStream.write(reinterpret_cast<const char*>(&vec2Offset), sizeof(vec2Offset));

		outputStream.write(reinterpret_cast<const char*>(&hash1Offset), sizeof(hash1Offset));

		outputStream.write(reinterpret_cast<const char*>(&hash2Offset), sizeof(hash2Offset));
		outputStream.close();
		// Ensure the file streams are properly closed
		if (inputStream.is_open()) inputStream.close();
		if (outputStream.is_open()) outputStream.close();

		
	}

};

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
		container.clear();
		container.seekp(offset);

		// computer checksum (somple XOR checksum)
		checksum = 0;
		for (size_t i = 0; i < buffersize; ++i) {
			checksum ^= buffer[i];
		}

		// write the block's data to thec ontainer
		content_offset = offset + sizeof(Block);
		
		container.clear();
		container.seekp(offset);
		if (!container) {
			throw runtime_error("Error: Failed to seek to block offset");
		}
		serialize(container);
		
		// Write the block's data to the container
		container.seekp(content_offset);
		if (!container) {
			throw runtime_error("Error: Failed to see to content offset");
		}
		container.write(buffer, buffersize);
		if (!container) {
			throw runtime_error("Error: Failed to write blocjk data to container");
		}

	}
	string hashBlock(const char* block, size_t size) {
		string hash;
		for (size_t i = 0; i < size; ++i) {
			hash += to_string((block[i] + i) % 256);
		}
		return hash;
	}

};



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

void InsertionSort(vector<uint64_t>& array);
std::vector<uint64_t> allocatedContiguosBlocks(ResourceManager& re, size_t requiredBlocks, size_t blockSize);
void deleteFile(ResourceManager& re, string& fileName);
void InitializeContainer(ResourceManager& re, string& containerPath);
void ls(ResourceManager& re, const char* command);
void printBlockAndContent(ResourceManager& re, uint64_t offset);
void printMeta(ResourceManager& re, uint64_t offset);
void cpout(ResourceManager& re, string fileName, const string outputPath);
bool hasExtention(const string& fileName);
void cpin(ResourceManager& re, string& sourceName, string& fileName, size_t blockSize);
void md(ResourceManager& re, string& directoryName);
void cd(ResourceManager& re, string& directoryName);
void cdDots(ResourceManager& re);
void cdRoot(ResourceManager& re);
void DeleteDir(ResourceManager& re, ifstream& containerRead, ofstream& containerWrite, Metadata* currentMeta);
void rd(ResourceManager& re, string& dirName);

int main(int argc, char* argv[]) {
	/*if (argc < 2) {
		std::cerr << "Ussage: <command> [arguments]\n";
		return 1;
	}*/
	string fileSystem = "container.bin";
	string newDir = "Ehee";
	string outfile = "C:\\Users\\vboxuser\\Desktop\\aaa.txt";
	string fileName = "Ehee\\bbb.txt";
	ResourceManager resource(fileSystem);
	string command = "cpin";
	if (command == "md") {
		md(resource, newDir);
	}
	if (command == "cpin") {
		cpin(resource,outfile, fileName, 4096);
	}
	//else if (command == "ls") {
	//	ls();
	//}
	//else if (command == "rm") {
	//	deleteFile("bbb.txt");
	//}

	return 0;
}

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

std::vector<uint64_t> allocatedContiguosBlocks(ResourceManager& re, size_t requiredBlocks, size_t blockSize) {
	std::vector<uint64_t> allocatedBlocks;
	InsertionSort(re.getFreeblocks());
	size_t contiguouscount = 1;

	for (size_t i = 1; i < re.getFreeblocks().size(); ++i) {
		if (re.getFreeblocks()[i] == re.getFreeblocks()[i - 1] + blockSize) {
			++contiguouscount;
			if (contiguouscount == requiredBlocks) {
				allocatedBlocks.assign(re.getFreeblocks().begin() + i - requiredBlocks +1, re.getFreeblocks().begin() +i +1);
				re.getFreeblocks().erase(re.getFreeblocks().begin() + i - requiredBlocks +1, re.getFreeblocks().begin() + i+1);
				return allocatedBlocks;
			}
		}
		else {
			contiguouscount = 1; // reset if not contiguous
		}
	}
	return allocatedBlocks; // Empty if no contiguous space is found

}

void deleteFile(ResourceManager& re , string& fileName) {
	/*string actual_fileName;
	uint64_t parent_offset = findDestName(fileName.c_str(), actual_fileName);
	if (parent_offset == -1) {
		return;
	}
	fileName = actual_fileName;*/
	uint64_t parent_offset = 12;

	if(!re.getMetadataHashTable().exists(fileName)) {
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
	uint64_t offset = re.getMetadataHashTable().get(fileName);

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
		re.getFreeblocks().push_back(re.getBlockHashTable().get(key)); // getting the offset of the actual block, that will lead to the content of the block
		// remove the block from hashtable
		re.getBlockHashTable().remove(key); // remove the key from the block hash table 
	}
	// clear the list
	fileMeta->keys.clear(); 
	// Update the metadata in the container to rteflect the deletion
	// go to the location of the fileMeta so we can overwrite it as deleted
	containerWrite.seekp(offset, ios::beg);
	fileMeta->serialize(containerWrite);
	// add it to the free meta file
	re.getFreeMeta().push_back(offset);

	string s(fileMeta->name);
	re.getMetadataHashTable().remove(s);
	delete fileMeta;
}



void ls(ResourceManager& re, const char* command) {
	// we need to find the offset of the dir that we want to ls 
	/*string fileName;
	uint64_t last_offset = findDestName(command, fileName, false);
	if (last_offset == -1) {
		return;
	}*/
	uint64_t last_offset = 12;
	ifstream& container = re.getInputStream();
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

void printBlockAndContent(ResourceManager & re, uint64_t offset) {
	ifstream& file = re.getInputStream();

	// seek the offset
	file.clear();
	file.seekg(offset, ios::beg);
	if (!file) {
		std::cerr << "Error: Seek failed. Offset may be invalid.\n";
		return;
	}
	Block* block = Block::deserialize(file);
	file.clear();
	file.seekg(block->content_offset, ios::beg);
	vector<char> buffer(block->size);

	file.read(buffer.data(), block->size);

	cout << "Block content: ";
	for (char c : buffer) {
		cout << c;
	}
	delete block;

	
}

void printMeta(ResourceManager& re, uint64_t offset) {
	ifstream& file = re.getInputStream();
	file.clear();
	file.seekg(offset, ios::beg);
	Metadata* meta = Metadata::deserialize(file);
	cout << "Parent meta name is  " << meta->name << " and size of the file or direcotry is " << meta->size << endl;
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
			uint64_t of = re.getBlockHashTable().get(child_block_key);
			printBlockAndContent(re,of);

		}
	}
	delete meta;
}

// TASK: WHEN A  FILE GET DELETED EVERY BLOCK HAS ITS OWN BLOCK META, SO I NEED TO CONSIDER THIS 
void cpout(ResourceManager& re,  string fileName, const string outputPath) {
	/*string actualFile;
	uint64_t parent_offset = findDestName(fileName.c_str(), actualFile);
	if (parent_offset == -1) {
		return;
	}
	fileName = actualFile;*/
	uint64_t parent_offset = 12;


	ifstream& container = re.getInputStream();

	if (!re.getMetadataHashTable().exists(fileName)) {
		cerr << "Error: File not found in the container.\n";
		return;
	}
	uint64_t metadataOffset = re.getMetadataHashTable().get(fileName);

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
		if (!re.getBlockHashTable().exists(blockKey)) {
			cerr << "Error: block not found for key " << blockKey << ".\n";
			return;
		}
		uint64_t blockOffset = re.getBlockHashTable().get(blockKey);

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

bool hasExtention(const string& fileName) {
	// Initialize variables to track the position of the last dot
	int dotPosition = -1;
	int length = 0;

	// Calculate the length of the string manually
	while (fileName[length] != '\0') {
		length++;
	}
	// Iterate through the string to find the last dot
	for (int i = 0; i < length; ++i) {
		if (fileName[i] == '.') {
			dotPosition = i; // Update the pos opf the last dot
		}
	}
	// Check if a valid dot exitsts and it's not at the end or beginning
	return (dotPosition != -1 && dotPosition != 0 && dotPosition != length - 1);
}

void cpin(ResourceManager& re , string& sourceName,string& fileName, size_t blockSize) {
	std::ifstream src(sourceName, std::ios::binary); // opens the file in binary mode , file is read byte by byte
	if (!src) {
		std::cerr << "Error: Cannot open source file. \n";
		return;
	}
	ifstream& containerRead = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	vector<string> directories = split(fileName.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t parent_offset;
	if (directories.size() > 1 && hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			// we need to check also if the dir is some of the first because the last doesnt exist;
			// check if current folder doesnt exist in the meta table
			 if (!re.getMetadataHashTable().exists(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			if (directories.back() == dir) {
				// check if there is a file with the same name in the last dir
				for (const auto& child_offset: parentMeta->children) {
					containerRead.clear();
					containerRead.seekg(child_offset);
					Metadata* child = Metadata::deserialize(containerRead);
					if (string(child->name) == dir) {
						cerr << "There is a file with the same name in the directory please choose a different name!\n";
						delete parentMeta;
						delete childMeta;
						return;
					}
				}
				break;
			}
			uint64_t offset = re.getMetadataHashTable().get(dir);
			containerRead.clear();
			containerRead.seekg(offset, ios::beg);
			childMeta = Metadata::deserialize(containerRead);
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
					return;

				}
			}
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		fileName = directories.back();
	}
	else if(directories.size() == 1 && hasExtention(directories.back())) {
		parent_offset = currentDirectoryOffset;
	}
	else {
		return;
	}
	

	ofstream& containerWrite = re.getOutputStream();
	containerWrite.clear();

	// Metadata for the file
	Metadata fileMeta;
	strncpy_s(fileMeta.name, fileName.c_str(), sizeof(fileMeta.name));
	fileMeta.isDirectory = false;
	fileMeta.isDeleted = false;
	fileMeta.size = 0;
	fileMeta.parent = parent_offset;


	// Get the file size 
	src.seekg(0, ios::end);
	size_t filesize = src.tellg();
	src.seekg(0, ios::beg);
	

	// Calculate the required number of blocks
	size_t requiredBlocks = (filesize + blockSize - 1) / blockSize; // ceiling divisiopn

	// Step 1: Try to allocated from free blocks
	vector<uint64_t> allocatedBlocks = allocatedContiguosBlocks(re,requiredBlocks, blockSize);

	// Step 2 : If no free blocks are available, append to the end of the file
	if (allocatedBlocks.empty()) {
		// Get the current end of the container
		containerRead.clear();
		containerRead.seekg(0, ios::end);
		uint64_t currentOffset = containerRead.tellg();

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
		if (!re.getBlockHashTable().exists(block.hashBl)) {
			block.writeToContainer(containerWrite, buffer, bytesRead, allocatedBlocks[i]);

			// add the block hjash to tghe hashtable
			re.getBlockHashTable().insert(block.hashBl, block.block_offset);
		}
		
		fileMeta.size += bytesRead;
		fileMeta.keys.push_back(block.hashBl);
		remainingSize -= bytesRead;
	}
	string str(fileMeta.name);
	//fileMeta.offset = container.tellp();
	uint64_t metaOff;
	if (!re.getFreeMeta().empty()) {
		metaOff = re.getFreeMeta().back();
		re.getFreeMeta().pop_back();
	}
	else {
		containerWrite.clear();
		containerWrite.seekp(0, ios::end);
		metaOff = containerWrite.tellp();
	}
	fileMeta.offset = metaOff;
	containerWrite.clear();
	containerWrite.seekp(metaOff, ios::beg);
	fileMeta.serialize(containerWrite);
	containerWrite.flush();
	re.getMetadataHashTable().insert(str, fileMeta.offset);

	printMeta(re,fileMeta.offset);

	// we need to update parent meta children
	containerRead.clear();
	containerRead.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(containerRead);
	parent->children.push_back(fileMeta.offset);
	containerWrite.clear();
	containerWrite.seekp(parent_offset,ios::beg);
	parent->serialize(containerWrite);
	delete parent;



	// Write metadata to the container 
	cout << "File copied to container.\n";
}

// when making a directory we have two options: 1. it is a single directory to make in the current dir (the same as cpin but whithout the extetion check (because its dir)) 
// 12/16 - works perfectly
void md(ResourceManager& re, string& directoryName) {
	

	ifstream& container = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	vector<string> directories = split(directoryName.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t parent_offset;
	container.clear();
	if (directories.size() > 1 && !hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			// we need to check also if the dir is some of the first because the last doesnt exist;
			if (!re.getMetadataHashTable().exists(dir) && directories.back() == dir) {
				delete parentMeta;
				delete childMeta;
				break;
			}
			else if (!re.getMetadataHashTable().exists(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			else if(re.getMetadataHashTable().exists(dir) && directories.back() == dir){
				cerr << "Error: directory with the same name already exits in the directory, please choose a different one\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			uint64_t offset = re.getMetadataHashTable().get(dir);
			container.seekg(offset, ios::beg);
			childMeta = Metadata::deserialize(container);
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
					return;

				}
			}
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		directoryName = directories.back();
	}
	else if (directories.size() == 1 && !hasExtention(directories.back())) {
		parent_offset = currentDirectoryOffset;
	}
	else {
		return;
	}
	ofstream& containerWrite = re.getOutputStream();
	containerWrite.clear();

	container.seekg(parent_offset, ios::beg);
	Metadata* currentMeta = Metadata::deserialize(container);

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
	re.getMetadataHashTable().insert(string(newDirMeta->name), newdirOffset);


	// Update current direcotry metadata wioth the new child
	currentMeta->children.push_back(newdirOffset);
	containerWrite.seekp(parent_offset, ios::beg);
	currentMeta->serialize(containerWrite);

	cout << "Directory " << directoryName << " created successfully\n";
	printMeta(re, newdirOffset);
}

// 2 options - cd Home\user\soemwhere or cd Home
void cd(ResourceManager& re, string& directoryName) {
	/*string directory;
	uint64_t parent_offset = findDestName(directoryName.c_str(), directory);
	if (parent_offset == -1) {
		return;
	}
	directoryName = directory;*/
	uint64_t parent_offset = 12;
	ifstream& container = re.getInputStream();
	container.seekg(parent_offset, ios::beg);
	Metadata* currentDirMeta = Metadata::deserialize(container);
	if (!currentDirMeta->isDirectory || currentDirMeta->isDeleted) {
		cerr << "Error: There is a corruptuion with the direcotries.\n";
		return;
	}
	currentDirectoryOffset = parent_offset;
	
	delete currentDirMeta;
}

void cdDots(ResourceManager& re) {
	ifstream& container = re.getInputStream();

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

void cdRoot(ResourceManager& re) {
	currentDirectoryOffset = 0;
}

void DeleteDir(ResourceManager& re, ifstream& containerRead, ofstream& containerWrite,Metadata* currentMeta) {
	for (const auto& off : currentMeta->children) {
		containerRead.seekg(off, ios::beg);
		Metadata* currentFile = Metadata::deserialize(containerRead);
		if (currentFile->isDirectory && !currentFile->isDeleted) {
			DeleteDir(re,containerRead, containerWrite, currentFile);
			currentFile->isDeleted = true;
			currentFile->children.clear();
			re.getFreeMeta().push_back(currentFile->offset);
			containerWrite.seekp(currentFile->offset, ios::beg);
			currentFile->serialize(containerWrite);
		}
		else {
			string name(currentFile->name);
			deleteFile(re, name);
		}
		delete currentFile;

	}
}

// 2 options rd Folderaa or rd Home\Folderaaa
void rd(ResourceManager& re, string& dirName) {
	// delete a folder in current directory, if there are other files or directories in that, thy are also deleted
	/*string dir;
	uint64_t parent_offset = findDestName(dirName.c_str(), dir);
	if (parent_offset == -1) {
		return;
	}
	dirName = dir;*/
	uint64_t parent_offset = 12;
	ifstream& containerRead = re.getInputStream();
	ofstream& containerWrite = re.getOutputStream();


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
			DeleteDir(re,containerRead, containerWrite, currentMeta);
			offset_remove = childMeta->offset;
			childMeta->isDeleted = true;
			re.getFreeMeta().push_back(childMeta->offset);
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