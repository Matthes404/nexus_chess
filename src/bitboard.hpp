// ===== BITBOARD UTILITIES =====
class BitboardUtils {
public:
    static void init();
    static Bitboard square_bb(Square s) { return 1ULL << s; }
    static bool more_than_one(Bitboard b) { return b & (b - 1); }
    static Square lsb(Bitboard b);
    static Square pop_lsb(Bitboard& b);
    static int popcount(Bitboard b);
    
private:
    static Bitboard attack_tables[PIECE_TYPE_NB][SQUARE_NB];
    static void init_magics();
};
