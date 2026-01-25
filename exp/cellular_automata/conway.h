#include <vector>
#include <cstdint>
#include <iostream>
#include <random>

class Conway {
public:
    Conway(int width, int height) : width(width), height(height) {
        if (width % 64 != 0) {
            throw std::invalid_argument("Width must be a multiple of 64");
        }
        num_clusters = (width * height) / 64;
        this->board.resize(num_clusters);
    }

    // x and y are 0 indexed coordinates
    void setCell(int x, int y, bool alive);

    bool getCell(int x, int y) const;

    void clear();

    void randomize();

    void setBoard(const std::vector<std::uint64_t>& newBoard);

    void calculate_next_board();

    void print_binary() const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::vector<std::uint64_t> getBoard() const { return board; }

private:
    int width;
    int height;
    int num_clusters = 0;
    std::vector<std::uint64_t> board;
};