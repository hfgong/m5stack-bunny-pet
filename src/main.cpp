#include <M5Unified.h>
#include <FastLED.h>
#include "Config.h"
#include "House.h"
#include "Pet.h"

// Hardware instances
CRGB leds[NUM_LEDS];
M5Canvas canvas(&M5.Display);
House house;
Pet pet;

// Camera & Continuous Viewport (World width = 960, Screen width = 320)
float cam_x = 320.0f; // Start in Living Room center
float cam_vx = 0.0f;

// Touch tracking for continuous fluid swiping & pet dragging
int touch_start_x = -1;
int touch_start_y = -1;
int last_touch_x = -1;
uint32_t touch_start_time = 0;
bool is_touch_dragging = false;
bool is_touching_pet = false;
float pet_drag_offset_x = 0.0f;
float pet_drag_offset_y = 0.0f;

// Haptic management (Non-blocking)
uint32_t vibrate_stop_time = 0;

void vibrateFor(uint8_t level, uint32_t duration_ms) {
    M5.Power.setVibration(level);
    vibrate_stop_time = millis() + duration_ms;
}

void updateHaptics() {
    if (vibrate_stop_time > 0 && millis() >= vibrate_stop_time) {
        M5.Power.setVibration(0);
        vibrate_stop_time = 0;
    }
}

// Time & Circadian state
int cur_hour = 14;
int cur_min = 30;
int cur_sec = 0;
uint32_t last_clock_tick = 0;
uint32_t time_toast_timer = 0;
String time_toast = "";

// Track previous goal to display toast
PetGoal prev_goal = GOAL_NONE;

// Upright Desk Tilt Calibration & Deadzone
float ax_baseline = 0.0f;
const float TILT_DEADZONE = 0.28f; // ~17° tilt required to steer; ignores desk tilt and sensor offset

// LED animation state
float led_phase = 0.0f;

void updateTimeFromRTC() {
    auto dt = M5.Rtc.getDateTime();
    if (dt.date.year >= 2024 && dt.time.hours <= 23) {
        cur_hour = dt.time.hours;
        cur_min = dt.time.minutes;
        cur_sec = dt.time.seconds;
    }
}

void setVirtualHour(int new_hour) {
    cur_hour = (new_hour + 24) % 24;
    m5::rtc_datetime_t dt = M5.Rtc.getDateTime();
    dt.time.hours = cur_hour;
    dt.time.minutes = cur_min;
    dt.time.seconds = cur_sec;
    M5.Rtc.setDateTime(dt);

    time_toast = "Time: " + String(cur_hour) + ":" + (cur_min < 10 ? "0" : "") + String(cur_min);
    time_toast_timer = millis() + 1600;
    M5.Speaker.tone(650, 35);
    vibrateFor(40, 25);
}

void setVirtualMinute(int new_min) {
    cur_min = (new_min + 60) % 60;
    m5::rtc_datetime_t dt = M5.Rtc.getDateTime();
    dt.time.hours = cur_hour;
    dt.time.minutes = cur_min;
    dt.time.seconds = 0;
    M5.Rtc.setDateTime(dt);

    time_toast = "Time: " + String(cur_hour) + ":" + (cur_min < 10 ? "0" : "") + String(cur_min);
    time_toast_timer = millis() + 1600;
    M5.Speaker.tone(680, 35);
    vibrateFor(40, 25);
}

void updateLEDs() {
    led_phase += 0.06f;
    uint32_t now = millis();

    CRGB base_color;
    bool is_night = (cur_hour >= 21 || cur_hour < 6);
    bool is_evening = (cur_hour >= 18 && cur_hour < 21);
    bool is_morning = (cur_hour >= 6 && cur_hour < 9);

    if (is_night) {
        base_color = CRGB(18, 5, 42); // Midnight indigo
    } else if (is_morning) {
        base_color = CRGB(255, 125, 30); // Golden sunrise peach
    } else if (is_evening) {
        base_color = CRGB(255, 75, 15); // Sunset amber
    } else {
        base_color = CRGB(50, 160, 255); // Daylight sky blue
    }

    bool active = pet.isActivelyInvolved();

    if (pet.state == PET_DIZZY) {
        bool blink = ((now / 120) % 2) == 0;
        CRGB col = blink ? CRGB(255, 0, 0) : CRGB(20, 0, 0);
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = col;
        FastLED.setBrightness(40);
    } else if (pet.state == PET_PETTED) {
        uint8_t pulse = 140 + sin(led_phase * 3.0f) * 80;
        CRGB pink_col = CRGB(pulse, 40, 95);
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = pink_col;
        FastLED.setBrightness(45);
    } else if (pet.state == PET_EAT) {
        for (int i = 0; i < NUM_LEDS; i++) {
            bool sparkle = ((i + (now / 140)) % 2) == 0;
            leds[i] = sparkle ? CRGB(255, 200, 30) : CRGB(120, 60, 10);
        }
        FastLED.setBrightness(40);
    } else if (active) {
        for (int i = 0; i < NUM_LEDS; i++) {
            float wave = sin(led_phase * 2.2f + i * 0.7f);
            uint8_t boost = (wave > 0) ? (uint8_t)(wave * 80) : 0;
            CRGB lit = base_color;
            lit.r = qadd8(lit.r, boost);
            lit.g = qadd8(lit.g, boost);
            lit.b = qadd8(lit.b, boost);
            leds[i] = lit;
        }
        FastLED.setBrightness(is_night ? 20 : 50);
    } else {
        uint8_t breath = (uint8_t)(sin(led_phase * 0.7f) * 10);
        for (int i = 0; i < NUM_LEDS; i++) {
            CRGB lit = base_color;
            lit.r = qadd8(lit.r, breath);
            lit.g = qadd8(lit.g, breath);
            lit.b = qadd8(lit.b, breath);
            leds[i] = lit;
        }
        FastLED.setBrightness(is_night ? 10 : 35);
    }

    FastLED.show();
}

void drawStatusBar() {
    canvas.fillRect(0, 0, SCREEN_W, STATUS_BAR_H, COLOR_BG_TOPBAR);
    canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, 0x39E7);

    // 1. Clock with Sun/Moon Icon (x = 6 to 58)
    bool is_night = (cur_hour >= 21 || cur_hour < 6);
    int icon_x = 7;
    int icon_y = 12;
    if (is_night) {
        canvas.fillCircle(icon_x, icon_y, 4, TFT_YELLOW);
        canvas.fillCircle(icon_x + 2, icon_y - 2, 3, COLOR_BG_TOPBAR);
    } else {
        canvas.fillCircle(icon_x, icon_y, 3, 0xFE40);
        canvas.drawPixel(icon_x - 4, icon_y, 0xFE40);
        canvas.drawPixel(icon_x + 4, icon_y, 0xFE40);
        canvas.drawPixel(icon_x, icon_y - 4, 0xFE40);
        canvas.drawPixel(icon_x, icon_y + 4, 0xFE40);
    }

    char time_str[12];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", cur_hour, cur_min);
    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE, COLOR_BG_TOPBAR);
    canvas.drawString(time_str, 16, 8);

    // 2. Continuous Mini-Map Radar (x = 62 to 176, width = 114)
    int map_x = 62;
    int map_y = 6;
    int map_w = 114;
    int map_h = 14;

    canvas.fillRoundRect(map_x, map_y, map_w, map_h, 3, 0x18C3);
    canvas.drawRoundRect(map_x, map_y, map_w, map_h, 3, 0x4A69);

    int sep1_x = map_x + map_w / 3;
    int sep2_x = map_x + (map_w * 2) / 3;
    canvas.drawFastVLine(sep1_x, map_y + 2, map_h - 4, 0x31A6);
    canvas.drawFastVLine(sep2_x, map_y + 2, map_h - 4, 0x31A6);

    canvas.setTextColor(0x73AE, 0x18C3);
    canvas.drawCenterString("K", map_x + map_w / 6, map_y + 3);
    canvas.drawCenterString("L", map_x + map_w / 2, map_y + 3);
    canvas.drawCenterString("B", map_x + (map_w * 5) / 6, map_y + 3);

    // Sliding Viewport Window Indicator
    int vp_w = map_w / 3;
    int vp_x = map_x + (int)((cam_x / 640.0f) * (map_w - vp_w));
    vp_x = constrain(vp_x, map_x, map_x + map_w - vp_w);
    canvas.drawRoundRect(vp_x, map_y, vp_w, map_h, 3, 0x5D3F);
    canvas.drawRoundRect(vp_x + 1, map_y + 1, vp_w - 2, map_h - 2, 2, TFT_WHITE);

    // Continuous Pet Marker Dot on Mini-Map (Soft pastel pink)
    int pet_dot_x = map_x + (int)((pet.x / (float)WORLD_W) * map_w);
    pet_dot_x = constrain(pet_dot_x, map_x + 2, map_x + map_w - 2);
    canvas.fillCircle(pet_dot_x, map_y + map_h / 2, 3, COLOR_BUNNY_PINK);
    canvas.drawCircle(pet_dot_x, map_y + map_h / 2, 3, TFT_WHITE);

    // 3. Three Vital Stat Gauges (x = 184 to 280): Fullness (Hunger), Joy (Happiness), Energy
    int fullness = max(0, 100 - pet.hunger);
    int joy = pet.happiness;
    int energy = pet.energy;

    // Stat 1: Fullness / Hunger 🍖 (x = 184)
    canvas.fillCircle(187, 11, 2, 0xFD20);
    canvas.drawFastHLine(185, 13, 5, 0x9300);
    canvas.drawRect(193, 9, 16, 7, 0x4A69);
    int bar1_w = map(constrain(fullness, 0, 100), 0, 100, 0, 14);
    uint16_t col1 = (fullness > 50) ? 0x24C6 : (fullness > 25 ? 0xFE40 : TFT_RED);
    if (bar1_w > 0) canvas.fillRect(194, 10, bar1_w, 5, col1);

    // Stat 2: Joy / Happiness 💖 (x = 217)
    canvas.fillCircle(218, 11, 2, COLOR_HEART_PINK);
    canvas.fillCircle(221, 11, 2, COLOR_HEART_PINK);
    canvas.fillTriangle(216, 12, 223, 12, 219, 15, COLOR_HEART_PINK);
    canvas.drawRect(226, 9, 16, 7, 0x4A69);
    int bar2_w = map(constrain(joy, 0, 100), 0, 100, 0, 14);
    uint16_t col2 = (joy > 50) ? COLOR_HEART_PINK : (joy > 25 ? 0xFE40 : TFT_RED);
    if (bar2_w > 0) canvas.fillRect(227, 10, bar2_w, 5, col2);

    // Stat 3: Energy ⚡ (x = 250)
    canvas.drawLine(252, 9, 250, 12, TFT_YELLOW);
    canvas.drawLine(250, 12, 253, 12, TFT_YELLOW);
    canvas.drawLine(253, 12, 251, 15, TFT_YELLOW);
    canvas.drawRect(258, 9, 16, 7, 0x4A69);
    int bar3_w = map(constrain(energy, 0, 100), 0, 100, 0, 14);
    uint16_t col3 = (energy > 50) ? 0x07FF : (energy > 25 ? 0xFE40 : TFT_RED);
    if (bar3_w > 0) canvas.fillRect(259, 10, bar3_w, 5, col3);

    // 4. Battery Level (x = 286)
    int bat_pct = M5.Power.getBatteryLevel();
    int bx = 286;
    int by = 8;
    canvas.drawRect(bx, by, 18, 9, 0x9CD3);
    canvas.fillRect(bx + 18, by + 2, 2, 5, 0x9CD3);
    int bar_w = map(constrain(bat_pct, 0, 100), 0, 100, 0, 14);
    uint16_t bat_col = (bat_pct > 20) ? 0x24C6 : TFT_RED;
    if (bar_w > 0) canvas.fillRect(bx + 2, by + 2, bar_w, 5, bat_col);

    // 5. Toast notification
    if (millis() < time_toast_timer) {
        canvas.fillRoundRect(50, STATUS_BAR_H + 4, 220, 18, 4, 0x18E3);
        canvas.drawRoundRect(50, STATUS_BAR_H + 4, 220, 18, 4, TFT_WHITE);
        canvas.setTextColor(TFT_WHITE, 0x18E3);
        canvas.drawCenterString(time_toast.c_str(), 160, STATUS_BAR_H + 8);
    }
}

void handleTouch() {
    if (M5.Touch.getCount() > 0) {
        auto t = M5.Touch.getDetail(0);
        int tx = t.x;
        int ty = t.y;

        if (t.wasPressed()) {
            touch_start_x = tx;
            touch_start_y = ty;
            last_touch_x = tx;
            touch_start_time = millis();
            is_touch_dragging = false;
            cam_vx = 0;

            float world_touch_x = cam_x + tx;
            // Hitbox around Bunny (52x62 sprite):
            if (ty >= STATUS_BAR_H && abs(world_touch_x - pet.x) < 32.0f && abs(ty - pet.y) < 35.0f) {
                is_touching_pet = true;
                pet_drag_offset_x = pet.x - world_touch_x;
                pet_drag_offset_y = pet.y - ty;
            } else {
                is_touching_pet = false;
            }
        }

        if (t.isPressed()) {
            int dx = tx - last_touch_x;
            int total_dist = abs(tx - touch_start_x) + abs(ty - touch_start_y);

            if (is_touching_pet) {
                if (total_dist > 7) {
                    if (!pet.is_dragged) {
                        pet.is_dragged = true;
                        pet.state = PET_DRAGGED;
                        pet.current_goal = GOAL_NONE;
                        vibrateFor(30, 20);
                        M5.Speaker.tone(720, 25);
                    }

                    // Directly track player finger
                    pet.x = constrain(cam_x + tx + pet_drag_offset_x, 36.0f, WORLD_W - 36.0f);
                    pet.y = constrain(ty + pet_drag_offset_y, 80.0f, FLOOR_Y_MAX);
                    pet.vx = 0;

                    // Smooth edge auto-scrolling when carrying pet near screen edges
                    if (tx > SCREEN_W - 35 && cam_x < 640.0f) {
                        cam_x = min(640.0f, cam_x + 3.5f);
                    } else if (tx < 35 && cam_x > 0.0f) {
                        cam_x = max(0.0f, cam_x - 3.5f);
                    }

                    // Gentle purr/flutter haptic while held in mid-air
                    if ((millis() / 250) % 2 == 0) {
                        vibrateFor(12, 10);
                    }
                }
            } else if (ty >= STATUS_BAR_H) {
                if (total_dist > 6) {
                    is_touch_dragging = true;
                }

                if (is_touch_dragging && dx != 0) {
                    cam_x -= dx;
                    cam_vx = -dx * 0.75f;
                    last_touch_x = tx;
                }
            }
        }

        if (t.wasReleased()) {
            if (is_touching_pet) {
                if (pet.is_dragged) {
                    // User was dragging and just released him!
                    pet.is_dragged = false;
                    vibrateFor(35, 25);
                    M5.Speaker.tone(480, 30);
                    pet.spawnParticle(pet.x, pet.y + 10, 3); // landing stars

                    // Smart placement based on where you dropped him:
                    // 1. Dropped near food bowl in Kitchen (x = 115)
                    if (abs(pet.x - 115.0f) < 35.0f) {
                        pet.y = 180.0f;
                        pet.setGoal(GOAL_HUNGER, "Snack Time");
                    }
                    // 2. Dropped near sofa in Living Room (x = 380)
                    else if (abs(pet.x - 380.0f) < 40.0f) {
                        pet.y = 168.0f;
                        pet.setGoal(GOAL_COUCH_RELAX, "Lounging");
                    }
                    // 3. Dropped near bed in Bedroom (x = 810)
                    else if (abs(pet.x - 810.0f) < 45.0f) {
                        pet.y = 152.0f;
                        pet.setGoal(GOAL_BEDTIME, "Bedtime");
                    }
                    // 4. Dropped on the open floor
                    else {
                        pet.y = 180.0f;
                        pet.state = PET_IDLE;
                        pet.next_goal_eval_time = millis() + 15000;
                    }
                } else {
                    // Tap without drag -> PET HIM!
                    pet.petPet();
                }
                is_touching_pet = false;
                pet.is_dragged = false;
                return;
            }

            if (!is_touch_dragging) {
                if (ty < STATUS_BAR_H) {
                    // Clock tap (hour / minute)
                    if (tx < 60) {
                        if (tx < 38) {
                            setVirtualHour(cur_hour + 1);
                        } else {
                            setVirtualMinute((cur_min + 5) % 60);
                        }
                        return;
                    }
                    // Mini-Map tap
                    if (tx >= 62 && tx <= 178) {
                        float map_ratio = (tx - 62) / 114.0f;
                        float target = map_ratio * 640.0f;
                        cam_vx = (target - cam_x) * 0.25f;
                        M5.Speaker.tone(600, 30);
                        vibrateFor(35, 20);
                        return;
                    }
                    // Stat meters tap (x = 184 to 282)
                    if (tx >= 184 && tx <= 282) {
                        int fullness = max(0, 100 - pet.hunger);
                        time_toast = "Full:" + String(fullness) + "% Joy:" + String(pet.happiness) + "% Pwr:" + String(pet.energy) + "%";
                        time_toast_timer = millis() + 2500;
                        M5.Speaker.tone(700, 40);
                        vibrateFor(30, 20);
                        return;
                    }
                }

                float world_touch_x = cam_x + tx;

                // Tap on Food Bowl (Kitchen x = 115)
                if (abs(world_touch_x - 115.0f) < 26.0f && abs(ty - 180.0f) < 20.0f) {
                    house.refillFood();
                    time_toast = "Bowl Refilled!";
                    time_toast_timer = millis() + 1500;
                    M5.Speaker.tone(700, 40);
                    return;
                }

                // Tap on Ball (Living room)
                if (abs(world_touch_x - house.ball.x) < 25.0f && abs(ty - house.ball.y) < 25.0f) {
                    house.ball.kick((random(0, 2) == 0 ? 3.5f : -3.5f));
                    M5.Speaker.tone(550, 30);
                    return;
                }
            }

            touch_start_x = -1;
            touch_start_y = -1;
            last_touch_x = -1;
            is_touch_dragging = false;
            is_touching_pet = false;
        }
    }
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(1);
    M5.Display.setBrightness(160);
    canvas.createSprite(SCREEN_W, SCREEN_H);

    M5.Speaker.setVolume(35);
    M5.Speaker.tone(500, 50);

    FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(35);
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    // Parse build timestamp from host computer local time
    int b_hour = 0, b_min = 0, b_sec = 0;
    sscanf(__TIME__, "%d:%d:%d", &b_hour, &b_min, &b_sec);

    char month_str[4] = {0};
    int b_day = 1, b_year = 2026;
    sscanf(__DATE__, "%3s %d %d", month_str, &b_day, &b_year);

    const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int b_month = 9;
    const char* found = strstr(months, month_str);
    if (found) {
        b_month = ((found - months) / 3) + 1;
    }

    auto dt = M5.Rtc.getDateTime();
    bool rtc_invalid = (dt.date.year < 2024 || dt.time.hours > 23);
    bool rtc_lagging = false;
    if (!rtc_invalid) {
        if (dt.date.year < b_year) rtc_lagging = true;
        else if (dt.date.year == b_year && dt.date.month < b_month) rtc_lagging = true;
        else if (dt.date.year == b_year && dt.date.month == b_month && dt.date.date < b_day) rtc_lagging = true;
        else if (dt.date.year == b_year && dt.date.month == b_month && dt.date.date == b_day) {
            int rtc_total_sec = dt.time.hours * 3600 + dt.time.minutes * 60 + dt.time.seconds;
            int b_total_sec = b_hour * 3600 + b_min * 60 + b_sec;
            if (rtc_total_sec + 60 < b_total_sec) {
                rtc_lagging = true;
            }
        }
    }

    if (rtc_invalid || rtc_lagging) {
        m5::rtc_datetime_t init_dt;
        init_dt.date.year = b_year;
        init_dt.date.month = b_month;
        init_dt.date.date = b_day;
        init_dt.time.hours = b_hour;
        init_dt.time.minutes = b_min;
        init_dt.time.seconds = b_sec;
        M5.Rtc.setDateTime(init_dt);
        cur_hour = b_hour;
        cur_min = b_min;
        cur_sec = b_sec;
    } else {
        cur_hour = dt.time.hours;
        cur_min = dt.time.minutes;
        cur_sec = dt.time.seconds;
    }

    // Sample resting IMU baseline while standing upright on desk
    float ax_sum = 0.0f;
    for (int i = 0; i < 20; i++) {
        float sax = 0, say = 0, saz = 0;
        M5.Imu.getAccel(&sax, &say, &saz);
        ax_sum += sax;
        delay(5);
    }
    ax_baseline = ax_sum / 20.0f;

    pet.init();
}

void loop() {
    M5.update();
    updateHaptics();

    uint32_t now = millis();

    // 1. Clock & Circadian Timer
    if (now - last_clock_tick > 1000) {
        last_clock_tick = now;
        cur_sec++;
        if (cur_sec >= 60) {
            cur_sec = 0;
            cur_min++;
            if (cur_min >= 60) {
                cur_min = 0;
                cur_hour = (cur_hour + 1) % 24;
            }
        }

        bool is_night = (cur_hour >= 21 || cur_hour < 6);
        M5.Display.setBrightness(is_night ? 65 : 170);
    }

    // 2. IMU Accelerometer & Gyro Reading (Upright Desk Calibration)
    float ax = 0, ay = 0, az = 0;
    M5.Imu.getAccel(&ax, &ay, &az);

    float gx = 0, gy = 0, gz = 0;
    M5.Imu.getGyro(&gx, &gy, &gz);

    // Auto-drift baseline when resting stationary on desk
    float gyro_motion = abs(gx) + abs(gy) + abs(gz);
    if (gyro_motion < 6.0f && abs(ax - ax_baseline) < 0.18f) {
        ax_baseline = ax_baseline * 0.994f + ax * 0.006f;
    }

    // Front Touch Buttons:
    // Button A (left circle): Hour - 1
    if (M5.BtnA.wasClicked()) {
        setVirtualHour(cur_hour - 1);
    }

    // Button B (center circle): Re-center tilt baseline
    if (M5.BtnB.wasClicked()) {
        ax_baseline = ax;
        time_toast = "Tilt Centered!";
        time_toast_timer = now + 1500;
        M5.Speaker.tone(850, 40);
        vibrateFor(35, 25);
    }

    // Button C (right circle): Hour + 1
    if (M5.BtnC.wasClicked()) {
        setVirtualHour(cur_hour + 1);
    }

    // Serial time synchronization (e.g. send "TIME:22:05:00\n" via USB)
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.startsWith("TIME:")) {
            int h = 0, m = 0, s = 0;
            if (sscanf(cmd.c_str(), "TIME:%d:%d:%d", &h, &m, &s) >= 2) {
                m5::rtc_datetime_t s_dt = M5.Rtc.getDateTime();
                s_dt.time.hours = (h + 24) % 24;
                s_dt.time.minutes = (m + 60) % 60;
                s_dt.time.seconds = (s + 60) % 60;
                M5.Rtc.setDateTime(s_dt);
                cur_hour = s_dt.time.hours;
                cur_min = s_dt.time.minutes;
                cur_sec = s_dt.time.seconds;
                time_toast = "Time: " + String(cur_hour) + ":" + (cur_min < 10 ? "0" : "") + String(cur_min);
                time_toast_timer = now + 1600;
                M5.Speaker.tone(750, 40);
            }
        }
    }

    float total_g = sqrt(ax * ax + ay * ay + az * az);
    bool is_shaken = (total_g > 2.5f);

    // Upright horizontal tilt steering with solid deadzone
    float raw_tilt = -(ax - ax_baseline);
    float tilt_x = 0.0f;
    if (raw_tilt > TILT_DEADZONE) {
        tilt_x = raw_tilt - TILT_DEADZONE;
    } else if (raw_tilt < -TILT_DEADZONE) {
        tilt_x = raw_tilt + TILT_DEADZONE;
    }

    // 3. User Touch & Gestures
    handleTouch();

    // 4. Inertial Camera Motion & Boundary Clamping
    if (!is_touch_dragging) {
        if (abs(tilt_x) > 0.02f) {
            float pet_screen_x = pet.x - cam_x;
            if (pet_screen_x > 230.0f) {
                cam_x += (pet.x - 230.0f - cam_x) * 0.12f;
            } else if (pet_screen_x < 90.0f) {
                cam_x += (pet.x - 90.0f - cam_x) * 0.12f;
            }
        }

        cam_x += cam_vx;
        cam_vx *= 0.88f;
        if (abs(cam_vx) < 0.1f) cam_vx = 0.0f;

        if (cam_x < 0.0f) {
            if (cam_vx < -1.5f) vibrateFor(40, 20);
            cam_x += (0.0f - cam_x) * 0.28f;
            cam_vx *= 0.5f;
        } else if (cam_x > 640.0f) {
            if (cam_vx > 1.5f) vibrateFor(40, 20);
            cam_x += (640.0f - cam_x) * 0.28f;
            cam_vx *= 0.5f;
        }
    } else {
        if (cam_x < 0.0f) cam_x = cam_x * 0.7f;
        if (cam_x > 640.0f) cam_x = 640.0f + (cam_x - 640.0f) * 0.7f;
    }

    // 5. Update House & Pet (Goal planning driven)
    house.update();
    pet.update(tilt_x, is_shaken, house, cur_hour, cur_min, cam_x);

    // Goal change toast notification
    if (pet.current_goal != prev_goal && pet.current_goal != GOAL_NONE && pet.state != PET_SLEEP) {
        prev_goal = pet.current_goal;
        time_toast = String("Goal: ") + pet.goal_description;
        time_toast_timer = now + 2000;
        M5.Speaker.tone(600, 30);
    }

    // 6. Double-Buffered Rendering (Zero flicker)
    house.draw(canvas, cam_x, cur_hour, cur_min);
    pet.draw(canvas, cam_x);
    drawStatusBar();

    canvas.pushSprite(0, 0);

    // 7. Dynamic LED Mood & Active Involvement Blinking
    updateLEDs();

    delay(15);
}
