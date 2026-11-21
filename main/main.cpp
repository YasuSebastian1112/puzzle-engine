#include <iostream>
#include "globals/rendering_global/rendering_global.h"
#include "classes/register_class_types.h"
#include "classes/interface/interface.h"
#include "classes/interface/label.h"
#include "classes/interface/button.h"

int main() {
    register_core_classes();

    auto& rg = RenderingGlobal::get();
    rg.init();

    auto win = rg.create_window(Size2i(800, 600), "Puzzle Engine - UI", Point2i(100, 100), Window::Type::Editor);
    if (!win) {
        return -1;
    }

    // Contenedor raíz de la interfaz (se ajusta dinámicamente al tamaño de la ventana)
    Interface ui_root("UIRoot");

    // Label centrado, desplazado hacia arriba del botón
    Label label("LabelTitulo");
    label.set_text("presionalo");
    label.set_horizontal_alignment(Label::ALIGN_CENTER);
    label.set_vertical_alignment(Label::VALIGN_CENTER);
    label.set_anchors_preset(Interface::LayoutPreset::Center);
    label.set_position(Vector2(0.0f, -40.0f));

    // Botón centrado en la ventana
    Button button("BotonAccion");
    button.set_text("presioname");
    button.set_size(Vector2(140.0f, 38.0f));
    button.set_anchors_preset(Interface::LayoutPreset::Center);
    button.set_position(Vector2(0.0f, 10.0f));

    int clicks = 0;
    button.set_on_pressed([&]() {
        clicks++;
        label.set_text("presionado: " + std::to_string(clicks));
    });

    ui_root.add_child(&label);
    ui_root.add_child(&button);

    // Dibujado de la interfaz dentro del viewport de la ventana
    win->set_frame_callback([&](double delta) {
        Size2i win_size = win->get_size();
        ui_root.set_size(Vector2(static_cast<float>(win_size.width), static_cast<float>(win_size.height)));

        button._process(static_cast<float>(delta));
        ui_root.propagate_draw(&win->get_rendering_geometry());
    });

    rg.run(nullptr, RenderingGlobal::ClosePolicy::WhenPrimaryClosed);
    return 0;
}