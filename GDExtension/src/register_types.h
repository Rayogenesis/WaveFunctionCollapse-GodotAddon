/* godot-cpp integration testing project.
 *
 * This is free and unencumbered software released into the public domain.
 */

#ifndef WFC_GENERATOR2D_REGISTER_TYPES_H
#define WFC_GENERATOR2D_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_wfc_generator2d_module(ModuleInitializationLevel p_level);
void uninitialize_wfc_generator2d_module(ModuleInitializationLevel p_level);

#endif // WFC_GENERATOR2D_REGISTER_TYPES_H
