// ===== MOVE GENERATION =====
class MoveGenerator {
public:
    static std::vector<Move> generate_moves(const Position& pos);
    static std::vector<Move> generate_captures(const Position& pos);
    static std::vector<Move> generate_quiet_moves(const Position& pos);
    
private:
    static void generate_pawn_moves(const Position& pos, std::vector<Move>& moves);
    static void generate_piece_moves(const Position& pos, std::vector<Move>& moves, PieceType pt);
    static void generate_castling_moves(const Position& pos, std::vector<Move>& moves);
};