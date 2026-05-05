#include "menu_system.h"

int main(void)
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
        {1, 0, 0, 15}  // saving
    };

    for (int row = 0; row < 9; row++)
    {

        menu_system_update(&menu, actions[row][0], actions[row][1], actions[row][2], actions[row][3]);
    }
    return 0;
}