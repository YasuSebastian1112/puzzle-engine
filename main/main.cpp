#include "globals/rendering_global/rendering_global.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include <chrono>
#include <vector>

// Representación de un "píxel" o celda para formar el mosaico
struct PixelRect {
    int x, y;
    int w, h;
    Color color;
};

int main() {
    RenderingGlobal& renderer = RenderingGlobal::get();

    const int screen_w = 1280;
    const int screen_h = 720;

    auto primary_window = renderer.create_window(
        Size2i(screen_w, screen_h),
        "Puzzle Engine - Pixel Art Render (ONLY RECTS)",
        Point2i(50, 50),
        Window::Type::Game
    );

    if (!primary_window) return -1;

    RenderingGeometry geometry(&primary_window->get_rendering_device());

    // Paleta de colores de la escena (basada en la imagen del conejito)
    const Color col_dark_bg(0.06f, 0.03f, 0.03f, 1.00f);      // Fondo base oscuro
    const Color col_fire_red(0.85f, 0.18f, 0.08f, 1.00f);     // Rojo fuego intenso
    const Color col_fire_orange(0.95f, 0.38f, 0.12f, 1.00f);   // Naranja ardiente
    const Color col_fire_yellow(0.98f, 0.65f, 0.18f, 1.00f);   // Amarillo / fuego claro
    const Color col_bunny_black(0.06f, 0.06f, 0.08f, 1.00f);   // Cuerpo y orejas
    const Color col_eye_orange(0.98f, 0.60f, 0.12f, 1.00f);    // Ojo izquierdo
    const Color col_eye_blue(0.12f, 0.62f, 0.98f, 1.00f);      // Ojo derecho
    const Color col_mouth_red(0.90f, 0.28f, 0.20f, 1.00f);     // Interior boca
    const Color col_bow_yellow(0.95f, 0.65f, 0.15f, 1.00f);    // Moño / pajarita
    const Color col_earth_brown(0.24f, 0.08f, 0.04f, 1.00f);   // Colinas de tierra
    const Color col_cross_orange(0.90f, 0.40f, 0.15f, 1.00f);  // Cruces de las tumbas

    // Dimensión de los píxeles gruesos (bloques)
    const int px_size = 10;
    const int grid_offset_x = (screen_w - 110 * px_size) / 2; // Centrar el pixel art
    const int grid_offset_y = (screen_h - 75 * px_size) / 2;

    std::vector<PixelRect> scene_pixels;

    // Helper para añadir bloques de píxeles
    auto add_px = [&](int x, int y, int w, int h, const Color& c) {
        scene_pixels.push_back({
            grid_offset_x + x * px_size,
            grid_offset_y + y * px_size,
            w * px_size,
            h * px_size,
            c
        });
    };

    // 1. FONDO DE FUEGO (Mosaico de bloques para recrear las llamas)
    // Fondo base y gradación vertical (de rojo a amarillo)
    add_px(0, 0, 110, 75, col_fire_red);
    // Llamas naranjas (formas ascendentes de fuego)
    add_px(15, 0, 15, 45, col_fire_orange);
    add_px(12, 10, 5, 20, col_fire_orange);
    add_px(32, 0, 48, 55, col_fire_orange); // Llamas del centro detrás de la cabeza
    add_px(38, 5, 35, 48, col_fire_yellow); // Núcleo amarillo
    add_px(80, 12, 15, 38, col_fire_orange);
    add_px(95, 0, 15, 52, col_fire_orange);

    // Salpicaduras de fuego a los lados (detalles verticales)
    add_px(2, 22, 6, 25, col_fire_orange);
    add_px(92, 15, 8, 30, col_fire_orange);
    add_px(4, 18, 3, 10, col_fire_yellow);
    add_px(100, 32, 4, 12, col_fire_yellow);

    // 2. COLINAS DE TIERRA Y CRUCES (En el plano inferior)
    // Tumbas traseras izquierda
    add_px(0, 58, 16, 17, col_earth_brown);
    add_px(16, 52, 20, 23, col_earth_brown);
    // Tumbas traseras derecha
    add_px(75, 52, 20, 23, col_earth_brown);
    add_px(95, 58, 15, 17, col_earth_brown);

    // Cruces de las tumbas de píxeles gruesos
    auto add_cross = [&](int cx, int cy) {
        add_px(cx, cy, 3, 10, col_cross_orange); // Vertical
        add_px(cx - 2, cy + 3, 7, 3, col_cross_orange); // Horizontal
    };
    add_cross(7, 63);
    add_cross(24, 57);
    add_cross(82, 57);
    add_cross(102, 63);

    // 3. PERSONAJE (Cuerpo y cuello)
    add_px(33, 44, 44, 31, col_bunny_black); // Hombros y torso
    // Curvatura del hombro izquierdo
    add_px(30, 54, 3, 21, col_bunny_black);
    add_px(28, 62, 2, 13, col_bunny_black);
    // Curvatura del hombro derecho
    add_px(77, 54, 3, 21, col_bunny_black);
    add_px(80, 62, 2, 13, col_bunny_black);

    // 4. CABEZA Y MANDÍBULA
    add_px(36, 14, 38, 26, col_bunny_black); // Cráneo principal
    add_px(33, 26, 44, 15, col_bunny_black); // Mandíbula y mejillas

    // 5. OREJAS GRANDES Y CAÍDAS
    // Oreja Izquierda
    add_px(28, 10, 18, 10, col_bunny_black); // Base superior
    add_px(22, 14, 8, 16, col_bunny_black);  // Codo
    add_px(18, 22, 8, 33, col_bunny_black);  // Tramo largo caído
    // Oreja Derecha
    add_px(64, 10, 18, 10, col_bunny_black); // Base superior
    add_px(80, 14, 8, 16, col_bunny_black);  // Codo
    add_px(84, 22, 8, 33, col_bunny_black);  // Tramo largo caído

    // 6. OJOS DE COLORES
    // Ojo Izquierdo Naranja
    add_px(44, 20, 6, 10, col_eye_orange);
    add_px(42, 22, 10, 6, col_eye_orange); // Forma redondeada / cruzada
    // Ojo Derecho Azul
    add_px(60, 20, 6, 10, col_eye_blue);
    add_px(58, 22, 10, 6, col_eye_blue);

    // 7. BOCA Y SONRISA DE DIABLILLO
    // Dientes / Borde amarillo exterior de la sonrisa
    add_px(42, 32, 26, 8, col_fire_yellow);
    add_px(39, 30, 3, 6, col_fire_yellow);
    add_px(68, 30, 3, 6, col_fire_yellow);
    // Interior de la boca
    add_px(44, 34, 22, 4, col_bunny_black);
    // Lengua o labios rojos interiores
    add_px(46, 35, 18, 2, col_mouth_red);

    // 8. MOÑO / PAJARITA EN EL CUELLO
    add_px(48, 52, 14, 4, col_bow_yellow); // Nudo central
    add_px(44, 49, 4, 10, col_bow_yellow); // Ala izquierda
    add_px(40, 48, 4, 11, col_bow_yellow);
    add_px(38, 50, 2, 7, col_bow_yellow);
    add_px(62, 49, 4, 10, col_bow_yellow); // Ala derecha
    add_px(66, 48, 4, 11, col_bow_yellow);
    add_px(70, 50, 2, 7, col_bow_yellow);

    while (!primary_window->should_close()) {
        renderer.poll_events();
        renderer.clear_screen(col_dark_bg);

        geometry.begin();

        // RENDERIZADO: Única y exclusivamente a través de la función de rectángulos por lotes (geometry.draw_rect)
        for (const auto& px : scene_pixels) {
            geometry.draw_rect(Rect2(px.x, px.y, px.w, px.h), px.color, true);
        }

        geometry.flush();
        primary_window->swap_buffers();
    }

    renderer.shutdown();
    return 0;
}