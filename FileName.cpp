#include <iostream>
#include <list>
#include <cstring>
#include <filesystem>
#include <vector>
#include <fstream>
#include <list>
#include <string>
#include <functional> 
#include <type_traits>



using namespace std;
template <typename T>
char* my_string(T value) {
	static_assert(is_integral<T>::value, "Only integral types are supported");
	static char buffer[21]; // Large enough for signed 64-bit numbers
	int index = 20;
	buffer[index] = '\0'; // null terminate the string

	bool is_Negative = false;

	// Handle negative values for signed types
	if constexpr (is_signed<T>::value) {
		if (value < 0) {
			is_Negative = true;
			value = -value; // Convert to positive for processing
		}
	}

	do {
		buffer[--index] = '0' + (value % 10); // extract the last digit
		value /= 10;
	} while (value > 0);

	if (is_Negative) {
		buffer[--index] = '-'; // add the negative sign if needed 
	}

	return &buffer[index]; // return a pointer to the start of the valids tring

}
struct Metadata {
	// if we have metas with the same names we can use numbering method (we can use incremental numbering)
	char name[100]; // file or directory name
	size_t id;
	bool isDirectory; // True if it's a directory
	uint64_t offset; // offset in the container
	uint64_t size; // size of the file (0 for directories)
	vector<string> keys; // keys to the hashtable for the blocks
	vector<uint64_t> children; // offsets of child files or directories 
	uint64_t parent; // Parent directory's index in the metadata (0 for root)
	bool isDeleted;

	void serialize(std::ofstream& out) {
		cout << "FileMeta name: " << name << endl
			<< "FileMeta size: " << size << endl

			<< "FileMeta id: " << id << endl

			<< "FileMeta isdeleted: " << isDeleted << endl

			<< "FileMeta isDirectory: " << isDirectory << endl

			<< "FileMeta parent: " << parent << endl
			<< "FileMeta offset: " << offset << endl;
		out.clear();
		out.write(name, sizeof(name));
		out.write(reinterpret_cast<const char*>(&id), sizeof(id));
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
		in.read(reinterpret_cast<char*>(&metadata->id), sizeof(metadata->id));
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
		cout << "FileMeta name: " << metadata->name << endl
			<< "FileMeta size: " << metadata->size << endl

			<< "FileMeta id: " << metadata->id << endl

			<< "FileMeta isdeleted: " << metadata->isDeleted << endl

			<< "FileMeta isDirectory: " << metadata->isDirectory << endl

			<< "FileMeta parent: " << metadata->parent << endl
			<< "FileMeta offset: " << metadata->offset << endl;



		return metadata;


	}

	string makeKey() {
		return string(name) +"_"+my_string(id);
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

	// this os only for metadata because they have duplicate names
	bool findByBaseName(const string& baseName) {

		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				if (kv.first.find(baseName + "_") == 0) {
					return true;
				}
			}
		}
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
	// this is for the fileMeta idenitfier if it has 
	size_t id = 0;
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
		rootDir.id = id;
		incrementId();

		rootDir.offset = 0;
		container.seekp(rootDir.offset, ios::beg);
		// Write the root directory metadata to the container
		metadataHashtable.insert("/", rootDir.offset);
		rootDir.serialize(container);


		cout << "Container initialized with root directory.\n";

		size_t placeholder = 0; // reserved for the hashtables and vectors offsets
		for (size_t i = 0; i < 5; ++i) {
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

		inputStream.read(reinterpret_cast<char*>(&id), sizeof(id));

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

	size_t& getNextId() { return id; }
	void incrementId() { id++; }

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
		outputStream.write(reinterpret_cast<const char*>(&id), sizeof(id));

		outputStream.write(reinterpret_cast<const char*>(&vec1Offset), sizeof(vec1Offset));
		outputStream.write(reinterpret_cast<const char*>(&vec2Offset), sizeof(vec2Offset));

		outputStream.write(reinterpret_cast<const char*>(&hash1Offset), sizeof(hash1Offset));

		outputStream.write(reinterpret_cast<const char*>(&hash2Offset), sizeof(hash2Offset));
		// Ensure the file streams are properly closed
		if (inputStream.is_open()) inputStream.close();
		if (outputStream.is_open()) outputStream.close();

		
	}

};

uint64_t currentDirectoryOffset = 0;
// this needs to be serialized and deserialzied


struct Block {
	uint64_t content_offset; // offset of the block content
	uint64_t block_offset; // offset of the block metadata
	long int numberOfFiles = 0;
	uint64_t size; // size of the block
	string hashBl;
	uint32_t checksum; // Resilliency checksum
	
	void serialize(std::ofstream& out) const {
		std::cout << "Serializing Block - content_offset: " << content_offset
			<< ", block_offset: " << block_offset
			<< ", numberOfFiles: " << numberOfFiles
			<< ", size: " << size
			<< ", checksum: " << checksum
			<< ", hashBl: " << hashBl << std::endl;
		out.clear();
		out.write(reinterpret_cast<const char*>(&content_offset), sizeof(content_offset));
		out.write(reinterpret_cast<const char*>(&block_offset), sizeof(block_offset));
		out.write(reinterpret_cast<const char*>(&numberOfFiles), sizeof(numberOfFiles));

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
		in.clear();
		in.read(reinterpret_cast<char*>(&block->content_offset), sizeof(block->content_offset));
		in.read(reinterpret_cast<char*>(&block->block_offset), sizeof(block->block_offset));
		in.read(reinterpret_cast<char*>(&block->numberOfFiles), sizeof(block->numberOfFiles));

		in.read(reinterpret_cast<char*>(&block->size), sizeof(block->size));
		in.read(reinterpret_cast<char*>(&block->checksum), sizeof(block->checksum));

		// Deserialize the hash string
		size_t hashLength;		
		in.read(reinterpret_cast<char*>(&hashLength), sizeof(hashLength)); 

		block->hashBl.resize(hashLength);
		in.read(&block->hashBl[0], hashLength);
		std::cout << "Deserializing Block - content_offset: " << block->content_offset
			<< ", block_offset: " << block->block_offset
			<< ", numberOfFiles: " << block->numberOfFiles
			<< ", size: " << block->size
			<< ", checksum: " << block->checksum
			<< ", hashBl: " << block->hashBl << std::endl;
		return block;

	}
	void writeToContainer(std::ofstream& container, const char* buffer, size_t buffersize,uint64_t offset) {
		block_offset = offset;
		size = buffersize;
		numberOfFiles = 1;
		container.clear();
		container.seekp(offset, ios::beg);

		// computer checksum (somple XOR checksum)
		checksum = 0;
		for (size_t i = 0; i < buffersize; ++i) {
			checksum ^= buffer[i];
		}

		// write the block's data to thec ontainer
		content_offset = offset + sizeof(Block);
		if (!container) {
			throw runtime_error("Error: Failed to seek to block offset");
		}
		serialize(container);
		container.flush();
		// Write the block's data to the container
		container.clear();
		container.seekp(content_offset, ios::beg);
		if (!container) {
			throw runtime_error("Error: Failed to see to content offset");
		}
		container.write(buffer, buffersize);
		if (!container) {
			throw runtime_error("Error: Failed to write blocjk data to container");
		}
		container.flush();
		if (!container) {
			throw runtime_error("Error: Failed to flush the container stream");
		}

	}
	string hashBlock(const char* block, size_t size) {
		string hash;
		for (size_t i = 0; i < size; ++i) {
			hash += to_string((block[i] + i) % 256);
		}
		return hash;
	}

	size_t getSerializedSize() const {
		size_t fiexedSize = sizeof(content_offset) + sizeof(block_offset) + sizeof(numberOfFiles)
			+ sizeof(size) + sizeof(checksum);

		size_t dynamicsize = sizeof(size_t) + hashBl.size();
		return fiexedSize + dynamicsize;
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
void rm(ResourceManager& re, string& fileName, uint64_t parentoffset=-1);
void InitializeContainer(ResourceManager& re, string& containerPath);
void ls(ResourceManager& re, string path = "");
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
bool checkNames(ifstream& container, uint64_t parent_offset, string dirName);
uint64_t getMetadataOff(ResourceManager& re, ifstream& containerRead, Metadata& fileMeta);


// IMP: WHEN we create a file i need to update the size of every parent directory up to the root size
int main(int argc, char* argv[]) {
	/*if (argc < 2) {
		std::cerr << "Ussage: <command> [arguments]\n";
		return 1;
	}*/
	string fileSystem = "container.bin";
	string newDir = "Ehaa";
	string outfile = "C:\\Users\\vboxuser\\Desktop\\aaa.txt";
	string fileName = "meg2.txt";

	ResourceManager resource(fileSystem);
	string command = "cpin";
	if (command == "md") {
		md(resource, newDir);
	}
	if (command == "cpin") {
		cpin(resource,outfile, fileName, 4096);
	}
	else if (command == "ls") {
		ls(resource, newDir);
	}
	else if (command == "cpout") {
		cpout(resource,newDir,"C:\\Users\\vboxuser\\Desktop\\eheeee.txt");
	}
	else if (command == "rm") {
		rm(resource, newDir);
	}
	else if (command == "cd") {
		cd(resource,newDir);
	}
	else if (command == "rd") {
		rd(resource, newDir);
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

uint64_t getOffsetByParent(ifstream& container,uint64_t parent_offset, string fileName) {
	container.clear();
	container.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(container);
	for (const auto& child_off : parent->children) {
		container.clear();
		container.seekg(child_off, ios::beg);
		Metadata* child = Metadata::deserialize(container);
		if (string(child->name) == fileName) {
			delete parent;	
			uint64_t off = child->offset;
			delete child;
			return off;

		}
		delete child;
	}
	delete parent;
	return -1;
}


void rm(ResourceManager& re , string& fileName, uint64_t actualOffset) {
	ifstream& containerRead = re.getInputStream();
	ofstream& containerWrite = re.getOutputStream();

	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	vector<string> directories = split(fileName.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t child_offset = currentDirectoryOffset;
	uint64_t parent_offset;

	if (!directories.empty() && hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			uint64_t offset;
			// we need to check also if the dir is some of the first because the last doesnt exist;
			// check if current folder doesnt exist in the meta table

			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			// problem is that when there are two files with the 
			// same names but in different folders we need to find the right file, so we need the offset of the right file
			if (actualOffset != -1) {
				offset = actualOffset;
			}
			else if (directories.size() == 1) {
				offset = getOffsetByParent(containerRead, child_offset, dir);
			}
			else {
				offset = getOffsetByParent(containerRead, child_offset, dir);
			}
			// check if the actual offfset exist in the parent children
			if (offset == -2) {
				cerr << "The file doesnt exist in that folder!\n";
				return;
			}
			// 2 options, one is the actual file is in root dir, so we deserialize the root and get the child that matches the name(two files with the same name cant be in a folder)
			// second option is the file is not in the root and is in a different folder, in that case we also deserialize the parent to get offset

			if (directories.back() == dir) {
				delete parentMeta;
				delete childMeta;
				offsets.push_back(offset);
				break;
			}
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
			child_offset = childMeta->offset;
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		child_offset = offsets.back();
	}
	else {
		return;
	}
	// get the child first 
	containerRead.clear();
	containerRead.seekg(child_offset, ios::beg);
	Metadata* child = Metadata::deserialize(containerRead);
	child->isDeleted = true;
	// get the offset of the parent
	parent_offset = child->parent;

	// remove the meta from parent children
	containerRead.clear();
	containerRead.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(containerRead);
	parent->children.erase(remove(parent->children.begin(), parent->children.end(), child_offset), parent->children.end());
	parent->size -= child->size;

	containerWrite.clear();
	containerWrite.seekp(parent_offset, ios::beg);
	parent->serialize(containerWrite);


	// but what if the actual block is being used by others, we wont delete it and we wont add it to the freeblocks 
	// to check if other files are using the block we need to have a variable for each block and the number of files that are using that block
	// delete all blocks assosiated with the meta file 
	for (string key: child->keys) {
		// firstly add all the blocks to a list of reusable blocks
		uint64_t off = re.getBlockHashTable().get(key);
		containerRead.clear();
		containerRead.seekg(off, ios::beg);
		Block* block = Block::deserialize(containerRead);
		if (block->numberOfFiles < 2) {
			block->size = 0;
			block->numberOfFiles = 0;
			containerWrite.clear();
			containerWrite.seekp(off, ios::beg);
			block->serialize(containerWrite);
			re.getFreeblocks().push_back(off); // getting the offset of the actual block, that will lead to the content of the block
			// remove the block from hashtable
			re.getBlockHashTable().remove(key); // remove the key from the block hash table 
		}
		else {
			block->numberOfFiles--;
			containerWrite.clear();
			containerWrite.seekp(off, ios::beg);
			block->serialize(containerWrite);
		}
	}
	// clear the list
	child->keys.clear(); 
	// Update the metadata in the container to rteflect the deletion
	// go to the location of the fileMeta so we can overwrite it as deleted
	containerWrite.clear();
	containerWrite.seekp(child_offset, ios::beg);
	child->serialize(containerWrite);
	// add it to the free meta file
	re.getFreeMeta().push_back(child_offset);
	re.getMetadataHashTable().remove(child->makeKey());
	delete parent;
	delete child;
}

uint64_t getMetadataOff(ResourceManager& re, ifstream& containerRead, Metadata& fileMeta) {
	uint64_t metaOff;
	if (!re.getFreeMeta().empty()) {
		metaOff = re.getFreeMeta().back();
		re.getFreeMeta().pop_back();
		// we need to get the deleted filemeta id and use it  in the new and not overwrite it
		containerRead.clear();
		containerRead.seekg(metaOff, ios::beg);
		Metadata* deletedMeta = Metadata::deserialize(containerRead);
		fileMeta.id = deletedMeta->id;
		delete deletedMeta;

	}
	else {
		containerRead.clear();
		containerRead.seekg(0, ios::end);
		metaOff = containerRead.tellg();
		// Increment id for the next meta because we wont be using the deleted
		re.incrementId();
	}
	return metaOff;
}

void ls(ResourceManager& re, string path) {
	ifstream& containerRead = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	vector<string> directories = split(path.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t parent_offset = currentDirectoryOffset;
	if (!directories.empty() && !hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			uint64_t offset;
			// we need to check also if the dir is some of the first because the last doesnt exist;
			// check if current folder doesnt exist in the meta table
			// checking if dir doesnt exist in the hashtable 
			// here the last name must be a dir
			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			// checking if the path actually exists by going over the children of every current parent
			if (directories.size() == 1) {
				offset = getOffsetByParent(containerRead, parent_offset, dir);
			}
			else {
				offset = getOffsetByParent(containerRead, parent_offset, dir);
			}
			if (offset == -1) {
				cerr << "Error: Path is invalid!\n";
				return;
			}
			if (directories.back() == dir) {
				delete parentMeta;
				delete childMeta;
				offsets.push_back(offset);
				break;
			}
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
			parent_offset = childMeta->offset;
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		path = directories.back();
	}
	else if (directories.empty()) {}
	else{
		return;
	}

	containerRead.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(containerRead);
	for (const auto& off : parent->children) {
		containerRead.seekg(off, ios::beg);
		Metadata* currentChild = Metadata::deserialize(containerRead);
		// check the parent
		containerRead.clear();
		containerRead.seekg(currentChild->parent);
		Metadata* parent = Metadata::deserialize(containerRead);
		cout << currentChild->name << " " << currentChild->size << ": parent is " << parent->name << endl;
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
	ifstream& containerRead = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	vector<string> directories = split(fileName.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t parent_offset = currentDirectoryOffset;
	if (directories.size() > 1 && hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			uint64_t offset;
			// we need to check also if the dir is some of the first because the last doesnt exist;
			// check if current folder doesnt exist in the meta table
			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}

			offset = getOffsetByParent(containerRead, parent_offset, dir);
			if (offset == -1) {
				cerr << "Error: Path of file invalid!\n";
				return;
			}
			
			if (directories.back() == dir) {
				// check if there is a file with the same name in the last dir
				delete parentMeta;
				delete childMeta;
				offsets.push_back(offset);
				break;
			}
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
			parent_offset = childMeta->offset;
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		fileName = directories.back();
	}
	else if (directories.size() == 1 && hasExtention(directories.back())) {
		parent_offset = getOffsetByParent(containerRead, currentDirectoryOffset, fileName);
		if (parent_offset == -1) {
			cerr << "The file doesnt exist in that folder!\n";
			return;
		}
	}
	else {
		return;
	}
	
	// seek to the metadata offset and deserialize the meta
	containerRead.clear();
	containerRead.seekg(parent_offset, ios::beg);
	Metadata* fileMeta = Metadata::deserialize(containerRead);

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
		containerRead.clear();

		containerRead.seekg(blockOffset, ios::beg);
		Block* block = Block::deserialize(containerRead);
		containerRead.clear();

		containerRead.seekg(block->content_offset, ios::beg);
		containerRead.read(buffer, block->size); // Adjust size if block sizes vary
		if (!containerRead) {
			cerr << "Error: Failed to read block data.\n"; 
			return;
		}

		outputFile.write(buffer, containerRead.gcount());
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
	uint64_t parent_offset = currentDirectoryOffset;
	if (directories.size() > 1 && hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			uint64_t offset;
			// we need to check also if the dir is some of the first because the last doesnt exist;
			// check if current folder doesnt exist in the meta table
			 if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			 

			 
			if (directories.back() == dir) {
				// check if there is a file with the same name in the last dir
				if (!checkNames(containerRead, parent_offset, dir)) {
					return;
				}
				break;
			}
			// always wherever i am we start from the currentoffset as parent offset
			offset = getOffsetByParent(containerRead, parent_offset, dir);
			if (offset == -1) {
				cerr << "The file doesnt exist in that folder!\n";
				return;
			}
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
			parent_offset = childMeta->offset;
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		fileName = directories.back();
		delete parentMeta;
		delete childMeta;
	}
	else if(directories.size() == 1 && hasExtention(directories.back()) && checkNames(containerRead,currentDirectoryOffset, fileName)) {
		parent_offset = currentDirectoryOffset; 
	}
	else {
		return;
	}
	

	ofstream& containerWrite = re.getOutputStream();
	containerWrite.clear();
	containerRead.clear();

	// GET THE PARENT OF THE FILE
	containerRead.seekg(parent_offset, ios::beg);
	// this is the parent of the actual file that we are adding
	Metadata* parent = Metadata::deserialize(containerRead);

	// CREATE THE FILE
	// Metadata for the file
	Metadata fileMeta;
	strncpy_s(fileMeta.name, fileName.c_str(), sizeof(fileMeta.name));
	fileMeta.isDirectory = false;
	fileMeta.isDeleted = false;
	fileMeta.size = 0;
	fileMeta.parent = parent_offset;
	fileMeta.id = re.getNextId();

	// first file metaoffset is 190 
	uint64_t metaOff = getMetadataOff(re,containerRead, fileMeta);
	
	// SERIALIZE THE FILEMETA
	fileMeta.offset = metaOff;
	re.getMetadataHashTable().insert(fileMeta.makeKey(), fileMeta.offset);

	// ADD THE FILEMETA OFFSET TO THE PARENT and serialize
	parent->children.push_back(fileMeta.offset);
	containerWrite.clear();
	containerWrite.seekp(parent_offset, ios::beg);
	parent->serialize(containerWrite);

	// serialize because of the offset we need it to be reserved
	containerWrite.clear();
	containerWrite.seekp(metaOff, ios::beg);
	fileMeta.serialize(containerWrite);
	containerWrite.flush();





	// Get the file size 
	src.seekg(0, ios::end);
	size_t filesize = src.tellg();
	src.seekg(0, ios::beg);
	
	// --------------------------- CALCULATION OF THE REQUIRED BLOCKS
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
		else { // this here means that the block exist and i need to get it and add 1 to numberOfFiles
			uint64_t off = re.getBlockHashTable().get(block.hashBl);
			// Validate the offset
			if (off < 0 || off > containerRead.tellg()) {
				throw std::runtime_error("Invalid offset retrieved from the hash table");
			}
			containerRead.clear();
			containerRead.seekg(off, ios::beg);
			Block* bl = Block::deserialize(containerRead);
			bl->numberOfFiles++;
			containerWrite.clear();
			containerWrite.seekp(off, ios::beg);
			bl->serialize(containerWrite);
			containerWrite.flush();

			delete bl;
			
		}
		
		fileMeta.size += bytesRead;
		fileMeta.keys.push_back(block.hashBl);
		remainingSize -= bytesRead;
		cout << "Before:" << endl;
	}

	containerWrite.clear();
	containerWrite.seekp(metaOff, ios::beg);
	fileMeta.serialize(containerWrite);
	containerWrite.flush();

	
	// Right here we need to loop over every parent till we reach the root and update their sizes 
	uint64_t offset = parent_offset;
	while (true) { // -1 because the parent of the root is -1 by default
		// update the size of the current meta
		parent->size += fileMeta.size;
		containerWrite.clear();
		containerWrite.seekp(offset, ios::beg);
		parent->serialize(containerWrite);
		containerWrite.flush();

		// take the parent of the current meta
		offset = parent->parent;
		// delete current meta

		delete parent;
		parent = nullptr;
		// check if the next offset is the root parent which doesnt exist and thats why we break
		if (offset == -1) break;
		//deserialize the parent of the current meta
		containerRead.clear();
		containerRead.seekg(offset, ios::beg);
		parent = Metadata::deserialize(containerRead);
	}
	containerWrite.clear();

	// Write metadata to the container 
	cout << "File copied to container.\n";
}

// this function checks in the parent folder if there are other files or directories with the same name as the one that we want to create 
bool checkNames(ifstream& container,uint64_t parent_offset, string dirName) {
	// First deserialize the parent
	container.clear();
	container.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(container); 

	for (const auto& child_offset : parent->children) {
		container.clear();
		container.seekg(child_offset);
		Metadata* child = Metadata::deserialize(container);
		if (string(child->name) == dirName) {
			cerr << "There is a file with the same name in the directory please choose a different name!\n";
			delete parent;
			delete child;
			return false;
		}
		delete child;
	}
	delete parent; 
	return true;
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
	uint64_t parent_offset = currentDirectoryOffset;
	container.clear();
	if (directories.size() > 1 && !hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			// we need to check also if the dir is some of the first because the last doesnt exist;
			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			if (directories.back() == dir) {
				// check if there is a file with the same name in the last dir
				if (!checkNames(container, parent_offset, dir)) {
					return;
				}
				break;
			}
			uint64_t offset = getOffsetByParent(container,parent_offset, dir);

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
			parent_offset = childMeta->offset;
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		directoryName = directories.back();
	}
	else if (directories.size() == 1 && !hasExtention(directories.back()) && checkNames(container,currentDirectoryOffset, directoryName)) {
		parent_offset = currentDirectoryOffset;
	}
	else {
		return;
	}
	ofstream& containerWrite = re.getOutputStream();
	containerWrite.clear();
	containerWrite.flush();

	container.seekg(parent_offset, ios::beg);
	Metadata* currentMeta = Metadata::deserialize(container);

	// Create new directory metadata
	Metadata* newDirMeta = new Metadata();
	strncpy_s(newDirMeta->name, directoryName.c_str(), sizeof(newDirMeta->name));
	newDirMeta->isDirectory = true;
	newDirMeta->size = 0;
	newDirMeta->parent = parent_offset;
	newDirMeta->isDeleted = false;
	newDirMeta->id = re.getNextId();

	// Determine the offset for the new directory metadata
	uint64_t newdirOffset = getMetadataOff(re, container, *newDirMeta);
	newDirMeta->offset = newdirOffset;

	// Write new directory metadata with the new child
	containerWrite.seekp(newdirOffset, ios::beg);
	newDirMeta->serialize(containerWrite);
	re.getMetadataHashTable().insert(newDirMeta->makeKey(), newdirOffset);


	// Update current direcotry metadata wioth the new child
	currentMeta->children.push_back(newdirOffset);
	containerWrite.seekp(parent_offset, ios::beg);
	currentMeta->serialize(containerWrite);

	cout << "Directory " << directoryName << " created successfully\n";
	printMeta(re, newdirOffset);
}

// 2 options - cd Home\user\soemwhere or cd Home
void cd(ResourceManager& re, string& directoryName) {
	ifstream& container = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	vector<string> directories = split(directoryName.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t parent_offset;
	container.clear();
	if (!directories.empty() && !hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			// we need to check also if the dir is some of the first because the last doesnt exist;
			uint64_t offset;
			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			if (directories.size() == 1) {
				offset = getOffsetByParent(container, currentDirectoryOffset, dir);
			}
			else {
				offset = getOffsetByParent(container, parentMeta->offset, dir);
			}
			if (directories.back() == dir) {
				delete parentMeta;
				delete childMeta;
				offsets.push_back(offset);
				break;
			}			container.seekg(offset, ios::beg);
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
	else if (directories.empty()) {
		parent_offset = currentDirectoryOffset;
	}
	else {
		return;
	}



	container.clear();
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
	container.clear();
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

// Imp - we need build a function where i can build up to the root to update size of the nodes 
void DeleteDir(ResourceManager& re, ifstream& containerRead, ofstream& containerWrite,Metadata* currentMeta) {
	for (const auto& off : currentMeta->children) {
		containerRead.clear();
		containerRead.seekg(off, ios::beg);
		Metadata* currentFile = Metadata::deserialize(containerRead);
		if (currentFile->isDirectory && !currentFile->isDeleted) {
			DeleteDir(re,containerRead, containerWrite, currentFile);
			currentFile->isDeleted = true;
			currentFile->children.clear();
			re.getFreeMeta().push_back(currentFile->offset);
			containerWrite.clear();
			containerWrite.seekp(currentFile->offset, ios::beg);
			currentFile->serialize(containerWrite);
		}
		else {
			string name(currentFile->name);
			rm(re, name, currentFile->offset);
		}
		delete currentFile;

	}
}

// 2 options rd Folderaa or rd Home\Folderaaa
void rd(ResourceManager& re, string& dirName) {
	ifstream& containerRead = re.getInputStream();
	ofstream& containerWrite = re.getOutputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	vector<string> directories = split(dirName.c_str(), '\\');
	vector<uint64_t> offsets;
	uint64_t parent_offset = currentDirectoryOffset;
	// after all of this i need to get the acutal last dir
	if (!directories.empty() && !hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			uint64_t offset;
			// we need to check also if the dir is some of the first because the last doesnt exist;
			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return;
			}
			if (directories.size() == 1) {
				offset = getOffsetByParent(containerRead, parent_offset, dir);
			}
			else {
				offset = getOffsetByParent(containerRead, parent_offset, dir);
			}
			if (directories.back() == dir) {
				offsets.push_back(offset);
				delete parentMeta;
				delete childMeta;
				break;
			}
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
			parent_offset = childMeta->offset;
			offsets.push_back(offset);
			delete parentMeta; // free prvious parent
			parentMeta = childMeta; // current child becomes the new parent
			childMeta = nullptr; // reset childmeta
		}
		parent_offset = offsets.back();
		dirName = directories.back();
	}
	else {
		cerr << "Please choose a directory to delete.\n";
		return;
	}

	// Reading the child meta
	containerRead.clear();
	containerRead.seekg(parent_offset, ios::beg);
	Metadata* currentMeta = Metadata::deserialize(containerRead);
	// Reading the parent meta
	containerRead.clear();
	containerRead.seekg(currentMeta->parent, ios::beg);
	parentMeta = Metadata::deserialize(containerRead);
	// deleting the currentMeta children 
	DeleteDir(re, containerRead, containerWrite, currentMeta);
	// updating the actual file for reuse
	currentMeta->isDeleted = true;
	currentMeta->children.clear();
	re.getFreeMeta().push_back(currentMeta->offset);
	containerWrite.clear();
	containerWrite.seekp(currentMeta->offset, ios::beg);
	currentMeta->serialize(containerWrite);

	// Removing the offset from children in parent and decreasing the size also
	parentMeta->children.erase(remove(parentMeta->children.begin(), parentMeta->children.end(), currentMeta->offset), parentMeta->children.end());
	parentMeta->size -= currentMeta->size;
	// serializing the child
	containerWrite.clear();
	containerWrite.seekp(parentMeta->offset, ios::beg);
	currentMeta->serialize(containerWrite);

	delete currentMeta;


}