// ===== BITBOARD UTILITIES =====
class BitboardUtils {
private:
    static Bitboard rook_attacks[SQUARE_NB][4096];
    static Bitboard bishop_attacks[SQUARE_NB][512];
    static Bitboard knight_attacks[SQUARE_NB];
    static Bitboard king_attacks[SQUARE_NB];
    static Bitboard pawn_attacks[COLOR_NB][SQUARE_NB];
    
    static const Bitboard rook_magics[SQUARE_NB];
    static const Bitboard bishop_magics[SQUARE_NB];
    
    static void init_knight_attacks();
    static void init_king_attacks();
    static void init_pawn_attacks();
    static Bitboard sliding_attacks(Square sq, Bitboard occupied, const int deltas[][2], int num_deltas);
    static Bitboard rook_mask(Square sq);
    static Bitboard bishop_mask(Square sq);
    static Bitboard index_to_bitboard(int index, Bitboard mask);

public:
    static Bitboard get_rook_attacks(Square sq, Bitboard occupied);
    static Bitboard get_bishop_attacks(Square sq, Bitboard occupied);
    static Bitboard get_queen_attacks(Square sq, Bitboard occupied);
    static Bitboard get_knight_attacks(Square sq);
    static Bitboard get_king_attacks(Square sq);
    static Bitboard get_pawn_attacks(Square sq, Color c);
};
