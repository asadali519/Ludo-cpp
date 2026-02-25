# include <iostream>
# include <ctime>
# include <cstdlib>
using namespace std;

void printGrid();
int rollDice();

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

int main()  {
    srand(time(0));

    cout << "Dice Roll: " << rollDice() << endl;
    cout << "Dice Roll: " << rollDice() << endl;
    cout << "Dice Roll: " << rollDice() << endl;

    Piece p11;
    p11.position = -1;

    cout << "Piece Position: " << p11.position << endl;

    Player p1;
    p1.name = "Asad Ali";
    p1.color = 'R';
    p1.pieces = new Piece[4];

    //initialize at home (-1)
    for (int i = 0; i < 4; i++) {
        p1.pieces[i].position = -1;
    }

    //printing
    cout << "Player 1: " << p1.name << endl;
    for (int i = 0; i < 4;i++)  {
        cout << "Piece " << i + 1 << " Position: " << p1.pieces[i].position << endl; 
    }

    delete[] p1.pieces;

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