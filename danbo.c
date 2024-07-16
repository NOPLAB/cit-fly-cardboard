#include <drawlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define DL_WAIT_TIME 0

#define DANBO_SIZE_X 80
#define DANBO_SIZE_Y 50

#define DIRECTION_SPEED 0.005
#define SIMULATION_SPEED 0.006

#define CAMERA_INITIAL_X -100
#define CAMERA_INITIAL_Y 300
#define CAMERA_LERP 0.008

#define POWER 200
#define POWER_CIRCLE_RADIUS 0.5

#define GROUND_HEGIHT (-DANBO_SIZE_Y)
#define GROUND_COLOR (new_color(59, 61, 54))

#define FLY_LENGTH_FONT_SIZE 1
#define FLY_LENGTH_FONT_THICKNESS 2
#define FLY_LENGTH_FONT_X (DL_WIDTH - DL_WIDTH / 2)
#define FLY_LENGTH_FONT_Y (DL_HEIGHT / 8)

#define FLY_TARGET_FONT_SIZE 1
#define FLY_TARGET_FONT_THICKNESS 2
#define FLY_TARGET_FONT_X (DL_WIDTH - DL_WIDTH / 2)
#define FLY_TARGET_FONT_Y (DL_HEIGHT / 5)

#define BACKGROUND_COLOR (new_color(149, 192, 236))

#define BACKGROUND_STAR_NUM 10
#define BACKGROUND_STAR_HEIGHT 1200

#define BACKGROUND_CLOUD_NUM 10
#define BACKGROUND_CLOUD_COLOR (new_color(255, 255, 255))
#define BACKGROUND_CLOUD_HEIGHT 1000
#define BACKGROUND_CLOUD_RANDOM_HEIGHT 1000

typedef enum
{
    TITLE,
    GAME_INIT,
    GAME_CLICK_DIRECTION,
    GAME_CLICK_POWER,
    GAME_RUN,
    GAME_WAIT,
    GAME_SHOW_SCORE,
    EXIT,
} GameState_t;

typedef struct
{
    uint64_t t;
    double sim_t;
    uint64_t wait_t;
    GameState_t state;
    double high_score;
    double fly_target;
} GameManager_t;

typedef struct
{
    float x;
    float y;
    float screen_x;
    float screen_y;
    float screen_cx;
    float screen_cy;
    float power;
    float power_direction;
    double fly_length;
} Danbo_t;

typedef struct
{
    float x;
    float y;
} Camera_t;

typedef struct
{
    float x;
    float y;
} Vector2_t;

Vector2_t scene_star[BACKGROUND_STAR_NUM] = {0};

typedef struct
{
    int size;
    Vector2_t p;
} Cloud_t;

Cloud_t scene_cloud[BACKGROUND_CLOUD_NUM] = {0};

typedef struct
{
    int r;
    int g;
    int b;
} Color_t;

Color_t new_color(int r, int g, int b)
{
    Color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    return color;
}

void calculate_screen(const Camera_t *camera, const float x, const float y, float *screen_x, float *screen_y)
{
    *screen_x = x - camera->x;
    *screen_y = -y + camera->y;
}

void calculate_danbo(Danbo_t *danbo, const Camera_t *camera)
{
    calculate_screen(camera, danbo->x, danbo->y, &(danbo->screen_x), &(danbo->screen_y));
    danbo->screen_cx = danbo->screen_x + DANBO_SIZE_X / 2;
    danbo->screen_cy = danbo->screen_y + DANBO_SIZE_Y / 2;
}

void warp_to_in_camera_x(float *x, const Camera_t *camera)
{
    if (*x < camera->x)
        *x += DL_WIDTH;
    else if (*x > camera->x + DL_WIDTH)
        *x -= DL_WIDTH;
}

void warp_to_in_camera_y(float *y, const Camera_t *camera)
{
    if (*y < camera->y - DL_HEIGHT)
        *y += DL_HEIGHT;
    else if (*y > camera->y)
        *y -= DL_HEIGHT;
}

bool is_in_camera_y(const float y, const Camera_t *camera)
{
    return camera->y - DL_HEIGHT < y && y < camera->y;
}

void draw_box(const float x, const float y, const float w, const float h, Color_t color, const Camera_t *camera)
{
    float screen_x, screen_y;
    calculate_screen(camera, x, y, &screen_x, &screen_y);
    dl_rectangle(screen_x, screen_y, screen_x + w, screen_y + h, dl_color_from_rgb(color.r, color.g, color.b), 1, 1);
}

void draw_circle(const float x, const float y, const float l, Color_t color, const Camera_t *camera)
{
    float screen_x, screen_y;
    calculate_screen(camera, x, y, &screen_x, &screen_y);
    dl_circle(screen_x, screen_y, l, dl_color_from_rgb(color.r, color.g, color.b), 1, 1);
}

void draw_box_screen(const float screen_x, const float screen_y, const float w, const float h, Color_t color)
{
    dl_rectangle(screen_x, screen_y, screen_x + w, screen_y + h, dl_color_from_rgb(color.r, color.g, color.b), 1, 1);
}

void draw_danbo(const float x, const float y)
{
    dl_rectangle(x, y, x + DANBO_SIZE_X, y + DANBO_SIZE_Y, dl_color_from_rgb(203, 169, 123), 1, 1);
}

void draw_direction(const float x, const float y, const float power_direction)
{
    dl_text("Mouse Click!", 30, 100, 1, DL_C("white"), 2);

    float cx = cos(power_direction);
    float sy = sin(power_direction);
    float endX = x + cx * 50;
    float endY = y + sy * 50;

    dl_line(x, y, endX, endY, DL_C("yellow"), 4);

    float arrowLength = 10;
    float arrowAngle = 0.5;

    dl_line(endX, endY,
            endX - arrowLength * cos(power_direction - arrowAngle),
            endY - arrowLength * sin(power_direction - arrowAngle),
            DL_C("yellow"), 4);

    dl_line(endX, endY,
            endX - arrowLength * cos(power_direction + arrowAngle),
            endY - arrowLength * sin(power_direction + arrowAngle),
            DL_C("yellow"), 4);
}

// void title(GameManager_t *manager)
// {
//     int e, k, x, y;
//     if (dl_get_event(&e, &k, &x, &y))
//     {
//         if (e == DL_EVENT_L_DOWN)
//         {
//             manager->state = GAME_INIT;
//         }
//     }

//     dl_text("Mouse Click!", 200, 100, 1, DL_C("white"), 2);
// }

Color_t color_lerp(Color_t color, float lerp)
{
    lerp = lerp < 0 ? 0 : lerp;
    lerp = lerp > 1 ? 1 : lerp;
    color.r = color.r * lerp;
    color.g = color.g * lerp;
    color.b = color.b * lerp;
    return color;
}

void scene_init()
{
    for (int i = 0; i < BACKGROUND_STAR_NUM; i++)
    {
        scene_star[i].x = rand() % DL_WIDTH;
        scene_star[i].y = rand() % DL_HEIGHT + DL_HEIGHT / 2;
    }

    for (int i = 0; i < BACKGROUND_CLOUD_NUM; i++)
    {
        scene_cloud[i].size = rand() % 100 + 50;
        scene_cloud[i].p.x = rand() % DL_WIDTH;
        scene_cloud[i].p.y = rand() % BACKGROUND_CLOUD_RANDOM_HEIGHT + BACKGROUND_CLOUD_HEIGHT;
    }
}

void scene_draw(Camera_t *camera)
{
    // Background
    Color_t color = color_lerp(BACKGROUND_COLOR, 1000 / camera->y);
    draw_box_screen(0, 0, DL_WIDTH, DL_HEIGHT, color);

    if (camera->y > BACKGROUND_STAR_HEIGHT)
    {
        for (int i = 0; i < BACKGROUND_STAR_NUM; i++)
        {
            warp_to_in_camera_x(&(scene_star[i].x), camera);
            warp_to_in_camera_y(&(scene_star[i].y), camera);
            draw_box(scene_star[i].x, scene_star[i].y, 1, 1, new_color(255, 255, 0), camera);
        }
    }

    if (camera->y > BACKGROUND_CLOUD_HEIGHT)
    {
        for (int i = 0; i < BACKGROUND_CLOUD_NUM; i++)
        {
            warp_to_in_camera_x(&(scene_cloud[i].p.x), camera);
            draw_circle(scene_cloud[i].p.x, scene_cloud[i].p.y, scene_cloud->size, BACKGROUND_CLOUD_COLOR, camera);
        }
    }

    if (is_in_camera_y(GROUND_HEGIHT, camera))
        draw_box(camera->x, GROUND_HEGIHT, DL_WIDTH, CAMERA_INITIAL_Y, GROUND_COLOR, camera);
}

void draw_fly_length(double len)
{
    char str[100];
    sprintf(str, "FlyLen: %.2fkm", len);
    dl_text(str, FLY_LENGTH_FONT_X, FLY_LENGTH_FONT_Y, FLY_LENGTH_FONT_SIZE, DL_C("white"), FLY_LENGTH_FONT_THICKNESS);
}

void draw_fly_target(double len)
{
    char str[100];
    sprintf(str, "Target: %.2fkm", len);
    dl_text(str, FLY_TARGET_FONT_X, FLY_TARGET_FONT_Y, FLY_TARGET_FONT_SIZE, DL_C("white"), FLY_TARGET_FONT_THICKNESS);
}

void game_click_direction(GameManager_t *manager, Camera_t *camera, Danbo_t *danbo)
{
    scene_draw(camera);

    draw_fly_target(manager->fly_target);

    danbo->power_direction = sin(manager->t * DIRECTION_SPEED) * M_PI / 4 - M_PI / 4;

    draw_danbo(danbo->screen_x, danbo->screen_y);
    draw_direction(danbo->screen_cx, danbo->screen_cy, danbo->power_direction);

    int e, k, x, y;
    if (dl_get_event(&e, &k, &x, &y))
    {
        if (e == DL_EVENT_L_DOWN)
        {
            manager->state = GAME_CLICK_POWER;
        }
    }
}

void draw_power(const float x, const float y, const float power)
{
    dl_text("Mouse Click!", 30, 100, 1, DL_C("white"), 2);
    dl_circle(x, y, power * POWER_CIRCLE_RADIUS, dl_color_from_rgb(255, 255, 255), 1, 1);
}

void game_click_power(GameManager_t *manager, Camera_t *camera, Danbo_t *danbo)
{
    scene_draw(camera);

    draw_fly_target(manager->fly_target);

    danbo->power = sin(manager->t * DIRECTION_SPEED) * POWER + POWER;

    draw_power(danbo->screen_cx, danbo->screen_cy, danbo->power);
    draw_danbo(danbo->screen_x, danbo->screen_y);
    draw_direction(danbo->screen_cx, danbo->screen_cy, danbo->power_direction);

    int e,
        k, x, y;
    if (dl_get_event(&e, &k, &x, &y))
    {
        if (e == DL_EVENT_L_DOWN)
        {
            manager->state = GAME_RUN;
        }
    }
}

void calculate_parabola(float *x, float *y, const float v, const float theta, const double t)
{
    *x = v * cos(theta) * t;
    *y = v * sin(theta) * t - 0.5 * 9.8 * t * t;
}

void simuration(Danbo_t *danbo, Camera_t *camera, double t)
{
    scene_draw(camera);

    draw_danbo(danbo->screen_x, danbo->screen_y);

    calculate_parabola(&(danbo->x), &(danbo->y), danbo->power, -danbo->power_direction, t);

    danbo->fly_length = danbo->x / 100;
}

void camera_move(Camera_t *camera, Danbo_t *danbo)
{
    static bool is_camera_x_moved;

    if (!is_camera_x_moved && (danbo->x) > abs(CAMERA_INITIAL_X) + 100)
        is_camera_x_moved = true;

    if (is_camera_x_moved)
        camera->x = camera->x + ((danbo->x - DL_WIDTH / 2) - camera->x) * CAMERA_LERP;

    camera->y = camera->y + ((danbo->y + DL_HEIGHT / 2) - camera->y) * CAMERA_LERP;
}

void game_run(GameManager_t *manager, Camera_t *camera, Danbo_t *danbo)
{
    simuration(danbo, camera, manager->sim_t);
    camera_move(camera, danbo);
    draw_fly_length(danbo->fly_length);
    draw_fly_target(manager->fly_target);

    if (danbo->y < 0)
    {
        manager->state = GAME_WAIT;
    }
}

void game_wait(GameManager_t *manager, Camera_t *camera, Danbo_t *danbo)
{
    manager->wait_t++;
    scene_draw(camera);
    camera_move(camera, danbo);
    draw_danbo(danbo->screen_x, danbo->screen_y);
    draw_fly_length(danbo->fly_length);
    draw_fly_target(manager->fly_target);

    if (manager->wait_t == 1000)
    {
        manager->state = GAME_SHOW_SCORE;
    }
}

void game_show_score(GameManager_t *manager, Danbo_t *danbo)
{
    char str[100];
    sprintf(str, "Diff: %.2fkm", danbo->fly_length - manager->fly_target);
    dl_text(str, 200, 140, 1, DL_C("yellow"), 2);
    dl_text("\"R\" key restart. Other key exit.", 80, 180, 1, DL_C("white"), 2);

    int e, k, x, y;
    if (dl_get_event(&e, &k, &x, &y))
    {
        if (e == DL_EVENT_KEY)
        {
            if (k == 'r')
                manager->state = GAME_INIT;
            else
                manager->state = EXIT;
        }
    }
}

int main(void)
{
    // drawlibの初期化
    dl_initialize(1.0);

    dl_clear(DL_C("black"));

    GameManager_t manager;
    manager.state = TITLE;
    manager.high_score = 0;

    Camera_t camera;

    Danbo_t danbo;

    float power = 0;
    float power_direction = 0;

    uint64_t run_t = 0;

    dl_text("Mouse Click!", 30, 100, 1, DL_C("white"), 2);

    scene_init();

    // --- メインループ ---
    while (1)
    {
        calculate_danbo(&danbo, &camera);

        // --- 描画処理 ---
        // 描画を一旦停止する（ちらつき防止）
        dl_stop();
        switch (manager.state)
        {
        case TITLE:
            // title(&manager);
            manager.state = GAME_INIT;
            break;
        case GAME_INIT:
            manager.t = 0;
            manager.sim_t = 0;
            manager.wait_t = 0;
            manager.high_score = 0;
            manager.fly_target = rand() % 100 + 100;

            camera.x = CAMERA_INITIAL_X;
            camera.y = CAMERA_INITIAL_Y;

            danbo.x = 0;
            danbo.y = 0;
            danbo.power = 0;
            danbo.power_direction = 0;
            danbo.fly_length = 0;

            run_t = 0;
            calculate_danbo(&danbo, &camera);

            manager.state = GAME_CLICK_DIRECTION;
            break;
        case GAME_CLICK_DIRECTION:
            game_click_direction(&manager, &camera, &danbo);
            break;
        case GAME_CLICK_POWER:
            game_click_power(&manager, &camera, &danbo);
            break;
        case GAME_RUN:
            if (run_t == 0)
                run_t = manager.t;
            manager.sim_t = (manager.t - run_t) * SIMULATION_SPEED;
            game_run(&manager, &camera, &danbo);
            break;
        case GAME_WAIT:
            manager.sim_t = (manager.t - run_t) * SIMULATION_SPEED;
            game_wait(&manager, &camera, &danbo);
            break;
        case GAME_SHOW_SCORE:
            game_show_score(&manager, &danbo);
            break;
        case EXIT:
            return 0;
        }
        // 描画を再開
        dl_resume();

        // 待機（wait_timeが0.01なので0.01秒待つ）
        // 短い時間で良いのでこれが無いと，描画の更新などが一切行われないので注意
        dl_wait(DL_WAIT_TIME);

        manager.t++;
    }

    return 0;
}