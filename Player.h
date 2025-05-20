#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "HashTable.h"
#include<fstream>
using namespace std;
using namespace sf;

struct FriendNode {
    string username;
    FriendNode* next;
    FriendNode(string name) : username(name), next(nullptr) {}
};

class Player {
public:
    int x, y;          // Current position (grid coordinates)
    int dx, dy;        // Current movement direction
    int trailValue;    // Value used in grid to mark player's trail (2 for P1, 3 for P2)
    int score;         // Player's score
    bool isConstructing;
    string username;
    string matchHistory;
    FriendNode* friendsHead = nullptr;           // Accepted friends
    FriendNode* pendingRequestsHead = nullptr;
    Keyboard::Key upKey, downKey, leftKey, rightKey;

    Player(string name,int startX, int startY, int trailVal,
        Keyboard::Key up, Keyboard::Key down,
        Keyboard::Key left, Keyboard::Key right)
    {
        username = name;
        x = startX;
        y = startY;
        dx = dy = 0;
        trailValue = trailVal;
        score = 0;
        matchHistory = "";
        upKey = up;
        downKey = down;
        leftKey = left;
        rightKey = right;
    }

    Player(string name, int s) {
        username = name;
        score = s;
    }

    Player(string name) {
        username = name;
        loadFriendsFromFile();
        loadRequestsFromFile();
    }

    void setTotalScore(int s) {
        score = s;
    }

    int getTotalScore(){
        return score;
    }

    string getUsername() {
        return username;
    }

    void loadFriendsFromFile() {
        ifstream file("friends_" + username + ".txt");
        string friendName;
        while (getline(file, friendName)) {
            cleanUsername(friendName);
            if (!friendName.empty()) addFriend(friendName);
        }
    }

    void loadRequestsFromFile() {
        ifstream file("pending_" + username + ".txt");
        string requester;
        while (getline(file, requester)) {
            cleanUsername(requester);
            if (!requester.empty()) receiveRequest(requester);
        }
    }

    void saveFriendsToFile() {
        ofstream file("friends_" + username + ".txt");
        FriendNode* current = friendsHead;
        while (current) {
            file << current->username << endl;
            current = current->next;
        }
    }

    void saveRequestsToFile() {
        ofstream file("pending_" + username + ".txt");
        FriendNode* current = pendingRequestsHead;
        while (current) {
            file << current->username << endl;
            current = current->next;
        }
    }

    void cleanUsername(string& uname) {
        uname.erase(remove_if(uname.begin(), uname.end(), [](char c) {
            return c == '\r' || c == '\n' || isspace(c);
            }), uname.end());
    }

    bool isFriend(string other) {
        cleanUsername(other);
        FriendNode* current = friendsHead;
        while (current) {
            if (current->username == other)
                return true;
            current = current->next;
        }
        return false;
    }

    // Checks if a friend request already exists
    bool hasPendingRequest(string other) {
        cleanUsername(other);
        FriendNode* current = pendingRequestsHead;
        while (current) {
            if (current->username == other)
                return true;
            current = current->next;
        }
        return false;
    }

    // Add to pending requests
    void receiveRequest(string from) {
        cleanUsername(from);
        cout << "receiveRequest: checking " << from << endl;
        if (hasPendingRequest(from) || isFriend(from)) {
            cout << "Request already exists or already friends\n";
            return;
            }
        cout << "Adding pending request from " << from << endl;
        FriendNode* newNode = new FriendNode(from);
        newNode->next = pendingRequestsHead;
        pendingRequestsHead = newNode;
    }

    // Accept a pending request
    bool acceptRequest(string from) {
        cleanUsername(from);
        FriendNode* prev = nullptr;
        FriendNode* current = pendingRequestsHead;
        
        while (current) {
            if (current->username == from) {
                // Remove from pending
                if (prev) prev->next = current->next;
                else pendingRequestsHead = current->next;

                delete current;

                // Add to friends
                addFriend(from);
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    void addFriend(string friendUsername) {
        cleanUsername(friendUsername);
       
        if (isFriend(friendUsername)) return;
        FriendNode* newNode = new FriendNode(friendUsername);
        newNode->next = friendsHead;
        friendsHead = newNode;
    }

    void viewFriends() {
        FriendNode* current = friendsHead;
        cout << username << "'s friends: ";
        while (current) {
            cout << current->username << " ";
            current = current->next;
        }
        cout << endl;
    }

    void viewPendingRequests() {
       
        FriendNode* current = pendingRequestsHead;
        cout << username << "'s pending friend requests: ";
        while (current) {
            cout << current->username << " ";
            current = current->next;
        }
        cout << endl;
    }

    void sendFriendRequest(string toUsername, Player* players[], HashTable& table) {
        cleanUsername(toUsername);
        int toIdx = table.get(toUsername);
        

        if (toIdx == -1 ) {
            cout << "Invalid request." << endl;
            return;
        }

        Player* from = this;
        Player* to = players[toIdx];

        if (from->isFriend(toUsername)) {
            cout << "Already friends." << endl;
            return;
        }

        if (to->hasPendingRequest(this->username)) {
            cout << "Request already sent." << endl;
            return;
        }

        to->receiveRequest(this->username);
        to->saveRequestsToFile();
        cout << "Added pending request to " << to->getUsername() <<  endl;
        saveFriendsToFile();
        saveRequestsToFile();
    }

    void acceptFriendRequest( string fromUsername, Player* players[], HashTable& table) {
        cleanUsername(fromUsername);
        int fromIdx = table.get(fromUsername);
        if (fromIdx == -1) {
            cout << "Invalid sender username." << endl;
            return;
        }

        Player* from = players[fromIdx];
        Player* to = this;  // 'this' is the player accepting the request

        if (to->acceptRequest(fromUsername)) {
            from->addFriend(to->username);  // add 'to' as friend in 'from's list
            to->addFriend(fromUsername);    // and vice versa
            cout << to->username << " accepted friend request from " << fromUsername << endl;
            from->saveFriendsToFile();
            to->saveFriendsToFile();
            to->saveRequestsToFile();
        }
        else {
            cout << "No pending request from " << fromUsername << " to accept." << endl;
        }
      
    }

    void saveProfile() {
        string profilePath = "profiles/" + username + ".txt";
        string selectedTheme = "";

        ifstream inFile(profilePath);
        if (inFile.is_open()) {
            string line;
            while (getline(inFile, line)) {
                if (line.find("Selected Theme:") != string::npos) {
                    selectedTheme = line; // Save the selected theme line
                }
            }
            inFile.close();
        }

        ofstream outFile(profilePath);
        if (outFile.is_open()) {
            outFile << "Username: " << username << endl;
            outFile << "Score: " << score << endl;
            outFile << "Match history: " << matchHistory << endl;

            // Re-append the selected theme if it exists
            if (!selectedTheme.empty()) {
                outFile << selectedTheme << endl;
            }

            outFile.close();
        }
        else {
            cout << "Failed to save profile for " << username << endl;
        }
    }

    bool loadProfile() {
        ifstream file("profiles/" + username + ".txt");
        if (!file) {
            cout << "Couldnt load file" << endl;
        }
        if (file.is_open()) {
            getline(file, username);
            file >> score;
            file.ignore(); // skip newline
            getline(file, matchHistory);
            file.close();
            cout << "profile loaded" << endl;
            return true;
        }
        return false;
    }

    void handleInput() {
        if (Keyboard::isKeyPressed(leftKey)) { dx = -1; dy = 0; }
        if (Keyboard::isKeyPressed(rightKey)) { dx = 1; dy = 0; }
        if (Keyboard::isKeyPressed(upKey)) { dx = 0; dy = -1; }
        if (Keyboard::isKeyPressed(downKey)) { dx = 0; dy = 1; }
    }

    void move(int maxN, int maxM) {
        x += dx;
        y += dy;

        if (x < 0) x = 0; if (x > maxN - 1) x = maxN - 1;
        if (y < 0) y = 0; if (y > maxM - 1) y = maxM - 1;
    }

    void resetMovement() {
        dx = dy = 0;
    }
};
