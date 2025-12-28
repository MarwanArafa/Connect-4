/*
    CONNECT 4
        [Core Features]
        - Algorithm: Minimax with Alpha-Beta Pruning.
        - Heuristics: Gravity-aware evaluation, strategic pattern recognition.
        - Optimization: Transposition table (memory cache) & dynamic move ordering.
        - Safety: Input validation and bounded memory usage.
        - Compatibility: Works on Linux (Native) & Windows (Auto-Color Fix).
*/

#include <iostream>
#include <vector>
#include <climits> 
#include <ctime>
#include <cstdlib>
#include <algorithm> 
#include <unordered_map> 
#include <string>
#include <fstream> 
#include <random>

// --- WINDOWS COMPATIBILITY BLOCK ---
#ifdef _WIN32
    #include <windows.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#endif

using namespace std;

// --- CONFIGURATION ---
const int ROWS = 6;
const int COLS = 7;
const int MAX_MEMORY_SIZE = 2000000; 

// Colors
const string RED = "\033[31m";
const string BLUE = "\033[34m";
const string RESET = "\033[0m";
const string YELLOW = "\033[33m";
const string CLEAR_SCREEN = "\033[2J\033[1;1H";

char board[ROWS][COLS];

// MEMORY CACHE
unordered_map<string, pair<int, int>> memo;

// --- SYSTEM SETUP ---

void setupConsole() {
    #ifdef _WIN32
        // Enable UTF-8 for box characters
        SetConsoleOutputCP(65001);
        // Enable ANSI Colors
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    #endif
}

// --- VISUALS & SETUP ---

void initBoard() {
    for (int i = 0; i < ROWS; i++) 
        for (int j = 0; j < COLS; j++) board[i][j] = ' '; 
}

string getBoardHash(char b[ROWS][COLS]) {
    string key = "";
    for (int i = 0; i < ROWS; i++) 
        for (int j = 0; j < COLS; j++) key += b[i][j];
    return key;
}

void showRules() {
    cout << CLEAR_SCREEN;
    cout << "\n " << YELLOW << "┌───────────────────────────────────────────────┐" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << "             GAME RULES & MODES                " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "├───────────────────────────────────────────────┤" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << " " << RED << "1. CLASSIC MODE" << RESET << "                               " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << "    - GOAL: Connect 4 pieces in a row.         " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << "    - ENDING: Game stops instantly on a win.   " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << "                                               " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << " " << BLUE << "2. SCORE ATTACK" << RESET << "                               " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << "    - GOAL: Get as many lines as possible.     " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "│" << RESET << "    - ENDING: Game ends only when full.        " << YELLOW << "│" << RESET << "\n";
    cout << " " << YELLOW << "└───────────────────────────────────────────────┘" << RESET << "\n";
    cout << "\n  Press [ENTER] to continue...";
    if (cin.peek() == '\n') cin.ignore();
    cin.get(); 
}

void printBoard(int p1Score, int p2Score, string modeName) {
    cout << CLEAR_SCREEN;
    cout << "\n=== " << modeName << " ===\n";
    cout << "  " << RED << "P1: " << p1Score << RESET;
    cout << "      " << BLUE << "P2 (AI): " << p2Score << RESET << "\n";
    cout << "  1    2    3    4    5    6    7\n";
    cout << "┌────┬────┬────┬────┬────┬────┬────┐\n";

    for (int i = 0; i < ROWS; i++) {
        cout << "│"; 
        for (int j = 0; j < COLS; j++) {
            char cell = board[i][j];
            if (cell == 'X') cout << " " << RED << "██" << RESET << " │";
            else if (cell == 'O') cout << " " << BLUE << "██" << RESET << " │";
            else cout << "    │";
        }
        cout << "\n"; 
        if (i < ROWS - 1) cout << "├────┼────┼────┼────┼────┼────┼────┤\n";
        else cout << "└────┴────┴────┴────┴────┴────┴────┘\n";
    }
}

// --- CORE MECHANICS ---

int getNextOpenRow(char b[ROWS][COLS], int col) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (b[r][col] == ' ') return r;
    }
    return -1;
}

bool checkWin(char b[ROWS][COLS], char p) {
    for (int r=0; r<ROWS; r++) 
        for (int c=0; c<COLS-3; c++)
            if (b[r][c]==p && b[r][c+1]==p && b[r][c+2]==p && b[r][c+3]==p) return true;
    for (int r=0; r<ROWS-3; r++) 
        for (int c=0; c<COLS; c++)
            if (b[r][c]==p && b[r+1][c]==p && b[r+2][c]==p && b[r+3][c]==p) return true;
    for (int r=0; r<ROWS-3; r++) {
        for (int c=0; c<COLS-3; c++)
            if (b[r][c]==p && b[r+1][c+1]==p && b[r+2][c+2]==p && b[r+3][c+3]==p) return true;
        for (int c=3; c<COLS; c++)
            if (b[r][c]==p && b[r+1][c-1]==p && b[r+2][c-2]==p && b[r+3][c-3]==p) return true;
    }
    return false;
}

bool dropPiece(int col, char player) {
    int row = getNextOpenRow(board, col);
    if (row != -1) {
        board[row][col] = player;
        return true;
    }
    return false;
}

// --- EVALUATION ENGINE ---

bool isValidPlacement(int r, int c, char b[ROWS][COLS]) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    if (r == ROWS - 1) return true; 
    return (b[r + 1][c] != ' ');    
}

int evaluateWindow(char c1, char c2, char c3, char c4, char piece) {
    int score = 0;
    int countPiece = (c1==piece)+(c2==piece)+(c3==piece)+(c4==piece);
    int countEmpty = (c1==' ')+(c2==' ')+(c3==' ')+(c4==' ');
    int countOpp = 4 - countPiece - countEmpty;

    // A. OFFENSE
    if (countPiece == 4) return 1000000;
     
    if (countPiece == 3 && countEmpty == 1) {
        bool left_open = (c1 == ' ');
        bool right_open = (c4 == ' ');
        if (left_open && right_open) score += 5000; // Unstoppable
        else score += 150; // Strong threat
    }
    else if (countPiece == 2 && countEmpty == 2) {
        bool connected = (c1==piece && c2==piece) || (c2==piece && c3==piece) || (c3==piece && c4==piece);
        if (connected) score += 50;  
        else score += 60; // Separated pieces (X_X) are valued higher for strategic gaps
    }

    // B. DEFENSE
    if (countOpp == 3 && countEmpty == 1) {
        bool left_open = (c1 == ' ');
        bool right_open = (c4 == ' ');
        if (left_open && right_open) score -= 10000; 
        else score -= 500; 
    }
    else if (countOpp == 2 && countEmpty == 2) score -= 50;

    return score;
}

int evaluateBoard(char b[ROWS][COLS], char piece) {
    int score = 0;
     
    // Center Control
    for (int r = 0; r < ROWS; r++) {
        if (b[r][COLS/2] == piece) score += 200;
        if (b[r][COLS/2-1] == piece || b[r][COLS/2+1] == piece) score += 100;
    }

    // Evaluate Windows (Horizontal, Vertical, Diagonal)
    for (int r = 0; r < ROWS; r++) 
        for (int c = 0; c < COLS - 3; c++) 
            score += evaluateWindow(b[r][c], b[r][c+1], b[r][c+2], b[r][c+3], piece);
     
    for (int c = 0; c < COLS; c++) 
        for (int r = 0; r < ROWS - 3; r++) 
            score += evaluateWindow(b[r][c], b[r+1][c], b[r+2][c], b[r+3][c], piece);
     
    for (int r = 0; r < ROWS - 3; r++) { 
        for (int c = 0; c < COLS - 3; c++) 
            score += evaluateWindow(b[r][c], b[r+1][c+1], b[r+2][c+2], b[r+3][c+3], piece);
        for (int c = 3; c < COLS; c++) 
            score += evaluateWindow(b[r][c], b[r+1][c-1], b[r+2][c-2], b[r+3][c-3], piece);
    }
    return score;
}

// --- AI UTILITIES ---

int countThreats(char b[ROWS][COLS], char piece) {
    int threats = 0;
    for (int col = 0; col < COLS; col++) {
        int row = getNextOpenRow(b, col);
        if (row != -1) {
            b[row][col] = piece;
            if (checkWin(b, piece)) threats++;
            b[row][col] = ' ';
        }
    }
    return threats;
}

int evaluateMoveSimple(char b[ROWS][COLS], char piece) {
    return evaluateBoard(b, piece);
}

vector<int> getOptimizedMoves(char b[ROWS][COLS], bool maximizingPlayer) {
    vector<pair<int, int>> moves_with_score; 
    char piece = maximizingPlayer ? 'O' : 'X';
     
    for (int col = 0; col < COLS; col++) {
        int row = getNextOpenRow(b, col);
        if (row == -1) continue;
        
        b[row][col] = piece;
        int score = 0;
        if (checkWin(b, piece)) score = 1000000;
        else score = evaluateMoveSimple(b, piece); 
        b[row][col] = ' '; 
        
        // Strategic Ordering Bonus
        score += 30 - abs(col - 3) * 10; // Prioritize Center
        if (row < ROWS-1 && b[row+1][col] == piece) score += 40; // Prioritize Building

        moves_with_score.push_back({col, score});
    }
     
    sort(moves_with_score.begin(), moves_with_score.end(), 
         [](auto& a, auto& b) { return a.second > b.second; });
     
    vector<int> ordered;
    for (auto& p : moves_with_score) ordered.push_back(p.first);
    return ordered;
}

int getAdaptiveDepth(char b[ROWS][COLS], int base_depth) {
    int threats = countThreats(b, 'O') + countThreats(b, 'X');
    if (threats >= 2) return base_depth + 2; 
    if (threats == 1) return base_depth + 1; 
     
    int moves_played = 0;
    for (int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) if(b[i][j]!=' ') moves_played++;
     
    if (moves_played < 8) return max(3, base_depth - 1); 
    if (moves_played > 30) return base_depth + 1; 
    return base_depth;
}

// --- MINIMAX ALGORITHM ---

pair<int, int> minimax(char b[ROWS][COLS], int depth, int alpha, int beta, bool maximizingPlayer, bool isScoreAttack, int original_depth) {
     
    string key = getBoardHash(b) + to_string(depth) + (maximizingPlayer ? "T" : "F");
    if (memo.find(key) != memo.end()) return memo[key];

    if (!isScoreAttack) {
        if (checkWin(b, 'O')) return {-1, 1000000 + depth}; 
        if (checkWin(b, 'X')) return {-1, -1000000 - depth}; 
    }
     
    int empty_cells = 0;
    for (int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) if(b[i][j]==' ') empty_cells++;
    if (empty_cells <= (original_depth * 2) && !isScoreAttack) depth = empty_cells;

    if (depth == 0) {
        return {-1, evaluateBoard(b, 'O')};
    }

    vector<int> valid_locs = getOptimizedMoves(b, maximizingPlayer);
    if (valid_locs.empty()) return {-1, 0};

    int bestCol = valid_locs[0];
    int bestScore = maximizingPlayer ? INT_MIN : INT_MAX;

    if (maximizingPlayer) {
        for (int col : valid_locs) {
            int row = getNextOpenRow(b, col);
            b[row][col] = 'O';
            int score = minimax(b, depth - 1, alpha, beta, false, isScoreAttack, original_depth).second;
            b[row][col] = ' '; 
            
            if (score > bestScore) {
                bestScore = score;
                bestCol = col;
                if (depth == original_depth && score > 900000) {
                    memo[key] = {bestCol, bestScore};
                    return {bestCol, bestScore};
                }
            }
            alpha = max(alpha, bestScore);
            if (alpha >= beta) break; 
        }
    } 
    else {
        for (int col : valid_locs) {
            int row = getNextOpenRow(b, col);
            b[row][col] = 'X';
            int score = minimax(b, depth - 1, alpha, beta, true, isScoreAttack, original_depth).second;
            b[row][col] = ' '; 
            
            if (score < bestScore) {
                bestScore = score;
                bestCol = col;
            }
            beta = min(beta, bestScore);
            if (beta <= alpha) break; 
        }
    }

    if (memo.size() < MAX_MEMORY_SIZE) memo[key] = {bestCol, bestScore};
    else memo.clear(); 

    return {bestCol, bestScore};
}

int calculateFinalScore(char player) {
    int score = 0;
    // Horizontal
    for (int r = 0; r < ROWS; r++) { 
        int streak = 0; 
        for (int c = 0; c < COLS; c++) { 
            if (board[r][c] == player) streak++; 
            else { 
                if (streak >= 4) score += (streak - 3); 
                streak = 0; 
            } 
        } 
        if (streak >= 4) score += (streak - 3);
    }
    // Vertical
    for (int c = 0; c < COLS; c++) {
        int streak = 0; 
        for (int r = 0; r < ROWS; r++) { 
            if (board[r][c] == player) streak++; 
            else { 
                if (streak >= 4) score += (streak - 3); 
                streak = 0; 
            } 
        } 
        if (streak >= 4) score += (streak - 3);
    }
    // Diagonal (Down-Right)
    for (int r = 0; r < ROWS - 3; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (board[r][c] == player && board[r+1][c+1] == player && board[r+2][c+2] == player && board[r+3][c+3] == player) { 
                score++; 
            }
        }
    }
    // Diagonal (Up-Right)
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (board[r][c] == player && board[r-1][c+1] == player && board[r-2][c+2] == player && board[r-3][c+3] == player) { 
                score++; 
            }
        }
    }
    return score;
}

// --- INPUT UTILITY ---
int getUserInput() {
    string inputStr;
    int choice = -1;
    while (true) {
        getline(cin, inputStr);
        try {
            choice = stoi(inputStr);
            return choice;
        } catch (...) {
            cout << " Invalid input. Please enter a number: ";
        }
    }
}

// --- MAIN LOOP ---

int main() {
    setupConsole(); // WINDOWS FIX APPLIED HERE

    initBoard();
    showRules();

    int gameMode, opponentMode;
    int aiDepth = 2; 

    cout << CLEAR_SCREEN; 
    cout << "\n " << RED << "┌─────────────────────────────────────────┐" << RESET << "\n";
    cout << " " << RED << "│" << RESET << "      CONNECT 4: PROFESSIONAL EDITION    " << RED << "│" << RESET << "\n";
    cout << " " << RED << "└─────────────────────────────────────────┘" << RESET << "\n";
    cout << "  1. CLASSIC MODE\n  2. SCORE ATTACK\n  Choice: ";
    gameMode = getUserInput();
    bool isScoreAttack = (gameMode == 2);

    cout << CLEAR_SCREEN; 
    cout << "\n  1. HUMAN VS HUMAN\n  2. HUMAN VS AI\n  Choice: ";
    opponentMode = getUserInput();

    bool isAI = (opponentMode == 2);
     
    if (isAI) {
        cout << CLEAR_SCREEN; 
        cout << "\n  1. EASY (Depth 2)\n  2. MEDIUM (Depth 4)\n  3. HARD (Depth 6)\n  4. EXPERT (Depth 7)\n  Choice: ";
        int diff = getUserInput();
        if (diff == 1) aiDepth = 2; else if (diff == 2) aiDepth = 4; else if (diff == 3) aiDepth = 6; else aiDepth = 7; 
    }

    string modeTitle = (gameMode == 1) ? "CLASSIC MODE" : "SCORE ATTACK";
    int maxMoves = ROWS * COLS;
    int moves = 0;
    char current = 'X';
    bool gameOver = false;
    int s1 = 0, s2 = 0;

    while (!gameOver && moves < maxMoves) {
        if (gameMode == 2) { s1 = calculateFinalScore('X'); s2 = calculateFinalScore('O'); }
        printBoard(s1, s2, modeTitle);

        int targetCol = -1;

        if (isAI && current == 'O') {
            cout << " AI is thinking (Depth " << aiDepth << ")..." << endl;
            
            int adaptive_depth = getAdaptiveDepth(board, aiDepth);
            char boardCopy[ROWS][COLS];
            for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) boardCopy[i][j] = board[i][j];
            
            for (int col = 0; col < COLS; col++) {
                int row = getNextOpenRow(boardCopy, col);
                if (row == -1) continue;
                boardCopy[row][col] = 'O';
                if (checkWin(boardCopy, 'O') && !isScoreAttack) { targetCol = col; break; }
                boardCopy[row][col] = ' ';
                boardCopy[row][col] = 'X';
                if (checkWin(boardCopy, 'X') && !isScoreAttack) { targetCol = col; break; }
                boardCopy[row][col] = ' ';
            }
            
            if (targetCol == -1) {
                pair<int, int> result = minimax(boardCopy, adaptive_depth, 
                                                    INT_MIN, INT_MAX, true, 
                                                    isScoreAttack, adaptive_depth);
                targetCol = result.first;
            }

            if (targetCol == -1) {
                for(int k=0; k<COLS; k++) if(getNextOpenRow(board, k) != -1) { targetCol = k; break; }
            }

        } else {
            cout << " Player " << (current == 'X' ? RED : BLUE) << current << RESET << ", choose column (1-7): ";
            int input = getUserInput();
            targetCol = input - 1; 
        }

        if (dropPiece(targetCol, current)) {
            moves++;
            if (gameMode == 1) { 
                if (checkWin(board, current)) {
                    if (current == 'X') s1 = 1; else s2 = 1;
                    printBoard(s1, s2, modeTitle);
                    if (current == 'O') cout << "\n 🤖 " << BLUE << "AI WINS!" << RESET << " 🤖\n";
                    else cout << "\n 🏆 " << RED << "PLAYER X WINS!" << RESET << " 🏆\n";
                    gameOver = true;
                }
            }
            if (!gameOver) current = (current == 'X') ? 'O' : 'X';
        } else {
            if (current == 'X') { cout << "Invalid move. Try again.\n"; }
        }
    }

    if (gameMode == 2) {
        s1 = calculateFinalScore('X'); s2 = calculateFinalScore('O');
        printBoard(s1, s2, modeTitle);
        cout << "\n FINAL SCORE: X=" << s1 << " | O=" << s2 << endl;
        if (s1 > s2) cout << " 🏆 PLAYER X WINS! 🏆\n";
        else if (s2 > s1) cout << " 🤖 AI WINS! 🤖" << endl;
        else cout << " 🤝 DRAW! 🤝\n";
    } else if (moves >= maxMoves && !gameOver) {
        cout << " 🤝 DRAW! Board is full.\n";
    }

    return 0;
}
