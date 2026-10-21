#include "globals/rendering_global/rendering_global.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "globals/input/input.h"
#include "core/mathematics/math_definitions.h"
#include "core/mathematics/math_functions.h"
#include <chrono>
#include <vector>
#include <random>
#include <cmath>

struct Vector3D {
    float x, y, z;
    Vector3D(float p_x = 0.0f, float p_y = 0.0f, float p_z = 0.0f) : x(p_x), y(p_y), z(p_z) {}

    Vector3D operator+(const Vector3D& o) const { return Vector3D(x + o.x, y + o.y, z + o.z); }
    Vector3D operator-(const Vector3D& o) const { return Vector3D(x - o.x, y - o.y, z - o.z); }
    Vector3D operator-() const { return Vector3D(-x, -y, -z); }
    Vector3D operator*(float s) const { return Vector3D(x * s, y * s, z * s); }
    Vector3D operator/(float s) const { return Vector3D(x / s, y / s, z / s); }

    float length_sq() const { return x * x + y * y + z * z; }
    float length() const { return Math::sqrt(length_sq()); }

    Vector3D normalized() const {
        float l = length();
        return (l > 0.00001f) ? (*this / l) : Vector3D(0.0f, 0.0f, 0.0f);
    }

    Vector3D cross(const Vector3D& o) const {
        return Vector3D(
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        );
    }
};

struct GeodesicPhoton {
    Vector3D pos;
    Vector3D prev_pos;
    Vector3D vel;
    Color color;
    bool active;
};

struct DiskFilament {
    float radius;
    float phi;
    float height_offset;
    float angular_speed;
    float thickness;
    Color color;
};

struct Planet {
    const char* name;
    float orbit_radius;
    float orbit_speed;
    float current_angle;
    float inclination;
    float radius;
    Color body_color;
    Color atmosphere_color;
    std::vector<Vector3D> orbit_points;
};

// -----------------------------------------------------------------------------
// PROYECCIÓN 3D CONVENCIONAL Y CORREGIDA (Orientada hacia +Z)
// -----------------------------------------------------------------------------
Vector2 project_3d(const Vector3D& p, const Vector3D& cam_pos, float cam_yaw, float cam_pitch, float fov_dist, const Vector2& screen_center, float& out_depth) {
    Vector3D rel = p - cam_pos;

    // Rotación Yaw (horizontal)
    float cy = Math::cos(cam_yaw);
    float sy = Math::sin(cam_yaw);
    float x1 = rel.x * cy - rel.z * sy;
    float y1 = rel.y;
    float z1 = rel.x * sy + rel.z * cy;

    // Rotación Pitch (vertical)
    float cp = Math::cos(cam_pitch);
    float sp = Math::sin(cam_pitch);
    float y2 = y1 * cp - z1 * sp;
    float z2 = y1 * sp + z1 * cp;

    out_depth = z2;
    float factor = (z2 > 1.0f) ? (fov_dist / z2) : 0.0f;

    return Vector2(
        screen_center.x + x1 * factor,
        screen_center.y - y2 * factor
    );
}

int main() {
    RenderingGlobal& renderer = RenderingGlobal::get();

    const int initial_w = 1280;
    const int initial_h = 720;

    auto primary_window = renderer.create_window(
        Size2i(initial_w, initial_h),
        "Puzzle Engine - Gargantua Relativistic Cinematic Flight",
        Point2i(50, 50),
        Window::Type::Game
    );

    if (!primary_window) return -1;

    RenderingGeometry geometry(&primary_window->get_rendering_device());

    // -------------------------------------------------------------------------
    // PARÁMETROS RELATIVISTAS DE GARGANTUA
    // -------------------------------------------------------------------------
    const float rs = 70.0f;
    const float r_photon = rs * 1.5f;
    const float r_isco = rs * 2.2f;
    const float r_outer = rs * 6.5f;
    const float c_speed = 460.0f;
    const float fov_dist = 640.0f;

    const Color col_core_white(1.0f, 0.98f, 0.92f, 1.0f);
    const Color col_hot_cyan(0.25f, 0.85f, 1.00f, 0.95f);
    const Color col_gold(1.0f, 0.70f, 0.18f, 0.85f);
    const Color col_orange(0.95f, 0.32f, 0.05f, 0.70f);
    const Color col_dark_red(0.40f, 0.06f, 0.02f, 0.35f);

    std::mt19937 rng(1337);

    // -------------------------------------------------------------------------
    // 1. SISTEMA PLANETARIO
    // -------------------------------------------------------------------------
    std::vector<Planet> planets = {
        { "Miller",  rs * 3.8f,  0.42f, 0.5f,  0.05f, 6.5f, Color(0.12f, 0.40f, 0.95f, 1.0f), Color(0.35f, 0.80f, 1.0f, 0.40f) },
        { "Mann",    rs * 7.2f,  0.19f, 2.1f, -0.09f, 8.0f, Color(0.85f, 0.92f, 0.98f, 1.0f), Color(0.55f, 0.75f, 1.0f, 0.30f) },
        { "Edmunds", rs * 11.5f, 0.09f, 4.2f,  0.14f, 8.5f, Color(0.85f, 0.50f, 0.22f, 1.0f), Color(0.90f, 0.70f, 0.40f, 0.25f) }
    };

    for (auto& pl : planets) {
        const int orb_segs = 96;
        for (int i = 0; i <= orb_segs; ++i) {
            float a = (static_cast<float>(i) / static_cast<float>(orb_segs)) * MathDefs::TAU;
            pl.orbit_points.push_back(Vector3D(
                pl.orbit_radius * Math::cos(a),
                pl.orbit_radius * Math::sin(a) * pl.inclination,
                pl.orbit_radius * Math::sin(a)
            ));
        }
    }

    // -------------------------------------------------------------------------
    // 2. DISCO DE ACRECIÓN VOLUMÉTRICO TOROIDAL
    // -------------------------------------------------------------------------
    const int filament_count = 280;
    std::vector<DiskFilament> filaments;
    filaments.reserve(filament_count);

    std::uniform_real_distribution<float> dist_sign(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dist_noise(0.0f, 1.0f);

    for (int i = 0; i < filament_count; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(filament_count);
        float r = r_isco + std::pow(t, 1.35f) * (r_outer - r_isco);
        float speed = 72.0f / Math::sqrt(r);
        float max_h = (r - r_isco) * 0.20f + 4.0f;
        float h_offset = dist_sign(rng) * max_h;
        float thickness = 1.8f + (1.0f - t) * 3.0f;

        Color c;
        if (t < 0.15f) {
            c = Color(Math::lerp(col_core_white.r, col_hot_cyan.r, t / 0.15f),
                      Math::lerp(col_core_white.g, col_hot_cyan.g, t / 0.15f),
                      Math::lerp(col_core_white.b, col_hot_cyan.b, t / 0.15f), 0.90f);
        } else if (t < 0.50f) {
            float ft = (t - 0.15f) / 0.35f;
            c = Color(Math::lerp(col_hot_cyan.r, col_gold.r, ft),
                      Math::lerp(col_hot_cyan.g, col_gold.g, ft),
                      Math::lerp(col_hot_cyan.b, col_gold.b, ft), 0.80f);
        } else if (t < 0.80f) {
            float ft = (t - 0.50f) / 0.30f;
            c = Color(Math::lerp(col_gold.r, col_orange.r, ft),
                      Math::lerp(col_gold.g, col_orange.g, ft),
                      Math::lerp(col_gold.b, col_orange.b, ft), 0.60f);
        } else {
            c = col_dark_red;
        }

        filaments.push_back({ r, static_cast<float>(dist_noise(rng) * MathDefs::TAU), h_offset, speed, thickness, c });
    }

    // -------------------------------------------------------------------------
    // 3. ENJAMBRE ULTRA-MASIVO DE FOTONES (24,000)
    // -------------------------------------------------------------------------
    const int total_photons = 24000;
    std::vector<GeodesicPhoton> photons;
    photons.resize(total_photons);

    std::uniform_real_distribution<float> dist_sphere_z(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dist_sphere_phi(0.0f, MathDefs::TAU);
    std::uniform_real_distribution<float> dist_sphere_r(rs * 5.0f, rs * 10.5f);
    std::uniform_real_distribution<float> dist_jitter(-15.0f, 15.0f);

    auto spawn_photon = [&](GeodesicPhoton& p) {
        float z_val = dist_sphere_z(rng);
        float phi = dist_sphere_phi(rng);
        float r_spawn = dist_sphere_r(rng);
        float sin_polar = Math::sqrt(1.0f - z_val * z_val);

        p.pos = Vector3D(
            r_spawn * sin_polar * Math::cos(phi),
            r_spawn * z_val,
            r_spawn * sin_polar * Math::sin(phi)
        );
        p.prev_pos = p.pos;

        Vector3D target(dist_jitter(rng), dist_jitter(rng), dist_jitter(rng));
        p.vel = (target - p.pos).normalized() * c_speed;
        p.active = true;

        float rnd = dist_noise(rng);
        if (rnd < 0.35f) {
            p.color = Color(0.35f, 0.85f, 1.0f, 0.65f);
        } else if (rnd < 0.70f) {
            p.color = Color(1.0f, 0.80f, 0.25f, 0.60f);
        } else {
            p.color = Color(0.95f, 0.35f, 0.15f, 0.40f);
        }
    };

    for (int i = 0; i < total_photons; ++i) {
        spawn_photon(photons[i]);
    }

    // -------------------------------------------------------------------------
    // CÁMARA 3D (Posicionada y orientada directo al agujero negro)
    // -------------------------------------------------------------------------
    Vector3D cam_pos(0.0f, 220.0f, -980.0f);
    float cam_yaw = 0.0f;
    float cam_pitch = -0.22f; // Inclinación suave hacia abajo, Gargantua centrado

    const float move_speed = 520.0f;
    const float turn_speed = 1.9f;

    auto last_time = std::chrono::high_resolution_clock::now();

    // -------------------------------------------------------------------------
    // ACTUALIZACIÓN DE FÍSICA Y SIMULACIÓN
    // -------------------------------------------------------------------------
    auto update_simulation = [&](float dt) {
        if (dt > 0.033f) dt = 0.033f;

        // Órbitas de planetas
        for (auto& pl : planets) {
            pl.current_angle += pl.orbit_speed * dt;
        }

        // Geodésicas de fotones
        for (auto& p : photons) {
            if (!p.active) {
                spawn_photon(p);
                continue;
            }

            p.prev_pos = p.pos;
            float r = p.pos.length();

            if (r <= rs || r > rs * 11.5f) {
                p.active = false;
                continue;
            }

            Vector3D h = p.pos.cross(p.vel);
            float h2 = h.length_sq();
            float accel_mag = (1.5f * rs * h2) / (r * r * r * r * r);
            Vector3D accel = p.pos * (-accel_mag);

            p.vel = p.vel + accel * dt;
            p.vel = p.vel.normalized() * c_speed;
            p.pos = p.pos + p.vel * dt;
        }

        // Filamentos del disco de acreción
        for (auto& fil : filaments) {
            fil.phi += fil.angular_speed * dt;
        }
    };

    // -------------------------------------------------------------------------
    // DIBUJADO DE LA ESCENA (Calcula el centro dinámico de la ventana)
    // -------------------------------------------------------------------------
    auto render_scene = [&]() {
        Size2i cur_size = primary_window->get_size();
        Vector2 screen_center(cur_size.width * 0.5f, cur_size.height * 0.5f);

        primary_window->clear(Color(0.006f, 0.006f, 0.010f, 1.0f));
        geometry.begin();

        // 1. Órbitas y planetas
        for (const auto& pl : planets) {
            Vector2 prev_orb_pt;
            float prev_d = 0.0f;
            for (size_t k = 0; k < pl.orbit_points.size(); ++k) {
                float d;
                Vector2 opt = project_3d(pl.orbit_points[k], cam_pos, cam_yaw, cam_pitch, fov_dist, screen_center, d);
                if (k > 0 && d > 1.0f && prev_d > 1.0f) {
                    geometry.draw_line(prev_orb_pt, opt, Color(0.30f, 0.40f, 0.55f, 0.15f), 1.0f);
                }
                prev_orb_pt = opt;
                prev_d = d;
            }

            Vector3D p_pos(
                pl.orbit_radius * Math::cos(pl.current_angle),
                pl.orbit_radius * Math::sin(pl.current_angle) * pl.inclination,
                pl.orbit_radius * Math::sin(pl.current_angle)
            );

            Vector3D to_cam = cam_pos - p_pos;
            float r_planet = p_pos.length();
            float cos_cam = (-p_pos.normalized()).x * (to_cam.normalized()).x +
                            (-p_pos.normalized()).z * (to_cam.normalized()).z;

            if (cos_cam > 0.15f && r_planet > rs) {
                float warp = (1.4f * rs * rs) / r_planet;
                p_pos.y += (cam_pos.y >= 0.0f ? 1.0f : -1.0f) * warp * cos_cam;
            }

            float p_depth;
            Vector2 planet_scr = project_3d(p_pos, cam_pos, cam_yaw, cam_pitch, fov_dist, screen_center, p_depth);

            if (p_depth > 1.0f) {
                float screen_r = (fov_dist / p_depth) * pl.radius;
                geometry.draw_circle(planet_scr, screen_r + 2.5f, pl.atmosphere_color, false, 24);
                geometry.draw_circle(planet_scr, screen_r, pl.body_color, true, 28);
                geometry.draw_circle(planet_scr, screen_r * 0.40f, col_core_white, true, 12);
            }
        }

        // 2. Enjambre masivo de fotones
        for (const auto& p : photons) {
            if (!p.active) continue;

            float d0, d1;
            Vector2 s0 = project_3d(p.prev_pos, cam_pos, cam_yaw, cam_pitch, fov_dist, screen_center, d0);
            Vector2 s1 = project_3d(p.pos, cam_pos, cam_yaw, cam_pitch, fov_dist, screen_center, d1);

            if (d0 > 1.0f && d1 > 1.0f) {
                geometry.draw_line(s0, s1, p.color, 1.0f);
            }
        }

        // 3. Disco volumétrico toroidal
        const int segs = 48;
        for (const auto& fil : filaments) {
            Vector2 prev_proj;
            float prev_depth = 0.0f;
            bool first = true;

            for (int j = 0; j <= segs; ++j) {
                float a = fil.phi + (static_cast<float>(j) / static_cast<float>(segs)) * MathDefs::TAU;
                float cos_a = Math::cos(a);
                float sin_a = Math::sin(a);

                Vector3D pt(
                    fil.radius * cos_a,
                    fil.height_offset * Math::cos(a * 3.0f),
                    fil.radius * sin_a
                );

                Vector3D to_cam = cam_pos - pt;
                float r_pt = pt.length();

                float cos_cam_align = (-pt.normalized()).x * (to_cam.normalized()).x + 
                                      (-pt.normalized()).z * (to_cam.normalized()).z;

                if (cos_cam_align > 0.15f && r_pt > rs) {
                    float lens_warp = (1.45f * rs * rs) / r_pt;
                    pt.y += (cam_pos.y >= 0.0f ? 1.0f : -1.0f) * lens_warp * cos_cam_align;
                }

                Vector3D orb_dir(-sin_a, 0.0f, cos_a);
                float doppler_factor = 1.0f + (orb_dir.x * to_cam.normalized().x + orb_dir.z * to_cam.normalized().z) * 0.40f;

                Color c = fil.color;
                c.r = Math::clamp(c.r * doppler_factor, 0.0f, 1.0f);
                c.g = Math::clamp(c.g * doppler_factor, 0.0f, 1.0f);
                c.b = Math::clamp(c.b * (doppler_factor > 1.0f ? doppler_factor * 1.3f : doppler_factor), 0.0f, 1.0f);

                float depth;
                Vector2 proj = project_3d(pt, cam_pos, cam_yaw, cam_pitch, fov_dist, screen_center, depth);

                if (depth > 1.0f) {
                    if (!first && prev_depth > 1.0f) {
                        geometry.draw_line(prev_proj, proj, c, fil.thickness);
                    }
                }

                prev_proj = proj;
                prev_depth = depth;
                first = false;
            }
        }

        // 4. Singularidad, horizonte de eventos y esfera de fotones
        float depth_bh;
        Vector2 bh_center = project_3d(Vector3D(0, 0, 0), cam_pos, cam_yaw, cam_pitch, fov_dist, screen_center, depth_bh);

        if (depth_bh > 1.0f) {
            float rad_horizon = (fov_dist / depth_bh) * rs;
            float rad_photon = (fov_dist / depth_bh) * r_photon;

            geometry.draw_circle(bh_center, rad_photon + 1.5f, Color(1.0f, 0.85f, 0.45f, 0.35f), false, 48);
            geometry.draw_circle(bh_center, rad_photon, col_core_white, false, 48);

            geometry.draw_circle(bh_center, rad_horizon, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 64);
            geometry.draw_circle(bh_center, rad_horizon + 1.0f, Color(0.20f, 0.65f, 1.0f, 0.70f), false, 48);
        }

        geometry.flush();
    };

    // -------------------------------------------------------------------------
    // CALLBACK CONTINUO MIENTRAS SE ARRASTRA O CAMBIA EL TAMAÑO DE LA VENTANA
    // -------------------------------------------------------------------------
    primary_window->get_rendering_device().set_render_callback([&]() {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        // Avanzar el tiempo y la física incluso dentro del bucle modal de Win32
        update_simulation(dt);

        // Renderizar inmediatamente en el nuevo tamaño de framebuffer
        render_scene();
    });

    // -------------------------------------------------------------------------
    // BUCLE PRINCIPAL
    // -------------------------------------------------------------------------
    while (!primary_window->should_close()) {
        auto cur_time = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float>(cur_time - last_time).count();
        last_time = cur_time;
        if (delta > 0.033f) delta = 0.033f;

        Input::update();
        primary_window->poll_events();

        // Alternar pantalla completa con F11
        if (Input::is_key_pressed(Key::F11)) {
            static bool fs = false;
            fs = !fs;
            primary_window->set_mode(fs ? Window::Mode::Fullscreen : Window::Mode::Windowed);
        }

        // Controles de cámara FPS naturales
        if (Input::is_key_pressed(Key::Up))    cam_pitch += turn_speed * delta; // Mirar arriba
        if (Input::is_key_pressed(Key::Down))  cam_pitch -= turn_speed * delta; // Mirar abajo
        if (Input::is_key_pressed(Key::Left))  cam_yaw   -= turn_speed * delta;
        if (Input::is_key_pressed(Key::Right)) cam_yaw   += turn_speed * delta;

        cam_pitch = Math::clamp(cam_pitch, -MathDefs::PI * 0.48f, MathDefs::PI * 0.48f);

        float cy = Math::cos(cam_yaw);
        float sy = Math::sin(cam_yaw);
        float cp = Math::cos(cam_pitch);
        float sp = Math::sin(cam_pitch);

        Vector3D forward(sy * cp, sp, cy * cp);
        Vector3D right(cy, 0.0f, -sy);

        if (Input::is_key_pressed(Key::W)) cam_pos = cam_pos + forward * (move_speed * delta);
        if (Input::is_key_pressed(Key::S)) cam_pos = cam_pos - forward * (move_speed * delta);
        if (Input::is_key_pressed(Key::A)) cam_pos = cam_pos - right * (move_speed * delta);
        if (Input::is_key_pressed(Key::D)) cam_pos = cam_pos + right * (move_speed * delta);

        // Avanzar estado de la simulación
        update_simulation(delta);

        // Dibujar y presentar
        render_scene();
        primary_window->swap_buffers();
    }

    renderer.shutdown();
    return 0;
}