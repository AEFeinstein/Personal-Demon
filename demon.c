#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define PRINT_F(...) printf(__VA_ARGS__)
#define TALLY_ACTION() do{static int t=0; t++; PRINT_F("\n    %s() %d times\n", __func__, t);}while(false)

#define INC_BOUND(base, inc, lbound, ubound) \
    do{                                   \
        if (base + inc > ubound) {        \
            base = ubound;                \
        } else if (base + inc < lbound) { \
            base = lbound;                \
        } else {                          \
            base += inc;                  \
        }                                 \
    } while(false)

// Every action modifies hunger somehow
#define HUNGER_GAINED_PER_FEEDING 15
#define HUNGER_LOST_PER_PLAY       8
#define HUNGER_LOST_PER_SCOLD      3
#define HUNGER_LOST_PER_MEDICINE   3
#define HUNGER_LOST_PER_FLUSH      3

#define OBESE_THRESHOLD        -20 // too fat (i.e. not hungry)
#define MALNOURISHED_THRESHOLD  20 // too skinny (i.e. hungry)

#define STOMACH_SIZE 5 // Max number of foods being digested

#define HAPPINESS_GAINED_PER_GAME 1 // Gained per game played

// Scolding decreases happiness, increases discipline
#define HAPPINESS_LOST_PER_SCOLDING    5
#define DISCIPLINE_GAINED_PER_SCOLDING 1

// Taking medicine makes decreases happiness
#define HAPPINESS_LOST_PER_MEDICINE      5

// Being around poop decreases happiness
#define HAPPINESS_LOST_PER_STANDING_POOP 5

// Health lost for every turn while sick
#define HEALTH_LOST_PER_SICKNESS         5

typedef struct
{
    int32_t hunger; ///< 0 hunger is perfect, positive means too hungry, negative means too full
    int32_t happy;
    int32_t discipline;
    int32_t health;
    int32_t poopCount;
    int32_t actionsTaken;
    bool isSick;
    int32_t stomach[STOMACH_SIZE];
} demon_t;

bool disciplineCheck(demon_t * pd);

const char name[] = "NAME";
bool autoMode = true;

/**
 * Feed a demon
 * Feeding makes the demon happier if it is hungry
 *
 * @param pd The demon
 */
void feedDemon(demon_t * pd)
{
    TALLY_ACTION();
    // Count feeding as an action
    INC_BOUND(pd->actionsTaken, 1, 0, INT16_MAX);

    // If the demon is sick, there's a 50% chance it refuses to eat
    if(pd->isSick && rand() % 2)
    {
        PRINT_F("%s was too sick to eat\n", name);
    }
    // If the demon is unruly, it may refuse to eat
    else if(disciplineCheck(pd))
    {
        PRINT_F("%s was too unruly eat\n", name);
    }
    else
    {
        // Make sure there's room in the stomach first
        for(int i = 0; i < STOMACH_SIZE; i++)
        {
            if(pd->stomach[i] == 0)
            {
                // If the demon eats when hungry, it gets happy, otherwise it gets sad
                INC_BOUND(pd->happy, pd->hunger,  INT32_MIN, INT32_MAX);

                // Give the food between 4 and 7 cycles to digest
                pd->stomach[i] = 3 + (rand() % 4);

                // Feeding always makes the demon less hungry
                INC_BOUND(pd->hunger, -HUNGER_GAINED_PER_FEEDING,  INT32_MIN, INT32_MAX);

                PRINT_F("%s ate the food\n", name);
                return;
            }
        }
        PRINT_F("%s was too full to eat\n", name);
    }
}

/**
 * Play with the demon
 *
 * @param pd The demon
 */
void playWithDemon(demon_t * pd)
{
    TALLY_ACTION();
    // Count playing as an action
    INC_BOUND(pd->actionsTaken, 1, 0, INT16_MAX);

    if(disciplineCheck(pd))
    {
        PRINT_F("%s was too unruly to play\n", name);
    }
    else
    {
        // Playing makes the demon happy
        INC_BOUND(pd->happy, HAPPINESS_GAINED_PER_GAME,  INT32_MIN, INT32_MAX);
        PRINT_F("You played with %s\n", name);
    }

    // Playing makes the demon hungry
    INC_BOUND(pd->hunger, HUNGER_LOST_PER_PLAY,  INT32_MIN, INT32_MAX);
}

/**
 * Scold the demon
 *
 * @param pd The demon
 */
void disciplineDemon(demon_t * pd)
{
    TALLY_ACTION();
    // Count discipline as an action
    INC_BOUND(pd->actionsTaken, 1, 0, INT16_MAX);

    // Discipline always reduces happiness
    INC_BOUND(pd->happy, -HAPPINESS_LOST_PER_SCOLDING,  INT32_MIN, INT32_MAX);

    // Discipline only increases if the demon is not sick
    if(false == pd->isSick)
    {
        INC_BOUND(pd->discipline, DISCIPLINE_GAINED_PER_SCOLDING,  INT32_MIN, INT32_MAX);
        PRINT_F("You scolded %s\n", name);
    }
    else
    {
        PRINT_F("You scolded %s, but it was sick\n", name);
    }

    // Disciplining makes the demon hungry
    INC_BOUND(pd->hunger, HUNGER_LOST_PER_SCOLD,  INT32_MIN, INT32_MAX);
}

/**
 * @brief
 *
 * @return true if the demon is being unruly (won't take action)
 */
bool disciplineCheck(demon_t * pd)
{
    if(pd->discipline < 0)
    {
        switch(pd->discipline)
        {
        case -1:
        {
            return rand() % 8 < 4;
        }
        case -2:
        {
            return rand() % 8 < 5;
        }
        case -3:
        {
            return rand() % 8 < 6;
        }
        case -4:
        {
            return rand() % 8 < 7;
        }
        default:
        {
            return true;
        }
        }
    }
    else
    {
        return false;
    }
}

/**
 * Give the demon medicine, works 7/8 times
 *
 * @param pd The demon
 */
void medicineDemon(demon_t * pd)
{
    TALLY_ACTION();
    // Giving medicine counts as an action
    INC_BOUND(pd->actionsTaken, 1, 0, INT16_MAX);

    // 7/8 chance the demon is healed
    if(rand() % 8 < 7)
    {
        PRINT_F("You gave %s medicine, and it was cured\n", name);
        pd->isSick = false;
    }
    else
    {
        PRINT_F("You gave %s medicine, but it didn't work\n", name);
    }

    // Giving medicine to the demon makes the demon hungry
    INC_BOUND(pd->happy, -HAPPINESS_LOST_PER_MEDICINE,  INT32_MIN, INT32_MAX);

    // Giving medicine to the demon makes the demon hungry
    INC_BOUND(pd->hunger, HUNGER_LOST_PER_MEDICINE,  INT32_MIN, INT32_MAX);
}

/**
 * Flush one poop
 *
 * @param pd The demon
 */
void clearPoop(demon_t * pd)
{
    TALLY_ACTION();
    // Flushing counts as an action
    INC_BOUND(pd->actionsTaken, 1, 0, INT16_MAX);

    // Clear a poop
    if(pd->poopCount > 0)
    {
        PRINT_F("You flushed a poop\n");
        INC_BOUND(pd->poopCount, -1,  INT32_MIN, INT32_MAX);
    }
    else
    {
        PRINT_F("You flushed nothing\n");
    }

    // Flushing makes the demon hungry
    INC_BOUND(pd->hunger, HUNGER_LOST_PER_FLUSH,  INT32_MIN, INT32_MAX);
}

/**
 * This is called after every action.
 * If there is poop, check if the demon becomes sick
 * If the demon is malnourished or obese, check if the demon becomes sick
 * If the demon is malnourished or obese, decrease health
 * If the demon is sick, decrease health (separately from obese / malnourised)
 * If food has been digested, make a poop
 * If health reaches zero, the demon is dead
 *
 * @param pd The demon
 */
void updateStatus(demon_t * pd)
{
    /***************************************************************************
     * Poop Status
     **************************************************************************/

    // Check if poop makes demon sick
    // 1 poop  -> 25% chance
    // 2 poop  -> 50% chance
    // 3 poop  -> 75% chance
    // 4+ poop -> 100% chance
    bool wasSick = pd->isSick;
    pd->isSick |= (rand() % 4 > (3 - pd->poopCount));
    if(!wasSick && pd->isSick)
    {
        PRINT_F("Poop made %s sick\n", name);
    }

    // Being around poop makes the demon sad
    if(pd->poopCount > 0)
    {
        INC_BOUND(pd->happy, HAPPINESS_LOST_PER_STANDING_POOP * -(pd->poopCount), INT32_MIN, INT32_MAX);
    }

    // Check if demon should poop
    for(int i = 0; i < STOMACH_SIZE; i++)
    {
        if(pd->stomach[i] > 0)
        {
            pd->stomach[i]--;
            // If the food was digested
            if(0 == pd->stomach[i])
            {
                // Make a poop
                pd->poopCount++;
                PRINT_F("%s pooped\n", name);
            }
        }
    }

    /***************************************************************************
     * Hunger Status
     **************************************************************************/

    // If the demon is too full (obese))
    if(pd->hunger < OBESE_THRESHOLD)
    {
        // 5/8 chance the demon becomes sick
        wasSick = pd->isSick;
        pd->isSick |= ((rand() % 8) >= 5);
        if(!wasSick && pd->isSick)
        {
            PRINT_F("Obesity made %s sick\n", name);
        }

        // decrease the health proportionally
        INC_BOUND(pd->health, -(OBESE_THRESHOLD - pd->hunger),  INT32_MIN, INT32_MAX);

        PRINT_F("%s lost health to obesity\n", name);
    }
    else if (pd->hunger > MALNOURISHED_THRESHOLD)
    {
        // 5/8 chance the demon becomes sick
        wasSick = pd->isSick;
        pd->isSick |= ((rand() % 8) >= 5);
        if(!wasSick && pd->isSick)
        {
            PRINT_F("Malnourishment made %s sick\n", name);
        }

        // decrease the health proportionally
        INC_BOUND(pd->health, -(pd->hunger - MALNOURISHED_THRESHOLD),  INT32_MIN, INT32_MAX);
        PRINT_F("%s lost health to malnourishment\n", name);
    }

    /***************************************************************************
     * Sick Status
     **************************************************************************/

    if(rand() % 32 == 0)
    {
        pd->isSick = true;
        PRINT_F("%s randomly got sick\n", name);
    }

    // If the demon is sick, decrease health
    if(pd->isSick)
    {
        INC_BOUND(pd->health, -HEALTH_LOST_PER_SICKNESS,  INT32_MIN, INT32_MAX);
        PRINT_F("%s lost health to sickness\n", name);
    }

    /***************************************************************************
     * Discipline Status
     **************************************************************************/

    // If unhappy, the demon might get a little less disciplined
    //  0 -> 25%
    // -1 -> 50%
    // -2 -> 75%
    // -3 -> 100%
    if(rand() % 4 < (1 - pd->happy))
    {
        INC_BOUND(pd->discipline, -1,  INT32_MIN, INT32_MAX);
        PRINT_F("%s became less disciplined\n", name);
    }

    /***************************************************************************
     * Health Status
     **************************************************************************/

    // Zero health means the demon died
    if(pd->health <= 0)
    {
        PRINT_F("%s died\n", name);
    }
}

/**
 * Print out a demon's current status
 *
 * @param pd The demon
 */
void printStats(demon_t * pd)
{
    PRINT_F("\n");
    PRINT_F("---------------\n");
    PRINT_F("  Hunger: %3d\n", pd->hunger);
    PRINT_F("  Happy : %3d\n", pd->happy);
    PRINT_F("  Discip: %3d\n", pd->discipline);
    PRINT_F("  Health: %3d\n", pd->health);
    PRINT_F("  Poop  : %3d\n", pd->poopCount);
    PRINT_F("  Action: %3d\n", pd->actionsTaken);
    PRINT_F("  Sick  : %s\n", pd->isSick ? "true" : "false");
    PRINT_F("---------------\n\n");
}

/**
 * Helper function to enable auto mode
 *
 * @return char
 */
char getInput(demon_t * pd)
{
    if(autoMode)
    {
        if (pd->health <= 0)
        {
            return 'q';
        }
        else if(pd->isSick)
        {
            return '4';
        }
        else if(pd->hunger > MALNOURISHED_THRESHOLD)
        {
            return '1';
        }
        else if(pd->poopCount > 0)
        {
            return '5';
        }
        else if(pd->hunger > MALNOURISHED_THRESHOLD - HUNGER_GAINED_PER_FEEDING)
        {
            return '1';
        }
        else if(pd->discipline < 0)
        {
            return '3';
        }
        else if(pd->hunger > 0)
        {
            return '1';
        }
        else
        {
            return '2';
        }
    }
    else
    {
        return getchar();
    }
}

/**
 * Print a menu of options, then wait for user input and perform one of the
 * actions
 *
 * @param pd
 * @return true
 * @return false
 */
bool takeAction(demon_t * pd)
{
    PRINT_F("  1. Feed demon\n");
    PRINT_F("  2. Play with demon\n");
    PRINT_F("  3. Discipline demon\n");
    PRINT_F("  4. Give medicine to demon\n");
    PRINT_F("  5. Clear poop\n");
    PRINT_F("  q. Quit\n");
    PRINT_F("  > ");

    bool invalidInput = true;
    while(invalidInput)
    {
        invalidInput = false;
        switch(getInput(pd))
        {
        case '1':
        {
            feedDemon(pd);
            break;
        }
        case '2':
        {
            playWithDemon(pd);
            break;
        }
        case '3':
        {
            disciplineDemon(pd);
            break;
        }
        case '4':
        {
            medicineDemon(pd);
            break;
        }
        case '5':
        {
            clearPoop(pd);
            break;
        }
        case 'q':
        {
            return true;
        }
        default:
        {
            PRINT_F("Pick a valid option please.\n  > ");
            invalidInput = true;
            break;
        }
        case '\r':
        case '\n':
        {
            // Ignore newlines
            invalidInput = true;
            break;
        }
        }
    }
    return false;
}

void resetDemon(demon_t * pd)
{
    memset(pd, 0, sizeof(demon_t));
    pd->health = 50;
}

/**
 * Main function, this waits for user input and manages statuses
 *
 * @return unused
 */
int main(void)
{
    srand (time(NULL));

    demon_t pd;
    resetDemon(&pd);

    autoMode = false;

    bool shouldQuit = false;
    while(!shouldQuit)
    {
        if(pd.health > 0)
        {
            printStats(&pd);
            shouldQuit = takeAction(&pd);
            updateStatus(&pd);
        }
        else
        {
            if(autoMode)
            {
                printf("%d actions taken\n", pd.actionsTaken);
                resetDemon(&pd);
            }
            else
            {
                PRINT_F("Press enter to quit\n");
                getchar();
                shouldQuit = true;
            }
        }
    }
    return 0;
}