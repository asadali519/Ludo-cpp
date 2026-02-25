# include <iostream>
# include <ctime>
# include <cstdlib>
using namespace std;



//piece struct
struct Piece {
    int position;
};

//player struct
struct Player {
    string name;
    char color;
    Piece* pieces;  // a Piece type pointer to point to my pieces
};

void printGrid();
int rollDice();
void initializePlayers(Player* players);

int main()  {
    srand(time(0));

    Player* players = new Player[4];
    initializePlayers(players);

    for (int i = 0; i < 4; i++) {
        cout << players[i].name << ", " << players[i].color << endl;
    }

    cout << "Dice Roll: " << rollDice() << endl;
    cout << "Dice Roll: " << rollDice() << endl;
    cout << "Dice Roll: " << rollDice() << endl;

    
    delete[] players;

    //printGrid();

    return 0;
}

void printGrid() {

    for (int i = 0; i < 15; i++)    {
        for (int j = 0; j < 15; j++)    {
            if ((i <= 5 && i >= 0) && (j <= 5 && j >= 0))   cout << "[R]" << " ";
            else if ((i <= 5 && i >= 0) && (j <= 14 && j >= 9))  cout << "[G]" << " ";
            else if ((i <= 14 && i >= 9) && (j <= 5 && j >= 0))  cout << "[Y]" << " ";
            else if ((i <= 14 && i >= 9) && (j <= 14 && j >= 9))  cout << "[B]" << " ";
            else if ((i <= 8 && i >= 6) && (j <= 8 && j >= 6))  cout << "[*]" << " ";
            else cout << "[ ]" << " ";
        }
        cout << endl;
    }
}

int rollDice()  {
    return rand() % 6 + 1;
}

void initializePlayers(Player* players)  {
    
    string names[] = {"Player 1", "Player 2", "Player 3", "Player 4"};
    char color[] = {'R', 'G', 'Y', 'B'};;

    for (int i = 0; i < 4; i++) {
        players[i].name = names[i];
        players[i].color = color[i];

        players[i].pieces = new Piece[4];
            for (int j = 0; j < 4; j++) {
                players[i].pieces[j].position = -1;
            }
    }
}