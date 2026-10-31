#include "globals/rendering_global/rendering_global.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "globals/input/input.h"
#include "core/mathematics/math_definitions.h"
#include "core/mathematics/math_functions.h"
#include "core/mathematics/math_2d.h"
#include "core/threading/job_system.h"
#include <chrono>
#include <random>
#include <algorithm>
#include <cstdio>

// -----------------------------------------------------------------------------
// ESTRUCTURAS DEL JUEGO (2D PURO)
// -----------------------------------------------------------------------------
struct Projectile {
    Vector2 pos;
    float vy;
    bool active;
    bool from_player;
};

struct Alien {
    Vector2 pos;
    bool alive;
    int type; // 0: ligero, 1: medio, 2: pesado
};

struct BunkerBlock {
    Rect2 rect;
    int hp;
};

// -----------------------------------------------------------------------------
// POOLS ESTÁTICOS (Cero asignaciones dinámicas en el heap)
// -----------------------------------------------------------------------------
constexpr int MAX_BULLETS = 64;
static Projectile bullets[MAX_BULLETS];

constexpr int ALIEN_COLS = 8;
constexpr int ALIEN_ROWS = 4;
constexpr int TOTAL_ALIENS = ALIEN_COLS * ALIEN_ROWS;
static Alien aliens[TOTAL_ALIENS];

constexpr int BUNKER_PARTS = 24; // 3 bunkers de 8 bloques cada uno
static BunkerBlock bunkers[BUNKER_PARTS];

int main() {
    JobSystem::get().initialize();
    RenderingGlobal& renderer = RenderingGlobal::get();

    const int win_w = 800;
    const int win_h = 600;

    auto primary_window = renderer.create_window(
        Size2i(win_w, win_h),
        "Puzzle Engine - Space Invaders 2D Classic",
        Point2i(100, 100),
        Window::Type::Game
    );

    if (!primary_window) return -1;

    primary_window->get_rendering_device().init_geometry_pipeline();
    RenderingGeometry geometry(&primary_window->get_rendering_device());

    // -------------------------------------------------------------------------
    // ESTADO DEL JUEGO
    // -------------------------------------------------------------------------
    Vector2 player_pos(400.0f, 540.0f);
    const Vector2 player_size(32.0f, 16.0f);
    int player_lives = 3;
    int score = 0;
    bool game_over = false;
    bool victory = false;

    float shoot_cooldown = 0.0f;
    float alien_move_dir = 1.0f;
    float alien_step_timer = 0.0f;
    float alien_speed = 40.0f;

    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> shoot_rand(0.0f, 1.0f);

    auto init_game = [&]() {
        player_pos = Vector2(400.0f, 540.0f);
        player_lives = 3;
        score = 0;
        game_over = false;
        victory = false;
        alien_move_dir = 1.0f;
        alien_speed = 40.0f;

        // Limpiar balas
        for (int i = 0; i < MAX_BULLETS; ++i) bullets[i].active = false;

        // Armar formación de aliens
        for (int r = 0; r < ALIEN_ROWS; ++r) {
            for (int c = 0; c < ALIEN_COLS; ++c) {
                int idx = r * ALIEN_COLS + c;
                aliens[idx].pos = Vector2(120.0f + c * 70.0f, 100.0f + r * 45.0f);
                aliens[idx].alive = true;
                aliens[idx].type = (r == 0) ? 2 : ((r < 2) ? 1 : 0);
            }
        }

        // Armar 3 bunkers defensivos
        int b_idx = 0;
        float bunker_xs[3] = { 180.0f, 400.0f, 620.0f };
        for (int b = 0; b < 3; ++b) {
            float bx = bunker_xs[b];
            for (int row = 0; row < 2; ++row) {
                for (int col = 0; col < 4; ++col) {
                    bunkers[b_idx].rect = Rect2(bx - 30.0f + col * 15.0f, 460.0f + row * 15.0f, 14.0f, 14.0f);
                    bunkers[b_idx].hp = 3;
                    b_idx++;
                }
            }
        }
    };

    init_game();

    // -------------------------------------------------------------------------
    // BUCLE PRINCIPAL (Window::run para barra de título y eventos intactos)
    // -------------------------------------------------------------------------
    primary_window->run([&](double p_delta) {
        float dt = static_cast<float>(p_delta);
        if (dt > 0.033f) dt = 0.016f;

        Size2i cur_sz = primary_window->get_size();

        // 1. INPUT DEL JUGADOR
        if (!game_over && !victory) {
            float move_input = 0.0f;
            if (Input::is_key_pressed(Key::A) || Input::is_key_pressed(Key::Left))  move_input -= 1.0f;
            if (Input::is_key_pressed(Key::D) || Input::is_key_pressed(Key::Right)) move_input += 1.0f;

            player_pos.x += move_input * 320.0f * dt;
            player_pos.x = Math::clamp(player_pos.x, 30.0f, static_cast<float>(cur_sz.width) - 30.0f);

            // Disparo del jugador
            shoot_cooldown -= dt;
            if (Input::is_key_pressed(Key::Space) && shoot_cooldown <= 0.0f) {
                for (int i = 0; i < MAX_BULLETS; ++i) {
                    if (!bullets[i].active) {
                        bullets[i].pos = Vector2(player_pos.x, player_pos.y - 12.0f);
                        bullets[i].vy = -550.0f;
                        bullets[i].active = true;
                        bullets[i].from_player = true;
                        shoot_cooldown = 0.28f;
                        break;
                    }
                }
            }
        } else {
            if (Input::is_key_pressed(Key::R)) {
                init_game();
            }
        }

        // 2. ACTUALIZACIÓN DE ALIENS Y BALAS
        if (!game_over && !victory) {
            // Movimiento lateral en bloque
            bool hit_wall = false;
            int alive_count = 0;

            for (int i = 0; i < TOTAL_ALIENS; ++i) {
                if (!aliens[i].alive) continue;
                alive_count++;
                aliens[i].pos.x += alien_move_dir * alien_speed * dt;

                if (aliens[i].pos.x > cur_sz.width - 40.0f || aliens[i].pos.x < 40.0f) {
                    hit_wall = true;
                }

                // Si los aliens tocan los bunkers o al jugador
                if (aliens[i].pos.y >= player_pos.y - 20.0f) {
                    game_over = true;
                }

                // Disparo aleatorio de aliens
                if (shoot_rand(rng) < (0.012f * dt)) {
                    for (int b = 0; b < MAX_BULLETS; ++b) {
                        if (!bullets[b].active) {
                            bullets[b].pos = Vector2(aliens[i].pos.x, aliens[i].pos.y + 12.0f);
                            bullets[b].vy = 260.0f;
                            bullets[b].active = true;
                            bullets[b].from_player = false;
                            break;
                        }
                    }
                }
            }

            if (alive_count == 0) {
                victory = true;
            }

            // Cambiar de fila al tocar bordes
            if (hit_wall) {
                alien_move_dir *= -1.0f;
                alien_speed = std::min(160.0f, alien_speed + 8.0f); // Se aceleran al descender
                for (int i = 0; i < TOTAL_ALIENS; ++i) {
                    if (aliens[i].alive) {
                        aliens[i].pos.y += 18.0f;
                    }
                }
            }

            // Movimiento de proyectiles y colisiones
            for (int i = 0; i < MAX_BULLETS; ++i) {
                if (!bullets[i].active) continue;
                bullets[i].pos.y += bullets[i].vy * dt;

                // Fuera de pantalla
                if (bullets[i].pos.y < 35.0f || bullets[i].pos.y > cur_sz.height) {
                    bullets[i].active = false;
                    continue;
                }

                // Impacto con Bunkers
                for (int b = 0; b < BUNKER_PARTS; ++b) {
                    if (bunkers[b].hp <= 0) continue;
                    if (bunkers[b].rect.has_point(bullets[i].pos)) {
                        bunkers[b].hp--;
                        bullets[i].active = false;
                        break;
                    }
                }
                if (!bullets[i].active) continue;

                // Bala del jugador contra Aliens
                if (bullets[i].from_player) {
                    for (int a = 0; a < TOTAL_ALIENS; ++a) {
                        if (!aliens[a].alive) continue;
                        Rect2 a_box(aliens[a].pos.x - 14.0f, aliens[a].pos.y - 10.0f, 28.0f, 20.0f);
                        if (a_box.has_point(bullets[i].pos)) {
                            aliens[a].alive = false;
                            bullets[i].active = false;
                            score += (aliens[a].type + 1) * 20;
                            break;
                        }
                    }
                } 
                // Bala enemiga contra Jugador
                else {
                    Rect2 p_box(player_pos.x - player_size.x * 0.5f, player_pos.y - player_size.y * 0.5f, player_size.x, player_size.y);
                    if (p_box.has_point(bullets[i].pos)) {
                        bullets[i].active = false;
                        player_lives--;
                        if (player_lives <= 0) {
                            game_over = true;
                        }
                    }
                }
            }
        }

        // 3. RENDERIZADO 2D PURO CON BATCHING
        primary_window->clear(Color(0.02f, 0.02f, 0.04f, 1.0f));
        geometry.begin();

        // Bunkers defensivos
        for (int i = 0; i < BUNKER_PARTS; ++i) {
            if (bunkers[i].hp <= 0) continue;
            float alpha = bunkers[i].hp / 3.0f;
            Color b_col(0.15f, 0.85f, 0.35f, alpha);
            geometry.draw_rect(bunkers[i].rect, b_col, true);
        }

        // Aliens
        for (int i = 0; i < TOTAL_ALIENS; ++i) {
            if (!aliens[i].alive) continue;
            Color col = (aliens[i].type == 2) ? Color(1.0f, 0.25f, 0.25f) :
                        ((aliens[i].type == 1) ? Color(0.95f, 0.85f, 0.15f) : Color(0.25f, 0.75f, 1.0f));

            // Cuerpo del alien
            geometry.draw_rect(Rect2(aliens[i].pos.x - 12.0f, aliens[i].pos.y - 8.0f, 24.0f, 16.0f), col, true);
            // Antenas / ojos
            geometry.draw_rect(Rect2(aliens[i].pos.x - 6.0f, aliens[i].pos.y - 4.0f, 4.0f, 4.0f), Color(0.0f, 0.0f, 0.0f), true);
            geometry.draw_rect(Rect2(aliens[i].pos.x + 2.0f, aliens[i].pos.y - 4.0f, 4.0f, 4.0f), Color(0.0f, 0.0f, 0.0f), true);
        }

        // Proyectiles
        for (int i = 0; i < MAX_BULLETS; ++i) {
            if (!bullets[i].active) continue;
            Color b_col = bullets[i].from_player ? Color(0.2f, 1.0f, 0.4f) : Color(1.0f, 0.3f, 0.3f);
            geometry.draw_rect(Rect2(bullets[i].pos.x - 2.0f, bullets[i].pos.y - 6.0f, 4.0f, 12.0f), b_col, true);
        }

        // Nave del Jugador
        if (!game_over) {
            geometry.draw_rect(Rect2(player_pos.x - player_size.x * 0.5f, player_pos.y, player_size.x, player_size.y * 0.5f), Color(0.2f, 0.8f, 1.0f), true);
            geometry.draw_rect(Rect2(player_pos.x - 4.0f, player_pos.y - 8.0f, 8.0f, 8.0f), Color(0.9f, 0.95f, 1.0f), true);
        }

        // HUD / OSD
        char score_str[64];
        std::snprintf(score_str, sizeof(score_str), "SCORE: %04d", score);
        geometry.draw_text(Vector2(24.0f, 42.0f), score_str, Color(0.2f, 0.85f, 1.0f), 1.8f);

        char lives_str[32];
        std::snprintf(lives_str, sizeof(lives_str), "LIVES: %d", player_lives);
        geometry.draw_text(Vector2(cur_sz.width - 150.0f, 42.0f), lives_str, Color(0.35f, 1.0f, 0.45f), 1.8f);

        if (game_over) {
            geometry.draw_text(Vector2(cur_sz.width * 0.5f - 110.0f, 260.0f), "GAME OVER", Color(1.0f, 0.2f, 0.2f), 3.0f);
            geometry.draw_text(Vector2(cur_sz.width * 0.5f - 150.0f, 310.0f), "PRESS [R] TO RESTART", Color(1.0f, 1.0f, 1.0f), 1.6f);
        } else if (victory) {
            geometry.draw_text(Vector2(cur_sz.width * 0.5f - 130.0f, 260.0f), "YOU WIN!", Color(0.2f, 1.0f, 0.4f), 3.0f);
            geometry.draw_text(Vector2(cur_sz.width * 0.5f - 150.0f, 310.0f), "PRESS [R] TO RESTART", Color(1.0f, 1.0f, 1.0f), 1.6f);
        }

        // Barra de título nativa con botones y arrastre
        primary_window->draw_title_bar(geometry);

        geometry.flush();
    });

    JobSystem::get().shutdown();
    renderer.shutdown();
    return 0;
}