/**
 * @file callback.h
 * @brief Global callback definitions for windowing and input handling.
 */

#pragma once

 /**
  * @brief Callback function called when the window needs to be repainted.
  */
void displayCallback();

/**
 * @brief Callback function called when the window is resized.
 * @param width The new width of the window.
 * @param height The new height of the window.
 */
void reshapeCallback(int width, int height);

/**
 * @brief Callback function for standard ASCII keyboard events.
 * @param key The ASCII code of the pressed key.
 * @param mouseX The X position of the mouse when the key was pressed.
 * @param mouseY The Y position of the mouse when the key was pressed.
 */
void keyboardCallback(unsigned char key, int mouseX, int mouseY);

/**
 * @brief Callback function for standard ASCII keyboard key release events.
 * @param key The ASCII code of the released key.
 * @param mouseX The X position of the mouse when the key was released.
 * @param mouseY The Y position of the mouse when the key was released.
 */
void keyboardUpCallback(unsigned char key, int mouseX, int mouseY);

/**
 * @brief Callback function for special keyboard events (e.g., arrow keys, function keys).
 * @param key The code of the special key pressed.
 * @param mouseX The X position of the mouse when the key was pressed.
 * @param mouseY The Y position of the mouse when the key was pressed.
 */
void specialCallback(int key, int mouseX, int mouseY);