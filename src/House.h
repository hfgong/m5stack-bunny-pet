#pragma once
#include <M5Unified.h>
#include "Config.h"

// Non-blocking haptic trigger helper declared in main
extern void vibrateFor(uint8_t level, uint32_t duration_ms);

struct ToyBall {
    float x = 540.0f;
    float y = 195.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    const float radius = 9.0f;

    void update() {
        x += vx;
        y += vy;
        vx *= 0.94f; // friction
        if (abs(vx) < 0.05f) vx = 0.0f;

        // Boundaries within living room
        if (x < 350.0f) { 
            x = 350.0f; 
            if (abs(vx) > 0.8f) vibrateFor(40, 15);
            vx = -vx * 0.6f; 
        }
        if (x > 610.0f) { 
            x = 610.0f; 
            if (abs(vx) > 0.8f) vibrateFor(40, 15);
            vx = -vx * 0.6f; 
        }

        if (y < FLOOR_Y_MIN) { y = FLOOR_Y_MIN; vy = -vy * 0.5f; }
        if (y > FLOOR_Y_MAX) { y = FLOOR_Y_MAX; vy = 0.0f; }
    }

    void kick(float impulse_x) {
        vx += impulse_x;
        vibrateFor(65, 25); // Tactile ball kick rumble
    }

    void draw(M5Canvas& canvas, float cam_x) {
        int sx = (int)(x - cam_x);
        int sy = (int)y;
        if (sx < -20 || sx > SCREEN_W + 20) return;

        // Ball shadow
        canvas.fillEllipse(sx, sy + 7, 8, 3, 0x9492);
        // Colorful beach/play ball (red/yellow/cyan stripes)
        canvas.fillCircle(sx, sy, (int)radius, TFT_RED);
        canvas.fillArc(sx, sy, 0, (int)radius, 45, 135, TFT_YELLOW);
        canvas.fillArc(sx, sy, 0, (int)radius, 225, 315, TFT_CYAN);
        // Highlight reflection
        canvas.fillCircle(sx - 3, sy - 3, 2, TFT_WHITE);
    }
};

class House {
public:
    int food_amount = 80; // 0 to 100
    ToyBall ball;

    void update() {
        ball.update();
    }

    void refillFood() {
        food_amount = 100;
        vibrateFor(50, 30);
    }

    void eatFood(int amount) {
        food_amount = max(0, food_amount - amount);
    }

    void draw(M5Canvas& canvas, float cam_x, int hour, int minute) {
        // Draw the seamless continuous house
        drawContinuousWalls(canvas, cam_x, hour, minute);
        drawContinuousFloors(canvas, cam_x);
        drawFurnitureAndProps(canvas, cam_x, hour);
        ball.draw(canvas, cam_x);
    }

private:
    void drawContinuousWalls(M5Canvas& canvas, float cam_x, int hour, int minute) {
        // Continuous wall band (y = STATUS_BAR_H to 142)
        int wall_h = 142 - STATUS_BAR_H;

        // 1. Kitchen Wall (World X: 0 to 320)
        int kw_sx = (int)(0 - cam_x);
        if (kw_sx + 320 > 0 && kw_sx < SCREEN_W) {
            int clip_left = max(0, kw_sx);
            int clip_right = min(SCREEN_W, kw_sx + 320);
            canvas.fillRect(clip_left, STATUS_BAR_H, clip_right - clip_left, wall_h, COLOR_KITCHEN_WALL);
        }

        // 2. Living Room Wall (World X: 320 to 640)
        int lw_sx = (int)(320 - cam_x);
        if (lw_sx + 320 > 0 && lw_sx < SCREEN_W) {
            int clip_left = max(0, lw_sx);
            int clip_right = min(SCREEN_W, lw_sx + 320);
            canvas.fillRect(clip_left, STATUS_BAR_H, clip_right - clip_left, wall_h, COLOR_LIVING_WALL);
        }

        // 3. Bedroom Wall (World X: 640 to 960)
        int bw_sx = (int)(640 - cam_x);
        if (bw_sx + 320 > 0 && bw_sx < SCREEN_W) {
            int clip_left = max(0, bw_sx);
            int clip_right = min(SCREEN_W, bw_sx + 320);
            canvas.fillRect(clip_left, STATUS_BAR_H, clip_right - clip_left, wall_h, COLOR_BED_WALL);
            
            // Bedroom wallpaper subtle star dots
            for (int wx = 660; wx < 940; wx += 35) {
                for (int wy = STATUS_BAR_H + 20; wy < 130; wy += 30) {
                    int sx = (int)(wx - cam_x);
                    if (sx >= 0 && sx < SCREEN_W) {
                        canvas.drawPixel(sx, wy, 0x73AE);
                    }
                }
            }
        }

        // Baseboard running across the entire continuous house
        canvas.fillRect(0, 138, SCREEN_W, 4, 0x8CD1);

        // Left End Wall (World X: 0..12)
        int left_wall_sx = (int)(0 - cam_x);
        if (left_wall_sx >= -15 && left_wall_sx < SCREEN_W) {
            canvas.fillRect(left_wall_sx, STATUS_BAR_H, 12, SCREEN_H - STATUS_BAR_H, 0x8410);
            canvas.fillRect(left_wall_sx + 10, STATUS_BAR_H, 2, SCREEN_H - STATUS_BAR_H, 0x630C);
        }

        // Right End Wall (World X: 948..960)
        int right_wall_sx = (int)(948 - cam_x);
        if (right_wall_sx < SCREEN_W) {
            canvas.fillRect(right_wall_sx, STATUS_BAR_H, 12, SCREEN_H - STATUS_BAR_H, 0x18C3);
            canvas.fillRect(right_wall_sx - 2, STATUS_BAR_H, 2, SCREEN_H - STATUS_BAR_H, 0x0841);
        }
    }

    void drawContinuousFloors(M5Canvas& canvas, float cam_x) {
        int floor_y = 142;
        int floor_h = SCREEN_H - floor_y;

        // 1. Kitchen Checkered Floor (World X: 0 to 320)
        int kf_sx = (int)(0 - cam_x);
        if (kf_sx + 320 > 0 && kf_sx < SCREEN_W) {
            int start_col = max(0, -kf_sx / 20);
            int end_col = min(16, (SCREEN_W - kf_sx) / 20 + 1);
            for (int col = start_col; col < end_col; col++) {
                int tile_x = kf_sx + col * 20;
                for (int ty = floor_y; ty < SCREEN_H; ty += 20) {
                    int c = (col + (ty / 20)) % 2 == 0 ? COLOR_KITCHEN_TILE1 : COLOR_KITCHEN_TILE2;
                    canvas.fillRect(tile_x, ty, 20, 20, c);
                }
            }
        }

        // 2. Living Room Oak Planks (World X: 320 to 640)
        int lf_sx = (int)(320 - cam_x);
        if (lf_sx + 320 > 0 && lf_sx < SCREEN_W) {
            int clip_left = max(0, lf_sx);
            int clip_right = min(SCREEN_W, lf_sx + 320);
            canvas.fillRect(clip_left, floor_y, clip_right - clip_left, floor_h, COLOR_LIVING_FLOOR);
            // Floor planks lines
            for (int y = floor_y; y < SCREEN_H; y += 18) {
                canvas.drawFastHLine(clip_left, y, clip_right - clip_left, 0xAB11);
            }
        }

        // 3. Bedroom Plush Carpet (World X: 640 to 960)
        int bf_sx = (int)(640 - cam_x);
        if (bf_sx + 320 > 0 && bf_sx < SCREEN_W) {
            int clip_left = max(0, bf_sx);
            int clip_right = min(SCREEN_W, bf_sx + 320);
            canvas.fillRect(clip_left, floor_y, clip_right - clip_left, floor_h, COLOR_BED_FLOOR);
        }

        // Doorway Floor Thresholds & Archway Trims
        // Archway 1 (Kitchen <-> Living Room, World X = 320)
        int arch1_sx = (int)(320 - cam_x);
        if (arch1_sx >= -20 && arch1_sx < SCREEN_W + 20) {
            // Threshold on floor
            canvas.fillRect(arch1_sx - 4, floor_y, 8, floor_h, 0x8A22);
            // Open wooden archway column
            canvas.fillRect(arch1_sx - 5, STATUS_BAR_H, 10, floor_y - STATUS_BAR_H, 0xB575);
            canvas.drawRect(arch1_sx - 5, STATUS_BAR_H, 10, floor_y - STATUS_BAR_H, 0x73AE);
            // Arch top beam
            canvas.fillRect(arch1_sx - 18, STATUS_BAR_H, 36, 8, 0x9492);
        }

        // Archway 2 (Living Room <-> Bedroom, World X = 640)
        int arch2_sx = (int)(640 - cam_x);
        if (arch2_sx >= -20 && arch2_sx < SCREEN_W + 20) {
            // Threshold on floor
            canvas.fillRect(arch2_sx - 4, floor_y, 8, floor_h, 0x6960);
            // Arched wooden frame with cozy curtain drape
            canvas.fillRect(arch2_sx - 5, STATUS_BAR_H, 10, floor_y - STATUS_BAR_H, 0x6185);
            canvas.fillRect(arch2_sx - 18, STATUS_BAR_H, 36, 8, 0x5144);
            // Soft hanging curtain drapes on the sides of doorway
            canvas.fillRoundRect(arch2_sx - 12, STATUS_BAR_H + 8, 7, 60, 3, 0x9334);
            canvas.fillRoundRect(arch2_sx + 5, STATUS_BAR_H + 8, 7, 60, 3, 0x9334);
        }
    }

    void drawFurnitureAndProps(M5Canvas& canvas, float cam_x, int hour) {
        // --- KITCHEN PROPS (World X: 0..320) ---
        // 1. Refrigerator at World X = 35
        int fx = (int)(35 - cam_x);
        int fy = 68;
        if (fx > -60 && fx < SCREEN_W) {
            canvas.fillRoundRect(fx, fy, 48, 112, 5, COLOR_FRIDGE);
            canvas.drawRoundRect(fx, fy, 48, 112, 5, 0x7BEF);
            canvas.drawFastHLine(fx, fy + 42, 48, 0x7BEF);
            canvas.fillRoundRect(fx + 40, fy + 16, 4, 18, 2, 0x4208);
            canvas.fillRoundRect(fx + 40, fy + 52, 4, 26, 2, 0x4208);
            canvas.fillRect(fx + 10, fy + 12, 10, 10, TFT_YELLOW);
            canvas.fillRect(fx + 24, fy + 18, 10, 8, COLOR_HEART_PINK);
        }

        // 2. Kitchen Counter & Stove at World X = 175
        int cx = (int)(175 - cam_x);
        int cy = 110;
        if (cx > -120 && cx < SCREEN_W) {
            canvas.fillRoundRect(cx, cy, 105, 65, 4, COLOR_COUNTER);
            canvas.fillRoundRect(cx - 4, cy, 113, 8, 3, 0xE71C);
            // Stove burner & pan
            canvas.fillEllipse(cx + 25, cy + 5, 12, 4, 0x3186);
            canvas.fillEllipse(cx + 25, cy + 2, 10, 3, 0x18C3);
            int steam_t = (millis() / 250) % 4;
            canvas.fillCircle(cx + 26, cy - 8 - steam_t * 3, 2 + steam_t, 0xEF7D);
            // Utensils rack
            canvas.drawFastHLine(cx + 10, cy - 25, 85, 0x8410);
            canvas.fillCircle(cx + 25, cy - 25, 2, 0x528A);
            canvas.fillCircle(cx + 55, cy - 25, 2, 0x528A);
            canvas.fillCircle(cx + 80, cy - 25, 2, 0x528A);
        }

        // 3. Food Bowl at World X = 115, Y = 180
        int bx = (int)(115 - cam_x);
        int by = 180;
        if (bx > -30 && bx < SCREEN_W + 30) {
            canvas.fillEllipse(bx, by + 12, 18, 6, 0xB575);
            canvas.fillEllipse(bx, by + 8, 16, 7, 0xFD20);
            canvas.fillEllipse(bx, by + 4, 15, 6, 0xFFE0);
            if (food_amount > 0) {
                int food_h = map(food_amount, 0, 100, 2, 6);
                canvas.fillEllipse(bx, by + 4, 12, food_h, 0x9300);
                if (food_amount > 50) {
                    canvas.fillCircle(bx - 4, by + 2, 1, TFT_WHITE);
                    canvas.fillCircle(bx + 5, by + 3, 1, TFT_WHITE);
                }
            }
            canvas.fillCircle(bx, by + 8, 2, TFT_WHITE);
        }

        // --- LIVING ROOM PROPS (World X: 320..640) ---
        // 1. Sofa at World X = 365
        int sx = (int)(365 - cam_x);
        int sy = 120;
        if (sx > -90 && sx < SCREEN_W + 20) {
            canvas.fillRoundRect(sx, sy, 70, 42, 8, COLOR_LIVING_COUCH);
            canvas.fillRoundRect(sx - 4, sy + 25, 78, 20, 6, 0x5CDA);
            canvas.fillRoundRect(sx - 8, sy + 18, 12, 28, 4, COLOR_LIVING_COUCH);
            canvas.fillRoundRect(sx + 66, sy + 18, 12, 28, 4, COLOR_LIVING_COUCH);
            canvas.fillRoundRect(sx + 10, sy + 20, 16, 16, 4, COLOR_LIVING_CUSHION);
        }

        // 2. Picture Window at World X = 480
        int wx = (int)(480 - 45 - cam_x);
        int wy = 44;
        int ww = 90;
        int wh = 74;
        if (wx > -ww && wx < SCREEN_W) {
            uint16_t sky_col = 0x6E5F;
            bool is_night = (hour >= 21 || hour < 6);
            bool is_sunset = (hour >= 18 && hour < 21);
            bool is_morning = (hour >= 6 && hour < 9);

            if (is_night) sky_col = 0x10C8;
            else if (is_sunset) sky_col = 0xEAA9;
            else if (is_morning) sky_col = 0xFCE8;

            canvas.fillRoundRect(wx, wy, ww, wh, 6, sky_col);
            if (is_night) {
                canvas.fillCircle(wx + 22, wy + 20, 8, TFT_YELLOW);
                canvas.fillCircle(wx + 26, wy + 18, 7, sky_col);
                canvas.fillCircle(wx + 52, wy + 16, 1, TFT_WHITE);
                canvas.fillCircle(wx + 72, wy + 26, 1, TFT_WHITE);
                canvas.fillCircle(wx + 38, wy + 42, 1, TFT_WHITE);
            } else {
                canvas.fillCircle(wx + 24, wy + 22, 10, 0xFE40);
                canvas.fillCircle(wx + 60, wy + 28, 9, TFT_WHITE);
                canvas.fillCircle(wx + 70, wy + 26, 11, TFT_WHITE);
                canvas.fillCircle(wx + 78, wy + 30, 8, TFT_WHITE);
            }
            canvas.drawRoundRect(wx, wy, ww, wh, 6, 0xFFFF);
            canvas.drawFastVLine(wx + ww / 2, wy, wh, 0xFFFF);
            canvas.drawFastHLine(wx, wy + wh / 2, ww, 0xFFFF);
            canvas.fillRoundRect(wx - 4, wy + wh - 2, ww + 8, 6, 2, 0xD6BA);
        }

        // 3. Living Room Rug at World X = 480, Y = 190
        int rug_sx = (int)(480 - cam_x);
        if (rug_sx > -80 && rug_sx < SCREEN_W + 80) {
            canvas.fillEllipse(rug_sx, 190, 75, 26, 0xDE74);
            canvas.fillEllipse(rug_sx, 190, 68, 22, 0xF717);
            canvas.drawEllipse(rug_sx, 190, 60, 18, 0xCE52);
        }

        // 4. Potted Plant at World X = 595
        int px = (int)(595 - cam_x);
        int py = 150;
        if (px > -25 && px < SCREEN_W + 25) {
            canvas.fillTriangle(px - 10, py, px + 10, py, px + 7, py + 18, 0xD3A6);
            canvas.fillTriangle(px - 10, py, px + 7, py + 18, px - 7, py + 18, 0xD3A6);
            canvas.fillRoundRect(px - 12, py - 3, 24, 4, 2, 0xE488);
            canvas.fillEllipse(px - 7, py - 12, 7, 12, 0x24C6);
            canvas.fillEllipse(px + 7, py - 10, 8, 11, 0x2DC7);
            canvas.fillEllipse(px, py - 16, 7, 13, 0x3DC8);
        }

        // --- BEDROOM PROPS (World X: 640..960) ---
        // 1. Nightstand with glowing Lamp at World X = 710
        int nx = (int)(710 - cam_x);
        int ny = 125;
        if (nx > -45 && nx < SCREEN_W + 45) {
            canvas.fillRoundRect(nx, ny + 15, 36, 40, 3, COLOR_BED_FRAME);
            canvas.fillRect(nx + 4, ny + 22, 28, 12, 0x6960);
            canvas.fillCircle(nx + 18, ny + 28, 2, 0xDE74);
            canvas.fillRoundRect(nx + 14, ny + 4, 8, 12, 2, 0xDE74);
            canvas.fillTriangle(nx + 6, ny + 6, nx + 30, ny + 6, nx + 25, ny - 14, 0xFFE0);
            canvas.fillTriangle(nx + 6, ny + 6, nx + 25, ny - 14, nx + 11, ny - 14, 0xFFE0);
            canvas.fillCircle(nx + 18, ny - 5, 22, 0x3A22); // subtle glow aura
        }

        // 2. Comfy Bed at World X = 770..890
        int bx2 = (int)(770 - cam_x);
        int by2 = 125;
        if (bx2 > -140 && bx2 < SCREEN_W + 30) {
            canvas.fillRoundRect(bx2 + 110, by2 - 20, 16, 75, 4, COLOR_BED_FRAME);
            canvas.fillRoundRect(bx2, by2 + 30, 120, 24, 4, COLOR_BED_FRAME);
            canvas.fillRoundRect(bx2 + 4, by2 + 10, 110, 32, 6, COLOR_BED_SHEET);
            canvas.fillRoundRect(bx2 + 4, by2 + 20, 75, 26, 6, COLOR_BED_BLANKET);
            canvas.fillRoundRect(bx2 + 78, by2 + 12, 28, 18, 6, TFT_WHITE);
            canvas.drawRoundRect(bx2 + 78, by2 + 12, 28, 18, 6, 0xCE79);
        }

        // 3. Wall Painting at World X = 835
        int pic_x = (int)(835 - cam_x);
        if (pic_x > -40 && pic_x < SCREEN_W + 40) {
            canvas.fillRect(pic_x, STATUS_BAR_H + 20, 30, 24, 0x8A22);
            canvas.fillRect(pic_x + 3, STATUS_BAR_H + 23, 24, 18, 0xFD20);
            canvas.fillCircle(pic_x + 15, STATUS_BAR_H + 32, 4, TFT_YELLOW);
        }
    }
};
