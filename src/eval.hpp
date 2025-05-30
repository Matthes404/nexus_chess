// ===== EVALUATION =====
class Evaluator {
public:
    static Score evaluate(const Position& pos);
    
private:
    static Score material_value(const Position& pos);
    static Score piece_square_value(const Position& pos);
    static Score mobility_value(const Position& pos);
    static Score king_safety_value(const Position& pos);
    static Score pawn_structure_value(const Position& pos);
    
    // Piece values in centipawns
    static constexpr Score piece_values[PIECE_TYPE_NB] = {
        100, 320, 330, 500, 900, 20000
    };
};
