#include <stdlib.h>
#include <allegro.h>
#include <sched.h>
#include <math.h>
#include <time.h>

volatile int timer = 0;

void timer_increment() {
    ++timer;
}
END_OF_FUNCTION(timer_increment)

typedef struct {
    int x, y;
} Koordinate;


Koordinate asteroid_shape[] = {
    {-20,20},
    {20,-20}, //Collision Rectangle

    {5,25}, //Shape
    {25,15},
    {25,-5},
    {10,-10},
    {20,-20},
    {-5,-25},
    {-15,-10},
    {-20,-15},
    {-25,5},
    {-7,20},
    {0,1},
    {5,25},
    {0,0}
};

Koordinate spaceship_shape[] = {
    {30,-10},
    {-10,10},

    {-5,-12},
    {30,0},
    {-5,12},
    {-5,-12},
    {10,-15},
    {-10,10},
    {-10,-10},
    {10,15},
    {-5,12},
    {0,0}
};

Koordinate bullet_shape[] = {
    {-1,1},
    {1,-1},
    {-3,0},
    {2,0},
    {0,0}
};

Koordinate particle_shape[] = {
    {1,1},
    {1,1},
    {1,1},
    {1,1},
    {0,0}
};

struct Object {
    float x, y;
    float dir;
    float move_dir;
    float speed;
    float size;
    Koordinate *shape;
    struct Object *next;
    int lifetime;
};
typedef struct Object Object;

typedef struct {
    int end, swait;
    Object player;
    Object *asteroids;
    Object *shot;
    Object *particles;
} Game;

void init_game(Game *g) {
    g->end = 0;
    g->swait = 0;
    g->player.speed = 0.0;
    g->player.dir = 0.0;
    g->player.x = 360.0;
    g->player.y = 240.0;
    g->player.shape = spaceship_shape;
    g->player.size = 1.0;

/*    Object **ptr = &(g->asteroids);
    for(int i = 0; i < 8; i++) {
        *ptr = malloc(sizeof(Object));
        (*ptr)->speed = 1.0;
        (*ptr)->dir = ((float)(rand()%628))/100.0;
        (*ptr)->move_dir = ((float)(rand()%628))/100.0;
        (*ptr)->x = rand()%SCREEN_W;
        (*ptr)->y = rand()%SCREEN_H;
        (*ptr)->shape = asteroid_shape;
        (*ptr)->size = 1.0;
        ptr = &((*ptr)->next);
    }
    *ptr = NULL;*/

    g->asteroids = NULL;
    for(int i = 0; i < 8; ++i) {
        Object *t = malloc(sizeof(Object));
        t->speed = 1.0;
        t->dir = ((float)(rand()%628))/100.0;
        t->move_dir = ((float)(rand()%628))/100.0;
        t->x = rand()%SCREEN_W;
        t->y = rand()%SCREEN_H;
        t->shape = asteroid_shape;
        t->size = 1.0;
        t->next = g->asteroids;
        g->asteroids = t;
    }

    g->shot = NULL;
    g->particles = NULL;
}

void delete_list(Object *o) {
    while(o) {
        Object *t = o->next;
//        free(o);
        o = t;
    }
}

void rotate(Object *o, float deg) {
    o->dir += deg;
}

void accelerate(Game *g, Object *o, float a) {
    float newdx = cos(o->move_dir)*o->speed + cos(o->dir) * a;
    float newdy = sin(o->move_dir)*o->speed + sin(o->dir) * a;
    o->speed = sqrt(newdx*newdx+newdy*newdy);
    o->move_dir = atan2(newdy, newdx);
    for(int i = 0; i < 3; i++) {
        Object *t = malloc(sizeof(Object));
        t->next = g->particles;
        t->shape = particle_shape;
        t->size = 1.0;
        t->speed = (rand()%10 + 40.0)/10.0;
        t->dir = t->move_dir = o->dir + (rand()%30 + 300.0)/100.0;
        t->x = o->x;
        t->y = o->y;
        t->lifetime = 20;
        g->particles = t;
    }
}

void shoot(Game *g) {
    if(g->swait <= 0) {
        Object *t = malloc(sizeof(Object));
        t->next = g->shot;
        t->shape = bullet_shape;
        t->size = 1.0;
        t->speed = 3.5;
        t->dir = t->move_dir = g->player.dir;
        t->x = g->player.x;
        t->y = g->player.y;
        t->lifetime = 180;
        g->shot = t;
        g->swait = 20;
    }
}

void update_object(Object *o) {
    o->x += cos(o->move_dir)*o->speed;
    o->y += sin(o->move_dir)*o->speed;

    if(o->x >= SCREEN_W)
        o->x -= SCREEN_W;
    else if(o->x < 0)
        o->x += SCREEN_W;

    if(o->y >= SCREEN_H)
        o->y -= SCREEN_H;
    else if(o->y < 0)
        o->y += SCREEN_H;

    --o->lifetime;
}

int check_collision(Object *o1, Object *o2) {
    if(!o1->shape || !o2->shape)
        return 0;
    Koordinate vert[] = {
        {o1->shape[0].x, o1->shape[0].y},
        {o1->shape[0].x, o1->shape[1].y},
        {o1->shape[1].x, o1->shape[1].y},
        {o1->shape[1].x, o1->shape[0].y}
    };
    Koordinate vert2[] = {
        {o2->shape[0].x, o2->shape[0].y},
        {o2->shape[0].x, o2->shape[1].y},
        {o2->shape[1].x, o2->shape[1].y},
        {o2->shape[1].x, o2->shape[0].y}
    };

    for(int i = 0; i < 4; i++) { 
        float a = atan2(vert[i].y, vert[i].x);
        float d = sqrt((vert[i].x)*(vert[i].x) +
                 (vert[i].y)*(vert[i].y)) * o1->size;
        vert[i].x = (int)(cos(o1->dir + a) * d + o1->x);
        vert[i].y = (int)(sin(o1->dir + a) * d + o1->y);        

        a = atan2(vert2[i].y, vert2[i].x);
        d = sqrt((vert2[i].x)*(vert2[i].x) +
                 (vert2[i].y)*(vert2[i].y))* o2->size;;
        vert2[i].x = (int)(cos(o2->dir + a) * d + o2->x);
        vert2[i].y = (int)(sin(o2->dir + a) * d + o2->y);        
    }

    for(int i = 0; i < 4; i++) {
        float m = (vert[1].y-vert[0].y)/(vert[1].x-vert[0].x+0.0000001);
        float m2 = (vert[2].y-vert[1].y)/(vert[2].x-vert[1].x+0.0000001);
        if(((vert2[i].y < m*(vert2[i].x-vert[1].x)+vert[1].y &&
             vert2[i].y > m*(vert2[i].x-vert[2].x)+vert[2].y) ||
            (vert2[i].y > m*(vert2[i].x-vert[1].x)+vert[1].y &&
             vert2[i].y < m*(vert2[i].x-vert[2].x)+vert[2].y)) &&
           ((vert2[i].y < m2*(vert2[i].x-vert[0].x)+vert[0].y &&
             vert2[i].y > m2*(vert2[i].x-vert[1].x)+vert[1].y) ||
            (vert2[i].y > m2*(vert2[i].x-vert[0].x)+vert[0].y &&
             vert2[i].y < m2*(vert2[i].x-vert[1].x)+vert[1].y))
        )
            return 1;
    }

    return 0;
}

void game_over(Game *g) {
    while(!key[KEY_ENTER])
        if(key[KEY_ESC]) {
            g->end = 1;
            return;
        }

    delete_list(g->asteroids);
    delete_list(g->shot);
    delete_list(g->particles);
    init_game(g);
}

void update_game(Game *g) {
    --(g->swait);
    update_object(&(g->player));
    if(g->player.speed > 0.0)
        g->player.speed -= 0.01;
    for(Object *o = g->asteroids; o; o = o->next) {
        rotate(o, 0.05);
        update_object(o);
        if(check_collision(&g->player, o) || check_collision(o, &g->player))
            game_over(g);
    }
    for(Object **o = &(g->shot); *o; o = &(*o)->next) {
        update_object(*o);
        if((*o)->lifetime == 0) {
            Object *t = *o;
            *o = (*o)->next;
            free(t);
            if(!*o)
                return;
        }
        for(Object **p = &(g->asteroids); *p; p = &(*p)->next) {
            if(check_collision(*p, *o)) {
                Object *t = *o;
                *o = (*o)->next;
                free(t);
                
                (*p)->size *= 0.75;
                if((*p)->size < 0.5) {
                    t = *p;
                    *p = (*p)->next;
                    free(t);
                } else {
                    t = (*p)->next;
                    (*p)->next = memcpy(malloc(sizeof(Object)), *p, sizeof(Object));
                    (*p)->next->next = t;
                    (*p)->dir += ((float)(rand()%628))/100.0;
                    accelerate(g, *p, 0.5);
                    (*p)->next->dir += ((float)(rand()%628))/100.0;
                    accelerate(g, (*p)->next, 1.0);
                }

                if(!*o)
                    return;
                if(!*p)
                    break;
            }
        }
    }
    for(Object **o = &(g->particles); *o;) {
        update_object(*o);
        if((*o)->lifetime == 0) {
            Object *t = *o;
            *o = (*o)->next;
            free(t);
        } else {
            o = &(*o)->next;
        }
    }
}

void draw_object(Object *o, BITMAP *scr) {
    if(o->shape) {
        int x, y, xalt, yalt;
        float a = atan2(o->shape[2].y, o->shape[2].x);
        float d = sqrt((o->shape[2].x)*(o->shape[2].x) +
                 (o->shape[2].y)*(o->shape[2].y)) * o->size;
        xalt = (int)(cos(o->dir + a) * d + o->x);
        yalt = (int)(sin(o->dir + a) * d + o->y);
        for(int i = 3; o->shape[i].x != 0 || o->shape[i].y != 0; ++i) {
            a = atan2(o->shape[i].y, o->shape[i].x);
            d = sqrt((o->shape[i].x)*(o->shape[i].x) +
                     (o->shape[i].y)*(o->shape[i].y)) * o->size;
            x = (int)(cos(o->dir + a) * d + o->x);
            y = (int)(sin(o->dir + a) * d + o->y);
            line(scr, x, y, xalt, yalt, makecol(255,255,255));
            xalt = x;
            yalt = y;
        }
    }
}

void draw_game(Game *g, BITMAP *scr) {
    draw_object(&(g->player), scr);
    for(Object *o = g->asteroids; o; o = o->next)
        draw_object(o, scr);
    for(Object *o = g->shot; o; o = o->next)
        draw_object(o, scr);
    for(Object *o = g->particles; o; o = o->next)
        draw_object(o, scr);
}

void get_input(Game *g) {
    if(key[KEY_ESC])
        g->end = 1;
    if(key[KEY_RIGHT])
        rotate(&g->player, +0.1);
    if(key[KEY_LEFT])
        rotate(&g->player, -0.1);
    if(key[KEY_UP])
        accelerate(g, &g->player, +0.1);
    if(key[KEY_SPACE])
        shoot(g);

}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));

    allegro_init();
    install_keyboard();
    install_timer();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0);
    BITMAP *orig_screen = screen;
    screen = create_bitmap(SCREEN_W, SCREEN_H);

    LOCK_VARIABLE(timer);
    LOCK_FUNCTION(timer_increment);
    install_int_ex(timer_increment, BPS_TO_TIMER(60));

    int need_redraw = 1;
    Game game;
    init_game(&game);

    while(!game.end) {
        for(; timer > 0; --timer) {
            need_redraw = 1;
            get_input(&game);
            update_game(&game);

            if(timer > 4) {
                timer = 0;
                break;
            }
        }

        if(need_redraw) {
            clear_to_color(screen, makecol(0,0,0));
            draw_game(&game, screen);
            blit(screen, orig_screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        }
        else
            sched_yield();
    }

    delete_list(game.asteroids);
    delete_list(game.shot);
    delete_list(game.particles);

    return 0;
}
END_OF_MAIN();
