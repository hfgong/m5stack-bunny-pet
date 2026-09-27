#pragma once
#include <Arduino.h>

// Screen Dimensions
#define SCREEN_W 320
#define SCREEN_H 240
#define STATUS_BAR_H 26

// Rooms
#define ROOM_KITCHEN 0
#define ROOM_LIVING  1
#define ROOM_BEDROOM 2
#define NUM_ROOMS    3

// World coordinates
#define ROOM_W 320
#define WORLD_W (ROOM_W * NUM_ROOMS) // 960

// Floor Boundaries
#define FLOOR_Y_MIN 160
#define FLOOR_Y_MAX 205

// AWS EduKit Base SK6812 LEDs
#define LED_PIN 25
#define NUM_LEDS 10

// Colors (RGB565)
#define COLOR_BG_TOPBAR   0x2124 // Dark slate
#define COLOR_TEXT_MUTED  0x9CD3
#define COLOR_GOLD        0xFDC0
#define COLOR_HEART_PINK  0xF9B4

// Bunny Plush Avatar Colors
#define COLOR_BUNNY_CREAM      0xFF3A // Warm cream sherpa wool (#FFF1D8)
#define COLOR_BUNNY_CREAM_DARK 0xDE53 // Shaded warm cream
#define COLOR_BUNNY_PINK       0xFCD5 // Soft pastel ear pink (#F69BB5)
#define COLOR_BUNNY_BLUSH      0xFBAE // Rosy cheek blush
#define COLOR_BUNNY_EYE        0x2882 // Deep warm dark bead eye
#define COLOR_BUNNY_LOGO_G     0x2A9A // Royal indigo embroidered "G" monogram

// Backward compatibility aliases
#define COLOR_MUSE_BLUE       COLOR_BUNNY_PINK
#define COLOR_MUSE_BLUE_DARK  COLOR_BUNNY_CREAM_DARK
#define COLOR_MUSE_BLUE_LIGHT COLOR_BUNNY_CREAM
#define COLOR_MUSE_FACE       COLOR_BUNNY_CREAM
#define COLOR_MUSE_LOGO       COLOR_BUNNY_LOGO_G
#define COLOR_MUSE_EYE        COLOR_BUNNY_EYE
#define COLOR_MUSE_BLUSH      COLOR_BUNNY_BLUSH

// Kitchen Colors
#define COLOR_KITCHEN_WALL   0xEF5B // Warm soft mint/cream
#define COLOR_KITCHEN_TILE1  0xF7BE // Light ivory tile
#define COLOR_KITCHEN_TILE2  0xE657 // Peach tile
#define COLOR_FRIDGE         0xDF7E // Pastel cyan/silver
#define COLOR_COUNTER        0xC552 // Wood counter

// Living Room Colors
#define COLOR_LIVING_WALL    0xFE38 // Warm cozy beige/sunlit
#define COLOR_LIVING_FLOOR   0xCDD0 // Oak wood floor
#define COLOR_LIVING_COUCH   0x4477 // Soft teal/slate couch
#define COLOR_LIVING_CUSHION 0xF52A // Pastel coral

// Bedroom Colors
#define COLOR_BED_WALL       0x296E // Twilight deep indigo
#define COLOR_BED_FLOOR      0x39AF // Dusty plum carpet
#define COLOR_BED_FRAME      0x8A22 // Warm walnut
#define COLOR_BED_SHEET      0xDE7B // Soft cloud blue
#define COLOR_BED_BLANKET    0x9334 // Cozy mauve
