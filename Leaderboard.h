#pragma once

#include<iostream>
using namespace std;

class Minheap {
	Player* heap[10];
	int size;
	int count;
public:
	Minheap() {
		size = 10;
		count = 0;
	}
	void heapify_Up(Player* p) {
		
			int i = count;
			
			while (i > 0 && (p->score < heap[(i - 1) / 2]->score)) {
				heap[i] = heap[(i - 1) / 2];
				i = (i - 1) / 2;
				

			}
			heap[i] = p;
			count++;
			
		
	}
	
	int findPlayerIndexByUsername(const string& username) {
		for (int i = 0; i < count; i++) {
			if (heap[i]->getUsername() == username) {
				return i;
			}
		}
		return -1;
	}

	void heapify_down(int i) {
		Player* temp = heap[i];
		int j = 2 * i + 1;  //left child
		
		while (j < count) {
			if (j + 1 < count && (heap[j + 1]->score < heap[j]->score)) {

				j = j + 1; //choose smaller child
			
			}
			if (temp->score > heap[j]->score) {
				heap[i] = heap[j];
				i = j;
				
				j = 2 * i + 1;
			}
			else {
				break;
			}
		}
		heap[i] = temp;
		
	}
	
	void insert(Player* p) {
		int existingIndex = findPlayerIndexByUsername(p->getUsername());
		if (existingIndex != -1) {
			// Player already exists: update score and re-heapify from their index
			heap[existingIndex]->score = p->score;
			heap[existingIndex]->setTotalScore(p->getTotalScore());

			// Re-heapify the heap from the index
			heapify_down(existingIndex);
			while (existingIndex > 0 &&
				heap[existingIndex]->score < heap[(existingIndex - 1) / 2]->score) {
				// Swap with parent
				Player* temp = heap[existingIndex];
				heap[existingIndex] = heap[(existingIndex - 1) / 2];
				heap[(existingIndex - 1) / 2] = temp;

				existingIndex = (existingIndex - 1) / 2;
			}
			return;
		}

		// Player is new
		if (count < 10) {
			heapify_Up(p);
		}
		else if (p->score > heap[0]->score) {
			
			heap[0] = p;
			heapify_down(0);
		}
	}

	void display( Font& font) {
		Player* temp[10];
		for (int i = 0; i < count; i++) {
			temp[i] = heap[i];
		}

		// Sort by score descending
		for (int i = 0; i < count; i++) {
			for (int j = 0; j < count - i - 1; j++) {
				if (temp[j]->score < temp[j + 1]->score) {
					Player* t = temp[j];
					temp[j] = temp[j + 1];
					temp[j + 1] = t;
				}
			}
		}
		Texture background;
		background.loadFromFile("themes/background.png");
		Sprite bg(background);
		
		bool viewing = true;
		RenderWindow window(VideoMode(800,600), "Leaderboard!");
		while (viewing && window.isOpen()) {
			Event e;
			while (window.pollEvent(e)) {
				if (e.type == Event::Closed)
					window.close();
				if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
					viewing = false;
			}

			window.clear(); // dark background
			window.draw(bg);
			// Title
			Text title(" Leaderboard ", font, 36);
			title.setFillColor(Color::Blue);
			title.setPosition(180, 20);
			window.draw(title);

			// Draw player cards
			for (int i = 0; i < count && i < 10; i++) {
				float x = 80;
				float y = 80 + i * 50;

				// Background rectangle (card)
				RectangleShape card(Vector2f(500, 40));
				card.setPosition(x, y);
				card.setFillColor(Color(30, 30, 30));
				card.setOutlineColor(Color::White);
				card.setOutlineThickness(2);
				window.draw(card);

				// Player text
				Text playerText;
				playerText.setFont(font);
				playerText.setCharacterSize(22);
				playerText.setFillColor(Color::White);
				playerText.setPosition(x + 15, y + 10);

				string entry = to_string(i + 1) + ". " +
					temp[i]->getUsername() + "   -   " +
					to_string(temp[i]->getTotalScore()) + " pts";

				playerText.setString(entry);
				window.draw(playerText);
			}

			// Hint
			Text backHint("Press ESC to return to menu", font, 18);
			backHint.setFillColor(Color(180, 180, 180));
			backHint.setPosition(100, 700);
			window.draw(backHint);

			window.display();
		}
	}

};

class Leaderboard {
	Minheap* topPlayers;
	
public:
	Leaderboard() {
		topPlayers = new Minheap();

	
	}

	void update(Player* p) {
		Player* clone = new Player(p->getUsername(), p->getTotalScore());
		topPlayers->insert(clone);
	}
	
	void savePlayerToFile(Player* p) {
		ifstream infile("players.txt");
		const int MAX = 100;
		string lines[MAX];
		int count = 0;
		bool found = false;

		string targetUsername = p->getUsername();

		// Read all lines into array
		string line;
		while (getline(infile, line) && count < MAX) {
			string existingUsername = "";
			int i = 0;

			// Skip "username: "
			while (i < line.length() && line[i] != ':') i++;
			i += 2;

			// Extract username
			while (i < line.length() && line[i] != ',') {
				existingUsername += line[i];
				i++;
			}

			if (existingUsername == targetUsername) {
				// Replace line with updated score
				string updated = "username: " + p->getUsername() + ", score: " + to_string(p->getTotalScore());
				lines[count++] = updated;
				found = true;
			}
			else {
				lines[count++] = line;
			}
		}
		infile.close();

		// If player wasn't found, add new entry
		if (!found && count < MAX) {
			lines[count++] = "username: " + p->getUsername() + ", score: " + to_string(p->getTotalScore());
		}

		// Write back to file
		ofstream outfile("players.txt", ios::trunc); // truncate to overwrite
		for (int i = 0; i < count; i++) {
			outfile << lines[i] << endl;
		}
		outfile.close();
	}

	void loadAllPlayersFromFile() {
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
			

			// Create and update player
			Player* p = new Player(username, score);

			update(p);
		}

		file.close();

	}
	
	void show( Font& font) {

		
		topPlayers->display(  font);  // Display top 10 players from Minheap
	}
	
	~Leaderboard() {
		delete topPlayers;
	}
};