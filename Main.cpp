#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include <time.h>
#include <string>
#include <iostream>
#include "Menu.h"
#include "Login.h"
#include "Player.h"
#include "Leaderboard.h"
#include "HashTable.h"
#include"GameState.h"
#include"MatchMaking.h"
#include"Inventory.h"
#include<ctime>
#include<fstream>
using namespace sf;
using namespace std;

const int M = 25;
const int N = 40;

int grid[M][N] = { 0 };
int ts = 18; //tile size
int gameMode = 1;

struct Enemy
{
    int x, y, dx, dy;

    Enemy()
    {
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
    }

    void move()
    {
        x += dx;
        if (grid[y / ts][x / ts] == 1 || grid[y / ts][x / ts] == 4 || grid[y / ts][x / ts] == 5) {
            dx = -dx;
            x += dx;
        }

        y += dy;
        if (grid[y / ts][x / ts] == 1 || grid[y / ts][x / ts] == 4 || grid[y / ts][x / ts] == 5) {

            dy = -dy;
            y += dy;
        }
    }

};

void drop(int y, int x)
{
    if (grid[y][x] == 0) grid[y][x] = -1;
    if (grid[y - 1][x] == 0) drop(y - 1, x);
    if (grid[y + 1][x] == 0) drop(y + 1, x);
    if (grid[y][x - 1] == 0) drop(y, x - 1);
    if (grid[y][x + 1] == 0) drop(y, x + 1);
}

int selectGameMode(Inventory& inventory,Music& music) {
    RenderWindow window(VideoMode(800, 600), "Select Game Mode");

    Texture background;
    themeApply(background, music, inventory,false);
    Sprite s(background);

    Font font;
    if (!font.loadFromFile("BlazeCircuitRegular-xRvxj.ttf")) {
        cout << "Failed to load Arial.ttf!" << endl;
        return 1;
    }

    string modes[2] = { "Single Player", "Multiplayer" };
    int selected = 0;
    
    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Up || e.key.code == Keyboard::W)
                    selected = (selected + 1) % 2;
                if (e.key.code == Keyboard::Down || e.key.code == Keyboard::S)
                    selected = (selected + 1) % 2;

                if (e.key.code == Keyboard::Enter) {
                    window.close();
                    return (selected == 0) ? 1 : 2;
                }
            }
        }

        window.clear();

        window.draw(s);
        Text title("Select Game Mode", font, 30);
        title.setPosition(88, 90);
        title.setFillColor(Color::White);
        window.draw(title);

        for (int i = 0; i < 2; i++) {
            Text option;
            option.setFont(font);
            option.setString(modes[i]);
            option.setCharacterSize(24);
            option.setPosition(110, 150 + i * 50);

            if (i == selected)
                option.setFillColor(Color::Red);
            else
                option.setFillColor(Color::White);

            window.draw(option);
        }

        window.display();
    }

    return 1; // fallback single player
}

string SaveInput(RenderWindow& window, Font& font, GameState& game) {
    string saveID = ""; // To store the SaveID entered by the user
    Text prompt("Enter your Save ID:", font, 30);
    prompt.setFillColor(Color::White);
    prompt.setPosition(20, 20);

    Text inputText("", font, 30);
    inputText.setFillColor(Color::Green);
    inputText.setPosition(20, 70);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return ""; // Return empty if the window is closed
            }

            if (event.type == Event::TextEntered) {
                // Handle text input
                if (event.text.unicode == '\b') { // Handle backspace
                    if (!saveID.empty()) {
                        saveID.pop_back();
                    }
                }
                else if (event.text.unicode == '\r') { // Handle Enter key
                    game.saveID=saveID;
                    return saveID;
                }

                else if (event.text.unicode < 128) { // Only allow ASCII characters
                    saveID += static_cast<char>(event.text.unicode);
                }
            }
        }

        // Update the input text
        inputText.setString(saveID);

        // Render the prompt and input
        window.clear();
        window.draw(prompt);
        window.draw(inputText);
        window.display();
    }

    return ""; // Fallback return if the window is closed
}

void runSinglePlayer( Font& font, Sprite& sTile, Sprite& sGameover, Sprite& sEnemy, Font& newfont, PlayerLogin& login, Leaderboard& board,bool saved)
{
    RenderWindow window(VideoMode(N * ts, M * ts), "SinglePlayer"); //40*18, 25*18
    window.setFramerateLimit(60);

    int score = 0;
    int enemyCount = 4;
    Enemy a[10];

    GameState gameState;
    time_t now = time(0);              // get current time
    char dt[26];                       // buffer to hold the date
    ctime_s(dt, sizeof(dt), &now);     // secure version of ctime

    gameState.timestamp = dt;          // store in your variable

    gameState.player1ID = login.getusername(); // Player's username
    gameState.tileHead = nullptr; // Initialize linked list of tiles
    gameState.player2ID = " "; // Player's username
    gameState.score2 = -1;

    Player player1(login.getusername(), 0, 0, 2, Keyboard::Up, Keyboard::Down, Keyboard::Left, Keyboard::Right);

    float timer = 0, delay = 0.07;
    Clock clock;
    
    int arr[] = { 3, 0, 4, 0, 4, 5, 6, 0, 1, 6, 5, 4 };
    int numPairs = 6;
    int pairIndex = rand() % numPairs;
    int index = pairIndex * 2;

    int tile1 = arr[index];
    int tile2 = arr[index + 1];

    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 40; j++) {
            grid[i][j] = 0;
        }
    }

    if (saved) {

        SaveInput(window, font, gameState);
        string s = gameState.saveID + ".txt";
        cout <<" the save id is " << s<<endl;

        if (gameState.loadGame(gameState, s))
        {
            cout << "Game loaded successfully!"<<endl;

            player1.x = gameState.player1X;
            player1.y = gameState.player1Y;
          

            // Fill the grid
            for (int i = 0; i < 25; i++) {
                for (int j = 0; j < 40; j++) {
                    grid[i][j] = 0;
                }
            }

            Tile* current = gameState.tileHead;

            while (current != nullptr) {
           // cout << current->x <<" " << current->y << " " << current->value << endl;
                int row = current->x;
                int col = current->y;
                int val = current->value;
                grid[row][col] = val;
                current = current->next;
                
            }
            score = gameState.score1;
            player1.score = gameState.score1;
        }
        else {
            cout << "Failed to load game.\n";
        }
    }

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) grid[i][j] = 1;

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed || e.key.code == Keyboard::Escape)
                window.close();
        }

        player1.handleInput();

        if (timer > delay) {
            player1.move(N, M);

            if (grid[player1.y][player1.x] == 0)
                grid[player1.y][player1.x] = player1.trailValue;

            timer = 0;
        }

        for (int i = 0; i < enemyCount; i++) a[i].move();

        if (grid[player1.y][player1.x] == 1) {
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            int tiles_captured = 0;
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) grid[i][j] = 0;
                    else if (grid[i][j] == 0 || grid[i][j] == player1.trailValue) {
                        tiles_captured++;
                        grid[i][j] = 1;
                    }
                }
            }
            score += (tiles_captured > 10) ? tiles_captured * 2 : tiles_captured;
        }

        for (int i = 0; i < enemyCount; i++) {
            int ex = a[i].x / ts;
            int ey = a[i].y / ts;
            if (grid[ey][ex] == player1.trailValue) {
                // Show Game Over menu for 5 seconds
                Clock endClock;
                while (endClock.getElapsedTime().asSeconds() < 4.0f && window.isOpen()) {
                    Event endEvent;
                    while (window.pollEvent(endEvent)) {
                        if (endEvent.type == Event::Closed)
                            window.close();
                    }
                    window.clear();
                    window.draw(sGameover);

                    // Custom end menu text
                    Text endText;
                    endText.setFont(newfont);
                    endText.setCharacterSize(32);
                    endText.setFillColor(Color::White);
                    endText.setString("Game Over!\nScore: " + to_string(score) + "\nReturning to menu...");
                    endText.setPosition(180, 250);
                    window.draw(endText);

                    window.display();
                }

                player1.matchHistory += "Score: " + to_string(score) + " | ";
                player1.score += score;
                player1.saveProfile();
                board.update(&player1);
                board.savePlayerToFile(&player1);

                return;
            }
        }


        if (Keyboard::isKeyPressed(Keyboard::Z))
        {
            gameState.saveID = SaveInput(window, newfont, gameState);
            if (!gameState.saveID.empty()) {
                // Clear any existing tiles in the linked list
                gameState.clearTiles();

                // Add all non-zero tiles to the linked list
                for (int i = 0; i < M; i++) {
                    for (int j = 0; j < N; j++) {
                        if (grid[i][j] != 0) {
                            gameState.addTile(i, j, grid[i][j]);
                        }
                    }
                }

                // Update player positions in the game state
                gameState.player1X = player1.x;
                gameState.player1Y = player1.y;

                gameState.score1 = score;
                gameState.score2 = -1;
                 gameState.saveGame();
            }
        }

        
        window.clear();
        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) continue;
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(tile1 *18, 0, ts, ts)); //border+ area
                else if (grid[i][j] == player1.trailValue) sTile.setTextureRect(IntRect(tile2 *18, 0, ts, ts));

                sTile.setPosition(j * ts, i * ts);
                window.draw(sTile);
            }

        if (save) score = gameState.score1;
        Text scoreText;
        scoreText.setFont(newfont);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, 10);
        scoreText.setString("Score: " + to_string(score));
        window.draw(scoreText);

        sTile.setTextureRect(IntRect(36, 0, ts, ts));
        sTile.setPosition(player1.x * ts, player1.y * ts);
        window.draw(sTile);

        sEnemy.rotate(10);
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y);
            window.draw(sEnemy);
        }

        window.display();
    }
}

void clearPlayerAreaAndTrail(int trailValue, int areaValue) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == trailValue || grid[i][j] == areaValue) {
                grid[i][j] = 0;
            }
        }
    }
}

bool hasTrail(int trailValue) {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (grid[i][j] == trailValue) return true;
    return false;
}

void runMultiPlayer( Font& font, Sprite& sTile, Sprite& sGameover, Sprite& sEnemy,Font& newfont, PlayerLogin& login, PlayerLogin& login2, Leaderboard& board)
{
    RenderWindow window(VideoMode(N * ts, M * ts), "MultiPlayer"); //40*18, 25*18
    window.setFramerateLimit(60);

    int score1 = 0;
    int score2 = 0;
    int enemyCount = 5;
    Enemy a[10];

    GameState gameState;
    time_t now = time(0);              // get current time
    char dt[26];                       // buffer to hold the date
    ctime_s(dt, sizeof(dt), &now);     // secure version of ctime

    gameState.timestamp = dt;          // store in your variable

    gameState.player1ID = login.getusername(); // Player's username
    gameState.tileHead = nullptr; // Initialize linked list of tiles
    gameState.player2ID = login2.getusername(); // Player's username

    Player player1(login.getusername(), 0, 0, 2, Keyboard::W, Keyboard::S, Keyboard::A, Keyboard::D);
    Player player2(login2.getusername(), N - 1, M - 1, 3, Keyboard::Up, Keyboard::Down, Keyboard::Left, Keyboard::Right);

    float timer = 0, delay = 0.07;
    Clock clock;
    bool player1_alive = true;
    bool player2_alive = true;
    
    int arr[] = { 3,4 , 4, 3, 6, 1, 4, 5, 3, 6, 6, 3 };
    int numPairs = 6;
    int pairIndex = rand() % numPairs;
    int index = pairIndex * 2;

    int tile1 = arr[index];
    int tile2 = arr[index + 1];

    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 40; j++) {
            grid[i][j] = 0;
        }
    }

    // Initialize borders
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;

    if (save) {

        SaveInput(window, font, gameState);
        string s = gameState.saveID + ".txt";
        cout << " the save id is " << s << endl;

        if (gameState.loadGame(gameState, s))
        {
            cout << "Game loaded successfully!" << endl;
            
            player1.x = gameState.player1X;
            player1.y = gameState.player1Y;
            player2.x = gameState.player2X;
            player2.y = gameState.player2Y;

            // Fill the grid
            for (int i = 0; i < 25; i++) {
                for (int j = 0; j < 40; j++) {
                    grid[i][j] = 0;
                }
            }

            Tile* current = gameState.tileHead;

            while (current != nullptr) {
               // cout << current->x << " " << current->y << " " << current->value << endl;
                int row = current->x;
                int col = current->y;
                int val = current->value;
                grid[row][col] = val;
                current = current->next;
            }

            player1.score = gameState.score1;
            player2.score = gameState.score2;
            score1 = gameState.score1;
            score2 = gameState.score2;
        }
        else {
            cout << "Failed to load game.\n";
        }
    }

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed || e.key.code == Keyboard::Escape)
                window.close();
        }

        // Ignore input if dead
        if (player1_alive) player1.handleInput();
        if (player2_alive) player2.handleInput();

        if (timer > delay) {
            if (player1_alive) player1.move(N, M);
            if (player2_alive) player2.move(N, M);

            // === Check player-vs-player trail collision BEFORE placing trails ===
            if (player1_alive && grid[player1.y][player1.x] == 5) {
                player1_alive = false;
                player1.x = player1.y = 0;
                clearPlayerAreaAndTrail(2, 4);
            }
            if (player2_alive && grid[player2.y][player2.x] == 4) {
                player2_alive = false;
                player2.x = N - 1;
                player2.y = M - 1;
                clearPlayerAreaAndTrail(3, 5);
            }

            // === Check player1 & player2 BOTH collide in same tile ===
            if (player1_alive && player2_alive &&
                player1.x == player2.x && player1.y == player2.y) {
                player1_alive = false;
                player2_alive = false;
            }

            // === Place trails only if alive ===
            if (player1_alive && grid[player1.y][player1.x] == 0)
                grid[player1.y][player1.x] = player1.trailValue;
            if (player2_alive && grid[player2.y][player2.x] == 0)
                grid[player2.y][player2.x] = player2.trailValue;

            timer = 0;
        }

        // === Game Over if both dead ===
        if (!player1_alive && !player2_alive) {
            window.clear();
            window.draw(sGameover);
            window.display();
            player1.matchHistory += "Score: " + to_string(score1) + " | ";
            player1.score += score1;
            player1.saveProfile();
            player2.matchHistory += "Score: " + to_string(score2) + " | ";
            player2.score += score2;
            player2.saveProfile();
            board.update(&player1);
            board.update(&player2);
            board.savePlayerToFile(&player1);
            board.savePlayerToFile(&player2);

            return;
        }

        // === Move enemies ===
        for (int i = 0; i < enemyCount; i++) a[i].move();

        // === PLAYER1 CAPTURE ===
        if (player1_alive && (grid[player1.y][player1.x] == 1 || grid[player1.y][player1.x] == 4) && hasTrail(player1.trailValue))
        {
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            int tiles_captured = 0;
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) grid[i][j] = 0;  // outside
                    else if (grid[i][j] == 0 || grid[i][j] == player1.trailValue) {
                        tiles_captured++;

                        grid[i][j] = 4;  // Player1 area
                    }
                }
            }
            score1 += (tiles_captured > 10) ? tiles_captured * 2 : tiles_captured;
        }

        // === PLAYER2 CAPTURE ===
        if (player2_alive && (grid[player2.y][player2.x] == 1 || grid[player2.y][player2.x] == 5) && hasTrail(player2.trailValue))
        {
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            int tiles_captured = 0;
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) grid[i][j] = 0;  // outside
                    else if (grid[i][j] == 0 || grid[i][j] == player2.trailValue) {
                        tiles_captured++;

                        grid[i][j] = 5;  // Player2 area
                    }
                }
            }
            score2 += (tiles_captured > 10) ? tiles_captured * 2 : tiles_captured;
        }

        // === Enemy hits player trail = game over ===
        for (int i = 0; i < enemyCount; i++) {
            int ex = a[i].x / ts;
            int ey = a[i].y / ts;

            if (grid[ey][ex] == player1.trailValue || grid[ey][ex] == player2.trailValue) {
                window.draw(sGameover);
                window.display();
                player1.matchHistory += "Score: " + to_string(score1) + " | ";
                player1.score += score1;
                player1.saveProfile();

                player2.matchHistory += "Score: " + to_string(score2) + " | ";
                player2.score += score2;
                player2.saveProfile();
                board.update(&player1);
                board.update(&player2);
                board.savePlayerToFile(&player1);
                board.savePlayerToFile(&player2);
                return;
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Z))
        {
            gameState.saveID = SaveInput(window, newfont, gameState);
            if (!gameState.saveID.empty()) {
                gameState.clearTiles();

                for (int i = 0; i < M; i++) {
                    for (int j = 0; j < N; j++) {
                        if (grid[i][j] != 0) {
                            gameState.addTile(i, j, grid[i][j]);
                        }
                    }
                }

                // Update player positions in the game state
                gameState.player1X = player1.x;
                gameState.player1Y = player1.y;
                gameState.player2X = player2.x;
                gameState.player2Y = player2.y;


                gameState.score1 = score1;
                gameState.score2 = score2;
                gameState.saveGame();
            }
        }

        // === RENDER ===
        window.clear();

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) continue;
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts)); // blue border

                // Player 1 (trail=2 OR area=4) -> same green tile
                if (grid[i][j] == 2 || grid[i][j] == 4)
                    sTile.setTextureRect(IntRect(tile1*18, 0, ts, ts));  // Green tile

                // Player 2 (trail=3 OR area=5) -> same orange tile
                if (grid[i][j] == 5 || grid[i][j] == 3)
                    sTile.setTextureRect(IntRect(tile2*18, 0, ts, ts));  // Orange tile

                sTile.setPosition(j * ts, i * ts);
                window.draw(sTile);
            }
        }

        // Score text
        Text scoreText;
        scoreText.setFont(newfont);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(15, 12);
        scoreText.setString("Score: " + to_string(score1));
        window.draw(scoreText);

        scoreText.setPosition(570, 12);
        scoreText.setString("Score: " + to_string(score2));
        window.draw(scoreText);

        // Draw players (only if alive)
        sTile.setTextureRect(IntRect(36, 0, ts, ts)); // player tile rect
        sTile.setPosition(player1.x * ts, player1.y * ts);
        window.draw(sTile);
        sTile.setPosition(player2.x * ts, player2.y * ts);
        window.draw(sTile);

        // Draw enemies
        sEnemy.rotate(10);
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y);
            window.draw(sEnemy);
        }

        window.display();
    }
   
}

void runMultiPlayer(Font& font, Sprite& sTile, Sprite& sGameover, Sprite& sEnemy, Font& newfont, PlayerLogin& login, PlayerLogin& login2, Leaderboard& board,bool saved)
{
    RenderWindow window(VideoMode(N * ts, M * ts), "MultiPlayer"); //40*18, 25*18
    window.setFramerateLimit(60);

    int score1 = 0;
    int score2 = 0;
    int enemyCount = 4;
    Enemy a[10];

    GameState gameState;
    time_t now = time(0);              // get current time
    char dt[26];                       // buffer to hold the date
    ctime_s(dt, sizeof(dt), &now);     // secure version of ctime

    gameState.timestamp = dt;          // store in your variable

    gameState.player1ID = login.getusername(); // Player's username
    gameState.tileHead = nullptr; // Initialize linked list of tiles
    gameState.player2ID = login2.getusername(); // Player's username

    Player player1(login.getusername(), 0, 0, 2, Keyboard::W, Keyboard::S, Keyboard::A, Keyboard::D);
    Player player2(login2.getusername(), N - 1, M - 1, 3, Keyboard::Up, Keyboard::Down, Keyboard::Left, Keyboard::Right);

    float timer = 0, delay = 0.07;
    Clock clock;
    bool player1_alive = true;
    bool player2_alive = true;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            grid[i][j] = 0;  // clear all first

    // THEN reapply borders
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed || e.key.code == Keyboard::Escape)
                window.close();
        }

        // Ignore input if dead
        if (player1_alive) player1.handleInput();
        if (player2_alive) player2.handleInput();

        if (timer > delay) {
            if (player1_alive) player1.move(N, M);
            if (player2_alive) player2.move(N, M);

            // === Check player-vs-player trail collision BEFORE placing trails ===
            if (player1_alive && grid[player1.y][player1.x] == 5) {
                player1_alive = false;
                player1.x = player1.y = 0;
                clearPlayerAreaAndTrail(2, 4);
            }
            if (player2_alive && grid[player2.y][player2.x] == 4) {
                player2_alive = false;
                player2.x = N - 1;
                player2.y = M - 1;
                clearPlayerAreaAndTrail(3, 5);
            }

            // === Check player1 & player2 BOTH collide in same tile ===
            if (player1_alive && player2_alive &&
                player1.x == player2.x && player1.y == player2.y) {
                player1_alive = false;
                player2_alive = false;
            }

            // === Place trails only if alive ===
            if (player1_alive && grid[player1.y][player1.x] == 0)
                grid[player1.y][player1.x] = player1.trailValue;
            if (player2_alive && grid[player2.y][player2.x] == 0)
                grid[player2.y][player2.x] = player2.trailValue;

            timer = 0;
        }

        // === Game Over if both dead ===
        if (!player1_alive && !player2_alive) {
            window.clear();
            window.draw(sGameover);
            window.display();
            player1.matchHistory += "Score: " + to_string(score1) + " | ";
            player1.score += score1;
            player1.saveProfile();
            player2.matchHistory += "Score: " + to_string(score2) + " | ";
            player2.score += score2;
            player2.saveProfile();
            board.update(&player1);
            board.update(&player2);
            board.savePlayerToFile(&player1);
            board.savePlayerToFile(&player2);

            return;
        }

        // === Move enemies ===
        for (int i = 0; i < enemyCount; i++) a[i].move();

        // === PLAYER1 CAPTURE ===
        if (player1_alive && (grid[player1.y][player1.x] == 1 || grid[player1.y][player1.x] == 4) && hasTrail(player1.trailValue))
        {
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            int tiles_captured = 0;
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) grid[i][j] = 0;  // outside
                    else if (grid[i][j] == 0 || grid[i][j] == player1.trailValue) {
                        tiles_captured++;

                        grid[i][j] = 4;  // Player1 area
                    }
                }
            }
            score1 += (tiles_captured > 10) ? tiles_captured * 2 : tiles_captured;
        }

        // === PLAYER2 CAPTURE ===
        if (player2_alive && (grid[player2.y][player2.x] == 1 || grid[player2.y][player2.x] == 5) && hasTrail(player2.trailValue))
        {
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            int tiles_captured = 0;
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) grid[i][j] = 0;  // outside
                    else if (grid[i][j] == 0 || grid[i][j] == player2.trailValue) {
                        tiles_captured++;

                        grid[i][j] = 5;  // Player2 area
                    }
                }
            }
            score2 += (tiles_captured > 10) ? tiles_captured * 2 : tiles_captured;
        }

        // === Enemy hits player trail = game over ===
        for (int i = 0; i < enemyCount; i++) {
            int ex = a[i].x / ts;
            int ey = a[i].y / ts;

            if (grid[ey][ex] == player1.trailValue || grid[ey][ex] == player2.trailValue) {
                window.draw(sGameover);
                window.display();
                player1.matchHistory += "Score: " + to_string(score1) + " | ";
                player1.score += score1;
                player1.saveProfile();

                player2.matchHistory += "Score: " + to_string(score2) + " | ";
                player2.score += score2;
                player2.saveProfile();
                board.update(&player1);
                board.update(&player2);
                board.savePlayerToFile(&player1);
                board.savePlayerToFile(&player2);
                return;
            }
        }

        // === RENDER ===
        window.clear();

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) continue;
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts)); // blue border

                // Player 1 (trail=2 OR area=4) -> same green tile
                if (grid[i][j] == 2 || grid[i][j] == 4)
                    sTile.setTextureRect(IntRect(54, 0, ts, ts));  // Green tile

                // Player 2 (trail=3 OR area=5) -> same orange tile
                if (grid[i][j] == 5 || grid[i][j] == 3)
                    sTile.setTextureRect(IntRect(72, 0, ts, ts));  // Orange tile

                sTile.setPosition(j * ts, i * ts);
                window.draw(sTile);
            }
        }

        // Score text
        Text scoreText;
        scoreText.setFont(newfont);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(15, 12);
        scoreText.setString("Score: " + to_string(score1));
        window.draw(scoreText);

        scoreText.setPosition(570, 12);
        scoreText.setString("Score: " + to_string(score2));
        window.draw(scoreText);

        // Draw players (only if alive)
        sTile.setTextureRect(IntRect(36, 0, ts, ts)); // player tile rect
        sTile.setPosition(player1.x * ts, player1.y * ts);
        window.draw(sTile);
        sTile.setPosition(player2.x * ts, player2.y * ts);
        window.draw(sTile);

        // Draw enemies
        sEnemy.rotate(10);
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y);
            window.draw(sEnemy);
        }

        window.display();
    }

}

void cleanUsername(string& uname) {
    uname.erase(remove_if(uname.begin(), uname.end(), [](char c) {
        return c == '\r' || c == '\n' || isspace(c);
        }), uname.end());
}

void manageFriendsSFML(Player* currentPlayer, Player* players[], HashTable& table, Font& font,Inventory& inventory,Music& music) {
    RenderWindow friendWindow(VideoMode(800, 600), "Friend Menu");

    Texture background;
    themeApply(background,music,inventory,true); //true for music starting
    Sprite bg(background);


    const int MENU_SIZE = 5;
    const char* menuItems[MENU_SIZE] = {
        "View Friends",
        "View Pending Requests",
        "Send Friend Request",
        "Accept Friend Request",
        "Back to Main Menu"
    };

    const int NONE = 0;
    const int SEND_REQUEST = 1;
    const int ACCEPT_REQUEST = 2;
 
    int selected = 0;
    string inputBuffer = "";
    bool enteringInput = false;
    int inputMode = NONE;

    while (friendWindow.isOpen()) {
        Event event;
        while (friendWindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                friendWindow.close();
            }

            // Handle typing input
            if (enteringInput && event.type == Event::TextEntered) {
                if (event.text.unicode == '\b') {
                    if (!inputBuffer.empty()) inputBuffer.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\n') {
                    inputBuffer += static_cast<char>(event.text.unicode);
                }
            }

            // Handle enter key during input
            if (event.type == Event::KeyPressed && enteringInput && event.key.code == Keyboard::Enter) {
                if (!inputBuffer.empty()) {
                    if (inputMode == SEND_REQUEST) {
                       
                        currentPlayer->sendFriendRequest(inputBuffer, players, table);
                        cout << "Friend request function completed!" << endl;
                    }
                    else if (inputMode == ACCEPT_REQUEST) {
                        currentPlayer->acceptFriendRequest(inputBuffer, players, table);
                    }
                }
                enteringInput = false;
                inputBuffer = "";
                inputMode = NONE;
                continue;
            }

            // Menu navigation
            if (event.type == Event::KeyPressed && !enteringInput) {
                if (event.key.code == Keyboard::Up) selected = (selected - 1 + MENU_SIZE) % MENU_SIZE;
                if (event.key.code == Keyboard::Down) selected = (selected + 1) % MENU_SIZE;
                if (event.key.code == Keyboard::Enter) {
                    if (selected == 2) {
                        inputBuffer = "";
                        inputMode = SEND_REQUEST;
                        enteringInput = true;
                    }
                    else if (selected == 3) {
                        inputBuffer = "";
                        inputMode = ACCEPT_REQUEST;
                        enteringInput = true;
                    }
                    else if (selected == 4) {
                        friendWindow.close();
                    }
                }
            }
        }

        // === DRAWING ===
        friendWindow.clear();
        friendWindow.draw(bg);
        Text heading(currentPlayer->getUsername() + "'s friend Profile: ", font, 30);
        heading.setPosition(94, 85);
        heading.setFillColor(Color::Red);
        friendWindow.draw(heading);
        // Menu options
        for (int i = 0; i < MENU_SIZE; i++) {
            Text option(menuItems[i], font, 24);
            option.setPosition(110, 150 + i * 60);
            option.setFillColor(i == selected ? Color::Red : Color::White);
            friendWindow.draw(option);
        }

        // Info display
        Text infoText("", font, 20);
        infoText.setPosition(110, 450);

        if (selected == 0) {
            string info = currentPlayer->getUsername() + "'s friends: ";
            FriendNode* f = currentPlayer->friendsHead;
            while (f) {
                info += f->username + " ";
                f = f->next;
            }
            infoText.setString(info);
        }
        else if (selected == 1) {
            string info = "Pending Requests: ";
            FriendNode* f = currentPlayer->pendingRequestsHead;
            while (f) {
                info += f->username + " ";
                f = f->next;
            }
            infoText.setString(info);
        }

        friendWindow.draw(infoText);

        // Draw input box if needed
        if (enteringInput) {
            // Prompt
            Text promptText((inputMode == SEND_REQUEST ? "Send request to: " : "Accept request from: "), font, 20);
            promptText.setPosition(110, 480);
            promptText.setFillColor(Color::Yellow);
            friendWindow.draw(promptText);

            // Input box
            RectangleShape inputBox;
            inputBox.setSize(Vector2f(400, 40));
            inputBox.setPosition(110, 510);
            inputBox.setFillColor(Color(50, 50, 50));
            inputBox.setOutlineColor(Color::White);
            inputBox.setOutlineThickness(2);
            friendWindow.draw(inputBox);

            // Input text
            Text inputText(inputBuffer + "_", font, 20);
            inputText.setPosition(115, 515);
            inputText.setFillColor(Color::White);
            friendWindow.draw(inputText);
        }

        friendWindow.display();
    }
}

bool randomTheme(Texture& background,Music& music)
{
    int random = rand() % 8;  //0 to 7
    random = 5;
    cout << "random variable is " << random << endl;
    switch (random) {
    case 0:
    {
        music.openFromFile("themes/macgyver.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/macgyver.png"))
            return true;
    }
    case 1:
    {
        music.openFromFile("themes/panda.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/panda.png"))
            return true;
    }
    case 2:
    {
        music.openFromFile("themes/panther.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/panther.jpg"))
            return true;
    }
    case 3:
    {
        music.openFromFile("themes/bean.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/bean.png"))
            return true;
    }
    case 4:
    {
        music.openFromFile("themes/war.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/war.png"))
            return true;
    }
    case 5:
    {
        music.openFromFile("themes/ghibli.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/ghibli.jpg"))
            return true;
    }
    case 6:
    {
        music.openFromFile("themes/background.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/background.png"))
            return true;
    }
    case 7:
     {
        music.openFromFile("themes/alien.ogg");
        music.setLoop(true);
        music.play();
        if (background.loadFromFile("themes/alien.png"))
            return true;
     }
            return false; 
    }
}

int initializer(Texture& t1, Texture& t2, Texture& t3,Texture& backgroundTexture,Music& music,Font& font, Font& altFont)
{
    if (!t1.loadFromFile("images/tiles.png") ||
        !t2.loadFromFile("images/gameover.png") ||
        !t3.loadFromFile("images/enemy.png") || !randomTheme(backgroundTexture,music)) {
        cout << "Failed to load one or more images!" << endl;
        return -1;
    }

    if (!font.loadFromFile("Audiowide-Regular.ttf")) {
        cout << "Failed to load font!" << endl;
        return -1;
    }

    if (!altFont.loadFromFile("BlazeCircuitRegular-xRvxj.ttf")) {
        cout << "Failed to load BlazeCircuitRegular!" << endl;
        return -1;
    }

}

void initialInventory(Inventory& invent)
{
    Theme T0(00, "Mac'Gyver","Outsmart every trap with gadgets and grit", "themes/macgyver.png", "themes/macgyver.ogg", 0);
    Theme T1(01, "Kung Fu Panda","Wanna be the Next Dragon Warrior ?", "themes/panda.png", "themes/panda.ogg", 1);
    Theme T2(02, "Pink Panther","Pookie Panther ?", "themes/panther.jpg", "themes/panther.ogg", 2);
    Theme T3(03, "Mr Bean","Xonix grid or parking lot? Bean doesn’t know either","themes/bean.png", "themes/bean.ogg",3);
    Theme T4(04, "Dil Dil Pakistan","PAF vs IAF, 6 - Nill ;)","themes/war.png", "themes/war.ogg",4);
    Theme T5(05, "Ghibli Art", "An aesthetic wonderland straight out of an Insta Trend", "themes/ghibli.jpg", "themes/ghibli.ogg", 5);
    Theme T6(06, "Xonix!","Step into the street for Classic arcade energy", "themes/background.png", "themes/background.ogg", 6);
    Theme T7(07, "Space Invaders","Defend the grid from a cosmic alien swarm", "themes/alien.png", "themes/alien.ogg", 7);

    invent.insertTheme(T0);
    invent.insertTheme(T1);
    invent.insertTheme(T2);
    invent.insertTheme(T3);
    invent.insertTheme(T4);
    invent.insertTheme(T5);
    invent.insertTheme(T6);
    invent.insertTheme(T7);
}

int main()
{
    srand(time(0));
    RenderWindow window(VideoMode(800, 600), "Xonix Game!"); //40*18, 25*18
    window.setFramerateLimit(60);

    // Load all textures and fonts
    Texture t1, t2, t3, backgroundTexture;
    Music music,musicTheme;

    Font font;
    Font altFont;
   
    if (initializer(t1,t2,t3,backgroundTexture,music,font,altFont) == -1) return -1;

    Sprite sTile(t1), sGameover(t2), sEnemy(t3), background(backgroundTexture);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    Inventory inventory;
    initialInventory(inventory);

    // Game components
    Menu menu;
    PlayerLogin login;
    PlayerLogin login2;
    Leaderboard board;
    Player* players[100];

    int playerCount = 0;
    HashTable playerTable;
    Queue match;
    ifstream file("Hash_users.txt");
    string uname;

    while (getline(file, uname)) {
        cleanUsername(uname);
        if (uname.empty()) continue;
        players[playerCount] = new Player(uname);         // Create Player object
        playerTable.insert(uname, playerCount);           // Map username to index
        playerCount++;
    }
    file.close();
    
    while (window.isOpen()) {
        // ===== MENU LOOP =====
        bool inMenu = true;
        bool inBoard = false;

        while (inMenu && window.isOpen()) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == Event::Closed  ) window.close();

                if (e.type == Event::KeyPressed) {
                    if (e.key.code == Keyboard::Up) menu.navigate(-1);
                    if (e.key.code == Keyboard::Down) menu.navigate(1);
                    if (e.key.code == Keyboard::Enter) {
                        int selected = menu.getoption();

                        if (selected == 0) {
                            // === MAIN PLAYER LOGIN ===
                            login.logining(altFont, true,inventory,musicTheme,background);
                            // adding theme 

                            string uname = login.getusername();
                            cleanUsername(uname);
                            int existingIdx = playerTable.get(uname);

                            if (!playerTable.exists(uname))
                            {
                                if (playerCount >= 100) {
                                    cout << "Too many players registered. Cannot add more.\n";
                                    continue;
                                }
                                players[playerCount] = new Player(uname);
                                playerTable.insert(uname, playerCount);

                                ofstream outFile("Hash_users.txt", ios::app);
                                outFile << uname << endl;
                                outFile.close();

                                playerCount++;
                            }

                            // Now get the current player by index
                            Player* currentPlayer = players[playerTable.get(uname)];
                            inMenu = false;
                            music.stop();
                            manageFriendsSFML(currentPlayer, players, playerTable, font,inventory,musicTheme);

                        }
                        else if (selected == 2) {   // Leaderboard
                            inBoard = true;
                            inMenu = false;
                        }
                        else if (selected == 3) {     //Match MaKInggg 
                            match.LoadFromFile();
                           string p1Name = match.dequeue();
                         string p2Name = match.dequeue();

                           login.setUsername(p1Name);
                           login2.setUsername(p2Name);
                          runMultiPlayer(font, sTile, sGameover, sEnemy, altFont, login, login2, board,false);
                        }
                        else if (selected == 4) {   // Exit
                            window.close();
                        }
                    }
                }

            }

            window.clear();
            window.draw(background);

            Text welcomeText("Welcome to Xonix Game!", font, 30);
            welcomeText.setPosition(110, 80);
            welcomeText.setFillColor(Color::Black);
            window.draw(welcomeText);

            for (int i = 0; i < 5; i++) {
                Text text;
                text.setFont(font);
                text.setString(menu.getOptionText(i)); 
                text.setCharacterSize(30);
                text.setPosition(115, 150 + i * 50);
                text.setFillColor((i == menu.getoption()) ? Color::Red : Color::White);
                window.draw(text);
            }

            window.display();
        }

         if (!window.isOpen()) return 0;

        // ===== SHOW LEADERBOARD IF SELECTED =====
        if (inBoard) {
            board.loadAllPlayersFromFile();
            board.show(altFont);
            inBoard = false;  // Return to the main menu after viewing the leaderboard
            continue;         // Continue the main menu loop
        }

        // ===== GAME MODE =====
        int mode = selectGameMode(inventory,musicTheme);
        if (mode == 1) {
            runSinglePlayer( font, sTile, sGameover, sEnemy, altFont, login, board, login.askToLoadGame(font, altFont,inventory,musicTheme));
        }
        else {
          //  login.logining(altFont, true);
            login2.logining(altFont, false,inventory,musicTheme,background);
            runMultiPlayer( font, sTile, sGameover, sEnemy, altFont, login, login2, board);
        }

    }
    return 0;
}