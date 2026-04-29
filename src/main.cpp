#include <iostream>
#include <string>
#include <cstdlib> // added to recognise srand/rand code
#include <ctime>    // added to recognise time code
#include <limits>

class Actor {
public:
    Actor(int _health, int _attackpower) : health(_health), posX(0.0f), posY(0.0f), targetHealth(nullptr), AttackPower(_attackpower) {} // .text: Constructor code
    ~Actor() {} // .text: Destructor code
    void Attack(Actor* _target, int _attackpower) {
        _target->health -= _attackpower;
    }

    std::string name;  // Heap: String data on heap; Stack: Object management on stack
    int health;        // Stack: Part of Actor on stack
    float posX, posY;  // Stack: Part of Actor on stack
    int* targetHealth; // Stack: Pointer on stack, points to heap/stack
    int AttackPower;
};

// Initialize Actors with 30 health
Actor* player = new Actor(30, 10); //***** assigned them to new actor with preallocated health
Actor* enemy = new Actor(30, 10);

int playerAttackChoice;

void GetPlayerChoice() {
    // Get player choice with basic validation
    std::cout << "Enter Attack (1 = Fire, 2 = Grass, 3 = Water): ";
    std::cin >> playerAttackChoice;
}

bool DidPlayerWin(int _playerChoice, int _enemyChoice) {
    // 1 = Fire, 2 = Grass, 3 = Water
    // Simple input validation
    if (_playerChoice == _enemyChoice) { //**** Enemy wins by default. == check for equality
        return false;
    }

    // Player wins: Fire > Grass, Grass > Water, Water > Fire
    if ((_playerChoice == 1 && _enemyChoice == 2) ||  // Fire beats Grass
        (_playerChoice == 2 && _enemyChoice == 3) ||  // Grass beats Water
        (_playerChoice == 3 && _enemyChoice == 1)) {  // Water beats Fire
        return true;
    }
    return false;
}

bool IsInputValid(int _playerChoice) { //checking player input against a range 0<x>4, which is 1 to 3
    return (_playerChoice > 0 && _playerChoice < 4); // true if inbetween 0 and 4, false otherwise
}

int main()
{
    // Initialize random seed once
    srand(time(0));  // Safe cast for portability
    
    // Game Loop
    while (player->health > 0 && enemy->health > 0)
    {       
        do
        {
            //get player input
            GetPlayerChoice();

            // TODO: Validate input
            if (!IsInputValid(playerAttackChoice)) 
            { //! means NOT, here it is used to invert the bool value
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cout << "Player Input Invalid. Please enter 1, 2, or 3.\n";
            }
        } while (!IsInputValid(playerAttackChoice));

        // Enemy choice
        int enemyChoice = rand() % 3 + 1;  //**** 1 to 3 instead of 0 to 2

        // Display choices
        std::cout << "Player used Attack: " << playerAttackChoice
            << ", Enemy used Attack: " << enemyChoice << "\n";

        // Determine winner and apply damage
        if (DidPlayerWin(playerAttackChoice, enemyChoice)) 
        {
            std::cout << "Player wins the round!\n";
            player->Attack(enemy, player->AttackPower);  // Player wins, enemy takes damage
        }
        else 
        { // enemy win
            std::cout << "Enemy damages Player!\n";
            enemy->Attack(player, enemy->AttackPower);
        }

        // Show health
        std::cout << "\n==========================================================\n"
            << " | Player Health : " << player->health
            << " | Enemy Health: " << enemy->health
            << "\n=========================================================="
            << "\n\n";
        
    }
    // Declare winner
    if (player->health > 0 && enemy->health <= 0) {
        std::cout << "Player Wins!!\n";
    }
    else if (enemy->health > 0 && player->health <= 0) {
        std::cout << "Enemy Wins!!\n";
    }
    else {
        std::cout << "It's a draw!\n"; // Both hit 0
    }

    // Clean up
    delete enemy;  // Free enemy memory
    delete player; // Free player memory
    
}