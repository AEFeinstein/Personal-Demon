/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

/*******************************************************************************
 * Defines
 ******************************************************************************/

#define lengthof(x) (sizeof(x) / sizeof(x[0]))

#define SQUARE(x) ((x)*(x))

#define PRINT_F(...) do{if(!autoMode){printf(__VA_ARGS__);}}while(false)
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

/*******************************************************************************
 * Structs
 ******************************************************************************/

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
    char name[16];
} demon_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void namegen(char * name, int namelen);
void feedDemon(demon_t * pd);
void playWithDemon(demon_t * pd);
void disciplineDemon(demon_t * pd);
bool disciplineCheck(demon_t * pd);
void medicineDemon(demon_t * pd);
void clearPoop(demon_t * pd);
void updateStatus(demon_t * pd);
void printStats(demon_t * pd);
char getInput(demon_t * pd);
bool takeAction(demon_t * pd);
void resetDemon(demon_t * pd);

/*******************************************************************************
 * Variables
 ******************************************************************************/

bool autoMode = true;

// const char *nm1[] = {"", "b", "br", "d", "dr", "g", "j", "k", "m", "r", "s", "t", "th", "tr", "v", "x", "z"};
// const char *nm2[] = {"a", "e", "i", "o", "u"};
// const char *nm3[] = {"g", "g'dr", "g'th", "gdr", "gg", "gl", "gm", "gr", "gth", "k", "l'g", "lg", "lgr", "llm", "lm", "lr", "lv", "n", "ngr", "nn", "r", "r'", "r'g", "rg", "rgr", "rk", "rn", "rr", "rthr", "rz", "str", "th't", "z", "z'g", "zg", "zr", "zz"};
// const char *nm4[] = {"a", "e", "i", "o", "u", "iu", "uu", "au", "aa"};
// const char *nm5[] = {"d", "k", "l", "ll", "m", "n", "nn", "r", "th", "x", "z"};
// const char *nm6[] = {"ch", "d", "g", "k", "l", "n", "r", "s", "th", "z"};
const char *nm1[] = {"", "", "", "", "b", "br", "d", "dr", "g", "j", "k", "m", "r", "s", "t", "th", "tr", "v", "x", "z"};
const char *nm2[] = {"a", "e", "i", "o", "u", "a", "a", "o", "o"};
const char *nm3[] = {"g", "g'dr", "g'th", "gdr", "gg", "gl", "gm", "gr", "gth", "k", "l'g", "lg", "lgr", "llm", "lm", "lr", "lv", "n", "ngr", "nn", "r", "r'", "r'g", "rg", "rgr", "rk", "rn", "rr", "rthr", "rz", "str", "th't", "z", "z'g", "zg", "zr", "zz"};
const char *nm4[] = {"a", "e", "i", "o", "u", "a", "a", "o", "o", "a", "e", "i", "o", "u", "a", "a", "o", "o", "a", "e", "i", "o", "u", "a", "a", "o", "o", "a", "e", "i", "o", "u", "a", "a", "o", "o", "a", "e", "i", "o", "u", "a", "a", "o", "o", "iu", "uu", "au", "aa"};
const char *nm5[] = {"d", "k", "l", "ll", "m", "m", "m", "n", "n", "n", "nn", "r", "r", "r", "th", "x", "z"};
const char *nm6[] = {"ch", "d", "g", "k", "l", "n", "n", "n", "n", "n", "r", "s", "th", "th", "th", "th", "th", "z"};

/*******************************************************************************
 * Functions
 ******************************************************************************/

/**
 * @brief Randomly generate a demon name
 *
 * @param name    A pointer to store the name in
 * @param namelen The length of the name
 */
void namegen(char * name, int namelen)
{
    int nTp = rand() % 3;
    int rnd = rand() % lengthof(nm1);
    int rnd2 = rand() % lengthof(nm2);
    int rnd3 = rand() % lengthof(nm6);
    int rnd4 = rand() % lengthof(nm3);
    int rnd5 = rand() % lengthof(nm4);
    while (nm3[rnd4] == nm1[rnd] || nm3[rnd4] == nm6[rnd3])
    {
        rnd4 = rand() % lengthof(nm3);
    }
    if (nTp == 0)
    {
        strlcat(name, nm1[rnd], namelen);
        strlcat(name, nm2[rnd2], namelen);
        strlcat(name, nm3[rnd4], namelen);
        strlcat(name, nm4[rnd5], namelen);
        strlcat(name, nm6[rnd3], namelen);
    }
    else
    {
        int rnd6 = rand() % lengthof(nm2);
        int rnd7 = rand() % lengthof(nm5);
        while (nm5[rnd7] == nm3[rnd4] || nm5[rnd7] == nm6[rnd3])
        {
            rnd7 = rand() % lengthof(nm5);
        }
        strlcat(name, nm1[rnd], namelen);
        strlcat(name, nm2[rnd2], namelen);
        strlcat(name, nm3[rnd4], namelen);
        strlcat(name, nm2[rnd6], namelen);
        strlcat(name, nm5[rnd7], namelen);
        strlcat(name, nm4[rnd5], namelen);
        strlcat(name, nm6[rnd3], namelen);
    }
    // testSwear(nMs);
}

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
        PRINT_F("%s was too sick to eat\n", pd->name);
    }
    // If the demon is unruly, it may refuse to eat
    else if(disciplineCheck(pd))
    {
        PRINT_F("%s was too unruly eat\n", pd->name);
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

                PRINT_F("%s ate the food\n", pd->name);
                return;
            }
        }
        PRINT_F("%s was too full to eat\n", pd->name);
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
        PRINT_F("%s was too unruly to play\n", pd->name);
    }
    else
    {
        // Playing makes the demon happy
        INC_BOUND(pd->happy, HAPPINESS_GAINED_PER_GAME,  INT32_MIN, INT32_MAX);
        PRINT_F("You played with %s\n", pd->name);
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
        PRINT_F("You scolded %s\n", pd->name);
    }
    else
    {
        PRINT_F("You scolded %s, but it was sick\n", pd->name);
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
        PRINT_F("You gave %s medicine, and it was cured\n", pd->name);
        pd->isSick = false;
    }
    else
    {
        PRINT_F("You gave %s medicine, but it didn't work\n", pd->name);
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
        PRINT_F("Poop made %s sick\n", pd->name);
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
                PRINT_F("%s pooped\n", pd->name);
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
            PRINT_F("Obesity made %s sick\n", pd->name);
        }

        // decrease the health proportionally
        INC_BOUND(pd->health, -(OBESE_THRESHOLD - pd->hunger),  INT32_MIN, INT32_MAX);

        PRINT_F("%s lost health to obesity\n", pd->name);
    }
    else if (pd->hunger > MALNOURISHED_THRESHOLD)
    {
        // 5/8 chance the demon becomes sick
        wasSick = pd->isSick;
        pd->isSick |= ((rand() % 8) >= 5);
        if(!wasSick && pd->isSick)
        {
            PRINT_F("Malnourishment made %s sick\n", pd->name);
        }

        // decrease the health proportionally
        INC_BOUND(pd->health, -(pd->hunger - MALNOURISHED_THRESHOLD),  INT32_MIN, INT32_MAX);
        PRINT_F("%s lost health to malnourishment\n", pd->name);
    }

    /***************************************************************************
     * Sick Status
     **************************************************************************/

    if(rand() % 32 == 0)
    {
        pd->isSick = true;
        PRINT_F("%s randomly got sick\n", pd->name);
    }

    // If the demon is sick, decrease health
    if(pd->isSick)
    {
        INC_BOUND(pd->health, -HEALTH_LOST_PER_SICKNESS,  INT32_MIN, INT32_MAX);
        PRINT_F("%s lost health to sickness\n", pd->name);
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
        PRINT_F("%s became less disciplined\n", pd->name);
    }

    /***************************************************************************
     * Health Status
     **************************************************************************/

    // Zero health means the demon died
    if(pd->health <= 0)
    {
        PRINT_F("%s died\n", pd->name);
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
    PRINT_F("  1. Feed %s\n", pd->name);
    PRINT_F("  2. Play with %s\n", pd->name);
    PRINT_F("  3. Discipline %s\n", pd->name);
    PRINT_F("  4. Give medicine to %s\n", pd->name);
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

/**
 * @brief Initialize the demon
 *
 * @param pd The demon to initialize
 */
void resetDemon(demon_t * pd)
{
    memset(pd, 0, sizeof(demon_t));
    pd->health = 50;
    namegen(pd->name, sizeof(pd->name)-1);
    pd->name[0] -= ('a' - 'A');

    PRINT_F("%s fell out of a portal\n", pd->name);
}

/**
 * Main function, this waits for user input and manages statuses
 *
 * @return unused
 */
int main(void)
{
    autoMode = true;

    // Seed the RNG
    srand(time(NULL));

    // Setup a demon for managing
    demon_t pd;
    resetDemon(&pd);

    // Set up space to save all the results
    demon_t autoModeDemons[10000] = {0};
    uint32_t autoModeResultIdx = 0;

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
                // Save the results
                memcpy(&autoModeDemons[autoModeResultIdx++], &pd, sizeof(pd));
                resetDemon(&pd);

                // If all results have been collected
                if(autoModeResultIdx == lengthof(autoModeDemons))
                {
                    // Find the average for all stats
                    int32_t len = lengthof(autoModeDemons);
                    demon_t avgDemon = {0};
                    for(uint32_t i = 0; i < lengthof(autoModeDemons); i++)
                    {
                        avgDemon.hunger += autoModeDemons[i].hunger;
                        avgDemon.happy += autoModeDemons[i].happy;
                        avgDemon.discipline += autoModeDemons[i].discipline;
                        avgDemon.health += autoModeDemons[i].health;
                        avgDemon.poopCount += autoModeDemons[i].poopCount;
                        avgDemon.actionsTaken += autoModeDemons[i].actionsTaken;
                    }
                    avgDemon.hunger /= len;
                    avgDemon.happy /= len;
                    avgDemon.discipline /= len;
                    avgDemon.health /= len;
                    avgDemon.poopCount /= len;
                    avgDemon.actionsTaken /= len;

                    // Find the standard deviation for all stats
                    demon_t stdevDemon = {0};
                    for(uint32_t i = 0; i < lengthof(autoModeDemons); i++)
                    {
                        stdevDemon.hunger += SQUARE(autoModeDemons[i].hunger - avgDemon.hunger);
                        stdevDemon.happy += SQUARE(autoModeDemons[i].happy - avgDemon.happy);
                        stdevDemon.discipline += SQUARE(autoModeDemons[i].discipline - avgDemon.discipline);
                        stdevDemon.health += SQUARE(autoModeDemons[i].health - avgDemon.health);
                        stdevDemon.poopCount += SQUARE(autoModeDemons[i].poopCount - avgDemon.poopCount);
                        stdevDemon.actionsTaken += SQUARE(autoModeDemons[i].actionsTaken - avgDemon.actionsTaken);
                    }
                    stdevDemon.hunger = sqrt(stdevDemon.hunger / len);
                    stdevDemon.happy = sqrt(stdevDemon.happy / len);
                    stdevDemon.discipline = sqrt(stdevDemon.discipline / len);
                    stdevDemon.health = sqrt(stdevDemon.health / len);
                    stdevDemon.poopCount = sqrt(stdevDemon.poopCount / len);
                    stdevDemon.actionsTaken = sqrt(stdevDemon.actionsTaken / len);

                    // Print everything
                    printf("             %4s %4s\n", "Avg", "Std");
                    printf("hunger       %4d %4d\n", avgDemon.hunger,       stdevDemon.hunger);
                    printf("happy        %4d %4d\n", avgDemon.happy,        stdevDemon.happy);
                    printf("discipline   %4d %4d\n", avgDemon.discipline,   stdevDemon.discipline);
                    printf("health       %4d %4d\n", avgDemon.health,       stdevDemon.health);
                    printf("poopCount    %4d %4d\n", avgDemon.poopCount,    stdevDemon.poopCount);
                    printf("actionsTaken %4d %4d\n", avgDemon.actionsTaken, stdevDemon.actionsTaken);

                    printf("\nPress enter to quit\n");
                    getchar();
                    return 0;
                }
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
