#include "bitboard_utils.hpp"
#include <bit>

// Static member initialization
Bitboard BitboardUtils::attack_tables[PIECE_TYPE_NB][SQUARE_NB];
Bitboard BitboardUtils::rook_attacks[SQUARE_NB][4096];
Bitboard BitboardUtils::bishop_attacks[SQUARE_NB][512];
Bitboard BitboardUtils::knight_attacks[SQUARE_NB];
Bitboard BitboardUtils::king_attacks[SQUARE_NB];
Bitboard BitboardUtils::pawn_attacks[COLOR_NB][SQUARE_NB];

// Magic bitboard constants
const Bitboard BitboardUtils::rook_magics[SQUARE_NB] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
    0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

const Bitboard BitboardUtils::bishop_magics[SQUARE_NB] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

void BitboardUtils::init() {
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_magics();
}

Square BitboardUtils::lsb(Bitboard b) {
    return Square(std::countr_zero(b));
}

Square BitboardUtils::pop_lsb(Bitboard& b) {
    Square s = lsb(b);
    b &= b - 1;
    return s;
}

int BitboardUtils::popcount(Bitboard b) {
    return std::popcount(b);
}

void BitboardUtils::init_knight_attacks() {
    for (Square s = A1; s <= H8; ++s) {
        int rank = s / 8;
        int file = s % 8;
        Bitboard attacks = 0ULL;
        
        // Knight moves: 2+1 in all directions
        int knight_moves[8][2] = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
            {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };
        
        for (int i = 0; i < 8; i++) {
            int new_rank = rank + knight_moves[i][0];
            int new_file = file + knight_moves[i][1];
            
            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
                attacks |= square_bb(new_rank * 8 + new_file);
            }
        }
        
        knight_attacks[s] = attacks;
    }
}

void BitboardUtils::init_king_attacks() {
    for (Square s = A1; s <= H8; ++s) {
        int rank = s / 8;
        int file = s % 8;
        Bitboard attacks = 0ULL;
        
        // King moves: 1 square in all directions
        for (int dr = -1; dr <= 1; dr++) {
            for (int df = -1; df <= 1; df++) {
                if (dr == 0 && df == 0) continue;
                
                int new_rank = rank + dr;
                int new_file = file + df;
                
                if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
                    attacks |= square_bb(new_rank * 8 + new_file);
                }
            }
        }
        
        king_attacks[s] = attacks;
    }
}

void BitboardUtils::init_pawn_attacks() {
    for (Square s = A1; s <= H8; ++s) {
        int rank = s / 8;
        int file = s % 8;
        
        // White pawn attacks (move up the board)
        Bitboard white_attacks = 0ULL;
        if (rank < 7) {  // Not on 8th rank
            if (file > 0) white_attacks |= square_bb((rank + 1) * 8 + (file - 1));
            if (file < 7) white_attacks |= square_bb((rank + 1) * 8 + (file + 1));
        }
        pawn_attacks[WHITE][s] = white_attacks;
        
        // Black pawn attacks (move down the board)
        Bitboard black_attacks = 0ULL;
        if (rank > 0) {  // Not on 1st rank
            if (file > 0) black_attacks |= square_bb((rank - 1) * 8 + (file - 1));
            if (file < 7) black_attacks |= square_bb((rank - 1) * 8 + (file + 1));
        }
        pawn_attacks[BLACK][s] = black_attacks;
    }
}

Bitboard BitboardUtils::sliding_attacks(Square sq, Bitboard occupied, const int deltas[][2], int num_deltas) {
    Bitboard attacks = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    
    for (int i = 0; i < num_deltas; i++) {
        int dr = deltas[i][0];
        int df = deltas[i][1];
        
        for (int step = 1; step < 8; step++) {
            int new_rank = rank + step * dr;
            int new_file = file + step * df;
            
            if (new_rank < 0 || new_rank >= 8 || new_file < 0 || new_file >= 8)
                break;
                
            Square target = new_rank * 8 + new_file;
            attacks |= square_bb(target);
            
            if (occupied & square_bb(target))
                break;
        }
    }
    
    return attacks;
}

void BitboardUtils::init_magics() {
    // Rook directions: horizontal and vertical
    int rook_deltas[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    
    // Bishop directions: diagonal
    int bishop_deltas[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    
    // Initialize rook attacks
    for (Square s = A1; s <= H8; ++s) {
        for (int i = 0; i < 4096; i++) {
            Bitboard occupied = index_to_bitboard(i, rook_mask(s));
            rook_attacks[s][i] = sliding_attacks(s, occupied, rook_deltas, 4);
        }
    }
    
    // Initialize bishop attacks
    for (Square s = A1; s <= H8; ++s) {
        for (int i = 0; i < 512; i++) {
            Bitboard occupied = index_to_bitboard(i, bishop_mask(s));
            bishop_attacks[s][i] = sliding_attacks(s, occupied, bishop_deltas, 4);
        }
    }
}

Bitboard BitboardUtils::rook_mask(Square sq) {
    Bitboard mask = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    
    // Horizontal (exclude edges)
    for (int f = 1; f < 7; f++) {
        if (f != file) mask |= square_bb(rank * 8 + f);
    }
    
    // Vertical (exclude edges)
    for (int r = 1; r < 7; r++) {
        if (r != rank) mask |= square_bb(r * 8 + file);
    }
    
    return mask;
}

Bitboard BitboardUtils::bishop_mask(Square sq) {
    Bitboard mask = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    
    // All four diagonal directions (exclude edges)
    int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    
    for (int d = 0; d < 4; d++) {
        int dr = directions[d][0];
        int df = directions[d][1];
        
        for (int step = 1; step < 7; step++) {
            int new_rank = rank + step * dr;
            int new_file = file + step * df;
            
            if (new_rank <= 0 || new_rank >= 7 || new_file <= 0 || new_file >= 7)
                break;
                
            mask |= square_bb(new_rank * 8 + new_file);
        }
    }
    
    return mask;
}

Bitboard BitboardUtils::index_to_bitboard(int index, Bitboard mask) {
    Bitboard result = 0ULL;
    int bit_count = popcount(mask);
    
    for (int i = 0; i < bit_count; i++) {
        Square sq = pop_lsb(mask);
        if (index & (1 << i)) {
            result |= square_bb(sq);
        }
    }
    
    return result;
}

// Attack getters using magic bitboards
Bitboard BitboardUtils::get_rook_attacks(Square sq, Bitboard occupied) {
    occupied &= rook_mask(sq);
    occupied *= rook_magics[sq];
    occupied >>= 52;  // 64 - 12 bits
    return rook_attacks[sq][occupied];
}

Bitboard BitboardUtils::get_bishop_attacks(Square sq, Bitboard occupied) {
    occupied &= bishop_mask(sq);
    occupied *= bishop_magics[sq];
    occupied >>= 55;  // 64 - 9 bits
    return bishop_attacks[sq][occupied];
}

Bitboard BitboardUtils::get_queen_attacks(Square sq, Bitboard occupied) {
    return get_rook_attacks(sq, occupied) | get_bishop_attacks(sq, occupied);
}

Bitboard BitboardUtils::get_knight_attacks(Square sq) {
    return knight_attacks[sq];
}

Bitboard BitboardUtils::get_king_attacks(Square sq) {
    return king_attacks[sq];
}

Bitboard BitboardUtils::get_pawn_attacks(Square sq, Color c) {
    return pawn_attacks[c][sq];
}