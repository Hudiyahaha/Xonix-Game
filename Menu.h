#ifndef Menu_H
#define Menu_H
using namespace std;

class Menu {
private:
    string options[5];
    int selectedoption;
public:
    Menu() {

        options[0] = "Start Game";
        options[1] = "Select level";
        options[2] = "Leaderboard";
        options[3] = "Match Making";
        options[4] = "Exit";
        selectedoption = 0;
    }

    int getoption() {
        return selectedoption;
    }

    void navigate(int direction) {
        selectedoption = (selectedoption + direction + 5) % 5;
    }
    
    string getOptionText(int index) {
        return options[index]; // Return the menu option text
    }
};
#endif