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
void takeTurn(Player& player);

int main()  {
    srand(time(0));
    int move;

    Player* players = new Player[4];
    initializePlayers(players);

    int round = 1;
    bool gameOver = false;
    while (!gameOver)    {
        for (int i = 0; i < 4; i++) {
            takeTurn(players[i]);
        }
        round++;

        if (round > 5) gameOver = true;
    }

    // for (int i = 0; i < 4; i++) {
    //     cout << players[i].name << ", " << players[i].color << endl;
    // }
    
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

void takeTurn(Player& player)   {

    int roll = rollDice();
    cout << player.name << " rolled: " << roll << endl;

    //if all at home
    bool allAtHome = true;
    for (int i = 0; i < 4; i++) {
        if (player.pieces[i].position != -1)    {
            allAtHome = false;
            break;
        }
    }

    if (allAtHome && roll != 6)    {
        cout << player.name << " cannot move. Turn Skipped" << endl;
        return;
    }

    cout << player.name << " can move!" << endl;

    cout << player.name << "'s Pieces: " << endl;
        for (int j = 0; j < 4; j++) {
    cout << "Piece " << j + 1 << ": ";
    if (player.pieces[j].position == -1) cout << "Home" << endl;
    else cout << "Position " << player.pieces[j].position << endl;
        }
        int choice;
        cout << "Pick a piece to move (1-4): ";
        cin >> choice;

    choice--; // convert from 1-4 to 0-3 index

    if (player.pieces[choice].position == -1 && roll == 6) {
        player.pieces[choice].position = 0;
        cout << "Piece " << choice+1 << " moved out of home!" << endl;
    } else if (player.pieces[choice].position != -1) {
        player.pieces[choice].position += roll;
        cout << "Piece " << choice+1 << " moved to position " << player.pieces[choice].position << endl;
    } else {
        cout << "Invalid move! Piece is at home and you need a 6 to move out." << endl;
    }
}