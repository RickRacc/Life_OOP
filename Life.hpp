#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <cctype>
#include <algorithm>
#include <sstream>

using namespace std;

class Cell;

/**
 * @brief Abstract base class for all cell types (e.g., ConwayCell, FredkinCell).
 */
class AbstractCell {
protected:
    bool _alive;

public:
    explicit AbstractCell(bool alive = false) : _alive(alive) {}
    virtual ~AbstractCell() = default;

    /**
     * @brief Clone this cell polymorphically.
     */
    virtual AbstractCell* clone() const = 0;

    /**
     * @brief Applies cell-specific evolution rules.
     * @param live_neighbors Number of live neighbors.
     */
    virtual void evolve(int live_neighbors) = 0;

    /**
     * @brief Updates neighbor count for valid neighbor directions.
     *
     * Only increments counts if the current cell is alive and the
     * neighboring cell considers the direction valid.
     */
    template <typename GridType>
    void update_neighbors(const vector<vector<GridType>>& grid,
                          vector<vector<int>>& count_grid, int row, int col) const {
        if (!_alive) return;

        // 8 possible neighbor directions (for Conway)
        static const vector<pair<int, int>> offsets = {
            {-1,-1}, {-1,0}, {-1,1},
            { 0,-1},         { 0,1},
            { 1,-1}, { 1,0}, { 1,1}
        };

        for (const auto& [dr, dc] : offsets) {
            int ni = row + dr, nj = col + dc;

            // Let the neighbor cell decide if this direction counts
            if (grid[ni][nj].is_neighbor(-dr, -dc)) {
                count_grid[ni][nj]++;
            }
        }
    }

    /**
     * @brief Checks if a direction (dr, dc) is a valid neighbor.
     */
    virtual bool is_neighbor(int dr, int dc) const = 0;

    /**
     * @brief Returns a character representing the current cell state.
     */
    virtual char display() const = 0;

    /**
     * @brief Fredkin-specific: should this cell mutate into Conway?
     */
    virtual bool should_mutate() const { return false; }
};

/**
 * @brief Classic Conway cell using 8-neighbor rules.
 */
class ConwayCell : public AbstractCell {
public:
    explicit ConwayCell(bool alive = false) : AbstractCell(alive) {}

    ConwayCell* clone() const override {
        return new ConwayCell(_alive);
    }

    void evolve(int live_neighbors) override {
        // Conway rules:
        // alive → stays alive with 2 or 3 neighbors
        // dead  → becomes alive with exactly 3 neighbors
        if (_alive)
            _alive = (live_neighbors == 2 || live_neighbors == 3);
        else
            _alive = (live_neighbors == 3);
    }

    bool is_neighbor(int dr, int dc) const override {
        // All 8 directions are valid neighbors
        return !(dr == 0 && dc == 0);
    }

    char display() const override {
        return _alive ? '*' : '.';
    }
};

/**
 * @brief Fredkin cell with orthogonal neighbor logic and aging/mutation.
 */
class FredkinCell : public AbstractCell {
private:
    size_t _age;

public:
    FredkinCell(bool alive = false, size_t age = 0) : AbstractCell(alive), _age(age) {}

    AbstractCell* clone() const override {
        return new FredkinCell(_alive, _age);
    }

    void evolve(int live_neighbors) override {
        if (_alive) {
            // Dies if 0, 2, or 4 neighbors
            _alive = !(live_neighbors == 0 || live_neighbors == 2 || live_neighbors == 4);
            if (_alive) ++_age;  // Age increases only if it survives
        } else {
            // Comes alive with 1 or 3 neighbors
            _alive = (live_neighbors == 1 || live_neighbors == 3);
        }
    }

    bool is_neighbor(int dr, int dc) const override {
        // Fredkin cells only consider orthogonal directions (up/down/left/right)
        return abs(dr) + abs(dc) == 1;
    }

    char display() const override {
        if (_alive) return (_age < 10 ? '0' + _age : '+');  // age-based display
        return '-';
    }

    bool should_mutate() const override {
        // Mutate to Conway if age reaches exactly 2
        return _alive && _age == 2;
    }
};

/**
 * @brief Polymorphic wrapper for cells. Supports Conway or Fredkin with mutation.
 */
class Cell {
private:
    AbstractCell* _p;

public:
    Cell() : _p(new FredkinCell(false)) {}  // default to dead Fredkin
    explicit Cell(AbstractCell* cell) : _p(cell) {}

    Cell(const Cell& other) : _p(other._p ? other._p->clone() : nullptr) {}
    Cell(Cell&& other) noexcept : _p(other._p) { other._p = nullptr; }

    Cell& operator=(const Cell& other) {
        if (this != &other) {
            delete _p;
            _p = other._p ? other._p->clone() : nullptr;
        }
        return *this;
    }

    Cell& operator=(Cell&& other) noexcept {
        if (this != &other) {
            delete _p;
            _p = other._p;
            other._p = nullptr;
        }
        return *this;
    }

    ~Cell() {
        delete _p;
    }

    /**
     * @brief Evolves the underlying cell and mutates Fredkin→Conway if needed.
     */
    void evolve(int live_neighbors) {
        if (_p) _p->evolve(live_neighbors);
        if (_p && _p->should_mutate()) {
            delete _p;
            _p = new ConwayCell(true);
        }
    }

    bool is_neighbor(int dr, int dc) const {
        return _p ? _p->is_neighbor(dr, dc) : false;
    }

    template <typename GridType>
    void update_neighbors(const vector<vector<GridType>>& grid,
                          vector<vector<int>>& count_grid, int row, int col) const {
        if (_p) _p->update_neighbors(grid, count_grid, row, col);
    }

    char display() const {
        return _p ? _p->display() : 'N';
    }

    Cell clone() const {
        return Cell(_p ? _p->clone() : nullptr);
    }
};

/**
 * @brief Generic grid-based Game of Life simulation engine.
 * @tparam T The cell type (ConwayCell, FredkinCell, or Cell wrapper).
 */
template <typename T>
class Life {
private:
    vector<vector<T>> _prev_grid;
    vector<vector<T>> _next_grid;
    size_t _rows, _cols;

public:
    Life(size_t rows, size_t cols)
        : _rows(rows), _cols(cols)
    {
        // Allocate padded grids (+2 for top/bottom/left/right padding)
        _prev_grid.resize(rows + 2, vector<T>(cols + 2));
        _next_grid.resize(rows + 2, vector<T>(cols + 2));
    }

    /**
     * @brief Performs one full generation step:
     * - count neighbors
     * - evolve all cells
     */
    void eval() {
        vector<vector<int>> neighbor_counts(_rows + 2, vector<int>(_cols + 2, 0));

        // Phase 1: each live cell updates its neighbors' counts
        for (size_t i = 1; i <= _rows; ++i) {
            for (size_t j = 1; j <= _cols; ++j) {
                _prev_grid[i][j].update_neighbors(_prev_grid, neighbor_counts, i, j);
            }
        }

        // Phase 2: apply evolution rules using neighbor counts
        for (size_t i = 1; i <= _rows; ++i) {
            for (size_t j = 1; j <= _cols; ++j) {
                _next_grid[i][j] = _prev_grid[i][j];  // copy current cell
                _next_grid[i][j].evolve(neighbor_counts[i][j]);
            }
        }

        // Swap current and next generation
        swap(_prev_grid, _next_grid);
    }

    /**
     * @brief Sets a cell at position (i,j). Offset due to padding.
     */
    void initialize_cell(size_t i, size_t j, const T& cell) {
        _prev_grid[i + 1][j + 1] = cell;
    }

    /**
     * @brief Prints the current grid and total population count.
     */
    void print_generation(int gen) const {
        int pop = 0;
        ostringstream out;
        for (size_t i = 1; i <= _rows; ++i) {
            for (size_t j = 1; j <= _cols; ++j) {
                char ch = _prev_grid[i][j].display();
                out << ch;
                if (ch == '*' || isdigit(ch) || ch == '+') ++pop;
            }
            out << '\n';
        }
        cout << "Generation = " << gen << ", Population = " << pop << ".\n";
        cout << out.str();
    }
};
