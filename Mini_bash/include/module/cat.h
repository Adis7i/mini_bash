/**
 * @file cat.hpp
 * @brief Declaration of a function that shows the content of a file
 * 
 * @author Adhyastha A.A
 * @date 2025-06-30
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_CAT_HPP
#define ADIS_CAT_HPP

#ifdef __cplusplus
extern "C"{
#endif
/**
 * @brief Shows content from a file
 * 
 */
void cat(const char* path);
void customsprint_(const char* msg);
#ifdef __cplusplus
}
#endif
#endif