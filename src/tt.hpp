// ===== TRANSPOSITION TABLE =====
struct TTEntry {
    uint64_t key;
    Score score;
    Move best_move;
    int depth;
    int flag; // EXACT, UPPER_BOUND, LOWER_BOUND
};

class TranspositionTable {
public:
    TranspositionTable(size_t mb_size);
    ~TranspositionTable();
    
    void store(uint64_t key, Score score, Move move, int depth, int flag);
    bool probe(uint64_t key, TTEntry& entry);
    void clear();
    
private:
    TTEntry* table;
    size_t size;
    size_t mask;
};

