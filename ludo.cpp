# include <iostream>
using namespace std;

void printGrid();

int main()  {
    printGrid();

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