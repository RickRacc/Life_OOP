#include "Life.hpp"
#include <iostream>
#include <string>


using namespace std;

int main() {
    int t;
    cin >> t;
    string line;
    getline(cin, line); // consume newline

    for (int test = 0; test < t; ++test) {
        if (test > 0) {
            getline(cin, line);
            cout << '\n'; 
        } 
        size_t r, c;
        cin >> r >> c;
        Life<Cell> life(r, c);

        int n;
        cin >> n;
        for (int i = 0; i < n; ++i) {
            size_t x, y;
            cin >> x >> y;
            life.initialize_cell(x, y, Cell(new FredkinCell(true)));
        }

        int s, f;
        cin >> s >> f;
        getline(cin, line); // consume newline after s and f

        cout << "*** Life<Cell> " << r << "x" << c << " ***\n\n";

        for (int gen = 0; gen <= s; ++gen) {
            if (gen % f == 0) {
                life.print_generation(gen);
                if (gen + f <= s) cout << '\n';
                
            }
            life.eval();
        }
    }

    return 0;
}
