#include "menu_system.h"
#include <unistd.h>
#include <stdio.h>

void simple_testing()
{
    MenuSystem_t menu;

    menu_system_init(&menu, 0, 0);

    int actions[][4] = {
        {0, 0, 1, 15}, // down and 15 ms interval
        {0, 0, 1, 15}, // down and 15 ms interval
        {0, 1, 0, 15}, // up and 15 ms interval
        {0, 1, 0, 15}, // up and 15 ms interval
        {1, 0, 0, 15}, // enter on settings and 15ms interval
        {1, 0, 0, 15}, // enter on volume
        {0, 1, 0, 15}, // volume up
        {0, 1, 0, 15}, // volume up
        {1, 0, 0, 15}, // saving
        {0, 0, 1, 15},
        {0, 0, 1, 15},
        {1, 0, 0, 15}};

    for (int row = 0; row < 12; row++)
    {

        menu_system_update(&menu, actions[row][0], actions[row][1], actions[row][2], actions[row][3]);
    }
}

void testing_with_key_repeat()
{
    MenuSystem_t menu;

    // menu_system_init(&menu, 500, 100);

    // menu_system_init(&menu, 100, 30);

    menu_system_init(&menu, 800, 200);

    printf("=== TESTING KEY REPEAT ===\n");
    printf("Simulating holding DOWN button for 2 seconds\n\n");

    printf("Step 1: Press Enter to enter Settings\n");
    menu_system_update(&menu, 1, 0, 0, 10);
    usleep(100000);

    printf("Step 2: Press Enter to edit Volume\n");
    menu_system_update(&menu, 1, 0, 0, 10);
    usleep(100000);

    printf("Step 3: HOLDING UP button...\n");

    // simulate holding UP button for 2 seconds (2000ms)
    // Each call is 10ms interval
    for (int i = 0; i < 200; i++)
    {
        // UP pressed = true, DOWN = false, ENTER = false
        menu_system_update(&menu, 0, 1, 0, 10);
        usleep(10000);

        if (i % 10 == 0)
        {
            printf(" Volume: %d\n", menu.volume);
        }
    }

    printf("\nFinal volume: %d\n", menu.volume);
    printf("Expected: around 15-20 increments (500ms delay + 150ms of repeats at 100ms each)\n");
}

int main(void)
{
    simple_testing();
    return 0;
}