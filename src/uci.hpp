// ===== UCI INTERFACE =====
class UCIInterface {
public:
    void run();
    
private:
    Position position;
    SearchEngine engine;
    
    void handle_uci();
    void handle_isready();
    void handle_position(const std::string& cmd);
    void handle_go(const std::string& cmd);
    void handle_stop();
    void handle_quit();
    
    std::vector<std::string> split_string(const std::string& str);
};
