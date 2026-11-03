// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "register_class_types.h"
#include "core/class_db.h"

// Clases base
#include "object.h"
#include "classes/node.h"

// Clases 2D
#include "classes/spatial_2d.h"
#include "classes/2d/node_2d.h"
#include "classes/2d/camera_2d.h"
#include "classes/2d/sprite_2d.h"

void register_core_classes() {
    // 1. Raíz de objetos
    ClassDB::register_class<Object>();

    // 2. Jerarquía de nodos
    ClassDB::register_class<Node>();
    ClassDB::register_class<Spatial2D>();
    ClassDB::register_class<Node2D>();

    // 3. Nodos hoja / especializados
    ClassDB::register_class<Camera2D>();
    ClassDB::register_class<Sprite2D>();
}

void unregister_core_classes() {
    // Limpieza o desregistro futuro si fuese necesario
}