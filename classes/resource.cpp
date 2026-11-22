// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //



#include "core/class_db.h"


void Resource::_bind_methods(){
    ClassDB::add_property("path", &Resource::set_path, &Resource::get_path);
    ClassDB::add_property("name", &Resource::set_name, &Resource::get_name);
}