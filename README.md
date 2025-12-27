# 🏛️ Connect 4

> **Status:** Completed | **Type:** Game AI Engine | **Language:** C++

A high-performance Connect 4 AI engine built from scratch in C++. This project demonstrates advanced game theory concepts, utilizing the **Minimax algorithm with Alpha-Beta Pruning** to simulate up to 9 moves into the future. It features a custom hybrid scoring engine ("Titan") that combines physics-aware logic with strategic pattern recognition.

---

## ⚡ Key Features

### 🧠 The "Titan" Intelligence Engine
Unlike basic bots that just look for lines, the Titan engine understands the board deeply:
* **Gravity Awareness:** It knows pieces must fall. It ignores "floating threats" that cannot physically be played, saving computation power.
* **Strategic Gap Logic:** It values "split threats" (e.g., `X_X`) higher than simple connections (`XX`), recognizing that gaps are harder for opponents to block.
* **Deadly Pattern Recognition:** Detects unstoppable traps (e.g., `_XXX_` with open ends) and assigns them massive priority.

### 🚀 Performance Optimization
* **Minimax Algorithm:** Simulates thousands of future board states to find the optimal path.
* **Alpha-Beta Pruning:** Drastically reduces computation time by "pruning" (ignoring) move branches that are clearly worse than options already found.
* **Transposition Table:** Uses a memory cache (`unordered_map`) to store previously calculated board positions, preventing redundant processing.
* **Smart Move Ordering:** Evaluates the best columns (Center) first, maximizing the efficiency of the pruning algorithm.

### 🛡️ Robust Architecture
* **Adaptive Depth:** Adjusts thinking depth based on the game phase (Opening vs. Endgame).
* **Safety Limits:** Features bounded memory usage (~200MB cap) to ensure stability on any hardware.
* **Crash Protection:** Custom input handling prevents crashes from invalid keystrokes.

---

## 🎮 How to Play

### Prerequisites
You need a C++ compiler (like `g++`).

### Installation & Run
1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/MarwanArafa/connect4-titan.git](https://github.com/MarwanArafa/connect4-titan.git)
    cd connect4-titan
    ```

2.  **Compile the code (Maximum Optimization):**
    ```bash
    g++ -o titan "18. CONNECT 4.cpp" -O3
    ```

3.  **Run the game:**
    ```bash
    ./titan
    ```

---

## ⚙️ Difficulty Levels

* **Easy (Depth 2):** Fast and casual. Good for beginners.
* **Medium (Depth 4):** Starts to see traps. A decent challenge.
* **Hard (Depth 6):** Strong positional play. Hard to beat.
* **Grandmaster (Depth 7):** The standard "Titan" experience. Plays nearly perfectly.
* **Legend (Depth 9):** "God Mode." Calculates ~40 million positions. (Warning: Can be slow on older CPUs).

---

## 📸 Technical Highlights

```cpp
// Example: The Hybrid Scoring Logic
// Combines Physics (Gravity) with Strategy (Deadly Patterns)

if (isPlayable(r, c, board)) {  // 1. Physics Check
    if (isDeadlyPattern) {
        score += 5000;          // 2. Unstoppable Threat
    } else {
        score += 200;           // 3. Standard Threat
    }
} else {
    score += 10;                // 4. Ignore Floating Threats
}
