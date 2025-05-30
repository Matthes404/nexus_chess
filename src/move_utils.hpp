// ===== MOVE UTILITIES =====
#include "position.hpp"

class MoveUtils {
public:
    static Move make_castling_move(Square from, Square to);
    static Move make_en_passant_move(Square from, Square to);
    static Move make_capture_move(Square from, Square to, PieceType promotion = PIECE_TYPE_NB);
    static Move make_promotion_move(Square from, Square to, PieceType promotion, bool is_capture = false);
    
    static bool is_capture(Move m);
    static bool is_quiet(Move m);
    static bool is_tactical(Move m);
    static Move null_move();
    static bool is_null(Move m);
    
    static Move from_string(const std::string& move_str);
    static std::string to_algebraic(Move m, const Position& pos);
    static std::string debug_string(Move m);
    
    static int get_move_score(Move m, const Position& pos);
    static int get_piece_value(PieceType pt);
    
    // Operators for move comparison
    static bool operator<(const Move& a, const Move& b);
    static bool operator==(const Move& a, const Move& b);
    static bool operator!=(const Move& a, const Move& b);
    static Move make_move(Square from, Square to, PieceType promotion = PIECE_TYPE_NB);
    static Square from_sq(Move m) { return m & 0x3F; }
    static Square to_sq(Move m) { return (m >> 6) & 0x3F; }
    static PieceType promotion_type(Move m) { return PieceType((m >> 12) & 0x7); }
    static bool is_promotion(Move m) { return m & (0x7 << 12); }
    static bool is_castling(Move m) { return m & (1 << 15); }
    static bool is_en_passant(Move m) { return m & (1 << 16); }
    static std::string to_string(Move m);
};
