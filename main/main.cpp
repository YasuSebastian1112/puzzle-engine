#include "globals/rendering_global/rendering_global.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "globals/input/input.h"
#include "core/mathematics/math_definitions.h"
#include "core/mathematics/math_functions.h"
#include "core/threading/job_system.h"
#include <chrono>
#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>

struct Vector3D {
    float x, y, z;
    Vector3D(float p_x = 0.0f, float p_y = 0.0f, float p_z = 0.0f) : x(p_x), y(p_y), z(p_z) {}

    Vector3D operator+(const Vector3D& o) const { return Vector3D(x + o.x, y + o.y, z + o.z); }
    Vector3D operator-(const Vector3D& o) const { return Vector3D(x - o.x, y - o.y, z - o.z); }
    Vector3D operator*(float s) const { return Vector3D(x * s, y * s, z * s); }
    Vector3D operator/(float s) const { return Vector3D(x / s, y / s, z / s); }

    float length_sq() const { return x * x + y * y + z * z; }
    float length() const { return Math::sqrt(length_sq()); }
};

struct QuantumElectronSample {
    Vector3D pos;
    Color color;
    float phase;
    int n;
};

struct Nucleon {
    Vector3D pos;
    bool is_proton;
};

struct OrbitalConfig {
    int n;
    int l;
    int m;
    int electrons;
    float z_eff;
};

// -----------------------------------------------------------------------------
// POLINOMIOS MECÁNICO-CUÁNTICOS (EVALUADOS EN PARALELO)
// -----------------------------------------------------------------------------
double legendre_poly(int l, int m, double x) {
    int abs_m = std::abs(m);
    double pmm = 1.0;
    if (abs_m > 0) {
        double somx2 = std::sqrt((1.0 - x) * (1.0 + x));
        double fact = 1.0;
        for (int i = 1; i <= abs_m; ++i) {
            pmm *= -fact * somx2;
            fact += 2.0;
        }
    }
    if (l == abs_m) return pmm;

    double pmmp1 = x * (2 * abs_m + 1) * pmm;
    if (l == abs_m + 1) return pmmp1;

    double pll = 0.0;
    for (int ll = abs_m + 2; ll <= l; ++ll) {
        pll = (x * (2 * ll - 1) * pmmp1 - (ll + abs_m - 1) * pmm) / (ll - abs_m);
        pmm = pmmp1;
        pmmp1 = pll;
    }
    return pll;
}

double laguerre_poly(int k, int alpha, double x) {
    if (k == 0) return 1.0;
    if (k == 1) return 1.0 + alpha - x;

    double l_prev2 = 1.0;
    double l_prev1 = 1.0 + alpha - x;
    double l_cur = 0.0;

    for (int i = 2; i <= k; ++i) {
        l_cur = ((2 * i - 1 + alpha - x) * l_prev1 - (i - 1 + alpha) * l_prev2) / i;
        l_prev2 = l_prev1;
        l_prev1 = l_cur;
    }
    return l_cur;
}

double factorial(int n) {
    if (n <= 1) return 1.0;
    double res = 1.0;
    for (int i = 2; i <= n; ++i) res *= i;
    return res;
}

double spherical_harmonic_real(int l, int m, double theta, double phi) {
    int abs_m = std::abs(m);
    double leg = legendre_poly(l, abs_m, std::cos(theta));
    double norm = std::sqrt(((2.0 * l + 1.0) / (4.0 * MathDefs::PI)) * 
                            (factorial(l - abs_m) / factorial(l + abs_m)));

    if (m > 0) {
        return std::sqrt(2.0) * norm * leg * std::cos(m * phi);
    } else if (m < 0) {
        return std::sqrt(2.0) * norm * leg * std::sin(abs_m * phi);
    } else {
        return norm * leg;
    }
}

double radial_wavefunction(int n, int l, double z_eff, double r) {
    double a0 = 1.0;
    double rho = (2.0 * z_eff * r) / (n * a0);
    int k = n - l - 1;
    int alpha = 2 * l + 1;
    double lag = laguerre_poly(k, alpha, rho);

    double norm = std::sqrt(std::pow((2.0 * z_eff) / (n * a0), 3.0) *
                            (factorial(n - l - 1) / (2.0 * n * factorial(n + l))));

    return norm * std::exp(-rho * 0.5) * std::pow(rho, l) * lag;
}

double hydrogen_psi(int n, int l, int m, double z_eff, double r, double theta, double phi) {
    return radial_wavefunction(n, l, z_eff, r) * spherical_harmonic_real(l, m, theta, phi);
}

Vector2 project_3d(const Vector3D& p, const Vector3D& cam_pos, float cam_yaw, float cam_pitch, float fov_dist, const Vector2& screen_center, float& out_depth) {
    Vector3D rel = p - cam_pos;

    float cy = Math::cos(cam_yaw);
    float sy = Math::sin(cam_yaw);
    float x1 = rel.x * cy - rel.z * sy;
    float y1 = rel.y;
    float z1 = rel.x * sy + rel.z * cy;

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

// -----------------------------------------------------------------------------
// TABLA PERIÓDICA DINÁMICA
// -----------------------------------------------------------------------------
const char* ELEMENT_SYMBOLS[] = {
    "None", "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar",
    "K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",
    "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe",
    "Cs", "Ba", "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu",
    "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
    "Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
    "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn", "Nh", "Fl", "Mc", "Lv", "Ts", "Og"
};

int main() {
    // 1. Inicializar el pool multihilo nativo
    JobSystem::get().initialize();

    RenderingGlobal& renderer = RenderingGlobal::get();

    const int initial_w = 1280;
    const int initial_h = 720;

    auto primary_window = renderer.create_window(
        Size2i(initial_w, initial_h),
        "Puzzle Engine - Multithreaded Quantum Atomic Simulator",
        Point2i(50, 50),
        Window::Type::Game
    );

    if (!primary_window) return -1;

    RenderingGeometry geometry(&primary_window->get_rendering_device());

    // -------------------------------------------------------------------------
    // ESTADO ATÓMICO DINÁMICO
    // -------------------------------------------------------------------------
    int current_Z = 1;
    const int total_samples = 40000; // Aumentado a 40k puntos gracias al multihilo
    std::vector<QuantumElectronSample> electron_cloud;
    electron_cloud.reserve(total_samples);

    std::vector<Nucleon> nucleus;
    float nucleus_pack_rad = 3.5f;
    float last_rebuild_ms = 0.0f;

    Color shell_colors[] = {
        Color(1.0f, 0.25f, 0.20f, 0.85f), // n=1: Carmesí
        Color(0.20f, 0.80f, 1.00f, 0.80f), // n=2: Cian
        Color(0.30f, 1.00f, 0.45f, 0.75f), // n=3: Esmeralda
        Color(1.00f, 0.85f, 0.20f, 0.70f), // n=4: Oro
        Color(0.85f, 0.35f, 1.00f, 0.65f), // n=5: Violeta
        Color(1.00f, 0.45f, 0.80f, 0.60f), // n=6: Rosa magenta
        Color(0.40f, 0.90f, 0.90f, 0.55f)  // n=7: Turquesa
    };

    struct SubshellOrder { int n; int l; };
    const std::vector<SubshellOrder> aufbau = {
        {1, 0}, {2, 0}, {2, 1}, {3, 0}, {3, 1}, {4, 0}, {3, 2}, {4, 1},
        {5, 0}, {4, 2}, {5, 1}, {6, 0}, {4, 3}, {5, 2}, {6, 1}, {7, 0},
        {5, 3}, {6, 2}, {7, 1}
    };

    // -------------------------------------------------------------------------
    // RECONSTRUCCIÓN ATÓMICA PARALELIZADA (JOB SYSTEM)
    // -------------------------------------------------------------------------
    auto rebuild_atom_state_parallel = [&](int Z) {
        auto t_start = std::chrono::high_resolution_clock::now();

        electron_cloud.clear();
        electron_cloud.shrink_to_fit();
        nucleus.clear();

        // 1. Desglose Aufbau y Regla de Hund (Secuencial ligero, <0.01 ms)
        std::vector<OrbitalConfig> occupied;
        int remaining_e = Z;

        for (const auto& sh : aufbau) {
            if (remaining_e <= 0) break;
            int max_e = 2 * (2 * sh.l + 1);
            int take_e = std::min(remaining_e, max_e);

            float shielding = (Z - take_e) * 0.38f;
            float z_eff = std::max(1.0f, static_cast<float>(Z) - shielding);

            int orbitals_count = 2 * sh.l + 1;
            std::vector<int> counts(orbitals_count, 0);

            for (int i = 0; i < orbitals_count && remaining_e > 0 && take_e > 0; ++i) {
                counts[i]++; take_e--; remaining_e--;
            }
            for (int i = 0; i < orbitals_count && remaining_e > 0 && take_e > 0; ++i) {
                counts[i]++; take_e--; remaining_e--;
            }

            for (int i = 0; i < orbitals_count; ++i) {
                if (counts[i] > 0) {
                    occupied.push_back({ sh.n, sh.l, i - sh.l, counts[i], z_eff });
                }
            }
        }

        // 2. Empaquetamiento del Núcleo en Paralelo
        int num_protons = Z;
        int num_neutrons = static_cast<int>(std::round(Z * (Z <= 20 ? 1.0f : (Z <= 60 ? 1.25f : 1.55f))));
        int total_nucleons = num_protons + num_neutrons;
        nucleus_pack_rad = std::pow(static_cast<float>(total_nucleons), 1.0f / 3.0f) * 3.4f;
        nucleus.resize(total_nucleons);

        JobSystem::get().parallel_for(total_nucleons, [&](size_t start, size_t end) {
            std::mt19937 local_rng(static_cast<unsigned int>(Z * 7919 + start * 101 + 42));
            std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);

            for (size_t i = start; i < end; ++i) {
                float theta = dist_01(local_rng) * MathDefs::PI;
                float phi = dist_01(local_rng) * MathDefs::TAU;
                float r = std::pow(dist_01(local_rng), 0.5f) * nucleus_pack_rad;
                nucleus[i] = {
                    Vector3D(r * std::sin(theta) * std::cos(phi), r * std::cos(theta), r * std::sin(theta) * std::sin(phi)),
                    (static_cast<int>(i) < num_protons)
                };
            }
        });

        // 3. Muestreo Cuántico Monte Carlo Paralelizado por Orbitales
        size_t orb_count = occupied.size();
        int samples_per_orb = total_samples / std::max(1, static_cast<int>(orb_count));
        float atom_spatial_scale = 130.0f;

        // Buffers temporales por cada orbital para evitar bloqueo de mutex
        std::vector<std::vector<QuantumElectronSample>> per_orbital_samples(orb_count);

        JobSystem::get().parallel_for(orb_count, [&](size_t start, size_t end) {
            for (size_t orb_idx = start; orb_idx < end; ++orb_idx) {
                const auto& orb = occupied[orb_idx];
                auto& out_samples = per_orbital_samples[orb_idx];
                out_samples.reserve(samples_per_orb);

                std::mt19937 local_rng(static_cast<unsigned int>(Z * 5471 + orb_idx * 997 + 13));
                std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);

                float r_max = (orb.n * orb.n * 2.6f) / orb.z_eff;
                int collected = 0;
                int attempts = 0;

                double max_prob = 0.0;
                for (int test = 0; test < 350; ++test) {
                    double tr = dist_01(local_rng) * r_max;
                    double ttheta = dist_01(local_rng) * MathDefs::PI;
                    double tphi = dist_01(local_rng) * MathDefs::TAU;
                    double val = hydrogen_psi(orb.n, orb.l, orb.m, orb.z_eff, tr, ttheta, tphi);
                    double prob = val * val * (tr * tr);
                    if (prob > max_prob) max_prob = prob;
                }
                if (max_prob < 1e-6) max_prob = 1.0;

                while (collected < samples_per_orb && attempts < samples_per_orb * 50) {
                    attempts++;
                    double r = dist_01(local_rng) * r_max;
                    double theta = dist_01(local_rng) * MathDefs::PI;
                    double phi = dist_01(local_rng) * MathDefs::TAU;

                    double psi = hydrogen_psi(orb.n, orb.l, orb.m, orb.z_eff, r, theta, phi);
                    double p_dens = (psi * psi) * (r * r);

                    if (dist_01(local_rng) * max_prob <= p_dens) {
                        float px = static_cast<float>(r * std::sin(theta) * std::cos(phi)) * atom_spatial_scale;
                        float py = static_cast<float>(r * std::cos(theta)) * atom_spatial_scale;
                        float pz = static_cast<float>(r * std::sin(theta) * std::sin(phi)) * atom_spatial_scale;

                        Color c = shell_colors[(orb.n - 1) % 7];
                        if (psi < 0.0) {
                            c.r = std::min(1.0f, c.r * 1.3f);
                            c.b = std::min(1.0f, c.b * 1.5f);
                        }

                        out_samples.push_back({ Vector3D(px, py, pz), c, static_cast<float>(psi), orb.n });
                        collected++;
                    }
                }
            }
        });

        // 4. Consolidar buffers en el arreglo continuo principal
        for (auto& bucket : per_orbital_samples) {
            electron_cloud.insert(electron_cloud.end(), bucket.begin(), bucket.end());
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        last_rebuild_ms = std::chrono::duration<float, std::milli>(t_end - t_start).count();

        std::cout << "[JobSystem-Parallel] Z=" << Z << " [" << ELEMENT_SYMBOLS[Z] << "]"
                  << " | Orbitales: " << occupied.size() 
                  << " | Muestras: " << electron_cloud.size()
                  << " | Hilos de Trabajo: " << JobSystem::get().get_worker_count()
                  << " | Tiempo: " << last_rebuild_ms << " ms" << std::endl;
    };

    // Reconstrucción inicial
    rebuild_atom_state_parallel(current_Z);

    // -------------------------------------------------------------------------
    // CÁMARA 3D
    // -------------------------------------------------------------------------
    Vector3D cam_pos(0.0f, 90.0f, -520.0f);
    float cam_yaw = 0.0f;
    float cam_pitch = -0.16f;

    const float move_speed = 380.0f;
    const float turn_speed = 1.9f;

    auto prev_time = std::chrono::high_resolution_clock::now();
    float global_spin = 0.0f;

    // -------------------------------------------------------------------------
    // HELPERS DE RENDERIZADO
    // -------------------------------------------------------------------------
    auto draw_text_centered = [&](const Vector2& center, const String& text, const Color& col, float scale) {
        float text_width = static_cast<float>(text.length()) * 9.0f * scale;
        float text_height = 8.0f * scale;
        Vector2 top_left(center.x - text_width * 0.5f, center.y - text_height * 0.5f);
        geometry.draw_text(top_left, text, col, scale);
    };

    auto render_scene = [&]() {
        Size2i cur_size = primary_window->get_size();
        Vector2 screen_center(cur_size.width * 0.5f, cur_size.height * 0.5f);

        primary_window->clear(Color(0.007f, 0.007f, 0.010f, 1.0f));
        geometry.begin();

        float c_spin = Math::cos(global_spin);
        float s_spin = Math::sin(global_spin);

        // 1. Nube de electrones cuántica
        for (const auto& pt : electron_cloud) {
            Vector3D rot(
                pt.pos.x * c_spin - pt.pos.z * s_spin,
                pt.pos.y,
                pt.pos.x * s_spin + pt.pos.z * c_spin
            );

            float depth;
            Vector2 scr = project_3d(rot, cam_pos, cam_yaw, cam_pitch, 640.0f, screen_center, depth);
            if (depth > 1.0f) {
                float rad = (640.0f / depth) * 1.35f;
                rad = Math::clamp(rad, 0.7f, 3.0f);
                geometry.draw_circle(scr, rad, pt.color, true, 6);
            }
        }

        // 2. Núcleo atómico
        for (const auto& n : nucleus) {
            Vector3D rot(
                n.pos.x * c_spin - n.pos.z * s_spin,
                n.pos.y,
                n.pos.x * s_spin + n.pos.z * c_spin
            );

            float d_nuc;
            Vector2 scr = project_3d(rot, cam_pos, cam_yaw, cam_pitch, 640.0f, screen_center, d_nuc);
            if (d_nuc > 1.0f) {
                float rad = (640.0f / d_nuc) * 2.8f;
                Color col = n.is_proton ? Color(1.0f, 0.18f, 0.18f, 0.95f) : Color(0.40f, 0.55f, 0.75f, 0.90f);
                geometry.draw_circle(scr, rad, col, true, 10);
            }
        }

        // 3. Pozo Coulombiano
        float d_center;
        Vector2 nuc_center = project_3d(Vector3D(0, 0, 0), cam_pos, cam_yaw, cam_pitch, 640.0f, screen_center, d_center);
        if (d_center > 1.0f) {
            float halo_r = (640.0f / d_center) * (nucleus_pack_rad * 1.35f);
            geometry.draw_circle(nuc_center, halo_r, Color(1.0f, 0.85f, 0.40f, 0.25f), false, 28);
        }

        // 4. INTERFAZ EN PANTALLA (HUD)
        char title_buf[64];
        std::snprintf(title_buf, sizeof(title_buf), "ATOM Z=%d [%s]", current_Z, ELEMENT_SYMBOLS[current_Z]);
        geometry.draw_text(Vector2(25.0f, 25.0f), title_buf, Color(0.20f, 0.85f, 1.0f, 1.0f), 2.5f);

        char perf_buf[128];
        std::snprintf(perf_buf, sizeof(perf_buf), "CALCULO PARALELO: %.2f ms | HILOS: %zu | MUESTRAS: %zu", 
                      last_rebuild_ms, JobSystem::get().get_worker_count(), electron_cloud.size());
        geometry.draw_text(Vector2(25.0f, 55.0f), perf_buf, Color(0.35f, 1.0f, 0.45f, 0.95f), 1.6f);

        geometry.draw_text(Vector2(25.0f, 85.0f), "[1-8] SALTO RAPIDO | [E/Q] +/- Z | [WASD + FLECHAS] CAMARA", 
                           Color(0.6f, 0.65f, 0.75f, 0.85f), 1.4f);

        geometry.flush();
    };

    primary_window->get_rendering_device().set_render_callback([&]() {
        render_scene();
    });

    // -------------------------------------------------------------------------
    // BUCLE PRINCIPAL
    // -------------------------------------------------------------------------
    while (!primary_window->should_close()) {
        auto cur_time = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float>(cur_time - prev_time).count();
        prev_time = cur_time;
        if (delta > 0.033f) delta = 0.033f;

        Input::update();
        primary_window->poll_events();

        global_spin += delta * 0.22f;

        // Mutaciones cuánticas
        int prev_Z = current_Z;

        if (Input::is_key_pressed(Key::Num1)) current_Z = 1;   // H
        if (Input::is_key_pressed(Key::Num2)) current_Z = 6;   // C
        if (Input::is_key_pressed(Key::Num3)) current_Z = 10;  // Ne
        if (Input::is_key_pressed(Key::Num4)) current_Z = 26;  // Fe
        if (Input::is_key_pressed(Key::Num5)) current_Z = 36;  // Kr
        if (Input::is_key_pressed(Key::Num6)) current_Z = 47;  // Ag
        if (Input::is_key_pressed(Key::Num7)) current_Z = 79;  // Au
        if (Input::is_key_pressed(Key::Num8)) current_Z = 118; // Og

        static float key_cooldown = 0.0f;
        key_cooldown -= delta;
        if (key_cooldown <= 0.0f) {
            if (Input::is_key_pressed(Key::E)) {
                if (current_Z < 118) current_Z++;
                key_cooldown = 0.10f;
            }
            if (Input::is_key_pressed(Key::Q)) {
                if (current_Z > 1) current_Z--;
                key_cooldown = 0.10f;
            }
        }

        if (current_Z != prev_Z) {
            rebuild_atom_state_parallel(current_Z);
        }

        if (Input::is_key_pressed(Key::F11)) {
            static bool fs = false;
            fs = !fs;
            primary_window->set_mode(fs ? Window::Mode::Fullscreen : Window::Mode::Windowed);
        }

        // Cámara FPS
        if (Input::is_key_pressed(Key::Up))    cam_pitch += turn_speed * delta;
        if (Input::is_key_pressed(Key::Down))  cam_pitch -= turn_speed * delta;
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

        render_scene();
        primary_window->swap_buffers();
    }

    JobSystem::get().shutdown();
    renderer.shutdown();
    return 0;
}