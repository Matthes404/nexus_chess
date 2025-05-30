// ===== POSITION CLASS =====
#include <vector>

class Position {
public:
    Position();
    Position(const std::string& fen);
    
    // Basic queries
    Piece piece_on(Square s) const { return board[s]; }
    Color side_to_move() const { return stm; }
    Bitboard pieces(Color c) const { return by_color[c]; }
    Bitboard pieces(PieceType pt) const { return by_type[pt]; }
    Bitboard pieces(Color c, PieceType pt) const { return by_color[c] & by_type[pt]; }
    
    // Position manipulation
    void do_move(Move m);
    void undo_move(Move m);
    void set_fen(const std::string& fen);
    std::string fen() const;
    
    // Game state
    bool in_check() const;
    bool is_legal(Move m) const;
    uint64_t key() const { return hash_key; }
    
private:
    struct UndoInfo {
        Square ep_square;
        int castling_rights;
        int halfmove_clock;
        uint64_t hash_key;
        Piece captured_piece;
    };

    Piece board[SQUARE_NB];
    Bitboard by_color[COLOR_NB];
    Bitboard by_type[PIECE_TYPE_NB];
    Color stm;
    Square ep_square;
    int castling_rights;
    int halfmove_clock;
    int fullmove_number;
    uint64_t hash_key;
    std::vector<UndoInfo> previous_states;
    bool is_attacked_by(Square sq, Color attacking_color) const;
    
    void update_bitboards();
    void calculate_hash();

};

private:

    
