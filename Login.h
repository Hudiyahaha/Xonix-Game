#ifndef Login_H
#define Login_H
#include <SFML/Graphics.hpp>
#include"Inventory.h"
#include <iostream>
#include <string>
#include<fstream>
using namespace std;
using namespace sf;

int themeID = -1;
string selectedtheme = "";
bool save = false;

bool themeApply(Texture& back, Music& music, Inventory& inventory, bool start)
{
    if (themeID == -1 && selectedtheme.empty()) {
        cout << "No theme selected!" << endl;
        return false;
    }

    treeNode* themeNode = nullptr;

    if (themeID != -1) {
        themeNode = inventory.searchByID(inventory.getRoot(), themeID);
    }
    else if (!selectedtheme.empty()) {
        themeNode = inventory.searchByName(inventory.getRoot(), selectedtheme);
    }

    if (!themeNode) {
        cout << "Theme not found in inventory!" << endl;
        return false;
    }

    // Retrieve the theme data
    Theme selectedTheme = themeNode->data;

    if (!back.loadFromFile(selectedTheme.imageString)) {
        cout << "Failed to load background image: " << selectedTheme.imageString << endl;
        return false;
    }

    if (start) {
        if (!music.openFromFile(selectedTheme.musicString)) {
            cout << "Failed to load music: " << selectedTheme.musicString << endl;
            return false;
        }

        music.setLoop(true);
        music.play();
    }

    cout << "Theme applied successfully: " << selectedTheme.name << endl;
    return true;
}

class PlayerLogin {
    string options[2]; // Fixed: 2 options now
    int selected;
    string username;
public:
    PlayerLogin() {
        options[0] = "Yes";
        options[1] = "No";
        selected = 0;
    }

    string getoptionstxt(int i) {
        return options[i];
    }
  
    void setUsername(string& n) {
        username = n;
    }

    int getoptions() {
        return selected;
    }
  
    string getusername() {
        return username;
    }

    bool askToLoadGame(Font font, Font newfont, Inventory& inventory,Music& music) {
        RenderWindow window(VideoMode(800, 600), "Load Game");

        Texture background;
        themeApply(background, music, inventory, false);
        Sprite s(background);

        Text prompt("Do you want to Load your previous Game?", font, 30);
        prompt.setFillColor(Color::White);
        prompt.setPosition(20, 20);

        while (window.isOpen()) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == Event::Closed) {
                    window.close();
                    return false; // Default to "No" if the window is closed
                }

                if (e.type == Event::KeyPressed) {
                    if (e.key.code == Keyboard::Up || e.key.code == Keyboard::Down)
                        selected = (selected + 1) % 2;

                    if (e.key.code == Keyboard::Enter) {
                        if (selected == 0) { // Yes
                            window.close();
                            return true;
                        }
                        else if (selected == 1) { // No
                            window.close();
                            return false;
                        }
                    }
                }
            }

            // Render the prompt and options
            window.clear();

            window.draw(s);
            window.draw(prompt);

            for (int i = 0; i < 2; i++) {
                Text text;
                text.setFont(newfont);
                text.setString(options[i]);
                text.setCharacterSize(30);
                text.setPosition(100, 190 + i * 50);

                if (i == selected)
                    text.setFillColor(Color::Red);
                else
                    text.setFillColor(Color::White);

                window.draw(text);
            }

            window.display();
        }

        return false; // Default to "No" if the window is closed
    }

    void selectTheme(Inventory& inventory, const string& username, Sprite& background) {
        RenderWindow window(VideoMode(800, 600), "Theme Selection");

        Font font;
        if (!font.loadFromFile("Arial.ttf")) {
            cout << "Failed to load font!" << endl;
            return;
        }

        Text title("Select a Theme", font, 30);
        title.setFillColor(Color::White);
        title.setPosition(20, 20);

        Text instruction("Press I to view themes, S to search by ID, Enter to select.", font, 20);
        instruction.setFillColor(Color::White);
        instruction.setPosition(20, 70);

        Text output("", font, 23);
        output.setFillColor(Color::White);
        output.setPosition(20, 120);

        Text inputPrompt("", font, 20);
        inputPrompt.setFillColor(Color::Yellow);
        inputPrompt.setPosition(20, 500);

        Text userInputText("", font, 20);
        userInputText.setFillColor(Color::Green);
        userInputText.setPosition(20, 530);

        string userInput = "";
        bool enteringInput = false;
        string inputMode = ""; // "search" or "select"
        string selectedTheme = "";
        bool themeSelected = false;

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }

                if (event.type == Event::KeyPressed) {
                    if (!enteringInput) {
                        if (event.key.code == Keyboard::I) {
                            // Display themes (in-order traversal as one big string)
                            stringstream ss;
                            inventory.displayInOrderToStream(ss);  // You need to implement this
                            output.setString(ss.str());
                        }
                        else if (event.key.code == Keyboard::S) {
                            // Start text input mode for search
                            inputMode = "search";
                            enteringInput = true;
                            userInput = "";
                            inputPrompt.setString("Enter Theme ID to search:");
                        }
                        else if (event.key.code == Keyboard::Enter) {
                            // Start text input mode for select
                            inputMode = "select";
                            enteringInput = true;
                            userInput = "";
                            inputPrompt.setString("Enter Theme ID to select:");
                        }
                    }
                    else {
                        if (event.key.code == Keyboard::BackSpace && !userInput.empty()) {
                            userInput.pop_back();
                        }
                        else if (event.key.code == Keyboard::Return) {
                            themeID = stoi(userInput);
                            if (inputMode == "search") {
                                    stringstream ss;
                                    inventory.findByIDToStream(themeID, ss);
                                    output.setString(ss.str());

                                    treeNode* themeNode = inventory.searchByID(inventory.getRoot(), themeID);
                                    if (themeNode) {
                                        selectedTheme = themeNode->data.name;
                                        themeSelected = true;

                                        ofstream outFile("profiles/" + username + ".txt", ios::app);
                                        if (outFile.is_open()) {
                                            outFile << "Selected Theme: " << selectedTheme << endl;
                                            outFile.close();
                                            output.setString("Theme selected: " + selectedTheme + "\nSaved to profile!");
                                        }
                                        else {
                                            output.setString("Failed to save theme to profile!");
                                        }
                                        window.display();
                                        window.close();
                                    }
                                

                            }
                            else if (event.key.code == Keyboard::Escape && enteringInput) {
                                enteringInput = false;
                                inputPrompt.setString("");
                                userInputText.setString("");
                                userInput = "";
                                output.setString("");

                            }

                            else if (inputMode == "select" ) {
                                treeNode* themeNode = inventory.searchByID(inventory.getRoot(), themeID);
                                if (themeNode) {
                                    selectedTheme = themeNode->data.name;
                                    themeSelected = true;

                                    ofstream outFile("profiles/" + username + ".txt", ios::app);
                                    if (outFile.is_open()) {
                                        outFile << "Selected Theme: " << selectedTheme << endl;
                                        outFile.close();
                                        output.setString("Theme selected: " + selectedTheme + "\nSaved to profile!");
                                    }
                                    else {
                                        output.setString("Failed to save theme to profile!");
                                    }
                                }
                                else {
                                    output.setString("Theme with ID " + to_string(themeID) + " not found!");
                                }

                                enteringInput = false;
                                inputPrompt.setString("");
                                userInputText.setString("");
                            }
                        }
                    }
                }

                // Handle text input
                if (enteringInput && event.type == Event::TextEntered) {
                    if (event.text.unicode >= 48 && event.text.unicode <= 57) { // Numbers only
                        userInput += static_cast<char>(event.text.unicode);
                    }
                    userInputText.setString(userInput);
                }
            }

            window.clear();
            window.draw(background);
            window.draw(title);
            window.draw(instruction);
            window.draw(output);
            window.draw(inputPrompt);
            window.draw(userInputText);
            window.display();

            if (themeSelected) {
                sleep(seconds(1));
                window.close();
            }
        }
    }
  
    bool logining(Font &newfont,bool count,Inventory& inventory,Music& music,Sprite& Background) {

        RenderWindow window(VideoMode(800, 600), "Login / Register");
        Font font;
        if (!font.loadFromFile("Arial.ttf")) {
            cout << "Failed to load Arial.ttf!" << endl;
            return 1;  // Default to single player if font fails
        }

        Texture background;
        Sprite backgroundSprite;

        if (!background.loadFromFile("themes/background.png")) {
            cout << "Failed to load background.jpg!" << endl;
            return false;
        }
        backgroundSprite.setTexture(background);

        bool loggedIn = false;
        string  password;
        bool entering_username = false;
        bool entering_password = false;
        bool input_complete = false;
        bool loginMode = false;
        username.clear();
        password.clear();
        Text inputText;
        inputText.setFont(newfont);
        inputText.setCharacterSize(24);
        inputText.setFillColor(Color::White);
        inputText.setPosition(50, 150);

        while (window.isOpen()) {
            Event e;
            while (window.pollEvent(e)) {

                if (e.type == Event::Closed)
                    window.close();

                if (!input_complete) {

                    if (e.type == Event::KeyPressed) {
                        if (!loggedIn && !entering_username && !entering_password) {
                            if (e.key.code == Keyboard::Up || e.key.code == Keyboard::Down)
                                selected = (selected + 1) % 2;

                            if (e.key.code == Keyboard::Enter) {
                                if (selected == 0) {
                                    loginMode = true;
                                    entering_username = true;
                                    loggedIn = true;
                                }

                                else if (selected == 1) {
                                    loggedIn = true;
                                    loginMode = false;
                                    entering_username = true;

                                }
                            }
                        }
                        else if ((entering_username || entering_password) && !input_complete) {
                            if (e.key.code == Keyboard::Enter) {
                                if (entering_username) {
                                    entering_username = false;
                                    entering_password = true;
                                }
                                else if (entering_password) {
                                    entering_password = false;
                                    input_complete = true;
                                }
                            }
                        }
                    }


                    if (e.type == Event::TextEntered && (entering_username || entering_password) && !input_complete) {
                        if (e.text.unicode == 8) { // backspace
                            if (entering_username && !username.empty())
                                username.pop_back();
                            else if (entering_password && !password.empty())
                                password.pop_back();
                        }
                        else if (e.text.unicode < 128) {
                            char typed = static_cast<char>(e.text.unicode);
                            if (entering_username)
                                username += typed;
                            else if (entering_password)
                                password += typed;
                        }
                    }



                    window.clear();
                    window.draw(backgroundSprite);

                    if (!loggedIn && !entering_username && !entering_password && !input_complete) {
                        Text text;
                        text.setFont(newfont);
                        text.setString("Welcome to Xonix Game!!");
                        text.setCharacterSize(30); // in pixels
                        text.setFillColor(Color::Magenta);
                        text.setStyle(Text::Bold);
                        text.setPosition(60, 40);
                        window.draw(text);

                        Text welcomeText("Do you already have an account?", newfont, 24);
                        welcomeText.setPosition(50, 130);
                        welcomeText.setFillColor(Color::Magenta);
                        window.draw(welcomeText);

                        for (int i = 0; i < 2; i++) {
                            Text text;
                            text.setFont(newfont);
                            text.setString(options[i]);
                            text.setCharacterSize(30);
                            text.setPosition(100, 190 + i * 70);

                            if (i == selected)
                                text.setFillColor(Color::Magenta);
                            else
                                text.setFillColor(Color::White);

                            window.draw(text);
                        }
                    }
                    else if (loggedIn && !input_complete) {
                        Text prompt;
                        prompt.setFont(newfont);
                        prompt.setCharacterSize(24);
                        prompt.setFillColor(Color::White);
                        prompt.setPosition(50, 100);

                        if (entering_username) {
                            prompt.setString("Enter Username:");
                            inputText.setString(username);
                        }
                        else if (entering_password) {
                            prompt.setString("Enter Password:");
                            inputText.setString(string(password.length(), '*')); // Mask password
                        }

                        window.draw(prompt);
                        window.draw(inputText);
                    }
                    else if (input_complete) {
                        if (loginMode) {
                            bool valid = false;
                            ifstream fin("user_data.txt");
                            string line;
                            while (getline(fin, line)) {
                                if (line == "Username: " + username) {
                                    getline(fin, line); // Read password line
                                    if (line == "Password: " + password) {
                                        valid = true;
                                    }
                                    username.erase(0, username.find_first_not_of(" \t\r\n"));
                                    username.erase(username.find_last_not_of(" \t\r\n") + 1);
                                    break;
                                }
                            }
                            fin.close();


                            if (valid) {
                                if (!count) {
                                    if (askToLoadGame(font, newfont, inventory, music)) {
                                        cout << "User chose to load previous game." << endl;
                                        save = true;
                                    }
                                    else {
                                        cout << "User chose NOT to load previous game." << endl;
                                        save = false;
                                    }
                                }

                                Text success("Login successful!", newfont, 25);
                                success.setPosition(50, 70);
                                success.setFillColor(Color::Green);
                                success.setStyle(Text::Bold);
                                window.draw(success);
                                window.close();

                                string profilePath = "profiles/" + username + ".txt";

                                ifstream inFile(profilePath);
                                if (inFile.is_open()) {
                                    string line;
                                    while (getline(inFile, line)) {
                                        if (line.find("Selected Theme:") != string::npos) {
                                            selectedtheme = line.substr(line.find(":") + 2); // Extract theme name
                                        }
                                    }
                                    inFile.close();
                                }

                                // Apply the saved theme if it exists
                                if (!selectedtheme.empty()) {
                                    themeApply(background, music, inventory, count);
                                }

                                
                                if ( count && selectedtheme.empty() ) {
                                    selectTheme(inventory, username,Background);
                                }
                            }

                            else {
                                Text fail("Invalid username or password.", newfont, 25);
                                fail.setPosition(50, 100);
                                fail.setFillColor(Color::Red);
                                fail.setStyle(Text::Bold);
                                window.draw(fail);
                                window.display();
                                sleep(seconds(1.5));

                                // Reset to retry
                                username.clear();
                                password.clear();
                                input_complete = false;
                                entering_username = true;
                            }
                        }
                        else {
                            // Register new user
                            bool username_taken = false;
                            ifstream input("user_data.txt");
                            string line;
                            while (getline(input, line)) {
                                if (line == "Username: " + username) {
                                    username_taken = true;
                                    break;
                                }
                                username.erase(0, username.find_first_not_of(" \t\r\n"));
                                username.erase(username.find_last_not_of(" \t\r\n") + 1);
                            }
                            input.close();
                            if (username_taken) {
                                Text taken("Username is already taken Please try another one", newfont, 25);
                                taken.setPosition(50, 100);
                                taken.setFillColor(Color::White);
                                taken.setStyle(Text::Bold);
                                window.draw(taken);
                                window.display();
                                sleep(seconds(2));
                                username.clear();
                                password.clear();
                                input_complete = false;
                                entering_username = true;
                            }
                            else
                            {
                                ofstream fout("user_data.txt", ios::app);
                                if (!fout) {
                                    cout << "Error writing user data!" << endl;
                                }
                                else {
                                    fout << "Username: " << endl;
                                    fout << username << endl;
                                    fout << "Password: " << password << endl;
                                    fout << "--------------------------" << endl;
                                    fout.close();
                                }
                                selectTheme(inventory, username, Background);

                                window.close();
                            }

                        }
                        Text result("Login Info Received!", newfont, 24);
                        result.setPosition(50, 100);
                        result.setFillColor(Color::Green);
                        window.draw(result);
                        loggedIn = true;
                        Text u("Username: " + username, newfont, 20);
                        u.setPosition(50, 150);
                        u.setFillColor(Color::White);
                        window.draw(u);

                        Text p("Password: " + string(password.length(), '*'), newfont, 20);
                        p.setPosition(50, 200);
                        p.setFillColor(Color::White);
                        window.draw(p);
                    }

                    window.display();

                }
            }

        }

        return loggedIn;
    }
};


#endif