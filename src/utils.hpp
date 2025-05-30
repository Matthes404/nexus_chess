// ===== MOVE UTILITIES =====
class MoveUtils {
public:
    static Move make_move(Square from, Square to, PieceType promotion = PIECE_TYPE_NB);
    static Square from_sq(Move m) { return m & 0x3F; }
    static Square to_sq(Move m) { return (m >> 6) & 0x3F; }
    static PieceType promotion_type(Move m) { return PieceType((m >> 12) & 0x7); }
    static bool is_promotion(Move m) { return m & (0x7 << 12); }
    static bool is_castling(Move m) { return m & (1 << 15); }
    static bool is_en_passant(Move m) { return m & (1 << 16); }
    static std::string to_string(Move m);
};
