#include <iostream>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <functional> 
#include <type_traits>
#include <iterator> // For std::iterator
using namespace std;
#include <iostream>
#include <cstdint>
#include <cstring>

class CRC32 {
private:
	static constexpr uint32_t polynomial = 0xEDB88320; // Standard CRC32 polynomial
	uint32_t crcTable[256]; // Precomputed CRC table

	void generateCRCTable() {
		for (uint32_t i = 0; i < 256; ++i) {
			uint32_t crc = i;
			for (uint32_t j = 0; j < 8; ++j) {
				if (crc & 1) {
					crc = (crc >> 1) ^ polynomial;
				}
				else {
					crc >>= 1;
				}
			}
			crcTable[i] = crc;
		}
	}

public:
	CRC32() {
		generateCRCTable(); // Generate table on construction
	}

	uint32_t calculate(const char* data, size_t length) {
		uint32_t crc = 0xFFFFFFFF; // Initial value
		for (size_t i = 0; i < length; ++i) {
			uint8_t byte = static_cast<uint8_t>(data[i]);
			crc = (crc >> 8) ^ crcTable[(crc ^ byte) & 0xFF];
		}
		return crc ^ 0xFFFFFFFF; // Final XOR value
	}
};


template <typename Key, typename Value>
class myPair {
public:
	Key first;
	Value second;

	myPair(const Key& key, const Value& value) : first(key), second(value) {}

	myPair(Key&& key, Value&& value): first(move(key)), second(move(value)) {}


	myPair() : first(), second() {}

	myPair(const myPair& other) : first(other.first), second(other.second) {}

	myPair(myPair&& other) noexcept : first(move(other.first)), second(move(other.second)) {}

	// Assigment oprator
	myPair& operator=(const myPair& other) {
		if (this != &other) {
			first = other.first;
			second = other.second;
		}
		return *this;
	}
	// Move asssigment oprator
	myPair& operator=(myPair && other) noexcept{
		if (this != &other) {
			first = move(other.first);
			second = move(other.second);
		}
		return *this;
	}

	bool operator==(const myPair& other) const {
		return first == other.first && second == other.second;
	}

	bool operator!=(const myPair& other) const {
		return !(*this == other);
	}

	
	

};


template<typename T>
class DynamicArray {
private:
	T* data;
	int capacity;
	int size;

public:
	DynamicArray(size_t inititalCapacity = 0) : capacity(inititalCapacity), size(0) {
		if (capacity == 0) {
			capacity = 10;
		}
		data = new T[capacity];
		for (size_t i = size; i < capacity; ++i) {
			data[i] = T();

		}
	}
	DynamicArray(size_t initialCapacity, const T& defaultvalue):capacity(initialCapacity), size(0) {
		if (capacity == 0) {
			capacity = 10;
		}
		data = new T[capacity];
		for (size_t i = size; i < capacity; ++i) {
			data[i] = defaultvalue;

		}
	}
	DynamicArray(const DynamicArray& other) : size(other.size), capacity(other.capacity){
		data = new T[capacity];
		for (size_t i = 0; i < size; ++i) {
			data[i] = other.data[i];
		}
	}

	DynamicArray(DynamicArray&& other) noexcept : data(other.data), capacity(other.capacity),
	size(other.size){
		other.data = nullptr;
		other.capacity = 0;
		other.size = 0;
	}
	DynamicArray& operator=(DynamicArray&& other) noexcept {
		if (this != &other) {
			delete[] data; // Free current memory
			data = other.data;
			capacity = other.capacity;
			size = other.size;

			other.data = nullptr;
			other.capacity = 0;
			other.size = 0;
		}
		return *this;
	}
	void insertionSort(function<bool(const T&, const T&)> comparator,bool ascending = true) {
		for (int i = 0; i < size; ++i) {
			T current = data[i];
			int j = i - 1;

			while (j >= 0 && (ascending ?comparator(current, data[j]): comparator(data[j], current))) {
				data[j + 1] = data[j];
				--j;
			}
			data[j + 1] = current;
		}
	}

	DynamicArray& operator=(const DynamicArray& other) {
		if (this != &other) {
			delete[] data;
			size = other.size;
			capacity = other.capacity;
			data = new T[capacity];
			for (size_t i = 0; i < size; ++i) {
				data[i] = other.data[i];
			}
		}
		return *this;
	}

	~DynamicArray() {
		if (data != nullptr) {
			delete[] data;
			data = nullptr;
		}
	}

	T* get_data() {
		return this->data;
	}

	const T* get_data() const {
		return this->data;
	}
	void resize(int newCapacity) {
		if (newCapacity > capacity) {
			T* newData = new T[newCapacity];

			for (size_t i = 0; i < size; ++i) {
				newData[i] = std::move(data[i]);
			}

			// Zero - initialize the res of the buffer
			for (size_t i = size; i < newCapacity; ++i) {
				newData[i] = T();

			}
			delete[] data;
			data = newData;
			capacity = newCapacity;
		}
	}

	void concatenate(const DynamicArray& other) {
		if (size + other.getSize() > capacity) {
			resize(size + other.getSize());
		}

		// append element from other to the current array
		for (int i = 0; i < other.getSize(); ++i) {
			data[size++] = other.data[i];
		}
	}

	void push_back(T &value) {
		if (size == capacity) {
			resize(capacity * 2);
		}
		data[size++] = value;
	}

	void push_back(T&& value) {
		if (size == capacity) {
			resize(capacity * 2);
		}
		data[size++] = move(value);
	}

	void remove(int index) {
		if (index < 0 || index >= size) {
			throw std::out_of_range("Index out of bounds.");

		}

		if constexpr (!std::is_trivially_destructible<T>::value) {
			data[index].~T();
		}

		for (int i = index; i < size - 1; ++i) {
			data[i] = move(data[i + 1]);
		}
		if constexpr (is_default_constructible<T>::value) {
			data[size - 1] = T(); // reset the last element
		}


		--size;
	}

	int indexOf(const T& value) const {
		for (int i = 0; i < size; ++i) {
			if (data[i] == value) {
				return i;
			}
		}
		return -1;
	}

	T& back() {
		if (size == 0) {
			throw out_of_range("Cannot access back of an empty array!\n");
		}
		return data[size-1];
	}

	bool empty() {
		return size == 0;
	}

	void clear() {
		delete[] data;
		size = 0;
		capacity = 10;
		data = new T[capacity];
	}

	void pop_back() {
		if (size == 0) {
			throw std::out_of_range("Cannot pop_back from an empty array");
		}

		// Call the destructior for the last element if it;s a not trivial
		data[size - 1].~T();

		--size;
	}

	
	

	template<typename... Args>
	void emplace_back(Args&&... args) {
		if (size == capacity) {
			resize(capacity * 2);
		}
		// use placement new to construct the object in place
		new (&data[size++]) T(forward<Args>(args)...);
	}

	int get(int index) const {
		if (index < 0 || index >= size) {
			throw out_of_range("Index out of bounds");
		}
		return data[index];
	}

	T& operator[](size_t index) {

		static T defaultValue = T();

		if (index < 0 || index >= capacity) {
			return defaultValue;
		}
		// adjust the= size only for valid indeces
		if (index >= size) {
			size = index + 1; // adjust size
		}
		return data[index];
	}

	const T& operator[](size_t index) const {
		static const T defaultValue = T(); // Default value for invalid indices (const)

		// Check for negative or out-of-bounds index
		if (index < 0 || index >= capacity) {
			return defaultValue; // Return the default value without resizing
		}

		return data[index];
	}

	int getSize() const { return size; }

	// Iterator support
	class iterator {
	private:
		T* ptr;

	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		explicit  iterator(T* p) : ptr(p) {}

		iterator& operator++() {
			++ptr;
			return *this;
		}
		iterator operator++(int) {
			iterator temp = *this;
			++(*this);
			return temp;
		}

		iterator& operator--() {
			--ptr;
			return *this;
		}
		iterator operator--(int) {
			iterator temp = *this;
			--(*this);
			return temp;
		}

		T& operator*() const { return *ptr; }
		T* operator->() const { return ptr; }

		bool operator==(const iterator& other) const { return ptr == other.ptr; }
		bool operator!=(const iterator& other) const { return ptr != other.ptr; }

		bool operator<(const iterator& other) const { return ptr < other.ptr; }

		bool operator>(const iterator& other) const { return ptr > other.ptr; }

		bool operator<=(const iterator& other) const { return ptr <=other.ptr; }
		bool operator>=(const iterator& other) const { return ptr >= other.ptr; }

		iterator operator+(difference_type n) const { return iterator(ptr + n); }
		iterator operator-(difference_type n) const { return iterator(ptr - n); }


		difference_type operator-(const iterator& other) const { return ptr - other.ptr; }
	};
	iterator begin() const { return iterator(data); }
	iterator end() const { return iterator(data + size); }


	iterator erase(iterator first, iterator last) {
		if (first < begin() || last > end() || first >= last) {
			throw out_of_range("Invalid range for erase");
		}

		// Calculate the range size
		size_t rangeSize = last - first;

		// Move elements after the range
		for (auto it = first; it + rangeSize < end(); ++it) {
			*it = move(*(it + rangeSize));
		}


		// Reduce the size of the array
		size -= rangeSize;

		return first;

	}

	// assign with iterators 
	template <typename InputIterator>
	void assign(InputIterator first, InputIterator last) {
		size_t newSize = distance(first, last);
		if (newSize > capacity) {
			resize(newSize);
		}
		size = newSize;
		size_t index = 0;
		for (auto it = first; it != last; ++it) {
			data[index++] = *it;
		}
	}


};

template<typename Value>
class node {
	Value val;
	node* next;

public:
	template <typename... Args>
	node(Args&&... args): val(std::forward<Args>(args)...) ,next(nullptr) {}

	Value& getval() {
		return val;
	}
	const Value& getval() const {
		return val;
	}

	node* getNext() {
		return next;
	}
	void setValue(Value value) {
		val = value;
	}
	void setNext(node* Next) {
		next = Next;
	}


};
template <typename Value>
class LinkedList {
	node<Value>* head;
	size_t size;

public:
	LinkedList() : head(nullptr), size(0) {}

	// add a new node to the end of the list
	void append(Value value) {
		node<Value>* newNode = new node<Value>(value);
		if (!head) {
			head = newNode;
		}
		else {
			node<Value>* temp = head;
			while (temp->getNext() != nullptr) {
				temp = temp->getNext();
			}
			temp->setNext(newNode);
		}
		++size;
	}

	// Emplace back: construct Value in-place in a new node
	template<typename... Args>
	void emplace_back(Args&&... args) {
		node<Value>* newNode = new node<Value>(forward<Args>(args)...);
		if (!head) {
			head = newNode;
		}
		else {
			node<Value>* temp = head;
			while (temp->getNext() != nullptr) {
				temp = temp->getNext();
			}
			temp->setNext(newNode);
		}
		++size;
	}

	size_t getSize() const {
		return size;
	}

	Value& operator[](int index) {
		static Value defaultValue = Value();
		if (index < 0 || static_cast<size_t>(index) >= size) {
			return defaultValue;
		}
		node<Value>* temp = head;
		for (int i = 0; i < index; ++i) {
			temp = temp->getNext();
		}
		return temp->getval();
	}
	const Value& operator[](int index) const {
		static Value defaultValue = Value();

		if (index < 0 || static_cast<size_t>(index) >= size) {
			return defaultValue;
		}
		node<Value>* temp = head;
		for (int i = 0; i < index; ++i) {
			temp = temp->getNext();
		}
		return temp->getval();
	}
	class Iterator {
		node<Value>* current;
		node<Value>* previous; // Track the previous node

	public:
		Iterator(node<Value>* start, node<Value>* prev = nullptr) : current(start), previous(prev) {}


		node<Value>* getCurrent() { return current; }
		const node<Value>* getCurrent() const { return current; }

		node<Value>* getPrevious() { return previous; }
		const node<Value>* getPrevious() const { return previous; }

		void setCurrent(node<Value>* newCurrent) { current = newCurrent; }


		// Pre-increment (++it)
		Iterator& operator++() {
			if (current) current = current->getNext();
			return *this;
		}

		// Post-increment (it++)
		Iterator operator++(int) {
			Iterator temp = *this;
			if (current) current = current->getNext();
			return temp;
		}

		Value& operator*() const { return current->getval(); }

		// Arrow operator (it->first, it->second)
		Value* operator->() const { return &(current->getval()); }

		bool operator!=(const Iterator& other) const { return current != other.getCurrent(); }

		
	};
	Iterator begin() const { return Iterator(head); }
	Iterator end() const { return Iterator(nullptr); }
	class ConstIterator {
        const node<Value>* current;

    public:
        explicit ConstIterator(const node<Value>* start) : current(start) {}

        const Value& operator*() const { return current->getval(); }
        const Value* operator->() const { return &current->getval(); }

        ConstIterator& operator++() {
            if (current) current = current->getNext();
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const ConstIterator& other) const { return current == other.current; }
        bool operator!=(const ConstIterator& other) const { return current != other.current; }
    };


	Iterator erase(Iterator it) {
		if (it.getCurrent() == nullptr) {
			out_of_range("Iterator out of range");
		}

		node<Value>* toDelete = it.getCurrent();

		if (toDelete == head) {
			// special case: removing the head
			head = head->getNext();
		}
		else {
			// General case: bypass  the current node
			if (it.getPrevious()) {
				it.getPrevious()->setNext(toDelete->getNext());
			}
		}
		it.setCurrent(toDelete->getNext());
		delete toDelete;
		--size;

		return it;

	}


	void clear() {
		node<Value>* temp;
		while (head != nullptr) {
			temp = head;
			head = head->getNext();
			delete temp;
		}
		head = nullptr;
		size = 0;
	}

	~LinkedList() {
		clear();
	}
};

template <typename Value, size_t TABLE_SIZE>
class SafeList {
	LinkedList<Value> table[TABLE_SIZE];

public:
	LinkedList<Value>& operator[](int index) {
		static LinkedList<Value> emptyList;
		if (index < 0 || index > static_cast<int>(TABLE_SIZE)) {
			return emptyList;
		}
		return table[index];
	}

	const LinkedList<Value>& operator[](int index) const {
		static LinkedList<Value> emptyList;

		if (index < 0 || index > static_cast<int>(TABLE_SIZE)) {
			return emptyList;
		}
		return table[index];
	}
};

class String {
private:
	char* data; // pointer to dynamically allocated memory for the string
	size_t length;

public:
	String() : data(nullptr), length(0) {}

	String(size_t len, char fillchar) : length(len) {
		data = new char[length + 1];
		for (size_t i = 0; i < length; ++i) {
			data[i] = fillchar; //Fill with the specified character
		}
		data[length] = '\0';
	}
	String(const char* str, size_t len = -1) {
		if (str == nullptr) {
			// Handle null pointer case
			length = 0;
			data = new char[1];
			data[0] = '\0'; // Null-terminate
			return;
		}

		if (len == static_cast<size_t>(-1)) {
			size_t temp_len = 0;
			while (str[temp_len] != '\0') {
				++temp_len;
			}

			len= temp_len;
		}

		// ensure provided len is valid
		size_t actualLen = 0;
		while (str[actualLen] != '\0') {
			++actualLen;
		}
		if (len > actualLen) {
			invalid_argument("Provided length exceeds the actual length");
		}
		length = len;

		// Calculate length manually (no strlen)
		data = new char[length + 1]; // Allocate memory for characters + null-terminator

		for (size_t i = 0; i < length; ++i) {
			data[i] = str[i]; // Copy each character
		}

		data[length] = '\0'; // Null-terminate
	}

	String(const String& other) {
		length = other.length;
		data = new char[length + 1]; // allocate memory for the new string
		custom_strcpy(data, other.data); // copy the string
	}

	String(String&& other) noexcept : data(other.data), length(other.length) {
		other.data = nullptr;
		other.length = 0;
	}

	String& operator=(const String& other) {
		if (this != &other) {
			delete[] data;
			length = other.length;
			data = new char[length + 1];
			for (size_t i = 0; i < length; ++i) {
				data[i] = other.data[i];
			}
			data[length] = '\0';
		}

		return *this;
	}
	void clear() {
		delete[] data;
		data = new char[1];
		data[0] = '\0';
		length = 0;
	}
	String& operator=(String&& other) noexcept {
		if (this != &other) {
			delete[] data;
			data = other.data;
			length = other.length;

			other.data = nullptr;
			other.length = 0;
		}
		return *this;
	}


	int find(const String& substring) const {
		if (substring.length == 0 || substring.length > length) {
			return -1;
		}
		for (size_t i = 0; i <= length - substring.length; ++i) {
			size_t j = 0;
			while (j < substring.length && data[i + j] == substring.data[j]) {
				++j;
			}
			if (j == substring.length) {
				return i; // Found the substring
			}
		}
		return -1;
	}

	bool operator==(const String& other) const {
		return custom_strcmp(data, other.data) == 0;
	}

	bool operator==(const char* otherData) const {
		return custom_strcmp(data, otherData) == 0;
	}

	static String to_string(int value) {
		char buffer[12];
		int index = 0;

		if (value == 0) {
			buffer[index++] = '0';
		}

		bool isNegative = (value < 0);
		if (isNegative) {
			value = -value;
		}

		// extract all the digits from the number
		while (value > 0) {
			buffer[index++] = '0' + (value % 10); // get the last digit as a character
			value /= 10;
		}
		// add negative sign
		if (isNegative) {
			buffer[index++] = '-';
		}
		buffer[index++] = '\0';

		// reverse the order to get the correct
		for (int i = 0, j = index - 2; i < j; ++i, --j) {
			char temp = buffer[i];
			buffer[i] = buffer[j];
			buffer[j] = temp;
		}
		return String(buffer);
	}
	bool operator!=(const String& other) const {
		return !(*this == other);
	}
	String& operator+=(const String& other) {
		if (other.length == 0) {
			return *this;
		}


		size_t newLenght = length + other.length;
		char* newData = new char[newLenght + 1];


		// copy current data
		if (data) {
			for (size_t i = 0; i < length; ++i) {
				newData[i] = data[i];
			}
		}
		// append other data
		for (size_t i = 0; i < other.length; ++i) {
			newData[length + i] = other.data[i];
		}

		newData[newLenght] = '\0';
		delete[] data;
		data = newData;
		length = newLenght;

		return *this;
	}

	// Conacatenation with another String object
	String operator+(const String& other) const {
		size_t newLength = length + other.length;
		char* newData = new char[newLength + 1];
		custom_strcpy(newData, data); // cpy the current string
		custom_strcat(newData, other.data); // Append the other stirng
		String result(newData);
		delete[] newData;
		return result;
	}

	void resize(size_t newSize, char fillchar = '\0') {
		if (newSize == length) {
			return; // n  resizeing needed
		}

		char* newData = new char[newSize + 1];
		if (newSize > length) {
			// copy existing data and fill extra space with fillchar
			if (data) {
				custom_strncpy(newData, data, length);
			}
			fill(newData + length, newData + newSize, fillchar);
		}
		else {
			if (data) {
				custom_strncpy(newData, data, newSize);
			}
		}

		newData[newSize] = '\0'; // Null-terminate the string

		delete[] data;
		data = newData;
		length = newSize;
	}

	// Concatenation with const char* (String + const char*)
	String operator+(const char* str) const {
		size_t newLength = length + custom_strlen(str);
		char* newData = new char[newLength + 1];
		custom_strcpy(newData, data);
		custom_strcat(newData, str);
		String res(newData); 
		delete[] newData;
		return res;
	}
	const char *getData()const  {
		return data;
	}
	char* getData(){
		return data;
	}

	operator const char* () const {
		return getData();
	}
	friend String operator+(const char* str, const String& s) {
		return String(str) + s; // Use the existing String + String operator
	}
	const char& operator[](size_t index) const {
		if (index >= length) {
			static const char defaultChar = '\0';
			return defaultChar;
		}
		return data[index];
	}
	char& operator[](size_t index) {
		if (index >= length) {
			static char defaultChar = '\0';
			return defaultChar;
		}
		return data[index];
	}

	size_t getSize() const { return length; }

	friend ostream& operator<<(ostream& os, const String& str) {
		os << str.data;
		return os;
	}

	size_t custom_strlen(const char* str) const{
		size_t length = 0;
		while (str[length] != '\0') {
			++length;
		}
		return length;
	}

	static char* custom_strcpy(char* dest, const char* src) {
		size_t i = 0;
		while (src[i] != '\0') {
			dest[i] = src[i];
			++i;
		}
		dest[i] = '\0';
		return dest;

	}

	static int custom_strcmp(const char* str1, const char* str2) {
		size_t i = 0;
		while (str1[i] != '\0' && str2[i] != '\0') {
			if (str1[i] != str2[i]) {
				return (str1[i] > str2[i]) ? 1 : -1;
			}
			++i;
		}
		if (str1[i] == '\0' && str2[i] == '\0') {
			return 0; // strings are eqeual;
		}
		return (str1[i] > str2[i]) ? 1 : -1;
	}

	static char* custom_strcat(char* dest, const char* src) {
		size_t dest_len = 0;
		while (dest[dest_len] != '\0') {
			++dest_len;
		}
		size_t i = 0;
		while (src[i] != '\0') {
			dest[dest_len + i] = src[i];
			++i;
		}

		dest[dest_len + i] = '\0';
		return dest;
	}

	static char* custom_strncpy(char* dest, const char* src, size_t n)  {
		size_t i = 0; 
		while (i < n && src[i] != '\0') {
			dest[i] = src[i];
			++i;
		}
		while (i < n) {
			dest[i] = '\0';
			++i;
		}
		return dest;
	}

	static int custom_strcncpy_s(char* dest, size_t destSize, const char* src, size_t count) {
		// check fo null pointers
		if (dest == nullptr || src == nullptr) {
			return -1;
		}

		// Check if dest size is valid
		if (destSize == 0) {
			return -2;
		}

		// ensure destination has enough space
		if (destSize <= count) {
			if (destSize > 0) {
				dest[0] = '\0';
			}
			return -3;
		}

		// copy up to count characters from 'src' to 'dest'
		size_t i = 0;
		while (i < count && src[i] != '\0') {
			dest[i] = src[i];
			++i;
		}


		dest[i] = '\0';

		return 0;
	}
	

	class Iterator {
	private:
		char* current;

	public:
		Iterator(char* ptr) : current(ptr){}

		char& operator*() {
			return *current;
		}
		char* operator->() {
			return current;
		}

		// Pre-Increment 
		Iterator& operator++() {
			++current;
			return *this;
		}
		Iterator operator++(int) {
			Iterator temp = *this;
			++current;
			return temp;
		}

		// Equality
		bool operator==(const Iterator& other) const {
			return current == other.current;
		}
		bool operator!=(const Iterator& other) const {
			return current != other.current;
		}
	};

	Iterator begin() {
		return Iterator(data);
	}
	Iterator end() {
		return Iterator(data + length);
	}
	class ConstIterator {
	private:
		const char* current;

	public:
		// Constructor
		ConstIterator(const char* ptr) : current(ptr) {}

		// Dereference operator
		const char& operator*() const {
			return *current;
		}

		// Arrow operator
		const char* operator->() const {
			return current;
		}

		// Pre-increment (++it)
		ConstIterator& operator++() {
			++current;
			return *this;
		}

		// Post-increment (it++)
		ConstIterator operator++(int) {
			ConstIterator temp = *this;
			++current;
			return temp;
		}

		// Equality comparison
		bool operator==(const ConstIterator& other) const {
			return current == other.current;
		}

		// Inequality comparison
		bool operator!=(const ConstIterator& other) const {
			return current != other.current;
		}
	};

	ConstIterator begin() const {
		return ConstIterator(data);
	}

	ConstIterator end() const {
		return ConstIterator(data + length);
	}

	~String() {
		if (data != nullptr) {
			delete[] data;
			data = nullptr;
		}
	}
};


class ResourceManager;


enum StructType {METADATA , BLOCK};

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
private:
	DynamicArray<String> keys; // keys to the hashtable for the blocks
	DynamicArray<uint64_t> children; // offsets of child files or directories 
public:
	static const uint32_t magicNumber = 0xBE50AA;
	char name[100]; // file or directory name
	size_t id;
	size_t max_capacity = 0; // if the metadata is for directory (this is donna help when we are making rewrites of the container)
	bool isDirectory; // True if it's a directory
	uint64_t offset; // offset in the container
	uint64_t size; 
	uint64_t parent; // Parent directory's index in the metadata (0 for root)
	uint64_t serialized_size = 0;
	bool isDeleted;
	void serialize(std::ofstream& out) {

		out.clear();
		out.write(name, sizeof(name));
		out.write(reinterpret_cast<const char*>(&max_capacity), sizeof(max_capacity));
		out.write(reinterpret_cast<const char*>(&id), sizeof(id));
		out.write(reinterpret_cast<const char*>(&isDirectory), sizeof(isDirectory));
		out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(&parent), sizeof(parent));
		out.write(reinterpret_cast<const char*>(&isDeleted), sizeof(isDeleted));
		out.write(reinterpret_cast<const char*>(&serialized_size), sizeof(serialized_size));
		out.write(reinterpret_cast<const char*>(&magicNumber), sizeof(magicNumber));




		// Write= the size of the vector
		size_t blockKeyCount = keys.getSize();
		out.write(reinterpret_cast<const char*>(&blockKeyCount), sizeof(blockKeyCount));
		size_t children_count = children.getSize();
		out.write(reinterpret_cast<const char*>(&children_count), sizeof(children_count));

		// Write each string in the vectopr
		for (const auto& key : keys) {
			size_t keyLength = key.getSize();
			out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
			out.write(key.getData(), keyLength);
		}
		for (const auto& child : children) {
			out.write(reinterpret_cast<const char*>(&child), sizeof(child));
		}
		out.flush();



	}
	void reset() {
		memset(name, 0, sizeof(name));
		max_capacity = 0;
		isDirectory = false;
		size = 0;
		parent = 0;
		isDeleted = true;

		keys.clear();
		children.clear();
	}
	static Metadata* deserialize(std::ifstream& in) {
		Metadata* metadata = new Metadata();
		in.clear();

		try {
			in.read(metadata->name, sizeof(metadata->name));
			in.read(reinterpret_cast<char*>(&metadata->max_capacity), sizeof(metadata->max_capacity));
			in.read(reinterpret_cast<char*>(&metadata->id), sizeof(metadata->id));
			in.read(reinterpret_cast<char*>(&metadata->isDirectory), sizeof(metadata->isDirectory));
			in.read(reinterpret_cast<char*>(&metadata->offset), sizeof(metadata->offset));
			in.read(reinterpret_cast<char*>(&metadata->size), sizeof(metadata->size));
			in.read(reinterpret_cast<char*>(&metadata->parent), sizeof(metadata->parent));
			in.read(reinterpret_cast<char*>(&metadata->isDeleted), sizeof(metadata->isDeleted));
			in.read(reinterpret_cast<char*>(&metadata->serialized_size), sizeof(metadata->serialized_size));
			
			uint32_t fileMagicNumber;
			in.read(reinterpret_cast<char*>(&fileMagicNumber), sizeof(fileMagicNumber));

			if (fileMagicNumber != Metadata::magicNumber) {
				throw std::runtime_error("Invalid magic number. Expected: 0x" +
					std::to_string(Metadata::magicNumber) +
					", Got: 0x" + std::to_string(fileMagicNumber));
			}

			size_t blockKeyCount;
			in.read(reinterpret_cast<char*>(&blockKeyCount), sizeof(blockKeyCount));
			size_t childrenCount;
			in.read(reinterpret_cast<char*>(&childrenCount), sizeof(childrenCount));
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
			if (childrenCount > 0) {
				metadata->children.resize(childrenCount);
				for (size_t i = 0; i < childrenCount; ++i) {
					in.read(reinterpret_cast<char*>(&metadata->children[i]), sizeof(metadata->children[i]));
				}
			}
			return metadata;
		}
		catch (const exception& e) {
			delete metadata; 
			metadata = nullptr;
			return metadata;
		}
	
	}

	String makeKey() {
		return String(name) +"_"+my_string(id);
	}

	DynamicArray<String>& getKyes() {
		return keys;
	}
	DynamicArray<uint64_t>& getChildren() {
		return children;
	}
	~Metadata(){}


	uint64_t getSerializedSize() const {
		uint64_t fixedSize = sizeof(name) + sizeof(id) + sizeof(isDirectory)
			+ sizeof(isDeleted) + sizeof(size) + sizeof(parent) + sizeof(offset) + sizeof(max_capacity) + sizeof(serialized_size) + sizeof(magicNumber);

		// dynamic size member keys (vector<string>)
		uint64_t keysSize = sizeof(size_t);
		for (const auto& key : keys) {
			keysSize += sizeof(size_t); // each string's length
			keysSize += key.getSize(); // each string's content
		}

		// Dynamic-size member: children
		uint64_t childrensize = sizeof(size_t); // sizeof vector length
		childrensize += children.getSize() * sizeof(uint64_t); // conent of vector

		// total size 
		return fixedSize + keysSize + childrensize;
	}

	



};


template<typename Key, typename Value> 
class HashTable {
private:
	static const int TABLE_SIZE = 100;
	SafeList<myPair<Key, Value>, TABLE_SIZE> table;;

	// custom hash function
	int hashFunction(const Key& key) const {
		if constexpr (std::is_integral<Key>::value) {
			// If key is an integer
			return key % TABLE_SIZE;
		}
		else if constexpr (std::is_convertible<Key, String>::value) {
			// If key is convertible to a string (e.g., std::string)
			int hash = 0;
			String keyStr = static_cast<String>(key);
			for (char c : keyStr) {
				hash = (hash * 31 + c) % TABLE_SIZE;
			}
			return hash;
		}
		else {
			// Fallback to std::hash for other types
			std::hash<Key> keyHash;
			return keyHash(key) % TABLE_SIZE;
		}
	}

public:
	// Insert a key value pair
	void insert(const Key& key, const Value& value) {
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
	uint64_t get(const Key& key) const {
		int index = hashFunction(key);

		for (const auto& kv : table[index]) {
			if (kv.first == key) { // Key found
				return kv.second;
			}
		}

		throw std::runtime_error("Key not found in the hash table");
	}

	bool exists(const Key& key) const {
		int index = hashFunction(key);

		for (const auto& kv : table[index]) {
			if (kv.first == key) { // Key exists
				return true;
			}
		}
		return false;
	}

	// this os only for metadata because they have duplicate names
	bool findByBaseName(const Key& baseName) {

		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				if (kv.first.find(baseName + "_") == 0) {
					return true;
				}
			}
		}
		return false;
	}


	bool remove(const String& key) {
		int index = hashFunction(key);

		for (auto it = table[index].begin(); it != table[index].end(); it++) {
			if (it->first == key) { // Key found
				table[index].erase(it);
				return true;
			}
		}
		return false; // key not found
	}

	DynamicArray<myPair<Key,Value>> getsortedByKey(bool key=true) const{
		DynamicArray<myPair<Key, Value>> allPairs;


		// exract all key-value pairs
		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				allPairs.emplace_back(kv);
			}
		}
		InsertionSort(allPairs, key);

		return allPairs;
		// sort the vector by key (ascending order by default)
		
	}

	void InsertionSort(DynamicArray<myPair<Key, Value>>& vec, bool key=true) const {
		for (size_t i = 1; i < vec.getSize(); i++) {
			auto current = vec[i];
			int j = i - 1;
			if (key) {
				while (j >= 0 && vec[j].first < current.first) {
					vec[j + 1] = vec[j];
					--j;
				}
				vec[j + 1] = current;
			}
			else {
				while (j >= 0 && vec[j].second < current.second) {
					vec[j + 1] = vec[j];
					--j;
				}
				vec[j + 1] = current;
			}
		}
	}

	void iterate(function<void(const String&, uint64_t&)> func) {
		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (auto& kv : table[i]) {
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
			totalCount += table[i].getSize();
		}

		out.write(reinterpret_cast<const char*>(&totalCount), sizeof(totalCount));

		// Write each key-value pair
		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				size_t keyLength = kv.first.getSize();
				out.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
				out.write(kv.first.getData(), keyLength);
				out.write(reinterpret_cast<const char*>(&kv.second), sizeof(kv.second));
			}
		}
	}

	size_t getSerializedSize() {
		size_t totalsize = sizeof(size_t); 
		for (int i = 0; i < TABLE_SIZE; ++i) {
			for (const auto& kv : table[i]) {
				totalsize += sizeof(size_t);                // For keyLength
				totalsize += kv.first.getSize();              // For key data
				totalsize += sizeof(kv.second);            // For value data
			}
		}

		return totalsize;

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

			String key(keyLength, '\0');
			in.read(&key[0], keyLength);

			uint64_t value;
			in.read(reinterpret_cast<char*>(&value), sizeof(value));

			//Inser the key-value pair into the hash table

			insert(key, value);
		}

	}
};
struct Block;
void updateInMemoryOffsetsfromBlock(ResourceManager* re, size_t& oldSize, Block& block, uint64_t newSize, bool backwards = false);
void updateInMemoryOffsets(ResourceManager* re, size_t& oldSize, Metadata& parent, uint64_t newSize, bool backwards =false);
class ResourceManager {
private:
	String currentOperation;
	ifstream inputStream;
	ofstream outputStream;
	DynamicArray<uint64_t> freeBlocks;
	DynamicArray<uint64_t> freeMeta;
	HashTable<String, uint64_t> blockHashTable;
	HashTable<String, uint64_t> metadataHashtable;
	String fileName;
	uint64_t currentDirectoryOffset = 0;

	// this is for the fileMeta idenitfier if it has 
	uint64_t root_offset;
	size_t id = 0;
	uint64_t vec1Offset;
	uint64_t vec2Offset;
	uint64_t hash1Offset;
	uint64_t hash2Offset;
	size_t vec1AllocatedSize = 1024; // Initially allocated size
	size_t vec2AllocatedSize = 1024;
	size_t hash1AllocatedSize = 1024;
	size_t hash2AllocatedSize = 1024;

	void serializeVector(DynamicArray<uint64_t>& vec, ofstream& out) {
		if (!out) {
			throw runtime_error("Error: Output stream is not valid");
		}

		// Write the size of the vector
		size_t size = vec.getSize();
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));

		// Write the elements of the vector
		for (const auto& element : vec) {
			out.write(reinterpret_cast<const char*>(&element), sizeof(element));
		}
	}

	DynamicArray<uint64_t> deserializeVector(ifstream& in) {
		if (!in) {
			throw runtime_error("Error: Input stream is not valid");
		}

		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));

		DynamicArray<uint64_t> vec(size);
		for (size_t i = 0; i < size; ++i) {
			in.read(reinterpret_cast<char*>(&vec[i]), sizeof(vec[i]));
		}
		return vec;
	}

	size_t getSerializedSize(DynamicArray<uint64_t>& vec) {
		size_t totalSize = sizeof(size_t);
		totalSize += vec.getSize() * sizeof(uint64_t); // each element's size
		return totalSize;
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
		// first we write the id and all the other offsets and stuff
		outputStream.clear();
		outputStream.seekp(0, ios::beg);
		size_t placeHolder = 0;
		for (int i = 0; i < 9; ++i) {
			outputStream.write(reinterpret_cast<const char*>(&placeHolder), sizeof(placeHolder));

		}
		vec1Offset = 9 * 8; // 9 placeholders * 8 bytes
		vec2Offset = vec1Offset + 1024;
		hash1Offset = vec2Offset + 1024;
		hash2Offset = hash1Offset + 1024;
		root_offset = hash2Offset + 1024;
		// we first write the offsets for the vectors and hashtables because they wont change in size
		// Create the root directory metadata
		Metadata rootDir;
		strncpy_s(rootDir.name, "/", sizeof(rootDir.name));
		rootDir.isDirectory = true;
		rootDir.size = 0;
		rootDir.parent = -1; // Root has no parent
		rootDir.isDeleted = false;
		rootDir.id = id;
		incrementId();

		rootDir.offset = root_offset;
		container.clear();
		container.seekp(rootDir.offset, ios::beg);
		rootDir.serialize(container);
		// Write the root directory metadata to the container
		metadataHashtable.insert("/", rootDir.offset);
		currentDirectoryOffset = rootDir.offset;
		// set the next point for wrting
		container.clear();
		std::cout << "Container initialized with root directory.\n";

		inputStream.open(fileName, ios::binary | ios::in);
		outputStream.open(fileName, ios::binary | ios::in | ios::out);

		if (!inputStream || !outputStream) {
			throw runtime_error("Error : Cannot open contaioner file for reading and writing.\n");
		}
		return true;
	}

public:
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	ResourceManager(ResourceManager&&) = delete;
	ResourceManager& operator=(ResourceManager&&) = delete;


	explicit ResourceManager(const String& file, String& op) : fileName(file) , currentOperation(op){
		if (InitializeContainer()) {
			return;
		}
		inputStream.clear();
		inputStream.seekg(0, ios::beg);
		inputStream.read(reinterpret_cast<char*>(&id), sizeof(id));
		inputStream.read(reinterpret_cast<char*>(&vec1Offset), sizeof(vec1Offset));

		inputStream.read(reinterpret_cast<char*>(&vec2Offset), sizeof(vec2Offset));

		inputStream.read(reinterpret_cast<char*>(&hash1Offset), sizeof(hash1Offset));

		inputStream.read(reinterpret_cast<char*>(&hash2Offset), sizeof(hash2Offset));

		inputStream.read(reinterpret_cast<char*>(&vec1AllocatedSize), sizeof(vec1AllocatedSize));

		inputStream.read(reinterpret_cast<char*>(&vec2AllocatedSize), sizeof(vec2AllocatedSize));
		inputStream.read(reinterpret_cast<char*>(&hash1AllocatedSize), sizeof(hash1AllocatedSize));

		inputStream.read(reinterpret_cast<char*>(&hash2AllocatedSize), sizeof(hash2AllocatedSize));

		// in order to go to that offset we need to deserialize the root calculate the offset dynamically and then


		inputStream.clear();

		inputStream.seekg(vec1Offset,ios::beg);
		freeBlocks = move(deserializeVector(inputStream));

		inputStream.clear();
		inputStream.seekg(vec2Offset, ios::beg);

		freeMeta = move(deserializeVector(inputStream));

		inputStream.clear();
		inputStream.seekg(hash1Offset, ios::beg);
		blockHashTable.deserialize(inputStream);


		inputStream.clear();
		inputStream.seekg(hash2Offset, ios::beg);
		metadataHashtable.deserialize(inputStream);

		currentDirectoryOffset = metadataHashtable.get("/");

	}

	ifstream& getInputStream() { return inputStream; }
	ofstream& getOutputStream() { return outputStream; }

	uint64_t& getCurrentDirOffset() { return currentDirectoryOffset; }


	size_t& getNextId() { return id; }
	void incrementId() { id++; }

	DynamicArray<uint64_t>& getFreeblocks() {
		return freeBlocks;
	}
	DynamicArray<uint64_t>& getFreeMeta() {
		return freeMeta;
	}

	HashTable<String, uint64_t>& getBlockHashTable() {
		return blockHashTable;
	}
	HashTable<String, uint64_t>& getMetadataHashTable() {
		return metadataHashtable;
	}

	void updateFilesInRoot() {
		inputStream.clear();
		inputStream.seekg(root_offset, ios::beg);
		Metadata* root = Metadata::deserialize(inputStream);
		for (auto& child_offset : root->getChildren()) {
			inputStream.clear();
			inputStream.seekg(child_offset, ios::beg);
			Metadata* child = Metadata::deserialize(inputStream);
			child->parent = root->offset;
			outputStream.clear();
			outputStream.seekp(child_offset, ios::beg);
			child->serialize(outputStream);
			delete child;
		}
		delete root;
	}
	
	// if ls - 0, the same as the last time
	// if its cpin - we need the last offset like the last time
	~ResourceManager() {
		// Serialize data when the program end
	




		// here i need to allocate new space to the vector and hashtables if their limit arise 
		// we need to get all the current sizes of vectors and hashtables 
		size_t vec1CurrSize = getSerializedSize(freeBlocks);
		size_t vec2CurrSize = getSerializedSize(freeMeta);

		size_t hashtable1 = blockHashTable.getSerializedSize();
		size_t hashtable2 = metadataHashtable.getSerializedSize();

		if (vec1CurrSize > vec2AllocatedSize || vec2CurrSize > vec2AllocatedSize || hashtable1 > hash1AllocatedSize || hashtable2 > hash2AllocatedSize) {
			// here we need to call a function that firstly - updates the sizes of the in memory, this means that we need to get all from the root directory to the last offset for upgrading 
			// then we need to get all the vecs and hashtables that are after the upgrading one and update it them too - so basically we will double the size - this mean 1024
			// so firstly we call the function updateInMemoryOffsets
			inputStream.clear();
			inputStream.seekg(currentDirectoryOffset, ios::beg);
			Metadata* root = Metadata::deserialize(inputStream);
			size_t updated_size; 
			size_t old_size;
			if (vec1CurrSize > vec1AllocatedSize) {
				updated_size = vec1CurrSize * 2;
				old_size = vec1AllocatedSize;
				updateInMemoryOffsets(this, vec1AllocatedSize, *root, vec1CurrSize*2);
			}
			if (vec2CurrSize > vec2AllocatedSize) {
				updated_size = vec2CurrSize * 2;
				old_size = vec2AllocatedSize;

				updateInMemoryOffsets(this, vec2AllocatedSize, *root, vec2CurrSize * 2);
			}
			if (hashtable1 > hash1AllocatedSize) {
				updated_size = hashtable1 * 2;
				old_size = hash1AllocatedSize;

				updateInMemoryOffsets(this, hash1AllocatedSize, *root, hashtable1 * 2);
			}
			if (hashtable2 > hash2AllocatedSize) {
				updated_size = hashtable2 * 2;
				old_size = hash2AllocatedSize;

				updateInMemoryOffsets(this, hash2AllocatedSize, *root, hashtable2 * 2);
			}
			// offset now are update but the still is not updated, so ... 
			// we need to also update root as a parent offset of every file in the root dir

			size_t difference = updated_size - old_size;
			root->offset += difference;
			root_offset = root->offset;
			metadataHashtable.insert("/", root->offset);
			outputStream.clear();
			outputStream.seekp(root->offset, ios::beg);
			root->serialize(outputStream);
			updateFilesInRoot();

			// Now after we serialize the root we need to fix the vec and hashtableso
			// we need to get firstly the overlapping struct
			if (vec1CurrSize > vec1AllocatedSize) {
				hash2Offset += difference;
				outputStream.clear();
				outputStream.seekp(hash2Offset, ios::beg);
				metadataHashtable.serialize(outputStream);
				
				hash1Offset += difference;
				outputStream.clear();
				outputStream.seekp(hash1Offset, ios::beg);
				blockHashTable.serialize(outputStream);

				vec2Offset+= difference;
				outputStream.clear();
				outputStream.seekp(vec2Offset, ios::beg);
				serializeVector(freeMeta, outputStream);

				vec1AllocatedSize = updated_size;
			}

			if (vec2CurrSize > vec2AllocatedSize) {
				hash2Offset += difference;
				outputStream.clear();
				outputStream.seekp(hash2Offset, ios::beg);
				metadataHashtable.serialize(outputStream);
				
				hash1Offset += difference;
				outputStream.clear();
				outputStream.seekp(hash1Offset, ios::beg);
				blockHashTable.serialize(outputStream);

				vec2AllocatedSize = updated_size;
			}
			if (hashtable1 > hash1AllocatedSize) {
				hash2Offset += difference;
				outputStream.clear();
				outputStream.seekp(hash2Offset, ios::beg);
				metadataHashtable.serialize(outputStream);

				hash1AllocatedSize = updated_size;
			}
			if (hashtable2 > hash2AllocatedSize) {
				// update only the size of the hash2AllocatedSize
				hash2AllocatedSize += updated_size;
			}

		

		}

		outputStream.clear();
		outputStream.seekp(0, ios::beg);
		outputStream.write(reinterpret_cast<const char*>(&id), sizeof(id));
		outputStream.write(reinterpret_cast<const char*>(&vec1Offset), sizeof(vec1Offset));

		outputStream.write(reinterpret_cast<const char*>(&vec2Offset), sizeof(vec2Offset));

		outputStream.write(reinterpret_cast<const char*>(&hash1Offset), sizeof(hash1Offset));

		outputStream.write(reinterpret_cast<const char*>(&hash2Offset), sizeof(hash2Offset));

		outputStream.write(reinterpret_cast<const char*>(&vec1AllocatedSize), sizeof(vec1AllocatedSize));

		outputStream.write(reinterpret_cast<const char*>(&vec2AllocatedSize), sizeof(vec2AllocatedSize));
		outputStream.write(reinterpret_cast<const char*>(&hash1AllocatedSize), sizeof(hash1AllocatedSize));

		outputStream.write(reinterpret_cast<const char*>(&hash2AllocatedSize), sizeof(hash2AllocatedSize));
		// Move to the end of the file
		//Write vector 1
		outputStream.clear();

		outputStream.seekp(vec1Offset, ios::beg);
		serializeVector(freeBlocks, outputStream);

		// Write vector 2
		outputStream.clear();

		outputStream.seekp(vec2Offset, ios::beg);
		serializeVector(freeMeta, outputStream);

		// Write HaSh table 1
		outputStream.clear();

		outputStream.seekp(hash1Offset, ios::beg);
		blockHashTable.serialize(outputStream);

		// Write hash table 2
		outputStream.clear();

		outputStream.seekp(hash2Offset, ios::beg);
		metadataHashtable.serialize(outputStream);

		// Ensure the file streams are properly closed
		if (inputStream.is_open()) inputStream.close();
		if (outputStream.is_open()) outputStream.close();

		
	}

};
// this needs to be serialized and deserialzied
struct Block {

	static const uint32_t MAGIC_NUMBER = 0xDEADBEEF;
	uint64_t content_offset; // offset of the block content
	uint64_t block_offset; // offset of the block metadata
	long int numberOfFiles = 0;
	uint64_t size; // size of the block
	String hashBl;
	uint32_t checksum; // Resilliency checksum
	uint64_t serialized_size;

	void reset() {
		content_offset = 0;
		numberOfFiles = 0;
		size = 0;
		hashBl.clear();
		checksum = 0;
		serialized_size = 0;

	}
	void serialize(std::ofstream& out) const {

		out.clear();
		out.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
		out.write(reinterpret_cast<const char*>(&serialized_size), sizeof(serialized_size));

		out.write(reinterpret_cast<const char*>(&content_offset), sizeof(content_offset));
		out.write(reinterpret_cast<const char*>(&block_offset), sizeof(block_offset));
		out.write(reinterpret_cast<const char*>(&numberOfFiles), sizeof(numberOfFiles));

		out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));

		// serialize the hash string
		size_t hashLength = hashBl.getSize();
		out.write(reinterpret_cast<const char*>(&hashLength), sizeof(hashLength));
		//hashBl.data() gives a pointer to the raw character array inside the string.
		out.write(hashBl.getData(), hashLength);

		
	}
	static Block* deserialize(std::ifstream& in) {
		Block* block = new Block();
		in.clear();
		try {
			uint32_t magicNumber;
			in.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));
			if (in.gcount() < sizeof(magicNumber) || magicNumber != MAGIC_NUMBER) {
				throw runtime_error("magic number is not the same");
			}
			in.read(reinterpret_cast<char*>(&block->serialized_size), sizeof(block->serialized_size));


			in.read(reinterpret_cast<char*>(&block->content_offset), sizeof(block->content_offset));
			if (in.gcount() < sizeof(block->size)) {
				throw runtime_error("Failed to deserialize");
			}
			in.read(reinterpret_cast<char*>(&block->block_offset), sizeof(block->block_offset));
			if (in.gcount() < sizeof(block_offset)) {
				throw runtime_error("Failed to deserialize");
			}
			in.read(reinterpret_cast<char*>(&block->numberOfFiles), sizeof(block->numberOfFiles));
			if (in.gcount() < sizeof(numberOfFiles)) {
				throw runtime_error("Failed to deserialize");
			}
			in.read(reinterpret_cast<char*>(&block->size), sizeof(block->size));
			if (in.gcount() < sizeof(size)) {
				throw runtime_error("Failed to deserialize");
			}
			in.read(reinterpret_cast<char*>(&block->checksum), sizeof(block->checksum));
			if (in.gcount() < sizeof(checksum)) {
				throw runtime_error("Failed to deserialize");
			}
			size_t hashLength;
			in.read(reinterpret_cast<char*>(&hashLength), sizeof(hashLength));
			if (in.gcount() < sizeof(hashLength) || hashLength == 0) {
				throw runtime_error("Failed to deserialize");
			}
			block->hashBl.resize(hashLength);
			in.read(&block->hashBl[0], hashLength);
			if (in.gcount() < static_cast<streamsize>(hashLength)) {
				throw runtime_error("Failed to deserialize");
			}
			if (!validateBlock(in, *block)) {
				throw runtime_error("Failed to deserialize");
			}


			return block;

		}
		catch (const exception& e) {
			delete block;
			return nullptr;
		}

		// Deserialize the hash string

		
	}
	size_t getSerializedSize() const {
		size_t fiexedSize = sizeof(content_offset) + sizeof(block_offset) + sizeof(numberOfFiles)
			+ sizeof(size) + sizeof(checksum) + sizeof(serialized_size) + sizeof(MAGIC_NUMBER);

		size_t dynamicsize = sizeof(size_t) + hashBl.getSize();
		return fiexedSize + dynamicsize;
	}
	uint64_t writeToContainer(ResourceManager& re, const char* buffer, size_t buffersize, uint64_t offset, int nFile = 1,bool exist = true) {
		// firstly lets deserialize theo offset if exist
		serialized_size = getSerializedSize();
		block_offset = offset;
		ifstream& containerRead = re.getInputStream();
		ofstream& containerWrite = re.getOutputStream();

		if (!exist) {
			containerRead.clear();
			containerRead.seekg(offset, ios::beg);
			Block* currentBlock = Block::deserialize(containerRead);
			if (currentBlock->serialized_size > serialized_size) {
				updateInMemoryOffsetsfromBlock(&re,currentBlock->serialized_size,*this,serialized_size,true);
			}
			delete currentBlock;
			currentBlock = nullptr;

		}
		block_offset = offset;
		size = buffersize;
		numberOfFiles = nFile;
		size_t totalSize = std::max(buffersize, static_cast<size_t>(4096));
		char paddedBuffer[4096] = { 0 };

		memcpy(paddedBuffer, buffer, buffersize);


		// computer checksum (somple XOR checksum)
		CRC32 crc32;
		checksum = crc32.calculate(buffer,buffersize);

		containerWrite.clear();
		containerWrite.seekp(offset, ios::beg);


		// write the block's data to thec ontainer
		content_offset = offset +serialized_size;
		if (!containerWrite) {
			throw runtime_error("Error: Failed to seek to block offset");
		}
		serialize(containerWrite);
		// Write the block's data to the container
		containerWrite.clear();
		containerWrite.seekp(content_offset, ios::beg);
		if (!containerWrite) {
			throw runtime_error("Error: Failed to see to content offset");
		}
		containerWrite.write(paddedBuffer, totalSize);
		if (!containerWrite) {
			throw runtime_error("Error: Failed to write blocjk data to container");
		}
		containerWrite.flush();
		if (!containerWrite) {
			throw runtime_error("Error: Failed to flush the container stream");
		}
		containerWrite.clear();
		containerWrite.seekp(0, ios::end);
		return containerWrite.tellp();

	}
	String hashBlock(const char* block, size_t size) {
		String hash;
		for (size_t i = 0; i < size; ++i) {
			hash += String::to_string((block[i] + i) % 256);
		}
		return hash;
	}
	static bool validateBlock(std::ifstream& container, Block& block) {

		// Step 2: Read the block's content
		container.clear();
		container.seekg(block.content_offset, std::ios::beg);
		if (!container) {
			throw std::runtime_error("Error: Failed to seek to content offset");
		}

		char* buffer = new char[block.size];
		container.read(buffer, block.size);
		if (!container) {
			delete[] buffer;
			throw std::runtime_error("Error: Failed to read block content");
		}

		// Step 3: Recompute the checksum
		CRC32 crc32;
		uint32_t recomputedChecksum = crc32.calculate(buffer, block.size);

		// Step 4: Validate checksum
		bool isValid = (recomputedChecksum == block.checksum);

		// Clean up
		delete[] buffer;

		return isValid;
	}

	~Block(){}

};

void InsertionSort(DynamicArray<uint64_t>& array, bool descending = false);
DynamicArray<uint64_t> allocatedContiguosBlocks(ResourceManager& re, size_t requiredBlocks, size_t blockSize);
void rm(ResourceManager& re, String& fileName, uint64_t actualOffset = -1);
void InitializeContainer(ResourceManager& re, String& containerPath);
void ls(ResourceManager& re, String path = "");
void printBlockAndContent(ResourceManager& re, uint64_t offset);
void printMeta(ResourceManager& re, uint64_t offset);
void cpout(ResourceManager& re, String fileName, const String outputPath);
bool hasExtention(const String& fileName);
String& cpin(ResourceManager& re, String& sourceName, String& fileName, size_t blockSize);
String& md(ResourceManager& re, String& directoryName);
void cd(ResourceManager& re, String& directoryName);
void cdDots(ResourceManager& re);
void cdRoot(ResourceManager& re);
void DeleteDir(ResourceManager& re, ifstream& containerRead, ofstream& containerWrite, Metadata* currentMeta);
void rd(ResourceManager& re, String& dirName);
bool checkNames(ifstream& container, uint64_t parent_offset, String dirName);
bool getMetadataOff(ResourceManager& re, ofstream& contianerWrite, ifstream& containerRead, uint64_t& metaoff);

DynamicArray<String> split(const char* str, char delimeter) {
	DynamicArray<String> res;


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

void getBlockOffsets(ResourceManager& re,Metadata& childMeta, HashTable<uint64_t, StructType>& for_upgrading, uint64_t base_offset, HashTable<String, uint64_t>& dp) {
	for (auto& key : childMeta.getKyes()) {
		uint64_t off = re.getBlockHashTable().get(key);
		if (base_offset > off || dp.exists(key)) {
			continue;
		}
		dp.insert(key, off);
		for_upgrading.insert(off, BLOCK);

	}
}

void updateBlock(ResourceManager& re, uint64_t offset) {
	ifstream& containerRead = re.getInputStream();
	containerRead.clear();
	containerRead.seekg(offset, ios::beg);
	Block* block = Block::deserialize(containerRead);
	char buffer[4096];
	containerRead.clear();
	containerRead.seekg(block->content_offset, ios::beg);
	containerRead.read(buffer, block->size);
	block->writeToContainer(re, buffer, block->size, block->block_offset, block->numberOfFiles);
	delete block;
	block = nullptr;
}
// the plan is this - we go through every dir and ile and put them in a vector of offsets
void dfs(ResourceManager& re,ifstream& containerRead,Metadata& parent,HashTable<String, uint64_t>& dp, uint64_t base_offset, HashTable<uint64_t, StructType>& for_upgrading, int64_t sizeDifference, bool backwards) {

	String key = parent.makeKey();
	// check if the value for this parent is already computed 
	if (dp.exists(key)) {
		return;
	}
	dp.insert(key, parent.offset);
	if (!parent.getChildren().getSize()) {
		getBlockOffsets(re, parent, for_upgrading, base_offset, dp);

	}

	for (auto& child_offset: parent.getChildren()){
		// update the current offset of the child that we wanna deserialize
		if (base_offset > child_offset) {
			continue;
		}
		for_upgrading.insert(child_offset,METADATA);
		containerRead.clear();
		containerRead.seekg(child_offset, ios::beg);
		Metadata* childMeta = Metadata::deserialize(containerRead); // 4436
		// if this is through we have gone deep in the tree
		if (base_offset< parent.offset) {
			
			childMeta->parent += sizeDifference;
			re.getOutputStream().clear();
			re.getOutputStream().seekp(child_offset, ios::beg);
			childMeta->serialize(re.getOutputStream());
		}
		child_offset += sizeDifference;
		if (childMeta->isDirectory) {
			dfs(re,containerRead, *childMeta, dp, base_offset, for_upgrading,sizeDifference,backwards);
			re.getOutputStream().clear();
			re.getOutputStream().seekp(childMeta->offset, ios::beg);
			childMeta->serialize(re.getOutputStream());
		}
		else { // the file is not directory and we need to fix block offset and content offsets
			getBlockOffsets(re, *childMeta, for_upgrading, base_offset, dp);
		}
		delete childMeta;
	}
}
void updateInMemoryOffsets(ResourceManager* re, size_t& oldSize, Metadata& parent , uint64_t newSize,bool backwards) {

	// if the size has changed, update subsequent offsets in the hashtable
	if ((newSize > oldSize && !backwards) || (oldSize > newSize && backwards)) {
		int64_t sizeDifference = newSize - oldSize;

		ofstream& contasinerWrite = re->getOutputStream();
		ifstream& containerRead = re->getInputStream();

		// here we need to implement deph first seach
		HashTable<String, uint64_t> dp;
		// offsets that we need to loop deserialize change them and serialize again (first we need to sor them)
		HashTable<uint64_t, StructType> hash_offsets;


		auto& metaHaSh = re->getMetadataHashTable();
		auto& blockHash = re->getBlockHashTable();
		auto& freeMeta = re->getFreeMeta();
		auto& freeBlocks = re->getFreeblocks();
		DynamicArray<myPair<uint64_t, StructType>> freemetaandBlocks;
		// adding the delet meta
		for (auto& metas : freeMeta) {
			if (metas > parent.offset) {
				freemetaandBlocks.push_back(myPair<uint64_t, StructType>(metas, METADATA));

			}
		}
		// adding the deleted blcoks
		for (auto& blocks : freeBlocks) {
			if (blocks > parent.offset) {
				freemetaandBlocks.push_back(myPair<uint64_t, StructType>(blocks, BLOCK));

			}
		}

	/*	ifstream& containerRead1 = re.getInputStream();
		containerRead1.clear();
		containerRead1.seekg(970, ios::beg);
		Metadata* as = Metadata::deserialize(containerRead1);*/

		// Update offsets in the hashtable
		dfs(*re,re->getInputStream(),parent,dp,parent.offset, hash_offsets,sizeDifference,backwards);
		DynamicArray<myPair<uint64_t, StructType>> sorted_offsets = hash_offsets.getsortedByKey();
		sorted_offsets.concatenate(freemetaandBlocks);
		// sort in descending
		sorted_offsets.insertionSort([](const myPair<uint64_t, StructType>& a, const myPair<uint64_t, StructType>& b) {
			return a.first > b.first;
			},!backwards?true:false);


		for (int i = 0; i < sorted_offsets.getSize(); ++i) {
			uint64_t curr_offset = sorted_offsets[i].first;
			StructType curr_type = sorted_offsets[i].second;


			if (curr_type == METADATA) {
				containerRead.clear();
				containerRead.seekg(curr_offset, ios::beg);
				Metadata* meta = Metadata::deserialize(containerRead);
				meta->offset += sizeDifference;
				contasinerWrite.clear();
				contasinerWrite.seekp(meta->offset, ios::beg);
				meta->serialize(contasinerWrite);
				delete meta;
			}
			else if(curr_type == BLOCK) {
				containerRead.clear();
				containerRead.seekg(curr_offset, ios::beg);
				Block* block = Block::deserialize(containerRead);
				block->block_offset += sizeDifference;
				char buffer[4096];
				containerRead.clear();
				containerRead.seekg(block->content_offset, ios::beg);
				containerRead.read(buffer, block->size);
				block->writeToContainer(*re,buffer,block->size,block->block_offset, block->numberOfFiles);

				delete block;
				block = nullptr;
			}
		}

		metaHaSh.iterate([&](const String& key, uint64_t& of) {
			if (parent.offset < of) {
				of += sizeDifference; // Shift subsequent offsets
			}
		});

		auto& hashtable = re->getBlockHashTable();
		hashtable.iterate([&](const String& key, uint64_t& of) {
			if (parent.offset < of) {
				of += sizeDifference;// Shift subsequent offsets
			}
			});
		for (auto& off : re->getFreeblocks()) {
			if (off > parent.offset) {
				off += sizeDifference;
			}
		}
		for (auto& off : re->getFreeMeta()) {
			if (off > parent.offset) {
				off += sizeDifference;
			}
		}

	}

}

// every blcok has its own metadata and whenevrr we delete and accomodate new data there it get bigger or smaller so we need to updat  offsets

void updateInMemoryOffsetsfromBlock(ResourceManager* re, size_t& oldSize, Block& block, uint64_t newSize, bool backwards) {
	if ((newSize > oldSize && !backwards) || (oldSize > newSize && backwards)) {
		int64_t sizeDifference = newSize - oldSize;

		ofstream& contasinerWrite = re->getOutputStream();
		ifstream& containerRead = re->getInputStream();

		// here we need to implement deph first seach
		HashTable<String, uint64_t> dp;
		// offsets that we need to loop deserialize change them and serialize again (first we need to sor them)
		HashTable<uint64_t, StructType> hash_offsets;
		// ideally, we can first sort the metahashtable, get first filemeta that is bigger than the block offset and dfs from there , that would be the best scenario
		// for the freemeta and free blocks we only need to change their offsets, but they must be sorted
		// now we take the 
		auto& metaHaSh = re->getMetadataHashTable();
		auto& blockHash = re->getBlockHashTable();
		auto& freeMeta = re->getFreeMeta();
		auto& freeBlocks= re->getFreeblocks();
		DynamicArray<myPair<uint64_t, StructType>> freemetaandBlocks;
		// adding the delet meta
		for (auto& metas : freeMeta) {
			if (metas > block.block_offset) {
				freemetaandBlocks.push_back(myPair<uint64_t, StructType>(metas, METADATA));

			}
		}
		// adding the deleted blcoks
		for (auto& blocks : freeBlocks) {
			if (blocks > block.block_offset) {
				freemetaandBlocks.push_back(myPair<uint64_t, StructType>(blocks, BLOCK));

			}
		}


		uint64_t fileMeta = 0;
		uint64_t blockMeta = 0;
		Metadata* meta = nullptr;
		Block* currentBlock= nullptr;
		// filemeta, we need to append also the delete filemeta 
		// here we sort the hashtable and then add if there is any free meta into it
		DynamicArray<myPair<String,uint64_t>> sorted = metaHaSh.getsortedByKey(false);
		// we need to see if there are 
		for (const auto& filemeta : sorted) {
			if (filemeta.second > block.block_offset) {
				fileMeta = filemeta.second;
				break;
			}
		}
		if (fileMeta != 0) {
			containerRead.clear();
			containerRead.seekg(fileMeta, ios::beg);
			meta = Metadata::deserialize(containerRead);
			// now we need to get the parent of that
			containerRead.clear();
			containerRead.seekg(meta->parent, ios::beg);
			delete meta;
			meta = nullptr;
			// so now that is the parent
			meta = Metadata::deserialize(containerRead);
			if (!meta) {
				return;
			}
			updateBlock(*re,block.block_offset);
		}
		else {
			// here we understand that we dont have to to upgrade after the block so we need to update only the content offset
			updateBlock(*re, block.block_offset);
			return;

		}
		// now we need to be sure if there is blcoks between the filemeta and the current blcok  
		// so we need to sort the freeBlocks, take the first and comapre it with the 
		
		/*	ifstream& containerRead1 = re.getInputStream();
			containerRead1.clear();
			containerRead1.seekg(970, ios::beg);
			Metadata* as = Metadata::deserialize(containerRead1);*/

			// Update offsets in the hashtable
		dfs(*re, re->getInputStream(), *meta, dp, block.block_offset, hash_offsets, sizeDifference,backwards);
		DynamicArray<myPair<uint64_t, StructType>> sorted_offsets = hash_offsets.getsortedByKey();
		sorted_offsets.concatenate(freemetaandBlocks);
		// sort in descending
		sorted_offsets.insertionSort([](const myPair<uint64_t, StructType>& a, const myPair<uint64_t, StructType>& b) {
			return a.first > b.first;
			}, !backwards ? true : false);


		for (int i = 0; i < sorted_offsets.getSize(); ++i) {
			uint64_t curr_offset = sorted_offsets[i].first;
			StructType curr_type = sorted_offsets[i].second;

			if (curr_type == METADATA) {
				containerRead.clear();
				containerRead.seekg(curr_offset, ios::beg);
				Metadata* meta = Metadata::deserialize(containerRead);
				meta->offset += sizeDifference;
				contasinerWrite.clear();
				contasinerWrite.seekp(meta->offset, ios::beg);
				meta->serialize(contasinerWrite);
				delete meta;
			}
			else if (curr_type == BLOCK) {
				containerRead.clear();
				containerRead.seekg(curr_offset, ios::beg);
				Block* block = Block::deserialize(containerRead);
				block->block_offset += sizeDifference;
				char buffer[4096];
				containerRead.clear();
				containerRead.seekg(block->content_offset, ios::beg);
				containerRead.read(buffer, block->size);
				block->writeToContainer(*re, buffer, block->size, block->block_offset, block->numberOfFiles);
				delete block;
				block = nullptr;
			}
		}

		metaHaSh.iterate([&](const String& key, uint64_t& of) {
			if (block.block_offset< of) {
				of += sizeDifference; // Shift subsequent offsets
			}
			});

		auto& hashtable = re->getBlockHashTable();
		hashtable.iterate([&](const String& key, uint64_t& of) {
			if (block.block_offset< of) {
				of += sizeDifference; // Shift subsequent offsets
			}
			});
		for (auto& off : re->getFreeblocks()) {
			if (off > block.block_offset) {
				off += sizeDifference;
			}
		}
		for (auto& off : re->getFreeMeta()) {
			if (off > block.block_offset) {
				off += sizeDifference;
			}
		}

	}
}



// IMP: WHEN we create a file i need to update the size of every parent directory up to the root size
int main(int argc, char* argv[]) {
	/*if (argc < 2) {
		std::cerr << "Ussage: <command> [arguments]\n";
		return 1;
	}*/
	String fileSystem = "container.bin";


	String newDir = "bal.txt";
	String outfile = "C:\\Users\\vboxuser\\Desktop\\muha.txt";
	String fileName = "bal.txt";
	String currentState = "root\\";
	while (true) {
		String command = "rm";
		cout << currentState << endl;
		if (command == "md") {
			ResourceManager resource(fileSystem, command);

			cout <<md(resource, newDir);
			break;
		}
		else if (command == "cpin") {
			ResourceManager resource(fileSystem, command);
			cpin(resource, outfile, fileName, 4096);
			break;
		}
		else if (command == "ls") {
			ResourceManager resource(fileSystem, command);

			ls(resource, newDir);
			break;
		}
		else if (command == "cpout") {
			ResourceManager resource(fileSystem, command);

			cpout(resource, newDir, "C:\\Users\\vboxuser\\Desktop\\eheeee.txt");
		}
		else if (command == "rm") {
			ResourceManager resource(fileSystem, command);

			rm(resource, newDir);
			break;
		}
		else if (command == "cd") {
			ResourceManager resource(fileSystem, command);

			cd(resource, newDir);
		}
		else if (command == "rd") {
			ResourceManager resource(fileSystem, command);

			rd(resource, newDir);
			break;
		}
		else {
			std::cerr << "Error: Unknown command '" << command << "'\n";
		}
	}

	

	return 0;
}

void InsertionSort(DynamicArray<uint64_t>& array, bool descending) {
	size_t len = array.getSize();
	for (int i = 1; i < len; ++i) {
		int key = array[i];
		int j = i - 1;
		if (!descending) {
			while (j >= 0 && array[j] > key) {
				array[j + 1] = array[j];
				--j;
			}
		}
		else {
			while (j >= 0 && array[j] < key) {
				array[j + 1] = array[j];
				--j;
			}
		}
		
		array[j + 1] = key;
	}
}

DynamicArray<uint64_t> allocatedContiguosBlocks(ResourceManager& re, size_t requiredBlocks, size_t blockSize) {
	DynamicArray<uint64_t> allocatedBlocks;
	InsertionSort(re.getFreeblocks());

	// case if only one block is required
	if (requiredBlocks == 1) {
		for (size_t i = 0; i < re.getFreeblocks().getSize(); ++i) {
			allocatedBlocks.push_back(re.getFreeblocks()[i]);
			re.getFreeblocks().pop_back();
			return allocatedBlocks;
		}
	}


	size_t contiguouscount = 1;

	for (size_t i = 1; i < re.getFreeblocks().getSize(); ++i) {
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

uint64_t getOffsetByParent(ifstream& container,uint64_t parent_offset, String fileName) {
	container.clear();
	container.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(container);
	for (const auto& child_off : parent->getChildren()) {
		container.clear();
		container.seekg(child_off, ios::beg);
		Metadata* child = Metadata::deserialize(container);
		if (String(child->name) == fileName) {
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


void rm(ResourceManager& re , String& fileName, uint64_t actualOffset) {
	ifstream& containerRead = re.getInputStream();
	ofstream& containerWrite = re.getOutputStream();

	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	DynamicArray<String> directories = move(split(fileName.getData(), '\\'));
	DynamicArray<uint64_t> offsets;
	uint64_t child_offset = re.getCurrentDirOffset();
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
			else if (directories.getSize() == 1) {
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
				for (const auto& off : parentMeta->getChildren()) {
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
	// first we need to bring back the parent child back, then resize the filermeta back to 170,bring back the block to original size,
	// get the child first 
	containerRead.clear();
	containerRead.seekg(child_offset, ios::beg);
	Metadata* child = Metadata::deserialize(containerRead);
	// get the offset of the parent
	parent_offset = child->parent;

	// remove the meta from parent children
	containerRead.clear();
	containerRead.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(containerRead);
	size_t oldSize = parent->getSerializedSize();
	parent->getChildren().remove(parent->getChildren().indexOf(child_offset));
	// here we need to resize the meta instead of doing it when we create the file so 
	re.getFreeMeta().push_back(child_offset);


	// children 0 , capacity 1
	// but what if the actual block is being used by others, we wont delete it and we wont add it to the freeblocks 
	// to check if other files are using the block we need to have a variable for each block and the number of files that are using that block
	// delete all blocks assosiated with the meta file 
	for (String key: child->getKyes()) {
		// firstly add all the blocks to a list of reusable blocks
		uint64_t off = re.getBlockHashTable().get(key);
		containerRead.clear();
		containerRead.seekg(off, ios::beg);
		Block* block = Block::deserialize(containerRead);
		if (block->numberOfFiles < 2) {
			size_t oldSize = block->getSerializedSize();
			re.getFreeblocks().push_back(off);
			block->reset();
			containerWrite.clear();
			containerWrite.seekp(block->block_offset, ios::beg);
			block->serialize(containerWrite);
			updateInMemoryOffsetsfromBlock(&re, oldSize, *block, block->getSerializedSize(), true);
			containerWrite.clear();
			containerWrite.seekp(off, ios::beg);
			block->serialize(containerWrite);// getting the offset of the actual block, that will lead to the content of the block
			// remove the block from hashtable
			re.getBlockHashTable().remove(key); // remove the key from the block hash table 
		}
		else {
			block->numberOfFiles--;
			containerWrite.clear();
			containerWrite.seekp(off, ios::beg);
			block->serialize(containerWrite);
		}
		delete block;
		block = nullptr;
	}
	re.getMetadataHashTable().remove(child->makeKey());
	child->reset();
	containerWrite.clear();
	containerWrite.seekp(child_offset, ios::beg);
	child->serialize(containerWrite);
	// here we first update evertyhing from the parent because we removed 8 byte from his children array - 8 byte because of parent
	updateInMemoryOffsets(&re, oldSize, *parent, parent->getSerializedSize(), true);
	updateInMemoryOffsets(&re, child->serialized_size, *child,child->getSerializedSize(), true);



	parent->serialized_size = parent->getSerializedSize();
	containerWrite.clear();
	containerWrite.seekp(parent_offset, ios::beg);
	parent->serialize(containerWrite);

	//// here we need to resize again, because we need the defauklt size of the filemeta
	// clear the list
	// Update the metadata in the container to rteflect the deletion
	// go to the location of the fileMeta so we can overwrite it as deleted
	

	// add it to the free meta file

	uint64_t offset = parent_offset;
	while (true) { // -1 because the parent of the root is -1 by default
		// update the size of the current meta
		parent->size -= child->size;
		containerWrite.clear();
		containerWrite.seekp(offset, ios::beg);
		parent->serialize(containerWrite);
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
	delete parent;
	parent = nullptr;
	delete child;
	child = nullptr;



}

bool getMetadataOff(ResourceManager& re, ofstream& containerWrite,ifstream& containerRead,uint64_t& metaoff) {
	if (!re.getFreeMeta().empty()) {
		metaoff = re.getFreeMeta().back();
		re.getFreeMeta().pop_back();
		return true;

	}
	else {

		containerWrite.clear();
		containerWrite.seekp(0, ios::end);
		metaoff = containerWrite.tellp();
		// Increment id for the next meta because we wont be using the deleted
		return false;
	}
}

void ls(ResourceManager& re, String path) {
	ifstream& containerRead = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	DynamicArray<String> directories = split(path.getData(), '\\');
	DynamicArray<uint64_t> offsets;
	uint64_t parent_offset = re.getCurrentDirOffset();
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
			if (directories.getSize() == 1) {
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
				for (const auto& off : parentMeta->getChildren()) {
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
	for (const auto& off : parent->getChildren()) {
		containerRead.seekg(off, ios::beg);
		Metadata* currentChild = Metadata::deserialize(containerRead);
		if (!currentChild->isDirectory) {
			for (auto& key : currentChild->getKyes()) {
				printBlockAndContent(re, re.getBlockHashTable().get(key));
			}

		}
		// check the parent
		containerRead.clear();
		containerRead.seekg(currentChild->parent);
		Metadata* parent = Metadata::deserialize(containerRead);
		cout << currentChild->name << " " << currentChild->size << ": parent is " << parent->name << endl;
		delete currentChild;
	}
	delete parent;
	
}

void printBlockAndContent(ResourceManager & re, uint64_t offset) {
	ifstream& containerRead = re.getInputStream();

	// seek the offset
	containerRead.clear();
	containerRead.seekg(offset, ios::beg);
	if (!containerRead) {
		std::cerr << "Error: Seek failed. Offset may be invalid.\n";
		return;
	}
	Block* block = Block::deserialize(containerRead);
	containerRead.clear();
	containerRead.seekg(block->content_offset, ios::beg);
	DynamicArray<char> buffer(block->size,0);

	containerRead.read(buffer.get_data(), block->size);
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
		for (const auto& child_offset : meta->getChildren()) {
			file.seekg(child_offset, ios::beg);
			Metadata* child_meta = Metadata::deserialize(file);
			cout << child_meta->name << " " << child_meta->size;
			delete child_meta;
		}
	}
	else {
		for (const auto& child_block_key : meta->getKyes()) {
			uint64_t of = re.getBlockHashTable().get(child_block_key);
			printBlockAndContent(re,of);

		}
	}
	delete meta;
}

// TASK: WHEN A  FILE GET DELETED EVERY BLOCK HAS ITS OWN BLOCK META, SO I NEED TO CONSIDER THIS 
void cpout(ResourceManager& re, String fileName, const String outputPath) {
	ifstream& containerRead = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	DynamicArray<String> directories = split(fileName.getData(), '\\');
	DynamicArray<uint64_t> offsets;
	uint64_t parent_offset = re.getCurrentDirOffset();
	if (directories.getSize() > 1 && hasExtention(directories.back())) {
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
				for (const auto& off : parentMeta->getChildren()) {
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
	else if (directories.getSize() == 1 && hasExtention(directories.back())) {
		parent_offset = getOffsetByParent(containerRead, re.getCurrentDirOffset(), fileName);
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
	for (const auto& blockKey : fileMeta->getKyes()) {
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

bool hasExtention(const String& fileName) {
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

String& cpin(ResourceManager& re , String& sourceName, String& fileName, size_t blockSize) {
	std::ifstream src(sourceName, std::ios::binary); // opens the file in binary mode , file is read byte by byte
	String errorMessage("");
	if (!src) {
		
		return errorMessage += "Error: Cannot open file contianer!";
	}
	ifstream& containerRead = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	DynamicArray<String> directories = split(fileName.getData(), '\\');
	DynamicArray<uint64_t> offsets;
	uint64_t parent_offset = re.getCurrentDirOffset();
	if (directories.getSize() > 1 && hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			uint64_t offset;
			// we need to check also if the dir is some of the first because the last doesnt exist;
			// check if current folder doesnt exist in the meta table
			 if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {
				cerr << "Error: path doesnt exits\n";
				delete parentMeta;
				delete childMeta;
				return errorMessage;
			}
			 
			if (directories.back() == dir) {
				// check if there is a file with the same name in the last dir
				if (!checkNames(containerRead, parent_offset, dir)) {
					return errorMessage;
				}
				break;
			}
			// always wherever i am we start from the currentoffset as parent offset
			offset = getOffsetByParent(containerRead, parent_offset, dir);
			if (offset == -1) {
				cerr << "The file doesnt exist in that folder!\n";
				return errorMessage;
			}
			containerRead.clear();
			containerRead.seekg(offset, ios::beg);
			childMeta = Metadata::deserialize(containerRead);
			if (parentMeta != nullptr) {
				bool isPart = false;
				for (const auto& off : parentMeta->getChildren()) {
					if (off == childMeta->offset) {
						isPart = true;
						break;
					}
				}
				if (!isPart) {
					cerr << "Error: " << dir << " is not a valid child of its parent\n";
					delete parentMeta;
					delete childMeta;
					return errorMessage;

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
	else if(directories.getSize() == 1 && hasExtention(directories.back()) && checkNames(containerRead,re.getCurrentDirOffset(), fileName)) {
		parent_offset = re.getCurrentDirOffset(); 
	}
	else {
		return errorMessage;
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
	// first file metaoffset is 190 
	Metadata* inUse = nullptr;
	uint64_t metaOff;
	if (getMetadataOff(re, containerWrite, containerRead, metaOff)) {
		containerRead.clear();
		containerRead.seekg(metaOff, ios::beg);
		inUse = Metadata::deserialize(containerRead);
		//if (inUse->serialized_size > inUse->getSerializedSize()) {
		//	// this resizes the filemeta, pussing bacjkt he block and conentent
		//	updateInMemoryOffsets(&re,inUse->serialized_size,*inUse,inUse->getSerializedSize(),true);
		//}
		inUse->parent = parent_offset;
		String::custom_strncpy(inUse->name, fileName.getData(), sizeof(inUse->name));
		inUse->isDirectory = false;
		inUse->isDeleted = false;
		inUse->size = 0;

	}
	else {
		inUse = new Metadata();
		String::custom_strncpy(inUse->name, fileName.getData(), sizeof(inUse->name));
		inUse->isDirectory = false;
		inUse->isDeleted = false;
		inUse->size = 0;
		inUse->parent = parent_offset;
		inUse->id = re.getNextId();
		re.incrementId();


	}
	inUse->offset = metaOff;
	String key1 = inUse->makeKey();
	re.getMetadataHashTable().insert(key1, inUse->offset);
	containerWrite.clear();
	containerWrite.seekp(inUse->offset, ios::beg);
	inUse->serialize(containerWrite);

	// SERIALIZE THE FILEMETA
	// ADD THE FILEMETA OFFSET TO THE PARENT and serialize - here we need to update susbequent

	containerRead.clear();
	containerRead.seekg(inUse->offset, ios::beg);
	Metadata* as = Metadata::deserialize(containerRead);
	cout << "filemeta name is " << as->name << "id is " << as->id << endl;

	uint64_t oldsize = parent->getSerializedSize();
	// capacity 1 - children - 1
	parent->getChildren().push_back(inUse->offset);
	parent->serialized_size = parent->getSerializedSize();
	updateInMemoryOffsets(&re, oldsize, *parent, parent->getSerializedSize());
	inUse->offset = re.getMetadataHashTable().get(key1);
	containerWrite.clear();
	containerWrite.seekp(parent_offset, ios::beg);
	parent->serialize(containerWrite);

	//containerRead.clear();
	//containerRead.seekg(re.getFreeblocks()[0], ios::beg);
	//Block* blo1 = Block::deserialize(containerRead);
	

	// capacity- 2 - children 1


	//containerRead.clear();
	//containerRead.seekg(re.getFreeblocks()[0],ios::beg);
	//Block* blo = Block::deserialize(containerRead);



	// Get the file size 
	src.seekg(0, ios::end);
	size_t filesize = src.tellg();
	src.seekg(0, ios::beg);
	
	// --------------------------- CALCULATION OF THE REQUIRED BLOCKS
	// Calculate the required number of blocks
	size_t requiredBlocks = (filesize + blockSize - 1) / blockSize; // ceiling divisiopn

	// Step 1: Try to allocated from free blocks
	DynamicArray<uint64_t> allocatedBlocks = allocatedContiguosBlocks(re,requiredBlocks, blockSize);
	bool notEmpty= false;
	// Step 2 : If no free blocks are available, append to the end of the file
	if (allocatedBlocks.empty()) {
		// Get the current end of the container
		containerRead.clear();
		containerRead.seekg(0, ios::end);
		uint64_t currentOffset = containerRead.tellg();
		notEmpty = true;
		allocatedBlocks.push_back(currentOffset);


	}
	// copy file content in chunks
	char buffer[4096];
	size_t remainingSize = filesize;
	uint64_t oldSize = inUse->getSerializedSize();
	for(size_t i =0;i< requiredBlocks;++i){

		size_t chunk_size = std::min(blockSize, remainingSize);
		src.read(buffer, chunk_size);
		size_t bytesRead = src.gcount();

		Block* block = new Block();
		block->hashBl = block->hashBlock(buffer, bytesRead);
		block->size = bytesRead;
		// block doesnt exitsts
		if (!re.getBlockHashTable().exists(block->hashBl)) {
			allocatedBlocks.push_back(block->writeToContainer(re, buffer, bytesRead, allocatedBlocks[i], 1,notEmpty));
			// add the block hjash to tghe hashtable
			re.getBlockHashTable().insert(block->hashBl, block->block_offset);
		}
		else { // this here means that the block exist and i need to get it and add 1 to numberOfFiles
			uint64_t off = re.getBlockHashTable().get(block->hashBl);
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
			bl = nullptr;
			
		}
		inUse->size += bytesRead;
		inUse->getKyes().push_back(block->hashBl);
		remainingSize -= bytesRead;
		cout << "Before:" << endl;
		delete block;
		block = nullptr;
	}
	// this only is true if we've got a delete file that once had a size
	// Here we need to update all susbequent offsets, if the old size if 
	// this pushes the block and content away


	updateInMemoryOffsets(&re, oldSize, *inUse, inUse->getSerializedSize());
	inUse->serialized_size = inUse->getSerializedSize();	
	containerWrite.clear();
	containerWrite.seekp(inUse->offset, ios::beg);
	inUse->serialize(containerWrite);
	// Right here we need to loop over every parent till we reach the root and update their sizes 
	uint64_t offset = parent_offset;
	while (true) { // -1 because the parent of the root is -1 by default
		// update the size of the current meta
		parent->size += inUse->size;
		containerWrite.clear();
		containerWrite.seekp(offset, ios::beg);
		parent->serialize(containerWrite);
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
	// Write metadata to the container 
	return errorMessage;
}

// this function checks in the parent folder if there are other files or directories with the same name as the one that we want to create 
bool checkNames(ifstream& container,uint64_t parent_offset, String dirName) {
	// First deserialize the parent
	container.clear();
	container.seekg(parent_offset, ios::beg);
	Metadata* parent = Metadata::deserialize(container); 

	for (auto& child_offset : parent->getChildren()) {
		container.clear();
		container.seekg(child_offset);
		Metadata* child = Metadata::deserialize(container);// 4690
		if (String(child->name) == dirName) {
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
String& md(ResourceManager& re, String& directoryName) {
	String errorMessage("");
	ifstream& container = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	DynamicArray<String> directories = split(directoryName.getData(), '\\');
	DynamicArray<uint64_t> offsets;
	uint64_t parent_offset = re.getCurrentDirOffset();
	container.clear();
	if (directories.getSize() > 1 && !hasExtention(directories.back())) {
		for (const auto& dir : directories) {
			// we need to check also if the dir is some of the first because the last doesnt exist;
			if (!re.getMetadataHashTable().findByBaseName(dir) && directories.back() != dir) {				
				delete parentMeta;
				delete childMeta;
				return errorMessage += "Path doesn't exist!";
			}
			if (directories.back() == dir) {
				// check if there is a file with the same name in the last dir
				if (!checkNames(container, parent_offset, dir)) {
					return errorMessage += "There is a file or directory with the same name!";
				}
				break;
			}
			uint64_t offset = getOffsetByParent(container,parent_offset, dir);

			container.seekg(offset, ios::beg);
			childMeta = Metadata::deserialize(container);
			if (parentMeta != nullptr) {
				bool isPart = false;
				for (const auto& off : parentMeta->getChildren()) {
					if (off == childMeta->offset) {
						isPart = true;
						break;
					}
				}
				if (!isPart) {
					delete parentMeta;
					delete childMeta;
					return errorMessage += "Path is invalid!";

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
	else if (directories.getSize() == 1 && !hasExtention(directories.back()) && checkNames(container,re.getCurrentDirOffset(), directoryName)) {
		parent_offset = re.getCurrentDirOffset();
	}
	else {
		return errorMessage;
	}
	ofstream& containerWrite = re.getOutputStream();
	containerWrite.clear();
	containerWrite.flush();

	container.seekg(parent_offset, ios::beg);
	parentMeta = Metadata::deserialize(container);

	// Create new directory metadata
	Metadata* newDirMeta = nullptr;
	// Determine the offset for the new directory metadata
	uint64_t metaOff;
	if (getMetadataOff(re, containerWrite, container, metaOff)) {
		container.clear();
		container.seekg(metaOff, ios::beg);
		newDirMeta = Metadata::deserialize(container);
		String::custom_strncpy(newDirMeta->name, directoryName.getData(), sizeof(newDirMeta->name));
		newDirMeta->isDirectory = true;
		newDirMeta->size = 0;
		newDirMeta->parent = parent_offset;
		newDirMeta->isDeleted = false;

	}
	else {
		newDirMeta = new Metadata();
		String::custom_strncpy(newDirMeta->name, directoryName.getData(), sizeof(newDirMeta->name));
		newDirMeta->isDirectory = true;
		newDirMeta->size = 0;
		newDirMeta->parent = parent_offset;
		newDirMeta->isDeleted = false;
		newDirMeta->id = re.getNextId();
		re.incrementId();
	}

	// SERIALIZE THE FILEMETA
	newDirMeta->offset = metaOff;
	String key1 = newDirMeta->makeKey();
	re.getMetadataHashTable().insert(key1, newDirMeta->offset);
	containerWrite.clear();
	containerWrite.seekp(metaOff, ios::beg);
	newDirMeta->serialize(containerWrite);
	// ADD THE FILEMETA OFFSET TO THE PARENT and serialize - here we need to update susbequent

	uint64_t oldsize = parentMeta->getSerializedSize();
	parentMeta->getChildren().push_back(newDirMeta->offset);
	parentMeta->serialized_size = parentMeta->getSerializedSize();


	updateInMemoryOffsets(&re, oldsize, *parentMeta, parentMeta->getSerializedSize());
	newDirMeta->offset = re.getMetadataHashTable().get(key1);
	


	containerWrite.clear();
	containerWrite.seekp(parent_offset, ios::beg);
	parentMeta->serialize(containerWrite);
	delete parentMeta;
	parentMeta = nullptr;
	delete newDirMeta;
	newDirMeta = nullptr;
	return directoryName;
}

// 2 options - cd Home\user\soemwhere or cd Home
void cd(ResourceManager& re, String& directoryName) {
	ifstream& container = re.getInputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	DynamicArray<String> directories = split(directoryName.getData(), '\\');
	DynamicArray<uint64_t> offsets;
	uint64_t parent_offset = re.getCurrentDirOffset();
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
			if (directories.getSize() == 1) {
				offset = getOffsetByParent(container, parent_offset, dir);
			}
			else {
				offset = getOffsetByParent(container, parent_offset, dir);
			}
			if (directories.back() == dir) {
				delete parentMeta;
				delete childMeta;
				offsets.push_back(offset);
				break;
			}			
			container.seekg(offset, ios::beg);
			childMeta = Metadata::deserialize(container);
			if (parentMeta != nullptr) {
				bool isPart = false;
				for (const auto& off : parentMeta->getChildren()) {
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
	else if (directories.empty()) {
		parent_offset = re.getCurrentDirOffset();
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
	re.getCurrentDirOffset() = parent_offset;
	
	delete currentDirMeta;
}

void cdDots(ResourceManager& re) {
	ifstream& container = re.getInputStream();
	container.clear();
	container.seekg(re.getCurrentDirOffset(), ios::beg);
	Metadata* currentMeta = Metadata::deserialize(container);
	if (!currentMeta->isDirectory || currentMeta->isDeleted) {
		cerr << "Error: There is a corruptuion with the direcotries.\n";
		return;
	}
	if (currentMeta->parent == 0) {
		cout << "Already in home.\n";
		return;
	}
	re.getCurrentDirOffset() = currentMeta->parent;

}

// Imp - we need build a function where i can build up to the root to update size of the nodes 
void DeleteDir(ResourceManager& re, ifstream& containerRead, ofstream& containerWrite,Metadata* currentMeta) {
	for (const auto& off : currentMeta->getChildren()) {
		containerRead.clear();
		containerRead.seekg(off, ios::beg);
		Metadata* currentFile = Metadata::deserialize(containerRead);
		if (currentFile->isDirectory && !currentFile->isDeleted) {
			DeleteDir(re,containerRead, containerWrite, currentFile);
			currentFile->reset();
			// here we need to delete the dir from the parent and resize the container
			size_t oldsize = currentMeta->getSerializedSize();
			currentMeta->getChildren().remove(currentMeta->getChildren().indexOf(currentFile->offset));
			re.getFreeMeta().push_back(currentFile->offset);
			//updateInMemoryOffsets(&re,oldsize,*currentMeta,currentMeta->getSerializedSize(),true);
			containerWrite.clear();
			containerWrite.seekp(currentMeta->offset, ios::beg);
			currentMeta->serialize(containerWrite);
			containerWrite.clear();
			containerWrite.seekp(currentFile->offset, ios::beg);
			currentFile->serialize(containerWrite);
		}
		else {
			String name(currentFile->name);
			rm(re, name, currentFile->offset);
		}
		delete currentFile;

	}
}

// 2 options rd Folderaa or rd Home\Folderaaa
void rd(ResourceManager& re, String& dirName) {
	ifstream& containerRead = re.getInputStream();
	ofstream& containerWrite = re.getOutputStream();
	Metadata* parentMeta = nullptr;
	Metadata* childMeta = nullptr;
	// Here basically we are checking if the path specified exitsts 
	DynamicArray<String> directories = move(split(dirName.getData(), '\\'));
	DynamicArray<uint64_t> offsets;
	uint64_t parent_offset = re.getCurrentDirOffset();
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
			if (directories.getSize() == 1) {
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
				for (const auto& off : parentMeta->getChildren()) {
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
	currentMeta->reset();
	re.getFreeMeta().push_back(currentMeta->offset);
	containerWrite.clear();
	containerWrite.seekp(currentMeta->offset, ios::beg);
	currentMeta->serialize(containerWrite);

	// Removing the offset from children in parent and decreasing the size also
	size_t oldSize = parentMeta->getSerializedSize();
	parentMeta->getChildren().remove(parentMeta->getChildren().indexOf(currentMeta->offset));
	/*updateInMemoryOffsets(&re,oldSize,*parentMeta,parentMeta->getSerializedSize(),true);*/
	// serializing the child
	containerWrite.clear();
	containerWrite.seekp(parentMeta->offset, ios::beg);
	parentMeta->serialize(containerWrite);
	delete currentMeta;
	delete parentMeta;
	parentMeta = nullptr;
	currentMeta = nullptr;
}