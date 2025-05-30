// ===== SEARCH ENGINE =====
class SearchEngine {
public:
    SearchEngine();
    
    struct SearchInfo {
        int max_depth = 64;
        int max_time_ms = 5000;
        int max_nodes = 1000000;
        bool infinite = false;
    };
    
    Move search(const Position& pos, const SearchInfo& info);
    void stop_search() { stop_flag = true; }
    
private:
    TranspositionTable tt;
    bool stop_flag;
    int nodes_searched;
    
    Score search_root(Position& pos, int depth);
    Score search(Position& pos, int depth, int ply, Score alpha, Score beta);
    Score quiescence_search(Position& pos, int ply, Score alpha, Score beta);
    
    void order_moves(const Position& pos, std::vector<Move>& moves, Move tt_move);
    bool is_draw(const Position& pos);
};

