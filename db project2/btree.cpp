#pragma warning(disable: 4996)

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <sstream>

#define emptyElem std::make_pair(0,0)
#define __key first
#define __val second

#define __ptr second

using namespace std;

using Entry=pair<int, int>;

class BPTree {
private:
	int rootBid;
	int blockSize;
	char* fileName;
	int depth;
	fstream fs;
	int headerSize;
	int entrySize; // size of entry array
	int availBid; // to make next block


	class Node { // base Node
	private:
		Entry* entry;
		int sz;
		int bid;
	public:
		Node(int _sz, int _bid) :bid(_bid) { sz = _sz; entry = new Entry[_sz + 1]; memset(entry, 0, (_sz + 1) * sizeof(int)); }
		~Node() { delete[] entry; }
		friend class BPTree;

	};
	class leafNode : public Node { // leaf Node
	private:										// first <- key, second <- value
		int next_bid;
	public:
		leafNode(int _sz, int _bid) :Node(_sz, _bid) { next_bid = 0; }
		// read node from file
		leafNode(fstream& is, int _sz, int _bid, int headerSize, int blockSize) : Node(_sz, _bid) {
			is.seekg(headerSize + (_bid - 1) * blockSize);
			for (int i = 0; i < sz; i++) {
				is.read((char*)& entry[i].__key, sizeof(int));
				is.read((char*)& entry[i].__val, sizeof(int));
			}
			is.read((char*)& next_bid, sizeof(int));
		}
		// write node to file
		void writeNode(fstream& os, int headerSize, int block_size) {
			os.seekp(headerSize + (bid - 1) * block_size);
			for (int i = 0; i < sz; i++) {
				os.write((const char*)& entry[i].__key, sizeof(int));
				os.write((const char*)& entry[i].__val, sizeof(int));
			}
			os.write((const char*)& next_bid, sizeof(int));
		}
		void setNextBid(int _b) {
			next_bid = _b;
		}
		friend class BPTree;
	};
	class nonLeafNode : public Node {
	private:
		int least_bid;
	public:
		nonLeafNode(int _sz, int _bid) :Node(_sz, _bid) { least_bid = 0; }
		//read leafNode from file
		nonLeafNode(fstream& is, int _sz, int _bid, int headerSize, int blockSize) : Node(_sz, _bid) {
			is.seekg(headerSize + (_bid - 1) * blockSize);
			is.read((char*)& least_bid, sizeof(int));
			for (int i = 0; i < _sz; i++) {
				is.read((char*)& entry[i].__key, sizeof(int));
				is.read((char*)& entry[i].__ptr, sizeof(int));
			}
		}
		void setleastBid(int b) {
			least_bid = b;
		}
		//Write node to file
		void writeNode(fstream& os, int headerSize, int blockSize) {
			os.seekp(headerSize + (bid - 1) * blockSize);
			os.write((const char*)& least_bid, sizeof(int));
			for (int i = 0; i < sz; i++) {
				os.write((const char*)& entry[i].__key, sizeof(int));
				os.write((const char*)& entry[i].__ptr, sizeof(int));
			}
		}
		friend class BPTree;

	};



public:
	using NODE_PATH=vector<nonLeafNode*>;
	using Entry_list=vector<Entry>;

	//create file, if file already open clear that file
	BPTree(const char* fileName, int blockSize) : fs(fileName, ios::binary | ios::out | ios::in | ios::trunc) {
		rootBid = 0;

		this->blockSize = blockSize;
		this->fileName = strdup(fileName);
		entrySize = (blockSize - 4) / 8;
		availBid = 1;
		depth = 0;
		this->updateHeader();
	}

	//open file and calculate nextbid
	BPTree(const char* fileName) : fs(fileName, ios::binary | ios::out | ios::in) {
		this->fileName = strdup(fileName);
		readHeader();
		entrySize = (blockSize - 4) / 8;
		fs.seekg(0, ios::end);
		availBid = ((int)fs.tellg() - headerSize) / blockSize + 1;
	}

	~BPTree() {
		fs.close();
		delete[] fileName;
	}

	//update header in file
	void updateHeader() {
		fs.seekp(0);
		// write block size
		fs.write((const char*)& blockSize, sizeof(int));
		// write block id of root
		fs.write((const char*)& rootBid, sizeof(int));
		// write depth of tree, initally 0
		fs.write((const char*)& depth, sizeof(int));
		headerSize = 3 * sizeof(int);
	}

	//read header from file
	void readHeader() {
		fs.seekg(0);
		fs.read((char*)& blockSize, sizeof(int));
		fs.read((char*)& rootBid, sizeof(int));
		fs.read((char*)& depth, sizeof(int));
		headerSize = 3 * sizeof(int);
	}

	// find entry and return if key does not exist, return empty entry
	Entry find(int key) {
		int cnt = depth;
		int ptr = rootBid;
		nonLeafNode* nf = NULL;
		leafNode* lf = NULL;
		Entry ret = emptyElem;
		while (cnt > 1) { // searching in leafnode
			nf = new nonLeafNode(fs, entrySize, ptr, headerSize, blockSize);
			for (int i = 0; i <= entrySize; i++) {
				if (nf->entry[i].__key > key) {
					if (i == 0) {
						ptr = nf->least_bid;
					}
					else
						ptr = nf->entry[i - 1].__ptr;
					break;
				}
				else if (nf->entry[i].__key == NULL) {
					ptr = nf->entry[i - 1].__ptr;
					break;
				}
			}
			cnt--;
			delete nf;
		}

		if (ptr >= 0) {
			lf = new leafNode(fs, entrySize, ptr, headerSize, blockSize);
			int i;
			for (i = 0; i <= entrySize; i++) {
				if (lf->entry[i].__key == key) {
					ret = lf->entry[i];
					break;
				}
			}
			delete lf;
		}
		return ret;
	}

	int insert(int key, int val) {
		leafNode* lf = NULL;
		nonLeafNode* nf = NULL;
		NODE_PATH path;
		leafNode* newleaf = NULL;
		//make first leaf
		//and set that leaf as root
		if (depth == 0) {
			lf = new leafNode(entrySize, availBid++);
			lf->entry[0].__key = key;
			lf->entry[0].__val = val;
			rootBid = lf->bid;
			depth += 1;
			updateHeader();
			lf->writeNode(fs, headerSize, blockSize);
			delete lf;
			return 1;
		}
		int ptr = rootBid;
		//find position to insert node
		if (depth != 1) {
			int cnt = depth - 1;
			while (cnt--) {
				nf = new nonLeafNode(fs, entrySize, ptr, headerSize, blockSize);
				path.push_back(nf);
				for (int i = 0; i <= entrySize; i++) {
					if (nf->entry[i].__key > key) {
						if (i == 0)
							ptr = nf->least_bid;
						else
							ptr = nf->entry[i - 1].__ptr;
						break;
					}
					else if (nf->entry[i].__key == NULL) {
						ptr = nf->entry[i - 1].__ptr;
						break;
					}
				}
			}
		}
		int pos = ptr;
		//insert to leafnode
		if (pos != -1) {
			//chek if key is already exist
			lf = new leafNode(fs, entrySize, pos, headerSize, blockSize);
			Entry newEntry;
			newEntry.__key = key;
			newEntry.__val = val;

			for (int i = 0; i <= entrySize; i++) {
				//insert key is already exist
				if (key == lf->entry[i].__key) {
					delete lf;
					for (nonLeafNode* n : path)
						delete n;
					return -1;
				}
				//insert newEntry to leafnode and sort
				else if (lf->entry[i].__key == NULL) {
					lf->entry[i] = newEntry;
					sort(lf->entry, lf->entry + i + 1);
					break;
				}
			}

		}
		// leafNode is not full
		if (lf->entry[entrySize].__key == NULL) {
			lf->writeNode(fs, headerSize, blockSize);
		}
		// leafNode is full -> split node
		else {
			//split leaf node
			int split_idx = entrySize / 2 + 1;
			newleaf = new leafNode(entrySize, availBid++);
			for (int i = split_idx, newidx = 0; i <= entrySize; i++, newidx++) {
				newleaf->entry[newidx] = lf->entry[i];
				lf->entry[i] = emptyElem;
			}
			newleaf->setNextBid(lf->next_bid);
			lf->setNextBid(newleaf->bid);
			lf->writeNode(fs, headerSize, blockSize);
			newleaf->writeNode(fs, headerSize, blockSize);

			//add entry to nonLeafNode
			Entry upEntry = newleaf->entry[0];
			upEntry.__ptr = newleaf->bid;
			if (rootBid == lf->bid) {
				nonLeafNode* cur = new nonLeafNode(entrySize, availBid++);
				cur->setleastBid(lf->bid);
				for (int i = 0; i <= entrySize; i++) {
					if (cur->entry[i].__key == NULL) {
						cur->entry[i] = upEntry;
						sort(cur->entry, cur->entry + i + 1);
						break;
					}
				}
				depth++;
				rootBid = cur->bid;
				updateHeader();
				cur->writeNode(fs, headerSize, blockSize);
				delete cur;
			}
			for (auto iter = path.rbegin(); iter != path.rend(); iter++) {
				nonLeafNode* cur = *iter;
				for (int i = 0; i <= entrySize; i++) {
					if (cur->entry[i].__key == NULL) {
						cur->entry[i] = upEntry;
						sort(cur->entry, cur->entry + i + 1);
						break;
					}
				}
				//if nonLeafNode is full, split nonLeafNode
				if (cur->entry[entrySize].__key != NULL) {
					int split_idx = entrySize / 2;
					if (entrySize % 2 == 1)
						split_idx++;
					nonLeafNode* newNonleaf = new nonLeafNode(entrySize, availBid++);
					upEntry = cur->entry[split_idx];
					int tmp = cur->entry[split_idx].__ptr;
					cur->entry[split_idx] = emptyElem;
					upEntry.__ptr = newNonleaf->bid;
					//if root needs to be split, make new root
					if (cur->bid == rootBid) {
						nonLeafNode* newRoot = new nonLeafNode(entrySize, availBid++);
						newRoot->setleastBid(cur->bid);
						newRoot->entry[0] = upEntry;
						rootBid = newRoot->bid;
						depth++;
						newRoot->writeNode(fs, headerSize, blockSize);
						updateHeader();
						delete newRoot;
					}
					for (int i = split_idx + 1, newidx = 0; i <= entrySize; i++, newidx++) {
						newNonleaf->entry[newidx] = cur->entry[i];
						cur->entry[i] = emptyElem;
					}
					newNonleaf->setleastBid(tmp);
					cur->writeNode(fs, headerSize, blockSize);
					newNonleaf->writeNode(fs, headerSize, blockSize);
					delete newNonleaf;
				}
				else {
					cur->writeNode(fs, headerSize, blockSize);
					break;
				}
			}
		}
		for (nonLeafNode* n : path)
			delete n;
		delete lf;
		delete newleaf;
		return 1;
	}

	//find keys from lower to upper
	Entry_list rangeSerach(int lower, int upper) {
		int cnt = depth;
		int ptr = rootBid;
		Entry_list ret;
		nonLeafNode* nf = NULL;
		leafNode* lf = NULL;

		//search in nonleaf to leaf
		while (cnt > 1) {
			nf = new nonLeafNode(fs, entrySize, ptr, headerSize, blockSize);
			for (int i = 0; i <= entrySize; i++) {
				if (nf->entry[i].__key > lower) {
					if (i == 0)
						ptr = nf->least_bid;
					else
						ptr = nf->entry[i - 1].__ptr;
					break;
				}
				else if (nf->entry[i].__key == NULL) {
					ptr = nf->entry[i - 1].__ptr;
					break;
				}
			}
			cnt--;
			delete nf;
		}

		//search in leafNode
		while (true) {
			lf = new leafNode(fs, entrySize, ptr, headerSize, blockSize);
			int i;
			for (i = 0; i <= entrySize; i++) {
				if (lf->entry[i].__key == NULL)
					break;
				if (lf->entry[i].__key >= lower && lf->entry[i].__key <= upper) {
					ret.push_back(lf->entry[i]);
				}
				else if (lf->entry[i].__key > upper)
					return ret;
			}
			ptr = lf->next_bid;
			delete lf;
		}
		return ret;
	}

	//print level 0 and level 1 keys
	void print(char* output) {
		ofstream os(output);
		if (depth > 1) {
			//print level 0
			nonLeafNode* rootNode = new nonLeafNode(fs, entrySize, rootBid, headerSize, blockSize);
			os << "<0>\n";
			outNode(os, rootNode);
			os << "\n<1>\n";

			//print level 1 when level 1 is nonLeaf
			if (depth > 2) {
				nonLeafNode* nxt = new nonLeafNode(fs, entrySize, rootNode->least_bid, headerSize, blockSize);
				outNode(os, nxt);
				delete nxt;
				for (int i = 0; i <= entrySize; i++) {
					if (rootNode->entry[i].__ptr == NULL)
						break;
					nxt = new nonLeafNode(fs, entrySize, rootNode->entry[i].__ptr, headerSize, blockSize);
					outNode(os, nxt);
				}
			}

			//print level 1 when level 1 is leaf
			else {
				leafNode* nxt = new leafNode(fs, entrySize, rootNode->least_bid, headerSize, blockSize);
				outNode(os, nxt);

				delete nxt;
				for (int i = 0; i <= entrySize; i++) {
					if (rootNode->entry[i].__ptr == NULL)
						break;
					nxt = new leafNode(fs, entrySize, rootNode->entry[i].__ptr, headerSize, blockSize);
					outNode(os, nxt);
				}
			}
		}
		// print level 0 when level 0 is leaf
		else {
			os << "<0>\n";
			leafNode* root = new leafNode(fs, entrySize, rootBid, headerSize, blockSize);
			outNode(os, root);
			os << "\n<1>\n";
			os << "NULL";
		}
	}

	//print entry keys to ofstream
	void outNode(ofstream& os, Node* node) {
		for (int i = 0; i <= entrySize; i++) {
			if (node->entry[i].__key == NULL)
				break;
			os << node->entry[i].__key << ", ";
		}
	}

};

int main(int agrc, char* argv[]) {
	char command = argv[1][0];
	BPTree* bp;
	switch (command) {
	case 'c':
	case 'C': {
		bp = new BPTree(argv[2], atoi(argv[3]));
		delete bp;
		break;
	}
	case 'i':
	case 'I': {
		bp = new BPTree(argv[2]);
		fstream fs(argv[3], ios::in);
		string line;
		while (getline(fs, line)) {
			stringstream ss(line);
			getline(ss, line, ',');
			int key = atoi(line.c_str());
			getline(ss, line);
			int id = atoi(line.c_str());

			bp->insert(key, id);
		}
		delete bp;
		break;
	}
	case 's':
	case 'S': {
		bp = new BPTree(argv[2]);
		ifstream is(argv[3], ios::in);
		ofstream os(argv[4], ios::out);
		int key;
		while (is >> key) {
			Entry e = bp->find(key);
			os << e.__key << ", " << e.__val << "\n";
		}
		delete bp;
		break;
	}
	case 'r':
	case 'R': {
		bp = new BPTree(argv[2]);
		ifstream is(argv[3], ios::in);
		ofstream os(argv[4], ios::out);
		string line;
		while (getline(is, line)) {
			istringstream is(line);
			int lower, upper;
			getline(is, line, ',');
			lower = atoi(line.c_str());
			getline(is, line);
			upper = atoi(line.c_str());
			BPTree::Entry_list list = bp->rangeSerach(lower, upper);
			for (auto e : list)
				os << e.__key << ", " << e.__val << "\t";
			os << "\n";
		}
		delete bp;
		break;
	}
	case 'p':
	case 'P': {
		bp = new BPTree(argv[2]);
		bp->print(argv[3]);
		delete bp;
		break;
	}
	}
	return 0;
}