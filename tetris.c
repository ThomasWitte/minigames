#include <string.h>
#include <stdlib.h>
#include <allegro.h>

#define CBLACK makecol(0,0,0)
#define CWHITE makecol(255,255,255)
#define CRED makecol(255,0,0)
#define CGREEN makecol(0,255,0)
#define CBLUE makecol(0,0,255)
#define CORA makecol(255,128,0)
#define CPRPL makecol(255,0,255)

#define MSG_TETRIS_EVENT MSG_USER

#define RRIGHT 0
#define RLEFT 1
#define MRIGHT 2
#define MLEFT 3
#define TIMER_EVENT 4

void handle_left(void);
void handle_right(void);
void handle_rotate(int dir);
void handle_timer(void);

int tetris_view(int msg, DIALOG *d, int c);
int push_button(int msg, DIALOG *d, int c);
int rotate_watcher(int msg, DIALOG *d, int c);

void close_handler(void) {
    exit(0);
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));

    //initialize allegro
    allegro_init();
    install_keyboard();
    install_joystick(JOY_TYPE_AUTODETECT);
    install_mouse();
    install_timer();
    set_close_button_callback(close_handler);
    LOCK_FUNCTION(handle_timer);
    install_int_ex(handle_timer, BPS_TO_TIMER(2));

    //setup window
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT, 1366, 768, 0, 0);

    DIALOG ui[] = {
        //proc          x               y       w               h           fg      bg      key     flags   d1      d2      dp…
        {tetris_view,   100,            0,      SCREEN_W-200,   SCREEN_H,   0,      0,      0,      0,      0,      0,      NULL, NULL, NULL},
        {push_button,   0,              0,      100,            SCREEN_H,   CBLACK, CWHITE, 0,      0,      0,      0,      "<", handle_left, NULL},
        {push_button,   SCREEN_W-100,   0,      100,            SCREEN_H,   CBLACK, CWHITE, 0,      0,      0,      0,      ">", handle_right, NULL},
        {rotate_watcher,0,              0,      0,              0,          0,      0,      0,      0,      0,      0,      handle_rotate, NULL, NULL},
        {d_button_proc, SCREEN_W-200,   0,      100,            100,        CWHITE, CRED,   0,      D_EXIT, 0,      0,      "X", NULL, NULL},
        {NULL,          0,              0,      0,              0,          0,      0,      0,      0,      0,      0,      NULL, NULL, NULL}
    };

    //check for joystick
    if(num_joysticks <= 0 || joy[0].num_sticks <= 0 || joy[0].stick[0].num_axis <= 1) {
        allegro_message("could not find joystick!");
        return -1;
    }

    do_dialog(ui, -1);
    return 0;
}

void handle_left(void) {
    broadcast_dialog_message(MSG_TETRIS_EVENT, MLEFT);
}

void handle_right(void) {
    broadcast_dialog_message(MSG_TETRIS_EVENT, MRIGHT);
}

void handle_rotate(int dir) {
    broadcast_dialog_message(MSG_TETRIS_EVENT, dir);
}

void handle_timer(void) {
    broadcast_dialog_message(MSG_TETRIS_EVENT, TIMER_EVENT);
}
END_OF_FUNCTION(handle_timer)

int push_button(int msg, DIALOG *d, int c) {
    int ret = D_O_K;
    
    d->flags |= D_EXIT;
    ret |= d_button_proc(msg, d, c);
    
    if(ret & D_CLOSE) {
    	ret &= ~D_CLOSE;

        if(d->dp2)
            ((void(*)(void))d->dp2)();
    }
    return ret;
}

int rotate_watcher(int msg, DIALOG *d, int c) {
    if(msg == MSG_IDLE) {
        //poll_joystick();
        int pos = joy[0].stick[0].axis[0].d1 +
                2*joy[0].stick[0].axis[0].d2 +
                4*joy[0].stick[0].axis[1].d1 +
                8*joy[0].stick[0].axis[1].d2;

        if(pos == d->d2)
            switch(pos) {
                case 1: //right
                if(d->d1 == 8) //right rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RLEFT);
                if(d->d1 == 4) //left rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RRIGHT);
                d->d1 = pos;
                break;
                case 2: //left
                if(d->d1 == 4) //right rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RLEFT);
                if(d->d1 == 8) //left rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RRIGHT);
                d->d1 = pos;
                break;
                case 4: //180
                if(d->d1 == 1) //right rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RLEFT);
                if(d->d1 == 2) //left rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RRIGHT);
                d->d1 = pos;
                break;
                case 8: //normal
                if(d->d1 == 2) //right rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RLEFT);
                if(d->d1 == 1) //left rotation
                    broadcast_dialog_message(MSG_TETRIS_EVENT, RRIGHT);
                d->d1 = pos;
                break;
            }

        d->d2 = pos;
    }
    return D_O_K;
}

typedef struct {
    int data[16];
} BLOCK;

int tetris_view(int msg, DIALOG *d, int c) {
    BLOCK blocks[] = {
        { CWHITE, CWHITE, CWHITE, CWHITE,
          CWHITE, CBLACK, CBLACK, CWHITE,
          CWHITE, CBLACK, CBLACK, CWHITE,
          CWHITE, CWHITE, CWHITE, CWHITE },

        { CWHITE, CRED,   CWHITE, CWHITE,
          CWHITE, CRED,   CWHITE, CWHITE,
          CWHITE, CRED,   CRED,   CWHITE,
          CWHITE, CWHITE, CWHITE, CWHITE },

        { CWHITE, CBLUE, CWHITE, CWHITE,
          CWHITE, CBLUE, CWHITE, CWHITE,
          CWHITE, CBLUE, CWHITE, CWHITE,
          CWHITE, CBLUE, CWHITE, CWHITE },

        { CWHITE, CWHITE, CWHITE, CWHITE,
          CGREEN, CGREEN, CWHITE, CWHITE,
          CWHITE, CGREEN, CGREEN, CWHITE,
          CWHITE, CWHITE, CWHITE, CWHITE },

        { CWHITE, CWHITE, CWHITE, CWHITE,
          CWHITE, CWHITE, CORA,   CORA,
          CWHITE, CORA, CORA, CWHITE,
          CWHITE, CWHITE, CWHITE, CWHITE },

        { CWHITE, CWHITE, CPRPL, CWHITE,
          CWHITE, CWHITE, CPRPL, CWHITE,
          CWHITE, CPRPL, CPRPL, CWHITE,
          CWHITE, CWHITE, CWHITE, CWHITE }
    };

    switch(msg) {

    case MSG_START:
    {
        d->dp = malloc(200*sizeof(int));
        int i;
        for(i = 0; i < 200; i++) {
            ((int*)d->dp)[i] = CWHITE;
        }
        d->dp2 = memcpy(malloc(16*sizeof(int)), &blocks[random()%6], 16*sizeof(int));
        d->d1 = 3;
        d->d2 = 0;
    }
    break;

    case MSG_END:
        if(d->dp)
            free(d->dp);
        if(d->dp2)
            free(d->dp2);
    break;

    case MSG_TETRIS_EVENT:
        if(d->flags & D_DISABLED)
            break;

        switch(c) {
        case RRIGHT:
        {
            int i;
            BLOCK temp;
            int poss = 1;

            for(i = 0; i < 16; i++) {
                temp.data[i + 3 - 5*(i/4) + 3*(i%4)] = ((int*)d->dp2)[i];
            }

            for(i = 0; i < 16; i++)
                if(temp.data[i] != CWHITE)
                    if(i%4 + d->d1 > 9 ||
                        i%4 + d->d1 < 0 ||
                        i/4 + d->d2 > 19 ||
                        ((int*)d->dp)[10*(d->d2+i/4) + i%4 + d->d1] != CWHITE) {
                        poss = 0;
                        break;
                    }

            if(poss) {
                memcpy(d->dp2, &temp, 16*sizeof(int));
            }
        }
        break;

        case RLEFT:
        {
            int i;
            BLOCK temp;
            int poss = 1;

            for(i = 0; i < 16; i++) {
                temp.data[i + 12 - 5*(i%4) - 3*(i/4)] = ((int*)d->dp2)[i];
            }

            for(i = 0; i < 16; i++)
                if(temp.data[i] != CWHITE)
                    if(i%4 + d->d1 > 9 ||
                        i%4 + d->d1 < 0 ||
                        i/4 + d->d2 > 19 ||
                        ((int*)d->dp)[10*(d->d2+i/4) + i%4 + d->d1] != CWHITE) {
                        poss = 0;
                        break;
                    }

            if(poss) {
                memcpy(d->dp2, &temp, 16*sizeof(int));
            }
        }
        break;

        case MRIGHT:
        {
            int i;
            int poss = 1;
            for(i = 0; i < 16; i++)
                if(((int*)d->dp2)[i] != CWHITE)
                    if(i%4 + d->d1 >= 9 || ((int*)d->dp)[10*(i/4 + d->d2) + i%4 + d->d1 + 1] != CWHITE) {
                        poss = 0;
                        break;
                    }

            if(poss)
                d->d1++;
        }
        break;

        case MLEFT:
        {
            int i;
            int poss = 1;
            for(i = 0; i < 16; i++)
                if(((int*)d->dp2)[i] != CWHITE)
                    if(i%4 + d->d1 <= 0 || ((int*)d->dp)[10*(i/4 + d->d2) + i%4 + d->d1 - 1] != CWHITE) {
                        poss = 0;
                        break;
                    }

            if(poss)
                d->d1--;
        }
        break;

        case TIMER_EVENT:
        {
            int i,j;
            int poss = 1;
            for(i = 0; i < 16; i++)
                if(((int*)d->dp2)[i] != CWHITE)
                    if(i/4 + d->d2 >= 19 || ((int*)d->dp)[10*(i/4 + 1 + d->d2) + i%4 + d->d1] != CWHITE) {
                        poss = 0;
                        break;
                    }

            if(poss) {
                d->d2++;
            } else {
                for(i = 0; i < 16; i++)
                    if(((int*)d->dp2)[i] != CWHITE) {
                        ((int*)d->dp)[10*(i/4 + d->d2) + i%4 + d->d1] = ((int*)d->dp2)[i];
                    }

                for(i = 10; i < 20; i++)
                    if(((int*)d->dp)[i] != CWHITE) {
                        d->flags |= D_DISABLED;
                        break;
                    }

                for(i = 0; i < 20; i++) {
                    poss = 1;

                    for(j = 0; j < 10; j++)
                        if(((int*)d->dp)[i*10 + j] == CWHITE)
                            poss = 0;

                    if(poss == 1) {
                        memmove(d->dp + 10*sizeof(int), d->dp, 10*i*sizeof(int));
                    }
                }

                memcpy(d->dp2, &blocks[random()%6], 16*sizeof(int));
                d->d1 = 3;
                d->d2 = 0;
            }
        }
        break;

        }

    case MSG_DRAW:
    {
        if(d->flags & D_DISABLED) {
            //show gameover-screen
            BITMAP *go = create_bitmap(72,8);
            textout_ex(go, font, "GAME OVER", 0, 1, CRED, CBLACK);
            stretch_blit(go, gui_get_screen(), 0, 0, 72, 8, d->x+d->w/2-360, d->y+d->h/2-120, 720, 240);
            destroy_bitmap(go);
            break;
        }

        int i;
        for(i = 0; i < 200; i++) {
            rectfill(gui_get_screen(),
                d->x + (d->w-320)/2 + (i%10)*32, d->y + (d->h-640)/2 + (i/10)*32,
                d->x + (d->w-320)/2 + (i%10)*32 + 32, d->y + (d->h-640)/2 + (i/10)*32 + 32, ((int*)d->dp)[i]);
        }

        for(i = 0; i < 16; i++) {
            if(((int*)d->dp2)[i] != CWHITE)
                rectfill(gui_get_screen(),
                    d->x + (d->w-320)/2 + (d->d1 + i%4)*32, d->y + (d->h-640)/2 + (d->d2 + i/4)*32,
                    d->x + (d->w-320)/2 + (d->d1 + i%4)*32 + 32, d->y + (d->h-640)/2 + (d->d2 + i/4)*32 + 32, ((int*)d->dp2)[i]);
        }
    }
    break;

    }

    return D_O_K;
}
