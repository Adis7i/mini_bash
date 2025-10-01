/**
 * @file move.hpp
 * @brief Declare move
 * 
 * @author Adhyastha A.A
 * @date 2025-10-01
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_MOVE_H
#define ADIS_MOVE_H

void _customsprint(const char* msg);
void fmove(const char* old_path, const char* new_path, bool _CrossDevice, bool _AllowOverwrite);
#endif