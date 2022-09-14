#include <cstdint>
#include <string>
#include <ncurses.h>
#include <unistd.h>
#include <vector>

#include "game.h"
#include "ObjectField.h"

struct {
    vec2i pos;
    rect bounds;
    char disp_char;
    int energy;
} player;

WINDOW* main_wnd;
WINDOW* game_wnd;
rect game_area;
rect screen_area;
vec2ui cur_size;
ObjectField asteroids;
ObjectField stars;

int init()
{
    srand(time(0));
    main_wnd = initscr();
    cbreak();
    noecho();
    clear();
    refresh();
    curs_set(0);

    start_color();

    screen_area = { { 0,0 }, { 80, 24 }};
    int infopanel_height = 4;
    game_wnd = newwin( screen_area.height() - infopanel_height - 2,
                        screen_area.width() - 2,
                        screen_area.top() + 1,
                        screen_area.left() +1 );
    main_wnd = newwin(screen_area.height(), screen_area.width(), 0, 0);
    game_area = { { 0, 0 }, { screen_area.width() - 2, screen_area.height() - infopanel_height - 4 } };

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);

    wbkgd(main_wnd, COLOR_PAIR(1));

    keypad(main_wnd, true);
    keypad(game_wnd, true);
    nodelay(main_wnd, true);
    nodelay(game_wnd, true);
    

    if (!has_colors()) {
        endwin();
        printf("ERROR: Terminal does not support color.\n");
        exit(1);
    }

    return 0;
}

void run()
{
    int tick;

    player.disp_char = 'o';
    player.pos = {10, 5};

    asteroids.setBounds(game_area);
    stars.setBounds(game_area);

    int in_char = 0;
    bool exit_requested = false;
    bool game_over = false;

    /* Draw a box frame around the screen */
    wattron(main_wnd, A_BOLD);
    box(main_wnd, 0, 0);
    wattroff(main_wnd, A_BOLD);

    /* Draw divider between game and status */
    wmove(main_wnd, game_area.bot() + 3, 1);
    whline(main_wnd, '-', screen_area.width() - 2);

    /* Initial draw */
    wrefresh(main_wnd);
    wrefresh(game_wnd);

    const std::vector<std::string> story_text = {
    "Just another Monday, and you're on your way to work...",
    "When suddenly...",
    "You realize you left the oven on!",
    "Take a shortcut through that asteroid field!",
    "Get back to the house before your planet explodes!"
    };
    size_t story_part = 0;
    size_t story_position = 0;

    /* Display prompt */
    mvwprintw(main_wnd, 22, 57, "Press SPACE to skip...");

    while (1) {
        werase(game_wnd);
        in_char = wgetch(main_wnd);

        if (tick % 50 == 0)
            stars.update();

        for (auto s: stars.getData()) {
            mvwaddch(game_wnd, s.getPos().y, s.getPos().y, '.');
        }

        /* Check if position is within length */
        if (story_position < story_text[story_part].length()) {
            wattron(main_wnd, A_BOLD);
            mvwaddch(main_wnd, 20, 5 + story_position, story_text[story_part][story_position]);
            wattroff(main_wnd, A_BOLD);
            story_position++;
        }

        if (in_char == ' ') {
            story_part++;
            story_position = 0;
            mvwhline(main_wnd, 20, 1, ' ', screen_area.width() - 2);
        }
        
        else if (in_char == 'q') {
            exit_requested = true;
            break;
        }
        tick++;
        if (story_part >= story_text.size()) break;
        usleep(10000);
    }

    tick = 0;
    while(1) {
        /* Clear game window */
        werase(game_wnd);

        /* Read inputs and convert all to lowercase */
        in_char = wgetch(main_wnd);
        in_char = tolower(in_char);
        switch(in_char) {
            case 'q':
                exit_requested = true;
                break;
            case KEY_UP:
            case 'w':
            case 'i':
                if (player.pos.y > game_area.top())
                    player.pos.y--;
                break;
            case KEY_DOWN:
            case 's':
            case 'k':
                if (player.pos.y < game_area.bot())
                    player.pos.y++;
                break;
            case KEY_LEFT:
            case 'a':
            case 'j':
                if (player.pos.x > game_area.left() + 1)
                    player.pos.x--;
                break;
            case KEY_RIGHT:
            case 'd':
            case 'l':
                if (player.pos.x < game_area.right() - 2)
                    player.pos.x++;
            default: break;
        }

        /* update ObjectFields */
        if (tick % 7 == 0)
            stars.update();

        if (tick > 100 && tick % 20 == 0)
            asteroids.update();

        /* collision with asteroid */
        player.bounds = { { player.pos.x - 1, player.pos.y }, { 3, 1 } };
        for (size_t i = 0; i < asteroids.getData().size(); i++) {
            if (player.bounds.contains(asteroids.getData().at(i).getPos())) {
                asteroids.erase(i);
            }
        }

        /* Draw ObjectFields */
        for (auto s : stars.getData()) {
            mvwaddch(game_wnd, s.getPos().y, s.getPos().x, '.');
        }

        for (auto a : asteroids.getData()) {
            wattron(game_wnd, A_BOLD);
            mvwaddch(game_wnd, a.getPos().y, a.getPos().x, '*');
            wattroff(game_wnd, A_BOLD);
        }

        /* Player's body */
        wattron(game_wnd, A_BOLD);
        mvwaddch(game_wnd, player.pos.y, player.pos.x, player.disp_char);
        wattroff(game_wnd, A_BOLD);

        /* player ship */
        wattron(game_wnd, A_ALTCHARSET);
        mvwaddch(game_wnd, player.pos.y, player.pos.x - 1, ACS_LARROW);
        mvwaddch(game_wnd, player.pos.y, player.pos.x + 1, ACS_RARROW);
        
        /* jet flame */
        if ( (tick % 10) / 3) {
            wattron(game_wnd, COLOR_PAIR(tick % 2 ? 3 : 4));
            mvwaddch(game_wnd, player.pos.y + 1, player.pos.x, ACS_UARROW);
            wattroff(game_wnd, COLOR_PAIR(tick % 2 ? 3 : 4));
        }

        wattroff(game_wnd, A_ALTCHARSET);

        wrefresh(main_wnd);
        wrefresh(game_wnd);

        if(exit_requested || game_over) break;

        tick++;
        usleep(10000);
    }
}

void close() {
    delwin(main_wnd);
    delwin(game_wnd);
    endwin();
}