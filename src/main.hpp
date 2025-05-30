// ===== MAIN ENGINE CLASS =====
class ChessEngine {
public:
    static void initialize();
    static void run_uci();
    
private:
    static bool initialized;
};

#endif // CHESS_ENGINE_H

// ===== EXAMPLE USAGE =====
/*
int main() {
    ChessEngine::initialize();
    ChessEngine::run_uci();
    return 0;
}
*/