#pragma once
const int TABLE_SIZE = 101;  // A prime number is a good default

struct HashNode {
    string key;  // username
    int value;   // index in player array
    HashNode* next;

    HashNode(string k, int v) : key(k), value(v), next(nullptr) {}
};

class HashTable {
private:
    HashNode* table[TABLE_SIZE];

    int hashFunc(string key) {
        int hash = 0;
        for (int i = 0; i < key.length(); i++) {
            hash = (hash * 31 + key[i]) % TABLE_SIZE;
        }
        return hash;
    }

public:
    HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++)
            table[i] = nullptr;
    }

    void insert(string key, int value) {
        int index = hashFunc(key);
        HashNode* head = table[index];

        // Update if already exists
        HashNode* current = head;
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // Else, insert at head
        HashNode* newNode = new HashNode(key, value);
        newNode->next = head;
        table[index] = newNode;
    }

    int get(string key) {
        int index = hashFunc(key);
        HashNode* current = table[index];
        while (current != nullptr) {
            if (current->key == key)
                return current->value;
            current = current->next;
        }
        return -1;  // Not found
    }

    bool exists(string key) {
        return get(key) != -1;
    }
    
    bool usernameExists(string uname) {
        ifstream file("Hash_users.txt");
        string line;
        while (getline(file, line)) {
            if (line == uname) return true;
        }
        return false;
    }
   
    void saveUsernameIfNew(string uname, int playerIdx) {
        if (!usernameExists(uname)) {
            ofstream file("Hash_users.txt", ios::app);
            file << uname << endl;
            file.close();
        }

        // Insert into the HashTable (if not already present)
        if (!exists(uname)) {
            insert(uname, playerIdx);
        }
    }

    ~HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* current = table[i];
            while (current) {
                HashNode* next = current->next;
                delete current;
                current = next;
            }
        }
    }
};
