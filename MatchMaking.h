#pragma once
#include<iostream>
using namespace std;

class Queue {
	struct Node {
		string username;
		int score;
		Node* next;
	};
	int count;
	Node* top;
public:
	Queue() {
		top = nullptr;
		count = 0;
	}
	
	void enqueue(string name, int s) {
		Node* newnode = new Node();
		newnode->username = name;
		newnode->score = s;

		newnode->next = nullptr;
		if (top == nullptr|| s>top->score) {
			newnode->next = top;
			top = newnode;
		}
		else {
			Node* current = top;
			while (current->next != nullptr && current->next->score >= s) {
				current = current->next;
			}
			newnode->next = current->next;
			current->next = newnode;
		}
		count++;
	}
	
	string dequeue() {
		if (top == nullptr) 
			return "";

		Node* temp = top;
		string username = temp->username;
		int score = temp->score;
		top = top->next;
		delete temp;
		count--;
		return username;
	}

	bool hasAtLeastTwo() const {
		return count >= 2;
	}

	string peek() const {
		if (top == nullptr) return "";
		return top->username;
	}

	void LoadFromFile() {
		ifstream file("players.txt");
		string line;
		while (getline(file, line)) {
			string username = "";
			string scoreStr = "";
			int score = 0;
			int i = 0;

			// Skip "username: "
			while (i < line.length() && line[i] != ':') i++;
			i += 2; // move past ": "

			// Extract username until ','
			while (i < line.length() && line[i] != ',') {
				username += line[i];
				i++;
			}

			// Skip ", score: "
			while (i < line.length() && line[i] != ':') i++;
			i += 2;

			// Extract score string
			while (i < line.length()) {
				scoreStr += line[i];
				i++;
			}

			// Convert scoreStr to int manually
			score = 0;
			for (int j = 0; j < scoreStr.length(); j++) {
				score = score * 10 + (scoreStr[j] - '0');
			}
			enqueue(username, score);
		}

		file.close();
	}

	void display() {
		Node* current = top;
		while (current != nullptr) {
			cout << current->username << " " << current->score<<endl;
			current = current->next;
		}
	}

};