#include "Conway.h"

void Conway::print_binary() const {
    for (const auto& cluster : board) {
        for (int i = 63; i >= 0; --i) {
            std::cout << ((cluster >> i) & 1);
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void Conway::setBoard(const std::vector<std::uint64_t>& newBoard) {
    if (newBoard.size() != board.size()) {
        throw std::invalid_argument("New board size does not match current board size");
    }
    board = newBoard;
}

void Conway::clear() {
    std::fill(board.begin(), board.end(), 0);
}

void Conway::randomize() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.5); // 50% chance

    this->clear();

    for (int i = 0; i < num_clusters; i++) {
        for (int j = 0; j < 64; j++) {
            if (dist(gen)) {
                board[i] |= (1ULL << j);
            }
        }
    }
}

bool Conway::getCell(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("Coordinates out of bounds");
    }
    int index = (y * width + x) / 64;
    int bit = x % 64;
    return (board[index] & (1ULL << bit)) != 0;
}

void Conway::setCell(int x, int y, bool alive) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("Coordinates out of bounds");
    }
    int index = (y * width + x) / 64;
    int bit = x % 64;
    if (alive) {
        board[index] |= (1ULL << bit);
    }
    else {
        board[index] &= ~(1ULL << bit);
    }
}

void Conway::calculate_next_board() {

    int clusters_per_row = width / 64;
    std::vector<std::uint64_t> new_board(num_clusters, 0);

    for (int y = 0; y < height; y++) {
        int up_y = (y - 1 + height) % height;
        int down_y = (y + 1) % height;

        int up_row_start_index = up_y * clusters_per_row;
        int current_row_start_index = y * clusters_per_row;
        int down_row_start_index = down_y * clusters_per_row;

        for (int x = 0; x < clusters_per_row; x++) {
            int left_x = (x - 1 + clusters_per_row) % clusters_per_row;
            int right_x = (x + 1) % clusters_per_row;

            uint64_t current_cluster = board[current_row_start_index + x];

            // Neighbors from the row above
            uint64_t up_left = board[up_row_start_index + left_x];
            uint64_t up = board[up_row_start_index + x];
            uint64_t up_right = board[up_row_start_index + right_x];

            // Neighbors on the same row
            uint64_t left = board[current_row_start_index + left_x];
            uint64_t right = board[current_row_start_index + right_x];

            // Neighbors from the row below
            uint64_t down_left = board[down_row_start_index + left_x];
            uint64_t down = board[down_row_start_index + x];
            uint64_t down_right = board[down_row_start_index + right_x];

            uint64_t n_N = up;
            uint64_t n_S = down;

            uint64_t n_W = (current_cluster >> 1) | (left << 63);
            uint64_t n_E = (current_cluster << 1) | (right & 1ULL);

            uint64_t n_NW = (up >> 1) | (up_left << 63);
            uint64_t n_NE = (up << 1) | (up_right & 1ULL);
            uint64_t n_SW = (down >> 1) | (down_left << 63);
            uint64_t n_SE = (down << 1) | (down_right & 1ULL);

            // Stage 1: Add neighbors in three groups using full adders.

            // Add first 3 neighbors: n_N, n_S, n_E
            uint64_t s1 = n_N ^ n_S ^ n_E;
            uint64_t c1 = (n_N & n_S) | (n_S & n_E) | (n_N & n_E);

            // Add next 3 neighbors: n_W, n_NW, n_NE
            uint64_t s2 = n_W ^ n_NW ^ n_NE;
            uint64_t c2 = (n_W & n_NW) | (n_NW & n_NE) | (n_W & n_NE);

            // Add final 2 neighbors: n_SW, n_SE
            uint64_t s3 = n_SW ^ n_SE;
            uint64_t c3 = n_SW & n_SE;

            // Stage 2: Add the results of the first stage.

            // Add the sum bits from stage 1: s1, s2, s3
            uint64_t s_sum = s1 ^ s2 ^ s3;
            uint64_t s_carry = (s1 & s2) | (s2 & s3) | (s1 & s3);

            // Add the carry bits from stage 1: c1, c2, c3
            uint64_t c_sum = c1 ^ c2 ^ c3;
            uint64_t c_carry = (c1 & c2) | (c2 & c3) | (c1 & c3);

            // Stage 3: Combine all intermediate sums and carries to get the final count bits.
            // count = 1*b0 + 2*b1 + 4*b2 + 8*b3

            uint64_t b0 = s_sum;
            uint64_t b1 = s_carry ^ c_sum;
            uint64_t b2 = c_carry ^ (s_carry & c_sum);
            uint64_t b3 = c_carry & s_carry & c_sum;

            uint64_t born = b0 & b1 & ~b2 & ~b3;

            uint64_t survives = current_cluster & (~b0 & b1 & ~b2 & ~b3);

            uint64_t next_cluster = born | survives;

            new_board[current_row_start_index + x] = next_cluster;
        }
    }
    // Update the board with the new generation
    board = new_board;
}
