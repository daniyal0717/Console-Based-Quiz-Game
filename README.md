# Console-Based Quiz Game (C++)

A console-based quiz game developed in C++ using Visual Studio.  
The game allows players to test their knowledge across multiple categories with
difficulty levels, lifelines, timers, and persistent score tracking.

---

## Features

- Multiple quiz categories:
  - Science
  - Computer
  - Sports
  - History
  - IQ / Logic
- Three difficulty levels: Easy, Medium, Hard
- Timed questions with countdown display
- Lifelines:
  - 50/50
  - Skip Question
  - Replace Question
  - Extra Time
- Streak-based bonus scoring
- Negative marking for incorrect or timed-out answers
- Review incorrect answers after quiz completion
- High score leaderboard with file-based persistence
- Quiz session logging with timestamps

---

## Technologies Used

- C++
- Visual Studio (Windows)
- File handling (`.txt` files)
- Windows Console API
- Non-blocking keyboard input

---

## How to Run

1. Clone or download the repository
2. Open `PF LAB PROJECT QUIZ GAME.sln` in **Visual Studio**
3. Build and run the project
4. Make sure all required `.txt` files are present in the same directory as the executable

---

## Required Files

The following files are required for the game to function correctly:

- `science.txt`
- `computer.txt`
- `sports.txt`
- `history.txt`
- `iq.txt`
- `high_scores.txt`
- `quiz_logs.txt`

### Question File Format

Question|Option1|Option2|Option3|Option4|CorrectAnswerNumber

Example:
What is 2+2?|1|2|3|4|4

---

## Authors

- Daniyal Khalid  
- M. Hassan  
- Faris Ahmed  

---

## Notes

- This project is Windows-specific due to the use of `windows.h` and `conio.h`
- Designed as a Programming Fundamentals / PF Lab project
