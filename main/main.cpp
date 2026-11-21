#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdio>

// Plataforma y Ventana PRIMERO (carga glad.h antes que cualquier header de GLFW)
#include "globals/rendering_global/window.h"
#include <GLFW/glfw3.h>

// Reflexión y Registro de Clases
#include "classes/register_class_types.h"

// Nodos 2D y Recursos
#include "classes/node.h"
#include "classes/2d/node_2d.h"

// Texto Tipográfico y Entrada
#include "globals/rendering_global/text/rendering_text.h"
#include "globals/input/input.h"
#include "core/time/engine_time.h"

// Consola Interactiva
#include "script/puzzle_cli/engine_console.h"

// Helper ligero para formatear floats a string con decimales fijos
static String format_float(float val, int decimals = 2) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, val);
    return String(buf);
}

// ========================================================================= //
// ESTRUCTURAS DEL MODELO ESTÁNDAR Y CAMPOS CUÁNTICOS EN 3D (CAPAS ACOPLADAS)
// ========================================================================= //

enum class ParticleCategory {
    Quark,
    Lepton,
    GaugeBoson,
    ScalarBoson
};

struct StandardModelField {
    int id;
    String cli_name;      // Nombre del nodo en puzzle_cli (ej. "Electron", "Top", "Higgs")
    String symbol;        // Símbolo físico (ej. "u", "e-", "g", "H")
    String display_name;  // Nombre completo
    String mass_str;      // Masa en MeV/c^2 o GeV/c^2
    String charge_str;    // Carga eléctrica
    String spin_str;      // Espín
    ParticleCategory category;
    int table_col;        // Columna en la Tabla Periódica (0..4)
    int table_row;        // Fila en la Tabla Periódica (0..3)
    Color color;

    Node2D* cli_node = nullptr;

    // Malla 3D de onda (Ecuación de Klein-Gordon discretizada en XZ + amplitud en Y)
    static constexpr int NX = 26;
    static constexpr int NZ = 26;
    float height[NX][NZ] = {};
    float prev_height[NX][NZ] = {};
    float wave_speed = 16.0f;
    float mass_damping = 0.0f;
    float damping = 0.988f;
    float current_energy = 0.0f;

    void inject_energy(float norm_x, float norm_z, float energy) {
        int cx = static_cast<int>(norm_x * (NX - 1));
        int cz = static_cast<int>(norm_z * (NZ - 1));
        for (int i = -2; i <= 2; ++i) {
            for (int j = -2; j <= 2; ++j) {
                int gx = cx + i;
                int gz = cz + j;
                if (gx > 1 && gx < NX - 2 && gz > 1 && gz < NZ - 2) {
                    float r2 = static_cast<float>(i * i + j * j);
                    height[gx][gz] += energy * std::exp(-r2 * 0.45f);
                }
            }
        }
    }

    void step(float dt) {
        float c2_dt2 = (wave_speed * dt) * (wave_speed * dt);
        if (c2_dt2 > 0.24f) c2_dt2 = 0.24f;

        float next_h[NX][NZ] = {};
        float total_amp = 0.0f;

        for (int x = 1; x < NX - 1; ++x) {
            for (int z = 1; z < NZ - 1; ++z) {
                float laplacian = height[x + 1][z] + height[x - 1][z]
                                + height[x][z + 1] + height[x][z - 1]
                                - 4.0f * height[x][z];
                float accel = c2_dt2 * laplacian - (mass_damping * dt * dt * height[x][z]);
                next_h[x][z] = (2.0f * height[x][z] - prev_height[x][z] + accel) * damping;
                total_amp += std::abs(next_h[x][z]);
            }
        }

        for (int x = 0; x < NX; ++x) {
            for (int z = 0; z < NZ; ++z) {
                prev_height[x][z] = height[x][z];
                height[x][z] = next_h[x][z];
            }
        }

        current_energy = total_amp * 0.08f;
    }
};

// Cámara Orbital 3D con seguimiento vertical de capa (target_y)
struct Camera3DOrbit {
    float yaw = 0.65f;       // Rotación horizontal (A / D)
    float pitch = 0.48f;     // Inclinación vertical arriba/abajo (W / S)
    float distance = 22.0f;  // Zoom (Q / E)
    float target_y = 0.0f;   // Altura Y de la capa enfocada
    float fov = 45.0f;

    Vector2 project(float x, float y, float z, float vp_w, float vp_h) const {
        float cos_p = std::cos(pitch);
        float sin_p = std::sin(pitch);
        float cos_y = std::cos(yaw);
        float sin_y = std::sin(yaw);

        float cam_x = distance * cos_p * sin_y;
        float cam_y = target_y + distance * sin_p;
        float cam_z = distance * cos_p * cos_y;

        // Vector Forward hacia (0, target_y, 0)
        float fx = -cam_x;
        float fy = target_y - cam_y;
        float fz = -cam_z;
        float flen = std::sqrt(fx * fx + fy * fy + fz * fz);
        fx /= flen; fy /= flen; fz /= flen;

        // Right = normalize(Forward x WorldUp)
        float rx = -fz;
        float ry = 0.0f;
        float rz = fx;
        float rlen = std::sqrt(rx * rx + rz * rz);
        if (rlen > 0.0001f) { rx /= rlen; rz /= rlen; }

        // Camera Up = Right x Forward
        float ux = ry * fz - rz * fy;
        float uy = rz * fx - rx * fz;
        float uz = rx * fy - ry * fx;

        float dx = x - cam_x;
        float dy = y - cam_y;
        float dz = z - cam_z;

        float view_x = dx * rx + dy * ry + dz * rz;
        float view_y = dx * ux + dy * uy + dz * uz;
        float view_z = dx * fx + dy * fy + dz * fz;

        if (view_z < 0.1f) view_z = 0.1f;

        float aspect = vp_w / (vp_h > 1.0f ? vp_h : 1.0f);
        float tan_half_fov = std::tan((fov * 0.5f) * 3.14159265f / 180.0f);

        float ndc_x = view_x / (view_z * tan_half_fov * aspect);
        float ndc_y = view_y / (view_z * tan_half_fov);

        float screen_x = (ndc_x + 1.0f) * 0.5f * vp_w;
        float screen_y = (1.0f - ndc_y) * 0.5f * vp_h;
        return Vector2(screen_x, screen_y);
    }
};

int main() {
    // 1. Inicializar ClassDB y tipos centrales
    register_core_classes();

    // 2. Crear VENTANA 1: Simulación 3D Simultánea de Todos los Campos Cuánticos
    Window win_fields;
    if (!win_fields.initialize("Puzzle Engine - Campos Cuanticos 3D (Todos Simultaneos)", Size2i(920, 760), Point2i(40, 50), Window::Type::Game)) {
        return -1;
    }

    // 3. Crear VENTANA 2: Tabla Periódica del Modelo Estándar y Funciones
    Window win_table;
    if (!win_table.initialize("Puzzle Engine - Tabla Periodica del Modelo Estandar", Size2i(860, 760), Point2i(980, 50), Window::Type::Game)) {
        return -1;
    }

    // 4. Catálogo completo de los 17 campos fundamentales del Modelo Estándar
    Color col_quark(0.68f, 0.42f, 0.96f, 1.0f);   // Violeta (Quarks)
    Color col_lepton(0.24f, 0.82f, 0.54f, 1.0f);  // Verde Esmeralda (Leptones)
    Color col_gauge(0.96f, 0.36f, 0.42f, 1.0f);   // Rojo Coral (Bosones Gauge)
    Color col_higgs(0.98f, 0.76f, 0.20f, 1.0f);   // Dorado (Bosón de Higgs)

    // Ordenados verticalmente de arriba (índice 0) hacia abajo (índice 16)
    std::vector<StandardModelField> sm_fields = {
        // --- BOSÓN ESCALAR (Capa superior: índice 0) ---
        { 0, "Higgs",   "H", "Boson de Higgs",  "125.1 GeV","0",  "0", ParticleCategory::ScalarBoson, 4, 0, col_higgs, nullptr, {}, {}, 12.5f, 12.5f },

        // --- BOSONES GAUGE (Índices 1..4) ---
        { 1, "Gluon",   "g", "Gluon (Fuerte)",  "0",        "0",  "1", ParticleCategory::GaugeBoson, 3, 0, col_gauge, nullptr, {}, {}, 24.0f, 0.0f },
        { 2, "Foton",   "y", "Foton (EM)",      "0",        "0",  "1", ParticleCategory::GaugeBoson, 3, 1, col_gauge, nullptr, {}, {}, 24.0f, 0.0f },
        { 3, "BosonZ",  "Z", "Boson Z (Debil)", "91.2 GeV", "0",  "1", ParticleCategory::GaugeBoson, 3, 2, col_gauge, nullptr, {}, {}, 13.0f, 11.0f },
        { 4, "BosonW",  "W", "Boson W (Debil)", "80.4 GeV", "+-1","1", ParticleCategory::GaugeBoson, 3, 3, col_gauge, nullptr, {}, {}, 13.5f, 10.5f },

        // --- QUARKS (Índices 5..10) ---
        { 5, "Up",      "u", "Quark Up",        "2.2 MeV",   "+2/3", "1/2", ParticleCategory::Quark, 0, 0, col_quark, nullptr, {}, {}, 16.0f, 2.0f },
        { 6, "Down",    "d", "Quark Down",      "4.7 MeV",   "-1/3", "1/2", ParticleCategory::Quark, 0, 1, col_quark, nullptr, {}, {}, 16.0f, 2.5f },
        { 7, "Charm",   "c", "Quark Charm",     "1.28 GeV",  "+2/3", "1/2", ParticleCategory::Quark, 1, 0, col_quark, nullptr, {}, {}, 15.0f, 5.0f },
        { 8, "Strange", "s", "Quark Strange",   "96 MeV",    "-1/3", "1/2", ParticleCategory::Quark, 1, 1, col_quark, nullptr, {}, {}, 15.5f, 4.0f },
        { 9, "Top",     "t", "Quark Top",       "173.1 GeV", "+2/3", "1/2", ParticleCategory::Quark, 2, 0, col_quark, nullptr, {}, {}, 12.0f, 14.0f },
        {10, "Bottom",  "b", "Quark Bottom",    "4.18 GeV",  "-1/3", "1/2", ParticleCategory::Quark, 2, 1, col_quark, nullptr, {}, {}, 14.0f, 8.0f },

        // --- LEPTONES (Índices 11..16) ---
        {11, "Electron",    "e",   "Electron",        "0.511 MeV", "-1",   "1/2", ParticleCategory::Lepton, 0, 2, col_lepton, nullptr, {}, {}, 18.0f, 1.5f },
        {12, "NeutrinoE",   "ve",  "Neutrino e",      "<1.0 eV",   "0",    "1/2", ParticleCategory::Lepton, 0, 3, col_lepton, nullptr, {}, {}, 23.5f, 0.1f },
        {13, "Muon",        "mu",  "Muon",            "105.6 MeV", "-1",   "1/2", ParticleCategory::Lepton, 1, 2, col_lepton, nullptr, {}, {}, 16.5f, 4.0f },
        {14, "NeutrinoMu",  "vmu", "Neutrino mu",     "<0.17 MeV", "0",    "1/2", ParticleCategory::Lepton, 1, 3, col_lepton, nullptr, {}, {}, 23.5f, 0.1f },
        {15, "Tau",         "tau", "Tau",             "1.77 GeV",  "-1",   "1/2", ParticleCategory::Lepton, 2, 2, col_lepton, nullptr, {}, {}, 15.0f, 6.5f },
        {16, "NeutrinoTau", "vtau","Neutrino tau",    "<18.2 MeV", "0",    "1/2", ParticleCategory::Lepton, 2, 3, col_lepton, nullptr, {}, {}, 23.5f, 0.1f }
    };

    // Índices rápidos por nombre para acoplamientos físicos
    const int IDX_HIGGS   = 0;
    const int IDX_GLUON   = 1;
    const int IDX_FOTON   = 2;
    const int IDX_BOSON_Z = 3;
    const int IDX_BOSON_W = 4;
    const int IDX_UP      = 5;
    const int IDX_DOWN    = 6;
    const int IDX_TOP     = 9;
    const int IDX_BOTTOM  = 10;
    const int IDX_ELECTRON= 11;
    const int IDX_NEUTRINO= 12;

    // 5. Registrar nodos en SceneTree para manipularlos desde puzzle_cli
    Node root("root");
    Node2D sm_root("ModeloEstandar");
    root.add_child(&sm_root);

    std::vector<Node2D> field_nodes(sm_fields.size());
    for (size_t i = 0; i < sm_fields.size(); ++i) {
        field_nodes[i].set_name(sm_fields[i].cli_name);
        field_nodes[i].set_position(Vector2(0.5f, 0.5f));
        field_nodes[i].set_scale(Vector2(0.0f, 0.0f));
        sm_fields[i].cli_node = &field_nodes[i];
        sm_root.add_child(&field_nodes[i]);
    }

    // Nodos de disparo para Hadrones y Funciones de Interacción
    Node2D node_proton("Proton");
    node_proton.set_scale(Vector2(0.0f, 0.0f));
    sm_root.add_child(&node_proton);

    Node2D node_neutron("Neutron");
    node_neutron.set_scale(Vector2(0.0f, 0.0f));
    sm_root.add_child(&node_neutron);

    Node2D node_beta("DecaimientoBeta");
    node_beta.set_scale(Vector2(0.0f, 0.0f));
    sm_root.add_child(&node_beta);

    Node2D node_annihilate("Aniquilacion");
    node_annihilate.set_scale(Vector2(0.0f, 0.0f));
    sm_root.add_child(&node_annihilate);

    root.propagate_ready();

    // 6. Cargar fuentes tipográficas independientes para cada contexto OpenGL
    GLFWwindow* glfw_win1 = static_cast<GLFWwindow*>(win_fields.get_rendering_device().get_native_handle());
    GLFWwindow* glfw_win2 = static_cast<GLFWwindow*>(win_table.get_rendering_device().get_native_handle());

    glfwMakeContextCurrent(glfw_win1);
    Ref<RenderingText> font_win1 = new RenderingText();
    font_win1->load_font("JetBrainsMono-Regular.ttf", 17);

    glfwMakeContextCurrent(glfw_win2);
    Ref<RenderingText> font_win2_lg = new RenderingText();
    font_win2_lg->load_font("JetBrainsMono-Regular.ttf", 24);

    Ref<RenderingText> font_win2_sm = new RenderingText();
    font_win2_sm->load_font("JetBrainsMono-Regular.ttf", 13);

    // 7. Iniciar Puzzle CLI
    EngineConsole console;
    console.set_scene_root(&root);
    console.start();

    std::cout << "\n\033[38;2;189;25;57m=== SIMULADOR 3D DE CAMPOS ACOPLADOS DEL MODELO ESTANDAR ===\033[0m\n";
    std::cout << "Controles Ventana 1:\n";
    std::cout << "  [Flecha Arriba / Abajo] -> Moverse entre las 17 capas de campos (de arriba a abajo)\n";
    std::cout << "  [W / S]                 -> Inclinar camara 3D verticalmente\n";
    std::cout << "  [A / D]                 -> Orbitar camara 3D alrededor de los campos\n";
    std::cout << "  [Q / E]                 -> Acercar / Alejar zoom\n";
    std::cout << "  [Espacio]               -> Inyectar pulso de energia en la capa seleccionada\n\n";
    std::cout << "Ejemplos en Terminal (observa como las ondas se transfieren entre capas!):\n";
    std::cout << "  $ModeloEstandar/Higgs scale (6.0, 6.0)       -> Deja caer un Higgs: transfiere masa/energia a Top, W, Z, Quarks y Electron\n";
    std::cout << "  $ModeloEstandar/Proton scale (1.0, 1.0)      -> Forma un Proton: excita Quark Up, Quark Down y Gluones acoplados\n";
    std::cout << "  $ModeloEstandar/Neutron scale (1.0, 1.0)     -> Forma un Neutron: excita Quark Up, Quark Down y Gluones\n";
    std::cout << "  $ModeloEstandar/DecaimientoBeta scale (1, 1) -> Quark Down decae via Boson W en Quark Up + Electron + Neutrino\n";
    std::cout << "  $ModeloEstandar/Aniquilacion scale (1, 1)    -> Electron + Positron transfieren su energia al campo del Foton\n\n";

    Camera3DOrbit cam3d;
    int selected_field_idx = IDX_HIGGS; // Inicia en la capa superior (Higgs)
    constexpr float LAYER_SPACING = 2.4f;
    String active_reaction_msg = "Usa [Flechas Arriba/Abajo] para recorrer las capas o lanza comandos CLI";

    // Excitación inicial de bienvenida en Higgs
    sm_fields[IDX_HIGGS].inject_energy(0.5f, 0.5f, 4.5f);

    Time::init();

    // 8. Bucle Principal Multi-Ventana
    while (!win_fields.should_close() && !win_table.should_close()) {
        Time::update();
        float dt = static_cast<float>(Time::get_process_delta());
        if (dt > 0.05f) dt = 0.05f;

        glfwPollEvents();
        console.process_commands();

        // --- CONTROLES DE CÁMARA 3D (WASD + QE) ---
        float rot_speed = 1.6f * dt;
        if (glfwGetKey(glfw_win1, GLFW_KEY_W) == GLFW_PRESS) {
            cam3d.pitch = std::min(1.42f, cam3d.pitch + rot_speed);
        }
        if (glfwGetKey(glfw_win1, GLFW_KEY_S) == GLFW_PRESS) {
            cam3d.pitch = std::max(-1.42f, cam3d.pitch - rot_speed);
        }
        if (glfwGetKey(glfw_win1, GLFW_KEY_A) == GLFW_PRESS) {
            cam3d.yaw -= rot_speed;
        }
        if (glfwGetKey(glfw_win1, GLFW_KEY_D) == GLFW_PRESS) {
            cam3d.yaw += rot_speed;
        }
        if (glfwGetKey(glfw_win1, GLFW_KEY_E) == GLFW_PRESS) {
            cam3d.distance = std::max(8.0f, cam3d.distance - 14.0f * dt);
        }
        if (glfwGetKey(glfw_win1, GLFW_KEY_Q) == GLFW_PRESS) {
            cam3d.distance = std::min(45.0f, cam3d.distance + 14.0f * dt);
        }

        // --- NAVEGACIÓN ENTRE CAPAS DE CAMPOS CON FLECHAS ARRIBA / ABAJO ---
        static bool up_was_pressed = false;
        static bool down_was_pressed = false;
        bool up_pressed = (glfwGetKey(glfw_win1, GLFW_KEY_UP) == GLFW_PRESS);
        bool down_pressed = (glfwGetKey(glfw_win1, GLFW_KEY_DOWN) == GLFW_PRESS);

        if (up_pressed && !up_was_pressed) {
            selected_field_idx = (selected_field_idx - 1 + static_cast<int>(sm_fields.size())) % static_cast<int>(sm_fields.size());
        }
        if (down_pressed && !down_was_pressed) {
            selected_field_idx = (selected_field_idx + 1) % static_cast<int>(sm_fields.size());
        }
        up_was_pressed = up_pressed;
        down_was_pressed = down_pressed;

        // Suavizado vertical de la cámara hacia la capa seleccionada
        float desired_target_y = -static_cast<float>(selected_field_idx) * LAYER_SPACING;
        cam3d.target_y += (desired_target_y - cam3d.target_y) * std::min(1.0f, 10.0f * dt);

        // Pulso manual con Espacio en la capa actual
        static bool space_was_pressed = false;
        bool space_pressed = (glfwGetKey(glfw_win1, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (space_pressed && !space_was_pressed) {
            sm_fields[selected_field_idx].inject_energy(0.5f, 0.5f, 4.0f);
        }
        space_was_pressed = space_pressed;

        // --- PROCESAR COMANDOS DE TERMINAL SOBRE LOS 17 CAMPOS ---
        for (size_t i = 0; i < sm_fields.size(); ++i) {
            Vector2 energy_cmd = sm_fields[i].cli_node->get_scale();
            if (energy_cmd.x > 0.001f) {
                Vector2 pos_cmd = sm_fields[i].cli_node->get_position();
                float nx = std::clamp(pos_cmd.x, 0.05f, 0.95f);
                float nz = std::clamp(pos_cmd.y, 0.05f, 0.95f);

                sm_fields[i].inject_energy(nx, nz, energy_cmd.x);
                selected_field_idx = static_cast<int>(i);
                active_reaction_msg = "Inyeccion directa en " + sm_fields[i].display_name + " (E=" + format_float(energy_cmd.x, 1) + ")";

                std::cout << "\033[36m[Campo 3D] " << sm_fields[i].display_name.c_str()
                          << " excitado con E = " << energy_cmd.x << " en (" << nx << ", " << nz << ")\033[0m\n";

                sm_fields[i].cli_node->set_scale(Vector2(0.0f, 0.0f));
            }
        }

        // --- PROCESAR FUNCIONES DE INTERACCIÓN (HADRONES Y REACCIONES) ---
        if (node_proton.get_scale().x > 0.001f) {
            float mult = node_proton.get_scale().x;
            sm_fields[IDX_UP].inject_energy(0.42f, 0.45f, 4.5f * mult);    // Quark Up 1
            sm_fields[IDX_UP].inject_energy(0.58f, 0.45f, 4.5f * mult);    // Quark Up 2
            sm_fields[IDX_DOWN].inject_energy(0.50f, 0.58f, 4.5f * mult);  // Quark Down
            sm_fields[IDX_GLUON].inject_energy(0.50f, 0.50f, 5.5f * mult); // Gluones de enlace
            selected_field_idx = IDX_UP;
            active_reaction_msg = "PROTON (uud): Energia reflejada en Quark Up, Quark Down y Gluones!";
            std::cout << "\033[35m[Hadron] Proton (uud) creado! Usa Flecha Abajo para ver Quark Down y Arriba para Gluones.\033[0m\n";
            node_proton.set_scale(Vector2(0.0f, 0.0f));
        }

        if (node_neutron.get_scale().x > 0.001f) {
            float mult = node_neutron.get_scale().x;
            sm_fields[IDX_UP].inject_energy(0.50f, 0.42f, 4.5f * mult);    // Quark Up
            sm_fields[IDX_DOWN].inject_energy(0.42f, 0.58f, 4.5f * mult);  // Quark Down 1
            sm_fields[IDX_DOWN].inject_energy(0.58f, 0.58f, 4.5f * mult);  // Quark Down 2
            sm_fields[IDX_GLUON].inject_energy(0.50f, 0.50f, 5.5f * mult); // Gluones de enlace
            selected_field_idx = IDX_DOWN;
            active_reaction_msg = "NEUTRON (udd): Energia reflejada en Quark Down, Quark Up y Gluones!";
            std::cout << "\033[35m[Hadron] Neutron (udd) creado! Observa las capas u, d y g vibrando juntas.\033[0m\n";
            node_neutron.set_scale(Vector2(0.0f, 0.0f));
        }

        if (node_beta.get_scale().x > 0.001f) {
            sm_fields[IDX_DOWN].inject_energy(0.42f, 0.50f, 4.0f);     // Quark Down decae
            sm_fields[IDX_BOSON_W].inject_energy(0.50f, 0.50f, 5.0f);  // Bosón W mediador
            sm_fields[IDX_UP].inject_energy(0.42f, 0.50f, 4.0f);       // Se transforma en Quark Up
            sm_fields[IDX_ELECTRON].inject_energy(0.66f, 0.44f, 4.8f); // Emite Electrón
            sm_fields[IDX_NEUTRINO].inject_energy(0.66f, 0.56f, 3.8f); // Emite Neutrino e
            selected_field_idx = IDX_BOSON_W;
            active_reaction_msg = "DECAIMIENTO BETA-: d -> u + W- -> u + e- + ve (5 campos acoplados)";
            std::cout << "\033[32m[Interaccion Debil] Decaimiento Beta- ejecutado!\033[0m\n";
            node_beta.set_scale(Vector2(0.0f, 0.0f));
        }

        if (node_annihilate.get_scale().x > 0.001f) {
            sm_fields[IDX_ELECTRON].inject_energy(0.50f, 0.50f, 3.5f); // e- / e+ colisionan
            sm_fields[IDX_FOTON].inject_energy(0.32f, 0.32f, 6.5f);    // Fotón 1
            sm_fields[IDX_FOTON].inject_energy(0.68f, 0.68f, 6.5f);    // Fotón 2
            selected_field_idx = IDX_FOTON;
            active_reaction_msg = "ANIQUILACION: e- + e+ -> 2 Fotones (Energia transferida al campo EM)";
            std::cout << "\033[33m[QED] Aniquilacion e- + e+ -> 2 Fotones (gamma)!\033[0m\n";
            node_annihilate.set_scale(Vector2(0.0f, 0.0f));
        }

        // --- ACOPLAMIENTO FÍSICO CONTINUO ENTRE CAMPOS (INTERACCIÓN DE HIGGS Y GLUONES) ---
        for (int x = 2; x < StandardModelField::NX - 2; x += 2) {
            for (int z = 2; z < StandardModelField::NZ - 2; z += 2) {
                float h_amp = sm_fields[IDX_HIGGS].height[x][z];
                if (std::abs(h_amp) > 0.12f) {
                    float transfer = h_amp * 1.4f * dt;
                    sm_fields[IDX_TOP].height[x][z]      += transfer * 1.00f; // Top acopla fuertísimo con Higgs
                    sm_fields[IDX_BOSON_Z].height[x][z]  += transfer * 0.65f; // Bosón Z adquiere masa
                    sm_fields[IDX_BOSON_W].height[x][z]  += transfer * 0.60f; // Bosón W adquiere masa
                    sm_fields[IDX_BOTTOM].height[x][z]   += transfer * 0.35f; // Quark Bottom
                    sm_fields[IDX_UP].height[x][z]       += transfer * 0.22f; // Quark Up
                    sm_fields[IDX_DOWN].height[x][z]     += transfer * 0.24f; // Quark Down
                    sm_fields[IDX_ELECTRON].height[x][z] += transfer * 0.18f; // Electrón
                }

                // 2. Interacción Fuerte (Gluones): acoplan continuamente Quark Up y Quark Down
                float g_amp = sm_fields[IDX_GLUON].height[x][z];
                if (std::abs(g_amp) > 0.10f) {
                    float strong_coup = g_amp * 1.2f * dt;
                    sm_fields[IDX_UP].height[x][z]   += strong_coup * 0.55f;
                    sm_fields[IDX_DOWN].height[x][z] += strong_coup * 0.55f;
                }
            }
        }

        // Actualizar ecuaciones de onda de los 17 campos simultáneamente
        for (auto& f : sm_fields) {
            f.step(dt);
        }

        // =====================================================================
        // RENDERIZADO VENTANA 1: PILA 3D SIMULTÁNEA DE TODOS LOS CAMPOS
        // =====================================================================
        glfwMakeContextCurrent(glfw_win1);
        int fb1_w = 0, fb1_h = 0;
        glfwGetFramebufferSize(glfw_win1, &fb1_w, &fb1_h);

        if (fb1_w > 0 && fb1_h > 0) {
            win_fields.get_rendering_device().set_screen_size(Size2i(fb1_w, fb1_h));
            win_fields.get_rendering_device().set_viewport(Point2i(0, 0), Size2i(fb1_w, fb1_h));
            win_fields.clear(Color(0.05f, 0.05f, 0.08f, 1.0f));

            RenderingGeometry& g1 = win_fields.get_rendering_geometry();
            g1.begin();

            float w1 = static_cast<float>(fb1_w);
            float h1 = static_cast<float>(fb1_h);
            float world_scale = 12.0f;

            // Dibujar TODOS los 17 campos apilados verticalmente
            for (int f_idx = static_cast<int>(sm_fields.size()) - 1; f_idx >= 0; --f_idx) {
                const auto& field = sm_fields[f_idx];
                bool is_selected = (f_idx == selected_field_idx);
                int layer_dist = std::abs(f_idx - selected_field_idx);

                if (!is_selected && layer_dist > 5 && field.current_energy < 0.25f) {
                    continue;
                }

                float base_y = -static_cast<float>(f_idx) * LAYER_SPACING;
                float alpha_layer = is_selected ? 0.95f : std::max(0.16f, 0.55f - layer_dist * 0.08f);
                float line_thick = is_selected ? 1.5f : 0.9f;

                for (int z = 0; z < StandardModelField::NZ; ++z) {
                    for (int x = 0; x < StandardModelField::NX; ++x) {
                        float nx = (static_cast<float>(x) / (StandardModelField::NX - 1)) - 0.5f;
                        float nz = (static_cast<float>(z) / (StandardModelField::NZ - 1)) - 0.5f;
                        float amp = field.height[x][z];

                        Vector2 p0 = cam3d.project(nx * world_scale, base_y + amp * 1.45f, nz * world_scale, w1, h1);

                        float energy_boost = std::clamp(std::abs(amp) * 0.85f, 0.0f, 0.85f);
                        float base_lum = is_selected ? 0.25f : 0.14f;
                        float lum = std::clamp(base_lum + energy_boost, 0.14f, 1.0f);

                        Color col(field.color.r * lum, field.color.g * lum, field.color.b * lum, alpha_layer);

                        if (x + 1 < StandardModelField::NX) {
                            float nx_r = (static_cast<float>(x + 1) / (StandardModelField::NX - 1)) - 0.5f;
                            float amp_r = field.height[x + 1][z];
                            Vector2 pr = cam3d.project(nx_r * world_scale, base_y + amp_r * 1.45f, nz * world_scale, w1, h1);
                            g1.draw_line(p0, pr, col, line_thick);
                        }
                        if (z + 1 < StandardModelField::NZ) {
                            float nz_d = (static_cast<float>(z + 1) / (StandardModelField::NZ - 1)) - 0.5f;
                            float amp_d = field.height[x][z + 1];
                            Vector2 pd = cam3d.project(nx * world_scale, base_y + amp_d * 1.45f, nz_d * world_scale, w1, h1);
                            g1.draw_line(p0, pd, col, line_thick);
                        }
                    }
                }

                // Etiqueta flotante 3D en el borde izquierdo de cada capa visible
                if (font_win1.is_valid() && (is_selected || layer_dist <= 3 || field.current_energy > 0.3f)) {
                    Vector2 edge_pos = cam3d.project(-0.55f * world_scale, base_y, 0.0f, w1, h1);
                    String tag = (is_selected ? "> " : "  ") + field.cli_name + " [" + field.symbol + "]";
                    Color tag_col = is_selected ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(field.color.r, field.color.g, field.color.b, 0.75f);
                    font_win1->draw_text(&g1, edge_pos + Vector2(-110.0f, -8.0f), tag, tag_col);
                }
            }

            // HUD informativo en Ventana 1
            const auto& af = sm_fields[selected_field_idx];
            if (font_win1.is_valid()) {
                font_win1->draw_text(&g1, Vector2(20.0f, 18.0f),
                    "CAPA ENFOCADA [" + std::to_string(selected_field_idx + 1) + "/17]: " + af.display_name + " (" + af.symbol + ")", af.color);
                font_win1->draw_text(&g1, Vector2(20.0f, 42.0f),
                    "Masa: " + af.mass_str + " | Carga: " + af.charge_str + " | Espin: " + af.spin_str + " | Energia en capa: " + format_float(af.current_energy, 2),
                    Color(0.85f, 0.88f, 0.92f, 1.0f));
                font_win1->draw_text(&g1, Vector2(20.0f, h1 - 32.0f),
                    "[Flechas Arriba/Abajo] Cambiar capa | [W/S] Inclinar | [A/D] Orbitar | [Q/E] Zoom | [Espacio] Pulso",
                    Color(0.62f, 0.68f, 0.75f, 1.0f));
            }

            g1.flush();
            win_fields.swap_buffers();
        }

        // =====================================================================
        // RENDERIZADO VENTANA 2: TABLA PERIÓDICA DEL MODELO ESTÁNDAR
        // =====================================================================
        glfwMakeContextCurrent(glfw_win2);
        int fb2_w = 0, fb2_h = 0;
        glfwGetFramebufferSize(glfw_win2, &fb2_w, &fb2_h);

        if (fb2_w > 0 && fb2_h > 0) {
            win_table.get_rendering_device().set_screen_size(Size2i(fb2_w, fb2_h));
            win_table.get_rendering_device().set_viewport(Point2i(0, 0), Size2i(fb2_w, fb2_h));
            win_table.clear(Color(0.07f, 0.07f, 0.10f, 1.0f));

            RenderingGeometry& g2 = win_table.get_rendering_geometry();
            g2.begin();

            float w2 = static_cast<float>(fb2_w);
            float h2 = static_cast<float>(fb2_h);

            if (font_win2_lg.is_valid()) {
                font_win2_lg->draw_text(&g2, Vector2(25.0f, 18.0f),
                    "TABLA PERIODICA DEL MODELO ESTANDAR", Color(0.95f, 0.95f, 0.98f, 1.0f));
            }

            float margin_x = 25.0f;
            float start_y = 75.0f;
            float gap = 12.0f;
            float cell_w = (w2 - margin_x * 2.0f - gap * 4.0f) / 5.0f;
            float cell_h = (h2 - start_y - 110.0f - gap * 3.0f) / 4.0f;

            if (font_win2_sm.is_valid()) {
                font_win2_sm->draw_text(&g2, Vector2(margin_x, 52.0f), "Gen I", Color(0.7f, 0.7f, 0.8f, 1.0f));
                font_win2_sm->draw_text(&g2, Vector2(margin_x + (cell_w + gap), 52.0f), "Gen II", Color(0.7f, 0.7f, 0.8f, 1.0f));
                font_win2_sm->draw_text(&g2, Vector2(margin_x + (cell_w + gap) * 2, 52.0f), "Gen III", Color(0.7f, 0.7f, 0.8f, 1.0f));
                font_win2_sm->draw_text(&g2, Vector2(margin_x + (cell_w + gap) * 3, 52.0f), "Bosones Gauge", col_gauge);
                font_win2_sm->draw_text(&g2, Vector2(margin_x + (cell_w + gap) * 4, 52.0f), "Escalar", col_higgs);
            }

            // Dibujar las 17 celdas de la tabla periódica cuántica
            for (size_t i = 0; i < sm_fields.size(); ++i) {
                const auto& f = sm_fields[i];
                float cx = margin_x + f.table_col * (cell_w + gap);
                float cy = start_y + f.table_row * (cell_h + gap);
                Rect2 card(cx, cy, cell_w, cell_h);

                bool is_selected = (static_cast<int>(i) == selected_field_idx);
                float activity = std::clamp(f.current_energy * 0.28f, 0.0f, 0.72f);

                Color bg(
                    f.color.r * (0.14f + activity),
                    f.color.g * (0.14f + activity),
                    f.color.b * (0.14f + activity),
                    1.0f
                );
                g2.draw_rect(card, bg, true);

                Color border = is_selected ? Color(1.0f, 1.0f, 1.0f, 1.0f) : f.color;
                g2.draw_rect(card, border, false);

                if (font_win2_lg.is_valid() && font_win2_sm.is_valid()) {
                    font_win2_lg->draw_text(&g2, Vector2(cx + 10.0f, cy + 8.0f), f.symbol, f.color);
                    font_win2_sm->draw_text(&g2, Vector2(cx + 10.0f, cy + 36.0f), f.cli_name, Color(0.95f, 0.95f, 0.95f, 1.0f));
                    font_win2_sm->draw_text(&g2, Vector2(cx + 10.0f, cy + cell_h - 36.0f), "m: " + f.mass_str, Color(0.75f, 0.78f, 0.82f, 1.0f));
                    font_win2_sm->draw_text(&g2, Vector2(cx + 10.0f, cy + cell_h - 20.0f), "q: " + f.charge_str + " | s: " + f.spin_str, Color(0.75f, 0.78f, 0.82f, 1.0f));
                }
            }

            // Panel inferior de Reacciones y Funciones Posibles
            Rect2 footer(margin_x, h2 - 90.0f, w2 - margin_x * 2.0f, 75.0f);
            g2.draw_rect(footer, Color(0.11f, 0.12f, 0.16f, 1.0f), true);
            g2.draw_rect(footer, Color(0.30f, 0.35f, 0.45f, 1.0f), false);

            if (font_win2_sm.is_valid()) {
                font_win2_sm->draw_text(&g2, Vector2(footer.position.x + 12.0f, footer.position.y + 10.0f),
                    "INTERACCION ACTIVA: " + active_reaction_msg, Color(0.40f, 0.92f, 0.65f, 1.0f));
                font_win2_sm->draw_text(&g2, Vector2(footer.position.x + 12.0f, footer.position.y + 32.0f),
                    "Nodos de Reaccion en CLI: Proton | Neutron | DecaimientoBeta | Aniquilacion", Color(0.85f, 0.85f, 0.90f, 1.0f));
                font_win2_sm->draw_text(&g2, Vector2(footer.position.x + 12.0f, footer.position.y + 52.0f),
                    "Ejemplo: $ModeloEstandar/Higgs scale (6, 6)  o  $ModeloEstandar/Proton scale (1, 1)", Color(0.65f, 0.72f, 0.82f, 1.0f));
            }

            g2.flush();
            win_table.swap_buffers();
        }

        Time::delay_to_target_fps();
    }

    console.stop();
    return 0;
}