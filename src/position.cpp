#include "position.hpp"
#include "bitboard_utils.hpp"
#include <sstream>
#include <cctype>
#include <random>

// Zobrist hash keys for position hashing
namespace {
    uint64_t piece_keys[PIECE_NB][SQUARE_NB];
    uint64_t castling_keys[16];
    uint64_t ep_keys[SQUARE_NB];
    uint64_t side_key;
    
    bool zobrist_initialized = false;
    
    void init_zobrist() {
        if (zobrist_initialized) return;
        
        std::mt19937_64 rng(12345); // Fixed seed for reproducibility
        
        // Initialize piece keys
        for (int piece = 0; piece < PIECE_NB; piece++) {
            for (int sq = 0; sq < SQUARE_NB; sq++) {
                piece_keys[piece][sq] = rng();
            }
        }
        
        // Initialize castling keys
        for (int i = 0; i < 16; i++) {
            castling_keys[i] = rng();
        }
        
        // Initialize en passant keys
        for (int sq = 0; sq < SQUARE_NB; sq++) {
            ep_keys[sq] = rng();
        }
        
        side_key = rng();
        zobrist_initialized = true;
    }
}

// Castling rights constants
constexpr int WHITE_OO = 1;
constexpr int WHITE_OOO = 2;
constexpr int BLACK_OO = 4;
constexpr int BLACK_OOO = 8;

Position::Position() {
    init_zobrist();
    set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Position::Position(const std::string& fen) {
    init_zobrist();
    set_fen(fen);
}

void Position::set_fen(const std::string& fen) {
    // Clear the board
    for (int i = 0; i < SQUARE_NB; i++) {
        board[i] = NO_PIECE;
    }
    
    for (int c = 0; c < COLOR_NB; c++) {
        by_color[c] = 0ULL;
    }
    
    for (int pt = 0; pt < PIECE_TYPE_NB; pt++) {
        by_type[pt] = 0ULL;
    }
    
    std::istringstream ss(fen);
    std::string board_str, side_str, castling_str, ep_str, halfmove_str, fullmove_str;
    
    ss >> board_str >> side_str >> castling_str >> ep_str >> halfmove_str >> fullmove_str;
    
    // Parse board position
    int rank = 7, file = 0;
    for (char c : board_str) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (std::isdigit(c)) {
            file += c - '0';
        } else {
            Piece piece = NO_PIECE;
            switch (std::tolower(c)) {
                case 'p': piece = std::isupper(c) ? W_PAWN : B_PAWN; break;
                case 'n': piece = std::isupper(c) ? W_KNIGHT : B_KNIGHT; break;
                case 'b': piece = std::isupper(c) ? W_BISHOP : B_BISHOP; break;
                case 'r': piece = std::isupper(c) ? W_ROOK : B_ROOK; break;
                case 'q': piece = std::isupper(c) ? W_QUEEN : B_QUEEN; break;
                case 'k': piece = std::isupper(c) ? W_KING : B_KING; break;
            }
            
            if (piece != NO_PIECE) {
                Square sq = rank * 8 + file;
                board[sq] = piece;
                file++;
            }
        }
    }
    
    // Parse side to move
    stm = (side_str == "w") ? WHITE : BLACK;
    
    // Parse castling rights
    castling_rights = 0;
    for (char c : castling_str) {
        switch (c) {
            case 'K': castling_rights |= WHITE_OO; break;
            case 'Q': castling_rights |= WHITE_OOO; break;
            case 'k': castling_rights |= BLACK_OO; break;
            case 'q': castling_rights |= BLACK_OOO; break;
        }
    }
    
    // Parse en passant square
    ep_square = SQUARE_NB;
    if (ep_str != "-" && ep_str.length() == 2) {
        int file = ep_str[0] - 'a';
        int rank = ep_str[1] - '1';
        if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            ep_square = rank * 8 + file;
        }
    }
    
    // Parse move counters
    halfmove_clock = halfmove_str.empty() ? 0 : std::stoi(halfmove_str);
    fullmove_number = fullmove_str.empty() ? 1 : std::stoi(fullmove_str);
    
    update_bitboards();
    calculate_hash();
}

std::string Position::fen() const {
    std::string result;
    
    // Board position
    for (int rank = 7; rank >= 0; rank--) {
        int empty_count = 0;
        for (int file = 0; file < 8; file++) {
            Square sq = rank * 8 + file;
            Piece piece = board[sq];
            
            if (piece == NO_PIECE) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    result += std::to_string(empty_count);
                    empty_count = 0;
                }
                
                char piece_char;
                switch (piece) {
                    case W_PAWN: piece_char = 'P'; break;
                    case W_KNIGHT: piece_char = 'N'; break;
                    case W_BISHOP: piece_char = 'B'; break;
                    case W_ROOK: piece_char = 'R'; break;
                    case W_QUEEN: piece_char = 'Q'; break;
                    case W_KING: piece_char = 'K'; break;
                    case B_PAWN: piece_char = 'p'; break;
                    case B_KNIGHT: piece_char = 'n'; break;
                    case B_BISHOP: piece_char = 'b'; break;
                    case B_ROOK: piece_char = 'r'; break;
                    case B_QUEEN: piece_char = 'q'; break;
                    case B_KING: piece_char = 'k'; break;
                    default: piece_char = '?'; break;
                }
                result += piece_char;
            }
        }
        
        if (empty_count > 0) {
            result += std::to_string(empty_count);
        }
        
        if (rank > 0) result += '/';
    }
    
    // Side to move
    result += (stm == WHITE) ? " w " : " b ";
    
    // Castling rights
    std::string castling;
    if (castling_rights & WHITE_OO) castling += 'K';
    if (castling_rights & WHITE_OOO) castling += 'Q';
    if (castling_rights & BLACK_OO) castling += 'k';
    if (castling_rights & BLACK_OOO) castling += 'q';
    if (castling.empty()) castling = "-";
    result += castling + " ";
    
    // En passant square
    if (ep_square < SQUARE_NB) {
        result += char('a' + (ep_square % 8));
        result += char('1' + (ep_square / 8));
    } else {
        result += "-";
    }
    
    // Move counters
    result += " " + std::to_string(halfmove_clock);
    result += " " + std::to_string(fullmove_number);
    
    return result;
}

void Position::update_bitboards() {
    // Clear bitboards
    for (int c = 0; c < COLOR_NB; c++) {
        by_color[c] = 0ULL;
    }
    
    for (int pt = 0; pt < PIECE_TYPE_NB; pt++) {
        by_type[pt] = 0ULL;
    }
    
    // Rebuild bitboards from board array
    for (Square sq = A1; sq <= H8; ++sq) {
        Piece piece = board[sq];
        if (piece == NO_PIECE) continue;
        
        Color color = (piece < B_PAWN) ? WHITE : BLACK;
        PieceType piece_type = PieceType(piece % 6);
        
        by_color[color] |= BitboardUtils::square_bb(sq);
        by_type[piece_type] |= BitboardUtils::square_bb(sq);
    }
}

void Position::calculate_hash() {
    hash_key = 0ULL;
    
    // Hash pieces
    for (Square sq = A1; sq <= H8; ++sq) {
        Piece piece = board[sq];
        if (piece != NO_PIECE) {
            hash_key ^= piece_keys[piece][sq];
        }
    }
    
    // Hash side to move
    if (stm == BLACK) {
        hash_key ^= side_key;
    }
    
    // Hash castling rights
    hash_key ^= castling_keys[castling_rights];
    
    // Hash en passant square
    if (ep_square < SQUARE_NB) {
        hash_key ^= ep_keys[ep_square];
    }
}

bool Position::in_check() const {
    Square king_sq = BitboardUtils::lsb(pieces(stm, KING));
    return is_attacked_by(king_sq, Color(stm ^ 1));
}

bool Position::is_attacked_by(Square sq, Color attacking_color) const {
    // Check pawn attacks
    Bitboard pawn_attackers = BitboardUtils::get_pawn_attacks(sq, Color(attacking_color ^ 1)) 
                             & pieces(attacking_color, PAWN);
    if (pawn_attackers) return true;
    
    // Check knight attacks
    Bitboard knight_attackers = BitboardUtils::get_knight_attacks(sq) 
                               & pieces(attacking_color, KNIGHT);
    if (knight_attackers) return true;
    
    // Check king attacks
    Bitboard king_attackers = BitboardUtils::get_king_attacks(sq) 
                             & pieces(attacking_color, KING);
    if (king_attackers) return true;
    
    // Check sliding piece attacks
    Bitboard occupied = by_color[WHITE] | by_color[BLACK];
    
    // Rook/Queen attacks
    Bitboard rook_attackers = BitboardUtils::get_rook_attacks(sq, occupied) 
                             & (pieces(attacking_color, ROOK) | pieces(attacking_color, QUEEN));
    if (rook_attackers) return true;
    
    // Bishop/Queen attacks
    Bitboard bishop_attackers = BitboardUtils::get_bishop_attacks(sq, occupied) 
                               & (pieces(attacking_color, BISHOP) | pieces(attacking_color, QUEEN));
    if (bishop_attackers) return true;
    
    return false;
}

void Position::do_move(Move m) {
    // Store previous state for undo
    previous_states.push_back({
        ep_square, castling_rights, halfmove_clock, hash_key, board[MoveUtils::to_sq(m)]
    });
    
    Square from = MoveUtils::from_sq(m);
    Square to = MoveUtils::to_sq(m);
    Piece moving_piece = board[from];
    Piece captured_piece = board[to];
    
    // Update hash for moving piece
    hash_key ^= piece_keys[moving_piece][from];
    hash_key ^= piece_keys[moving_piece][to];
    
    // Update hash for captured piece
    if (captured_piece != NO_PIECE) {
        hash_key ^= piece_keys[captured_piece][to];
    }
    
    // Move the piece
    board[from] = NO_PIECE;
    board[to] = moving_piece;
    
    // Handle special moves
    if (MoveUtils::is_promotion(m)) {
        PieceType promotion_type = MoveUtils::promotion_type(m);
        Piece promotion_piece = Piece((stm == WHITE ? 0 : 6) + promotion_type);
        board[to] = promotion_piece;
        
        // Update hash for promotion
        hash_key ^= piece_keys[moving_piece][to];
        hash_key ^= piece_keys[promotion_piece][to];
    }
    
    if (MoveUtils::is_castling(m)) {
        // Move the rook
        if (to == G1) { // White kingside
            board[H1] = NO_PIECE;
            board[F1] = W_ROOK;
            hash_key ^= piece_keys[W_ROOK][H1];
            hash_key ^= piece_keys[W_ROOK][F1];
        } else if (to == C1) { // White queenside
            board[A1] = NO_PIECE;
            board[D1] = W_ROOK;
            hash_key ^= piece_keys[W_ROOK][A1];
            hash_key ^= piece_keys[W_ROOK][D1];
        } else if (to == G8) { // Black kingside
            board[H8] = NO_PIECE;
            board[F8] = B_ROOK;
            hash_key ^= piece_keys[B_ROOK][H8];
            hash_key ^= piece_keys[B_ROOK][F8];
        } else if (to == C8) { // Black queenside
            board[A8] = NO_PIECE;
            board[D8] = B_ROOK;
            hash_key ^= piece_keys[B_ROOK][A8];
            hash_key ^= piece_keys[B_ROOK][D8];
        }
    }
    
    if (MoveUtils::is_en_passant(m)) {
        Square captured_pawn_sq = stm == WHITE ? (to - 8) : (to + 8);
        Piece captured_pawn = board[captured_pawn_sq];
        board[captured_pawn_sq] = NO_PIECE;
        hash_key ^= piece_keys[captured_pawn][captured_pawn_sq];
    }
    
    // Update castling rights
    hash_key ^= castling_keys[castling_rights];
    
    if (moving_piece == W_KING) {
        castling_rights &= ~(WHITE_OO | WHITE_OOO);
    } else if (moving_piece == B_KING) {
        castling_rights &= ~(BLACK_OO | BLACK_OOO);
    }
    
    if (from == A1 || to == A1) castling_rights &= ~WHITE_OOO;
    if (from == H1 || to == H1) castling_rights &= ~WHITE_OO;
    if (from == A8 || to == A8) castling_rights &= ~BLACK_OOO;
    if (from == H8 || to == H8) castling_rights &= ~BLACK_OO;
    
    hash_key ^= castling_keys[castling_rights];
    
    // Update en passant square
    hash_key ^= ep_keys[ep_square];
    ep_square = SQUARE_NB;
    
    if ((moving_piece == W_PAWN || moving_piece == B_PAWN) && abs(to - from) == 16) {
        ep_square = (from + to) / 2;
    }
    
    if (ep_square < SQUARE_NB) {
        hash_key ^= ep_keys[ep_square];
    }
    
    // Update move counters
    if (captured_piece != NO_PIECE || moving_piece == W_PAWN || moving_piece == B_PAWN) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }
    
    if (stm == BLACK) {
        fullmove_number++;
    }
    
    // Switch side to move
    hash_key ^= side_key;
    stm = Color(stm ^ 1);
    
    update_bitboards();
}

void Position::undo_move(Move m) {
    if (previous_states.empty()) return;
    
    auto& prev_state = previous_states.back();
    
    // Switch side back
    stm = Color(stm ^ 1);
    
    Square from = MoveUtils::from_sq(m);
    Square to = MoveUtils::to_sq(m);
    Piece moving_piece = board[to];
    
    // Handle promotion undo
    if (MoveUtils::is_promotion(m)) {
        moving_piece = (stm == WHITE) ? W_PAWN : B_PAWN;
    }
    
    // Move piece back
    board[from] = moving_piece;
    board[to] = prev_state.captured_piece;
    
    // Handle special moves
    if (MoveUtils::is_castling(m)) {
        if (to == G1) { // White kingside
            board[H1] = W_ROOK;
            board[F1] = NO_PIECE;
        } else if (to == C1) { // White queenside
            board[A1] = W_ROOK;
            board[D1] = NO_PIECE;
        } else if (to == G8) { // Black kingside
            board[H8] = B_ROOK;
            board[F8] = NO_PIECE;
        } else if (to == C8) { // Black queenside
            board[A8] = B_ROOK;
            board[D8] = NO_PIECE;
        }
    }
    
    if (MoveUtils::is_en_passant(m)) {
        Square captured_pawn_sq = stm == WHITE ? (to - 8) : (to + 8);
        board[captured_pawn_sq] = (stm == WHITE) ? B_PAWN : W_PAWN;
    }
    
    // Restore previous state
    ep_square = prev_state.ep_square;
    castling_rights = prev_state.castling_rights;
    halfmove_clock = prev_state.halfmove_clock;
    hash_key = prev_state.hash_key;
    
    if (stm == BLACK) {
        fullmove_number--;
    }
    
    previous_states.pop_back();
    update_bitboards();
}

bool Position::is_legal(Move m) const {
    // Quick check for basic validity
    Square from = MoveUtils::from_sq(m);
    Square to = MoveUtils::to_sq(m);
    
    if (from == to || from >= SQUARE_NB || to >= SQUARE_NB) return false;
    
    Piece moving_piece = board[from];
    if (moving_piece == NO_PIECE) return false;
    
    Color piece_color = (moving_piece < B_PAWN) ? WHITE : BLACK;
    if (piece_color != stm) return false;
    
    // Check if move leaves king in check
    Position temp = *this;
    temp.do_move(m);
    
    return !temp.is_attacked_by(
        BitboardUtils::lsb(temp.pieces(stm, KING)), 
        Color(stm ^ 1)
    );
}