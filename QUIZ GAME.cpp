/*
================================================================================
                    CONSOLE-BASED QUIZ GAME APPLICATION
================================================================================

PROJECT DESCRIPTION:
This is an interactive console-based quiz game that allows users to test their
knowledge across multiple categories including Science, Computer, Sports, History,
and IQ/Logic. The game features multiple difficulty levels, a timed question
system, lifelines to assist players, and comprehensive score tracking.

KEY FEATURES:
- Multiple quiz categories (Science, Computer, Sports, History, IQ/Logic)
- Three difficulty levels (Easy, Medium, Hard)
- Timed questions with countdown display
- Four lifelines: 50/50, Skip Question, Replace Question, Extra Time
- Streak bonus system for consecutive correct answers
- Negative marking for wrong/timed-out answers
- Review incorrect answers after quiz completion
- High score leaderboard with persistent storage
- Detailed quiz logs with timestamps
- Post-quiz menu with replay and review options

TECHNICAL FEATURES:
- Question shuffling for randomization
- Real-time timer display using Windows console API
- Non-blocking keyboard input detection
- File-based persistent storage for questions, scores, and logs
- Dynamic question loading based on difficulty and category

GAME MECHANICS:
- 10 questions per quiz session
- Base points: 1 point per correct answer
- Streak bonuses: +5 at 3 streak, +15 at 5 streak
- Negative marking: -2 (Easy), -3 (Medium), -5 (Hard)
- Timer: 15 seconds per question (extensible with lifeline)

FILES REQUIRED:
- science.txt, computer.txt, sports.txt, history.txt, iq.txt (question banks)
- high_scores.txt (leaderboard storage)
- quiz_logs.txt (detailed session logs)

QUESTION FORMAT IN FILES:
Question|Option1|Option2|Option3|Option4|CorrectAnswerNumber

AUTHORS: [Your Name/Team Name]
DATE: [Current Date]
VERSION: 1.0

================================================================================
*/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>
#include <windows.h>
#include <conio.h>
using namespace std;

// ---------- GLOBAL CONSTANTS ----------
const int totalquestions = 150;      // Maximum questions that can be loaded from file
const int sessionquestions = 10;     // Number of questions per quiz session

// ---------- GLOBAL VARIABLES ----------
string filename;                     // Name of the current question file
string questions[totalquestions];    // Array to store all loaded questions
int indices[50];                     // Array of question indices for current difficulty
int used_indices[50];                // Track which questions have been used
int used_count = 0;                  // Count of used questions
int score = 0;                       // Player's current score
int category = 0;                    // Selected quiz category (1-5)
int difficulty = 0;                  // Selected difficulty level (1-3)
string playername;                   // Player's name
int streak = 0;                      // Current streak of correct answers
int timer = 15;                      // Default time limit per question (seconds)
bool replace_requested = false;      // Flag for replace question lifeline

// Lifeline availability flags
bool lifeline_5050 = true;           // 50/50 lifeline (removes 2 wrong answers)
bool lifeline_skip = true;           // Skip question lifeline (no penalty)
bool lifeline_replace = true;        // Replace question lifeline (new question)
bool lifeline_extratime = true;      // Extra time lifeline (+10 seconds)

// Session tracking arrays for review feature
string incorrect_questions[sessionquestions];           // Store incorrect questions
string incorrect_options[sessionquestions][4];          // Store options for review
int incorrect_correct_pos[sessionquestions];            // Store correct answer positions
int incorrect_count = 0;             // Count of incorrect answers
int correct_count = 0;               // Count of correct answers
int wrong_count = 0;                 // Count of wrong answers

// ---------- FUNCTION DECLARATIONS ----------


 //Extracts and returns the substring before the first '|' delimiter

string cut(string& s);

// Shuffles an array of integers using Fisher-Yates algorithm

void shuffle_array(int arr[], int n);

//Loads questions from a file into the global questions array
int load_questions(string filename);

// Displays a question with its options and handles user interaction

void display_question(string question, string options[], int correct_pos, int q_num, bool is_review);

/**
 * Main quiz execution function - handles question flow and game logic
 */
void start_quiz();

// Saves player's score to the high scores file

void save_high_score(string player, int score, string category, string difficulty);

/**
 * Displays the high scores leaderboard sorted by score
 */
void show_high_scores();

// Gets user input with a countdown timer using non-blocking input

bool get_answer_with_timer(int& answer, int time_limit);

/**
 * Updates the timer display in the top-right corner of console
 */
void update_timer_display(int remaining);

// Activates 50/50 lifeline - removes 2 incorrect options

void use_lifeline_5050(string options[], int correct_pos);

/**
 * Activates skip question lifeline - skips current question without penalty
 */
void use_lifeline_skip();

// Activates replace question lifeline - loads a new unused question

bool use_lifeline_replace(int& current_index, int used_indices[], int& used_count);

// Activates extra time lifeline - adds 10 seconds to timer

void use_lifeline_extratime(int& timer_ref);

/**
 * Resets all lifelines to available state at start of new quiz
 */
void reset_lifelines();

// Gets current system date and time as formatted string

string get_current_datetime();

// Saves detailed quiz session log to file

void save_quiz_log(string player, string category_str, string difficulty_str, int correct, int wrong, int total_score);

/**
 * Displays all incorrectly answered questions for review
 */
void review_incorrect_questions();

/**
 * Displays post-quiz menu with options to review, replay, or exit
 */
void post_quiz_menu();

// ======================= MAIN ============================
/**
 * Main entry point - displays main menu and handles user navigation
 */
int main() {
    srand(time(0));  // Seed random number generator

    while (true) {
        system("cls");
        cout << "========================================\n";
        cout << "          CONSOLE-BASED QUIZ GAME\n";
        cout << "========================================\n";
        cout << "1. Start New Quiz\n";
        cout << "2. View High Scores\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 3) break;  // Exit program

        switch (choice) {

        case 1: {
            // Get player information and quiz preferences
            system("cls");
            cout << "Enter your name: ";
            getline(cin, playername);

            // Category selection
            cout << "=== SELECT CATEGORY ===\n";
            cout << "1. Science\n2. Computer\n3. Sports\n4. History\n5. IQ/Logic\n";
            cout << "Enter choice: ";
            cin >> category;
            cin.ignore();

            // Validate category
            if (category < 1 || category > 5) {
                cout << "Invalid!\nPress Enter...";
                cin.ignore();
                break;
            }

            // Difficulty selection
            cout << "\n=== SELECT DIFFICULTY ===\n";
            cout << "1. Easy\n2. Medium\n3. Hard\n";
            cout << "Enter choice: ";
            cin >> difficulty;
            cin.ignore();

            // Validate difficulty
            if (difficulty < 1 || difficulty > 3) {
                cout << "Invalid!\nPress Enter...";
                cin.ignore();
                break;
            }

            // Initialize quiz session variables
            streak = 0;
            timer = 15;
            incorrect_count = 0;
            correct_count = 0;
            wrong_count = 0;
            reset_lifelines();
            start_quiz();
            break;
        }

        case 2:
            // Display high scores
            system("cls");
            show_high_scores();
            break;

        default:
            cout << "Invalid choice. Press Enter...";
            cin.ignore();
        }
    }

    cout << "\nThank you for playing!\n";
    return 0;
}

// ======================= FUNCTION DEFINITIONS ============================

// ---------- Helper function to split string by '|' delimiter ----------
string cut(string& s) {
    int pos = s.find('|');
    if (pos == string::npos) {
        // No delimiter found, return entire string
        string result = s;
        s = "";
        return result;
    }
    // Extract part before delimiter
    string part = s.substr(0, pos);
    // Update string to part after delimiter
    s = s.substr(pos + 1);
    return part;
}

// ---------- Shuffle array using Fisher-Yates algorithm ----------
void shuffle_array(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // Swap elements
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// ---------- Load questions from file ----------
int load_questions(string filename) {
    ifstream fin(filename);
    if (!fin.is_open()) return 0;  // File not found

    int count = 0;
    // Read questions line by line
    while (getline(fin, questions[count]) && count < totalquestions) {
        count++;
    }

    fin.close();
    return count;  // Return number of questions loaded
}

// ---------- Get current date and time as formatted string ----------
string get_current_datetime() {
    time_t now = time(0);
    tm t;

    // Safe version of localtime
    localtime_s(&t, &now);

    // Extract date and time components
    int year = 1900 + t.tm_year;
    int month = 1 + t.tm_mon;
    int day = t.tm_mday;
    int hour = t.tm_hour;
    int minute = t.tm_min;
    int second = t.tm_sec;

    char buffer[50];

    // Format as string: YYYY-MM-DD HH:MM:SS
    sprintf_s(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
        year, month, day, hour, minute, second);

    return string(buffer);
}

// ======================= TIMER FUNCTIONS ============================

// ---------- Update timer display at top right corner of console ----------
void update_timer_display(int remaining) {
    // Get console handle for cursor manipulation
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    COORD origPos = csbi.dwCursorPosition;  // Save current cursor position

    // Set cursor to top-right position
    COORD timerPos;
    timerPos.X = 60;
    timerPos.Y = 0;
    SetConsoleCursorPosition(hConsole, timerPos);
    cout << "Time: " << remaining << "s ";
    cout.flush();

    // Restore original cursor position
    SetConsoleCursorPosition(hConsole, origPos);
}

// ---------- Get answer with countdown timer using non-blocking input ----------
bool get_answer_with_timer(int& answer, int time_limit) {
    auto start_time = chrono::steady_clock::now();
    bool input_received = false;

    while (!input_received) {
        // Calculate elapsed time
        auto current_time = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(current_time - start_time).count();
        int remaining = time_limit - (int)elapsed;

        // Update timer display
        if (remaining >= 0) {
            update_timer_display(remaining);
        }

        // Check if time expired
        if (elapsed >= time_limit) {
            cout << "\n\nTime's up!\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return false;
        }

        // Check for keyboard input (non-blocking)
        if (_kbhit()) {
            cin >> answer;
            cin.clear();
            cin.ignore(10000, '\n');
            return true;
        }

        // Sleep briefly to avoid busy-waiting
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return false;
}

// ======================= LIFELINE FUNCTIONS ============================

// ---------- Reset all lifelines to available state ----------
void reset_lifelines() {
    lifeline_5050 = true;
    lifeline_skip = true;
    lifeline_replace = true;
    lifeline_extratime = true;
}

// ---------- Lifeline 1: 50/50 - Remove 2 wrong answers ----------
void use_lifeline_5050(string options[], int correct_pos) {
    if (!lifeline_5050) {
        cout << "\n[!] 50/50 already used!\n";
        return;
    }

    lifeline_5050 = false;
    cout << "\n[LIFELINE USED: 50/50]\n";
    cout << "Removing 2 wrong answers...\n\n";

    // Remove 2 incorrect options
    int removed = 0;
    for (int i = 0; i < 4 && removed < 2; ++i) {
        if (i != correct_pos) {
            options[i] = "[REMOVED]";
            removed++;
        }
    }
}

// ---------- Lifeline 2: Skip - Skip question without penalty ----------
void use_lifeline_skip() {
    if (!lifeline_skip) {
        cout << "\n[!] Skip already used!\n";
        return;
    }

    lifeline_skip = false;
    cout << "\n[LIFELINE USED: Skip Question]\n";
    cout << "Question skipped without penalty!\n";
}

// ---------- Lifeline 3: Replace - Get a new question ----------
bool use_lifeline_replace(int& current_index, int used_indices[], int& used_count) {
    if (!lifeline_replace) {
        cout << "\n[!] Replace already used!\n";
        return false;
    }

    lifeline_replace = false;
    cout << "\n[LIFELINE USED: Replace Question]\n";
    cout << "Finding a new question...\n\n";

    // Find an unused question from current difficulty range
    int start_index = (difficulty - 1) * 50;
    for (int i = start_index; i < start_index + 50; i++) {
        bool is_used = false;
        // Check if question already used
        for (int j = 0; j < used_count; j++) {
            if (used_indices[j] == i) {
                is_used = true;
                break;
            }
        }
        // Found unused question
        if (!is_used) {
            current_index = i;
            used_indices[used_count++] = i;
            return true;
        }
    }
    return false;  // No unused questions available
}

// ---------- Lifeline 4: Extra Time - Add 10 seconds to timer ----------
void use_lifeline_extratime(int& timer_ref) {
    if (!lifeline_extratime) {
        cout << "\nExtra Time already used!\n";
        return;
    }

    lifeline_extratime = false;
    timer_ref += 10;  // Add 10 seconds
    cout << "\n[LIFELINE USED: Extra Time]\n";
    cout << "+10 seconds added! New time: " << timer_ref << "s\n";
}

// ======================= LOGGING FUNCTIONS ============================

// ---------- Save detailed quiz log to file ----------
void save_quiz_log(string player, string category_str, string difficulty_str, int correct, int wrong, int total_score) {
    ofstream fout("quiz_logs.txt", ios::app);  // Append mode
    if (!fout.is_open()) {
        cout << "Error opening quiz_logs.txt\n";
        return;
    }

    string datetime = get_current_datetime();

    // Write formatted log entry
    fout << "========================================\n";
    fout << "Player: " << player << "\n";
    fout << "Date & Time: " <<__DATE__<<__TIME__<< "\n";
    fout << "Category: " << category_str << "\n";
    fout << "Difficulty: " << difficulty_str << "\n";
    fout << "Questions Correct: " << correct << "\n";
    fout << "Questions Wrong: " << wrong << "\n";
    fout << "Total Score: " << total_score << "/" << sessionquestions << "\n";
    fout << "========================================\n\n";

    fout.close();
}

// ======================= REVIEW FUNCTIONS ============================

// ---------- Review all incorrectly answered questions ----------
void review_incorrect_questions() {
    if (incorrect_count == 0) {
        cout << "\nGreat job! You didn't answer any questions incorrectly!\n";
        cout << "Press Enter to continue...";
        cin.get();
        return;
    }

    system("cls");
    cout << "========================================\n";
    cout << "       REVIEWING INCORRECT ANSWERS\n";
    cout << "========================================\n";
    cout << "You answered " << incorrect_count << " question(s) incorrectly.\n\n";
    cout << "Press Enter to review them...";
    cin.get();

    // Display each incorrect question with correct answer
    for (int i = 0; i < incorrect_count; i++) {
        system("cls");
        cout << "Review Question " << i + 1 << " of " << incorrect_count << "\n\n";

        // Display in review mode (no timer/lifelines)
        display_question(incorrect_questions[i], incorrect_options[i], incorrect_correct_pos[i], 0, true);
    }

    cout << "\nReview complete!\n";
    cout << "Press Enter to continue...";
    cin.get();
}

// ======================= QUESTION DISPLAY ============================

// ---------- Display a question with options and handle user interaction ----------
void display_question(string question, string options[], int correct_pos, int q_num, bool is_review = false) {
    int map[4] = { 0,1,2,3 };  // Mapping array for shuffle tracking
    int current_timer = timer;

    // Create a copy of options for shuffling
    string shuffled_options[4];
    for (int i = 0; i < 4; i++) {
        shuffled_options[i] = options[i];
    }

    if (!is_review) {
        // Shuffle options randomly
        for (int i = 3; i > 0; i--) {
            int j = rand() % (i + 1);
            swap(shuffled_options[i], shuffled_options[j]);
            swap(map[i], map[j]);
        }
    }
    else {
        // In review mode, don't shuffle
        for (int i = 0; i < 4; i++) {
            map[i] = i;
        }
    }

    // Find new position of correct answer after shuffle
    int new_correct = 0;
    for (int i = 0; i < 4; i++)
        if (map[i] == correct_pos)
            new_correct = i;

    // Display question and options
    cout << question << "\n\n";
    for (int i = 0; i < 4; i++)
        cout << i + 1 << ") " << shuffled_options[i] << "\n";

    // If in review mode, just show answer and return
    if (is_review) {
        cout << "\n** Correct Answer: " << shuffled_options[new_correct] << " **\n";
        cout << "\nPress Enter to continue...";
        cin.get();
        return;
    }

    // Show available lifelines
    cout << "\n--- Lifelines Available ---\n";
    cout << "5) 50/50" << (lifeline_5050 ? " [AVAILABLE]" : " [USED]") << "\n";
    cout << "6) Skip Question" << (lifeline_skip ? " [AVAILABLE]" : " [USED]") << "\n";
    cout << "7) Replace Question" << (lifeline_replace ? " [AVAILABLE]" : " [USED]") << "\n";
    cout << "8) Extra Time" << (lifeline_extratime ? " [AVAILABLE]" : " [USED]") << "\n";

    int answer;
    cout << "\nYour answer (1-4) or lifeline (5-8): ";

    bool answered_in_time = get_answer_with_timer(answer, current_timer);

    // Handle lifeline choices
    if (answered_in_time) {
        if (answer == 5) {
            // Use 50/50 lifeline
            use_lifeline_5050(shuffled_options, new_correct);

            cout << "\nOptions after 50/50:\n";
            for (int i = 0; i < 4; i++) {
                cout << i + 1 << ") " << shuffled_options[i] << "\n";
            }

            cout << "\nNow answer (1-4): ";
            answered_in_time = get_answer_with_timer(answer, current_timer);

            if (!answered_in_time) {
                // Timed out after using 50/50
                cout << "Time's up after using 50/50!\n";
                int negativemark = 0;
                if (difficulty == 1) negativemark = 2;
                else if (difficulty == 2) negativemark = 3;
                else if (difficulty == 3) negativemark = 5;

                score -= negativemark;
                cout << "Negative Mark: -" << negativemark << " points\n";
                streak = 0;
                wrong_count++;

                // Store for review
                incorrect_questions[incorrect_count] = question;
                for (int i = 0; i < 4; i++) {
                    incorrect_options[incorrect_count][i] = shuffled_options[i];
                }
                incorrect_correct_pos[incorrect_count] = new_correct;
                incorrect_count++;

                cout << "\nCurrent Score: " << score << "\n";
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
                return;
            }
        }
        else if (answer == 6) {
            // Use skip lifeline
            use_lifeline_skip();
            cout << "\nPress Enter to Continue...";
            cin.ignore();
            cin.get();
            return;
        }
        else if (answer == 7) {
            // Use replace lifeline
            if (lifeline_replace) {
                use_lifeline_replace(indices[q_num], used_indices, used_count);
                replace_requested = true;
            }
            cout << "\n[Replace lifeline will load a new question in the next iteration]\n";
            cout << "Press Enter to continue...";
            cin.ignore();
            cin.get();
            return;
        }
        else if (answer == 8) {
            // Use extra time lifeline
            use_lifeline_extratime(current_timer);
            cout << "\nNow answer (1-4): ";
            answered_in_time = get_answer_with_timer(answer, current_timer);

            if (!answered_in_time) {
                // Timed out even with extra time
                cout << "Time's up even after extra time!\n";
                int negativemark = 0;
                if (difficulty == 1) negativemark = 2;
                else if (difficulty == 2) negativemark = 3;
                else if (difficulty == 3) negativemark = 5;

                score -= negativemark;
                cout << "Negative Mark: -" << negativemark << " points\n";
                streak = 0;
                wrong_count++;

                // Store for review
                incorrect_questions[incorrect_count] = question;
                for (int i = 0; i < 4; i++) {
                    incorrect_options[incorrect_count][i] = shuffled_options[i];
                }
                incorrect_correct_pos[incorrect_count] = new_correct;
                incorrect_count++;

                cout << "\nCurrent Score: " << score << "\n";
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
                return;
            }
        }
    }

    // Handle timeout scenario
    if (!answered_in_time) {
        cout << "You didn't answer in time! Correct answer: " << shuffled_options[new_correct] << "\n";

        int negativemark = 0;
        if (difficulty == 1) negativemark = 2;
        else if (difficulty == 2) negativemark = 3;
        else if (difficulty == 3) negativemark = 5;

        score -= negativemark;
        cout << "Negative Mark: -" << negativemark << " points\n";
        streak = 0;
        wrong_count++;

        // Store for review
        incorrect_questions[incorrect_count] = question;
        for (int i = 0; i < 4; i++) {
            incorrect_options[incorrect_count][i] = shuffled_options[i];
        }
        incorrect_correct_pos[incorrect_count] = new_correct;
        incorrect_count++;

        cout << "\nCurrent Score: " << score << "\n";
        cout << "Press Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }

    // Convert to 0-based index
    answer--;

    int negativemark = 0;

    // Check if answer is correct
    if (answer == new_correct) {
        cout << "\nCorrect!\n";
        score++;
        streak++;
        correct_count++;

        // Apply streak bonuses
        if (streak == 3) {
            score += 5;
            cout << "Streak Bonus +5 points!\n";
        }
        else if (streak == 5) {
            score += 15;
            cout << "Streak Bonus +15 points!\n";
            streak = 0;  // Reset after 5 streak
        }
    }
    else {
        // Wrong answer
        cout << "\nWrong! Correct answer: " << shuffled_options[new_correct] << "\n";

        // Apply negative marking based on difficulty
        if (difficulty == 1) negativemark = 2;
        else if (difficulty == 2) negativemark = 3;
        else if (difficulty == 3) negativemark = 5;

        score -= negativemark;
        cout << "Negative Mark: -" << negativemark << " points\n";
        streak = 0;
        wrong_count++;

        // Store for review
        incorrect_questions[incorrect_count] = question;
        for (int i = 0; i < 4; i++) {
            incorrect_options[incorrect_count][i] = shuffled_options[i];
        }
        incorrect_correct_pos[incorrect_count] = new_correct;
        incorrect_count++;
    }

    cout << "\nCurrent Score: " << score << "\n";
    cout << "Press Enter to continue...";
    cin.ignore();
    cin.get();
}

// ======================= HIGH SCORES ============================

// ---------- Save high score to file ----------
void save_high_score(string player, int score, string category, string difficulty) {
    ofstream fout("high_scores.txt", ios::app);  // Append mode
    if (!fout.is_open()) {
        cout << "Error opening high_scores.txt\n";
        return;
    }

    // Write in pipe-delimited format
    fout << player << "|" << score << "|" << category << "|" << difficulty << "\n";
    fout.close();
}

// ---------- Display all high scores sorted by score ----------
void show_high_scores() {
    const int maxentries = 100;
    string player[maxentries];
    int score[maxentries];
    string category[maxentries];
    string difficulty[maxentries];

    int count = 0;

    ifstream fin("high_scores.txt");
    if (!fin.is_open()) {
        cout << "No high scores found!\nPress Enter...";
        cin.ignore();
        cin.get();
        return;
    }

    // Read all high score entries
    string line;
    while (getline(fin, line) && count < maxentries) {
        string temp = line;
        player[count] = cut(temp);
        score[count] = stoi(cut(temp));
        category[count] = cut(temp);
        difficulty[count] = cut(temp);
        count++;
    }
    fin.close();

    // Bubble sort in descending order by score
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (score[j] < score[j + 1]) {
                swap(score[j], score[j + 1]);// Swapping in descending order 
                swap(player[j], player[j + 1]);
                swap(category[j], category[j + 1]);
                swap(difficulty[j], difficulty[j + 1]);
            }
        }
    }
    // Show High Score

    cout << "========================================\n";
    cout << "               HIGH SCORES\n";
    cout << "========================================\n\n";
    cout << "Player\tScore\tCategory\tDifficulty\n";

    for (int i = 0; i < count; i++) {
        cout << player[i] << "\t" << score[i] << "\t" << category[i] << "\t\t" << difficulty[i] << "\n";
    }

    cout << "\nPress Enter to return...";
    cin.ignore();
    cin.get();
}

// ======================= POST QUIZ MENU ============================

// ---------- post quiz menu ----------
void post_quiz_menu() {
    while (true) {
        system("cls");
        cout << "========================================\n";
        cout << "           QUIZ COMPLETE!\n";
        cout << "========================================\n";
        cout << "Your Final Score: " << score << "/" << sessionquestions << "\n";
        cout << "Correct Answers: " << correct_count << "\n";
        cout << "Wrong Answers: " << wrong_count << "\n\n";

        cout << "1. Review Incorrect Questions\n";
        cout << "2. Replay Quiz (New Questions)\n";
        cout << "3. Return to Main Menu\n";
        cout << "Enter choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            review_incorrect_questions();// To show incorrect questions
        }
        else if (choice == 2) {
            // Replay quiz with new questions
            streak = 0;
            timer = 15;
            incorrect_count = 0;
            correct_count = 0;
            wrong_count = 0;
            reset_lifelines();
            start_quiz();
            return;
        }
        else if (choice == 3) {
            return;
        }
        else {
            cout << "Invalid choice. Press Enter...";
            cin.ignore();
        }
    }
}

// ======================= QUIZ LOGIC ============================

// ---------- main quiz function ----------
void start_quiz() {
    switch (category) {
    case 1: filename = "science.txt"; break;
    case 2: filename = "computer.txt"; break;
    case 3: filename = "sports.txt"; break;
    case 4: filename = "history.txt"; break;
    case 5: filename = "iq.txt"; break;
    }

    int loaded = load_questions(filename);
    if (loaded == 0) {
        cout << "Failed to load questions.\nPress Enter...";
        cin.ignore();
        cin.get();
        return;
    }

    used_count = 0;
    int startindex = (difficulty - 1) * 50;
    for (int i = 0; i < 50; i++) indices[i] = startindex + i;

    shuffle_array(indices, 50);
    score = 0;

    for (int q = 0; q < sessionquestions; ) {
        system("cls");
        string line = questions[indices[q]];
        string question = cut(line);
        string a = cut(line), b = cut(line), c = cut(line), d = cut(line);
        string correct_str = cut(line);

        if (correct_str.empty() || question.empty()) {
            cout << "Error: Invalid question format!\n";
            cout << "Press Enter to skip...";
            cin.ignore();
            cin.get();
            continue;
        }

        int correct = stoi(correct_str) - 1;

        string options[4] = { a, b, c, d };

        cout << "Question " << q + 1 << " of " << sessionquestions << "\n\n";
        display_question(question, options, correct, q, false);

        if (replace_requested) {
            replace_requested = false;
            continue;
        }
        // Mark question as completed
        used_indices[used_count++] = indices[q];
        q++;
    }
    string category_str = (category == 1 ? "Science" :
        category == 2 ? "Computer" :
        category == 3 ? "Sports" :
        category == 4 ? "History" : "IQ");

    string difficulty_str = (difficulty == 1 ? "Easy" :
        difficulty == 2 ? "Medium" : "Hard");

    save_quiz_log(playername, category_str, difficulty_str, correct_count, wrong_count, score);
    save_high_score(playername, score, category_str, difficulty_str);
    post_quiz_menu();
}