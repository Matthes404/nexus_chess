#include "move_utils.hpp"
#include <sstream>

// Move encoding:
// bits 0-5: from square (6 bits)
// bits 6-11: to square (6 bits)  
// bits 12-14: promotion piece type (3 bits)
// bit 15: castling flag
// bit 16: en passant flag
// bit 17: capture flag
// bits 18-31: reserved for future use

constexpr Move CASTLING_FLAG = 1 << 15;
constexpr Move EN_PASSANT_FLAG = 1 << 16;
constexpr Move CAPTURE_FLAG = 1 << 17;

Move MoveUtils::make_move(Square from, Square to, PieceType promotion) {
    Move move = from | (to << 6);
    
    if (promotion != PIECE_TYPE_NB) {
        move |= (promotion << 12);
    }
    
    return move;
}

Move MoveUtils::make_castling_move(Square from, Square to) {
    return make_move(from, to) | CASTLING_FLAG;
}

Move MoveUtils::make_en_passant_move(Square from, Square to) {
    return make_move(from, to) | EN_PASSANT_FLAG | CAPTURE_FLAG;
}

Move MoveUtils::make_capture_move(Square from, Square to, PieceType promotion) {
    Move move = make_move(from, to, promotion);
    return move | CAPTURE_FLAG;
}

Move MoveUtils::make_promotion_move(Square from, Square to, PieceType promotion, bool is_capture) {
    Move move = make_move(from, to, promotion);
    if (is_capture) {
        move |= CAPTURE_FLAG;
    }
    return move;
}

Square MoveUtils::from_sq(Move m) {
    return Square(m & 0x3F);
}

Square MoveUtils::to_sq(Move m) {
    return Square((m >> 6) & 0x3F);
}

PieceType MoveUtils::promotion_type(Move m) {
    return PieceType((m >> 12) & 0x7);
}

bool MoveUtils::is_promotion(Move m) {
    return (m >> 12) & 0x7;
}

bool MoveUtils::is_castling(Move m) {
    return m & CASTLING_FLAG;
}

bool MoveUtils::is_en_passant(Move m) {
    return m & EN_PASSANT_FLAG;
}

bool MoveUtils::is_capture(Move m) {
    return m & CAPTURE_FLAG;
}

std::string MoveUtils::to_string(Move m) {
    if (m == 0) return "0000";
    
    Square from = from_sq(m);
    Square to = to_sq(m);
    
    std::string result;
    
    // From square
    result += char('a' + (from % 8));
    result += char('1' + (from / 8));
    
    // To square  
    result += char('a' + (to % 8));
    result += char('1' + (to / 8));
    
    // Promotion
    if (is_promotion(m)) {
        PieceType pt = promotion_type(m);
        switch (pt) {
            case QUEEN: result += 'q'; break;
            case ROOK: result += 'r'; break;
            case BISHOP: result += 'b'; break;
            case KNIGHT: result += 'n'; break;
            default: break;
        }
    }
    
    return result;
}

Move MoveUtils::from_string(const std::string& move_str) {
    if (move_str.length() < 4) return 0;
    
    // Parse from square
    int from_file = move_str[0] - 'a';
    int from_rank = move_str[1] - '1';
    if (from_file < 0 || from_file >= 8 || from_rank < 0 || from_rank >= 8) {
        return 0;
    }
    Square from = from_rank * 8 + from_file;
    
    // Parse to square
    int to_file = move_str[2] - 'a';
    int to_rank = move_str[3] - '1';
    if (to_file < 0 || to_file >= 8 || to_rank < 0 || to_rank >= 8) {
        return 0;
    }
    Square to = to_rank * 8 + to_file;
    
    // Parse promotion
    PieceType promotion = PIECE_TYPE_NB;
    if (move_str.length() == 5) {
        switch (move_str[4]) {
            case 'q': promotion = QUEEN; break;
            case 'r': promotion = ROOK; break;
            case 'b': promotion = BISHOP; break;
            case 'n': promotion = KNIGHT; break;
            default: return 0; // Invalid promotion
        }
    }
    
    return make_move(from, to, promotion);
}

std::string MoveUtils::to_algebraic(Move m, const Position& pos) {
    if (m == 0) return "null";
    
    Square from = from_sq(m);
    Square to = to_sq(m);
    Piece moving_piece = pos.piece_on(from);
    PieceType piece_type = PieceType(moving_piece % 6);
    
    std::string result;
    
    // Handle castling
    if (is_castling(m)) {
        if (to % 8 == 6) { // Kingside (g-file)
            return "O-O";
        } else { // Queenside (c-file)
            return "O-O-O";
        }
    }
    
    // Piece symbol (skip for pawns)
    if (piece_type != PAWN) {
        switch (piece_type) {
            case KING: result += 'K'; break;
            case QUEEN: result += 'Q'; break;
            case ROOK: result += 'R'; break;
            case BISHOP: result += 'B'; break;
            case KNIGHT: result += 'N'; break;
            default: break;
        }
    }
    
    // Check for ambiguity (simplified - would need move generation for full accuracy)
    // For now, just add file/rank disambiguation when needed
    bool needs_file_disambiguation = false;
    bool needs_rank_disambiguation = false;
    
    // Add disambiguation if needed
    if (needs_file_disambiguation) {
        result += char('a' + (from % 8));
    } else if (needs_rank_disambiguation) {
        result += char('1' + (from / 8));
    }
    
    // Capture symbol
    if (is_capture(m)) {
        if (piece_type == PAWN && !needs_file_disambiguation) {
            result += char('a' + (from % 8));
        }
        result += 'x';
    }
    
    // Destination square
    result += char('a' + (to % 8));
    result += char('1' + (to / 8));
    
    // Promotion
    if (is_promotion(m)) {
        result += '=';
        PieceType pt = promotion_type(m);
        switch (pt) {
            case QUEEN: result += 'Q'; break;
            case ROOK: result += 'R'; break;
            case BISHOP: result += 'B'; break;
            case KNIGHT: result += 'N'; break;
            default: break;
        }
    }
    
    // Check/checkmate symbols would require position analysis
    // result += '+' for check, '#' for checkmate
    
    return result;
}

bool MoveUtils::is_quiet(Move m) {
    return !is_capture(m) && !is_promotion(m);
}

bool MoveUtils::is_tactical(Move m) {
    return is_capture(m) || is_promotion(m);
}

Move MoveUtils::null_move() {
    return 0;
}

bool MoveUtils::is_null(Move m) {
    return m == 0;
}

// Move scoring for move ordering
int MoveUtils::get_move_score(Move m, const Position& pos) {
    int score = 0;
    
    // Captures: MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
    if (is_capture(m)) {
        Square from = from_sq(m);
        Square to = to_sq(m);
        Piece victim = pos.piece_on(to);
        Piece attacker = pos.piece_on(from);
        
        if (victim != NO_PIECE) {
            int victim_value = get_piece_value(PieceType(victim % 6));
            int attacker_value = get_piece_value(PieceType(attacker % 6));
            score += victim_value * 100 - attacker_value;
        }
        
        // En passant captures
        if (is_en_passant(m)) {
            score += 100; // Pawn value
        }
    }
    
    // Promotions
    if (is_promotion(m)) {
        PieceType pt = promotion_type(m);
        score += get_piece_value(pt) - get_piece_value(PAWN);
        
        // Queen promotions are usually best
        if (pt == QUEEN) {
            score += 50;
        }
    }
    
    // Castling gets a small bonus
    if (is_castling(m)) {
        score += 25;
    }
    
    return score;
}

int MoveUtils::get_piece_value(PieceType pt) {
    constexpr int piece_values[PIECE_TYPE_NB] = {
        100, 320, 330, 500, 900, 20000
    };
    return piece_values[pt];
}

bool MoveUtils::operator<(const Move& a, const Move& b) {
    return a < b;
}

bool MoveUtils::operator==(const Move& a, const Move& b) {
    return a == b;
}

bool MoveUtils::operator!=(const Move& a, const Move& b) {
    return a != b;
}

// Debug function to print move details
std::string MoveUtils::debug_string(Move m) {
    std::ostringstream ss;
    ss << "Move: " << to_string(m);
    ss << " [from=" << from_sq(m) << ", to=" << to_sq(m) << "]";
    
    if (is_capture(m)) ss << " CAPTURE";
    if (is_castling(m)) ss << " CASTLING";
    if (is_en_passant(m)) ss << " EN_PASSANT";
    if (is_promotion(m)) {
        ss << " PROMOTION(" << promotion_type(m) << ")";
    }
    
    return ss.str();
}