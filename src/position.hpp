// ===== POSITION CLASS =====
class Position {
private:
    struct UndoInfo {
        Square ep_square;
        int castling_rights;
        int halfmove_clock;
        uint64_t hash_key;
        Piece captured_piece;
    };
    
    std::vector<UndoInfo> previous_states;
    
    bool is_attacked_by(Square sq, Color attacking_color) const;
};