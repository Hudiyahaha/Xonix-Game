#include <ctime>
#include <sstream>
#include<iostream>
#include<string>
#include <fstream>

using namespace std;

struct Tile {
    int x, y; // Coordinates of tile
    int value;
    Tile* next; // Pointer to next tile

    Tile(int x, int y, int value) : x(x), y(y), value(value)
    {
        next = nullptr;
    }

    Tile(int x, int y, Tile* n) : x(x), y(y), next(n)
    { }
};

struct GameState {
    string timestamp;
    string saveID;

    string player1ID;
    int score1;
    int player1X, player1Y;

    string player2ID;
    int score2;
    int player2X, player2Y;

    Tile* tileHead;

    bool loadGame(GameState& gameState, string filename) {
        ifstream fin(filename);
        if (!fin) {
            cout << "Could not open save file!" << endl;
            return false;
        }

        // Clear old tiles
        gameState.clearTiles();

        getline(fin, gameState.timestamp);
        fin >> saveID;
        fin >> player1ID;
        fin >> score1;
        fin >> player1X >> player1Y; // Load player 1's position

        if(score2!= -1)
        fin >> player2ID;
        fin >> score2;
        fin >> player2X >> player2Y; // Load player 2's position

        int row, col, value;
        while (fin >> row >> col >> value) {
            gameState.addTile(row, col, value);
        }

        fin.close();
        return true;
    }

    void saveGame() {
        ofstream file(saveID + ".txt");

        if (!file) {
            cout << "Error saving game.\n";
            return;
        }

        file << timestamp << endl;
        file << saveID << endl;
        file << player1ID << endl;
        file << score1 << endl;
        file << player1X << " " << player1Y << endl; // Save player 1's position
        file << player2ID << endl;
        file << score2 << endl;
        file << player2X << " " << player2Y << endl; // Save player 2's position

        Tile* current = tileHead;
        while (current != nullptr) {
            file << current->x << " " << current->y << " " << current->value << endl;
            current = current->next;
        }

        file.close();
        cout << "Game saved successfully! Your Save ID is: " << saveID << endl;
    }

    void clearTiles() {
        Tile* curr = tileHead;
        while (curr != nullptr) {
            Tile* temp = curr;
            curr = curr->next;
            delete temp;
        }
        tileHead = nullptr;
    }

    void addTile(int row, int col, int value) {
        Tile* newNode = new Tile(row,col,value);
        newNode->next = tileHead;
        tileHead = newNode;
    }

    void writeString(ofstream& outFile, const string& s) {
        size_t length = s.size();
        outFile.write((char*)&length, sizeof(length));
        outFile.write(s.c_str(), length);
    }

    void writeInt(ofstream& outFile, int value) {
        outFile.write((char*)&value, sizeof(value));
    }
};

