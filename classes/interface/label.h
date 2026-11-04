// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //
#pragma once

#include "classes/interface/interface.h"
#include "classes/ref_counted.h"
#include "globals/rendering_global/text/rendering_text.h"

class Label : public Interface {
    CLASS_TYPE(Label, Interface);

public:
    enum HorizontalAlignment {
        ALIGN_LEFT,
        ALIGN_CENTER,
        ALIGN_RIGHT
    };

    enum VerticalAlignment {
        VALIGN_TOP,
        VALIGN_CENTER,
        VALIGN_BOTTOM
    };

    Label(const String& p_name = "Label");
    virtual ~Label() override = default;

    void set_text(const String& p_text);
    String get_text() const { return text; }

    void set_font(const Ref<RenderingText>& p_font);
    Ref<RenderingText> get_font() const { return font; }

    void set_horizontal_alignment(HorizontalAlignment p_alignment);
    HorizontalAlignment get_horizontal_alignment() const { return h_align; }

    void set_vertical_alignment(VerticalAlignment p_alignment);
    VerticalAlignment get_vertical_alignment() const { return v_align; }

    // Calcula el tamaño mínimo del nodo a partir del texto
    virtual Vector2 get_minimum_size() const;

    virtual void _draw(RenderingGeometry* p_geometry) override;


private:
    String text = "";
    Ref<RenderingText> font;

    HorizontalAlignment h_align = ALIGN_LEFT;
    VerticalAlignment v_align = VALIGN_TOP;
};