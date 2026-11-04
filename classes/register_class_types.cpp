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

// Clases de interfaz

#include "classes/interface/interface.h"
#include "classes/interface/button.h"


void register_core_classes() {
    // Raíz
    ClassDB::register_class<Object>();

    // Nodos Principales
    ClassDB::register_class<Node>();
    ClassDB::register_class<Spatial2D>();
    ClassDB::register_class<Node2D>();
    ClassDB::register_class<Interface>();

    // Nodos 2D
    ClassDB::register_class<Camera2D>();
    ClassDB::register_class<Sprite2D>();

    // Nodos Interfaz
    ClassDB::register_class<Button>();

}

void unregister_core_classes() {
    // Limpieza o desregistro futuro si fuese necesario
}