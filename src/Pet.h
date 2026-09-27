#pragma once
#include <M5Unified.h>
#include "Config.h"
#include "House.h"
#include "AvatarSprites.h"

extern void vibrateFor(uint8_t level, uint32_t duration_ms);

enum PetState {
    PET_IDLE,
    PET_WALK,
    PET_DRAGGED,
    PET_EAT,
    PET_PLAY,
    PET_SLEEP,
    PET_PETTED,
    PET_CELEBRATE,
    PET_YAWN,
    PET_TIRED,
    PET_DIZZY
};

enum PetGoal {
    GOAL_NONE,
    GOAL_HUNGER,         // Go eat at Kitchen food bowl
    GOAL_PLAY_BALL,      // Play & kick toy ball in Living Room
    GOAL_WINDOW_GAZE,    // Look out the big scenic window
    GOAL_COUCH_RELAX,    // Take a cozy rest on the sofa
    GOAL_BEDTIME,        // Head to bed for deep sleep
    GOAL_ASK_ATTENTION   // Seek player affection
};

struct Particle {
    float x, y;
    float vx, vy;
    int type; // 0 = heart, 1 = Zzz, 2 = crumb, 3 = music/star
    int life;
    int max_life;
};

class Pet {
public:
    float x = 480.0f; // Start in Living Room center
    float y = 180.0f;
    float vx = 0.0f;
    bool facing_right = true;
    PetState state = PET_IDLE;

    // Draggable & Movement State
    bool is_dragged = false;
    uint32_t tilt_start_time = 0;
    bool was_user_tilting = false;
    uint32_t last_idle_anim_time = 0;

    // Goal-Based Planning
    PetGoal current_goal = GOAL_NONE;
    int goal_step = 0;
    uint32_t goal_timer = 0;
    uint32_t next_goal_eval_time = 0;
    const char* goal_description = "Relaxing";

    // Intrinsic Drives (0 - 100)
    int hunger = 35;
    int energy = 85;
    int boredom = 40;
    int loneliness = 30;
    int happiness = 80;

    // Timers
    uint32_t state_timer = 0;
    uint32_t last_dizzy_time = 0;
    uint32_t last_drive_tick = 0;
    float walk_cycle = 0.0f;
    uint32_t last_footstep_haptic = 0;

    bool is_active_action = false;

    // Particles
    static const int MAX_PARTICLES = 12;
    Particle particles[MAX_PARTICLES];

    void init() {
        for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;
        next_goal_eval_time = millis() + 8000;
        last_drive_tick = millis();
    }

    int getRoom() const {
        if (x < ROOM_W) return ROOM_KITCHEN;
        if (x < ROOM_W * 2) return ROOM_LIVING;
        return ROOM_BEDROOM;
    }

    bool isActivelyInvolved() const {
        return (state == PET_EAT || 
                state == PET_PETTED || 
                state == PET_DIZZY || 
                state == PET_DRAGGED ||
                state == PET_CELEBRATE ||
                state == PET_YAWN ||
                current_goal == GOAL_PLAY_BALL ||
                (state == PET_WALK && abs(vx) > 0.6f) ||
                is_active_action);
    }

    void spawnParticle(float px, float py, int type) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life <= 0) {
                particles[i].x = px;
                particles[i].y = py;
                particles[i].vx = (random(-10, 11) / 10.0f);
                particles[i].vy = -1.0f - (random(0, 8) / 10.0f);
                particles[i].type = type;
                particles[i].max_life = (type == 1) ? 55 : 32;
                particles[i].life = particles[i].max_life;
                break;
            }
        }
    }

    // --- GOAL-BASED PLANNING ENGINE ---
    void evaluateGoals(int hour, House& house, float cam_x) {
        bool is_night = (hour >= 21 || hour < 7);
        bool is_meal_time = ((hour >= 7 && hour <= 9) || (hour >= 18 && hour <= 19));

        // Priority 1: Sleep at night or when exhausted
        if (is_night || energy < 18) {
            setGoal(GOAL_BEDTIME, "Bedtime");
            return;
        }

        // Priority 2: Hunger
        if (hunger > 55 || (is_meal_time && hunger > 25)) {
            setGoal(GOAL_HUNGER, "Snack Time");
            return;
        }

        // Priority 3: Crave attention if lonely
        if (loneliness > 65) {
            setGoal(GOAL_ASK_ATTENTION, "Needs Love");
            return;
        }

        // Priority 4: Play with ball if bored and energetic
        if (boredom > 50 && energy > 35) {
            setGoal(GOAL_PLAY_BALL, "Play Ball");
            return;
        }

        // Priority 5: Content / Resting in place (relaxing and enjoying room)
        if (random(0, 3) != 0) {
            next_goal_eval_time = millis() + random(18000, 32000);
            return;
        }

        // Priority 6: Sightseeing / Window gazing or Couch relaxation
        if (random(0, 2) == 0) {
            setGoal(GOAL_WINDOW_GAZE, "Stargazing");
        } else {
            setGoal(GOAL_COUCH_RELAX, "Lounging");
        }
    }

    void setGoal(PetGoal g, const char* desc) {
        current_goal = g;
        goal_step = 0;
        goal_timer = millis();
        goal_description = desc;
    }

    void executeGoal(House& house, float cam_x, int hour) {
        uint32_t now = millis();

        switch (current_goal) {
            case GOAL_HUNGER: {
                // Step 0: Walk to food bowl at x = 115
                if (goal_step == 0) {
                    if (abs(x - 115.0f) > 16.0f) {
                        walkTowards(115.0f, 1.1f);
                    } else {
                        vx = 0;
                        goal_step = 1;
                        goal_timer = now;
                    }
                } 
                // Step 1: Arrived at bowl
                else if (goal_step == 1) {
                    if (house.food_amount > 0) {
                        state = PET_EAT;
                        state_timer = now + 4000;
                        goal_step = 2;
                    } else {
                        // Empty bowl! Wait and plead
                        state = PET_IDLE;
                        if (now - goal_timer > 6000) {
                            // Give up for now
                            current_goal = GOAL_NONE;
                            next_goal_eval_time = now + 4000;
                        }
                    }
                }
                // Step 2: Finished eating
                else if (goal_step == 2) {
                    if (state != PET_EAT) {
                        hunger = max(0, hunger - 45);
                        happiness = min(100, happiness + 20);
                        spawnParticle(x, y - 18, 0); // heart
                        vibrateFor(40, 30);
                        current_goal = GOAL_NONE;
                        next_goal_eval_time = now + 20000;
                    }
                }
                break;
            }

            case GOAL_PLAY_BALL: {
                // Step 0: Move towards ball
                if (goal_step == 0) {
                    float target_ball = house.ball.x;
                    if (abs(x - target_ball) > 22.0f) {
                        walkTowards(target_ball, 1.3f);
                    } else {
                        // Kick ball!
                        float kick_dir = facing_right ? 3.8f : -3.8f;
                        house.ball.kick(kick_dir);
                        spawnParticle(house.ball.x, house.ball.y - 10, 3);
                        M5.Speaker.tone(600, 35);
                        boredom = max(0, boredom - 30);
                        happiness = min(100, happiness + 15);
                        goal_step = 1;
                        goal_timer = now + 1200;
                    }
                }
                // Step 1: Chase after rolling ball
                else if (goal_step == 1) {
                    if (now > goal_timer) {
                        float target_ball = house.ball.x;
                        if (abs(x - target_ball) > 20.0f) {
                            walkTowards(target_ball, 1.2f);
                        } else {
                            // Second victory kick!
                            house.ball.kick(facing_right ? 3.0f : -3.0f);
                            spawnParticle(x, y - 18, 3);
                            M5.Speaker.tone(700, 40);
                            boredom = 0;
                            current_goal = GOAL_NONE;
                            next_goal_eval_time = now + 24000;
                        }
                    }
                }
                break;
            }

            case GOAL_WINDOW_GAZE: {
                // Walk to window at x = 480
                if (goal_step == 0) {
                    if (abs(x - 480.0f) > 15.0f) {
                        walkTowards(480.0f, 1.0f);
                    } else {
                        vx = 0;
                        state = PET_IDLE;
                        goal_step = 1;
                        goal_timer = now + 8000; // gaze for 8 seconds
                    }
                } else if (goal_step == 1) {
                    if (now > goal_timer) {
                        boredom = max(0, boredom - 20);
                        happiness = min(100, happiness + 10);
                        current_goal = GOAL_NONE;
                        next_goal_eval_time = now + 20000;
                    }
                }
                break;
            }

            case GOAL_COUCH_RELAX: {
                // Walk to sofa at x = 380
                if (goal_step == 0) {
                    if (abs(x - 380.0f) > 15.0f) {
                        walkTowards(380.0f, 1.0f);
                    } else {
                        vx = 0;
                        y = 168.0f; // climb onto couch cushion
                        state = PET_IDLE;
                        goal_step = 1;
                        goal_timer = now + 10000; // lounge for 10 seconds
                    }
                } else if (goal_step == 1) {
                    if (now > goal_timer) {
                        y = 180.0f; // hop down
                        energy = min(100, energy + 15);
                        current_goal = GOAL_NONE;
                        next_goal_eval_time = now + 22000;
                    }
                }
                break;
            }

            case GOAL_BEDTIME: {
                // Walk to bedroom bed at x = 810
                if (goal_step == 0) {
                    if (abs(x - 810.0f) > 18.0f) {
                        walkTowards(810.0f, 0.9f);
                    } else {
                        vx = 0;
                        y = 152.0f; // tuck into bed
                        state = PET_SLEEP;
                        goal_step = 1;
                    }
                } else if (goal_step == 1) {
                    state = PET_SLEEP;
                    energy = min(100, energy + 1);
                }
                break;
            }

            case GOAL_ASK_ATTENTION: {
                // Seek screen center in view
                float screen_center = cam_x + 160.0f;
                if (goal_step == 0) {
                    if (abs(x - screen_center) > 20.0f) {
                        walkTowards(screen_center, 1.1f);
                    } else {
                        vx = 0;
                        state = PET_IDLE;
                        goal_step = 1;
                        goal_timer = now + 8000;
                    }
                } else if (goal_step == 1) {
                    // Waiting for player pet
                    if (state == PET_PETTED) {
                        loneliness = 0;
                        current_goal = GOAL_NONE;
                        next_goal_eval_time = now + 20000;
                    } else if (now > goal_timer) {
                        current_goal = GOAL_NONE;
                        next_goal_eval_time = now + 12000;
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    void walkTowards(float target_x, float speed) {
        state = PET_WALK;
        if (target_x > x + 4.0f) {
            vx = speed;
            facing_right = true;
        } else if (target_x < x - 4.0f) {
            vx = -speed;
            facing_right = false;
        } else {
            vx = 0;
            state = PET_IDLE;
        }
    }

    void update(float tilt_x, bool is_shaken, House& house, int hour, int minute, float cam_x) {
        uint32_t now = millis();
        is_active_action = false;

        // If dragged by player touch, skip normal physics and autonomy
        if (is_dragged) {
            vx = 0;
            state = PET_DRAGGED;
            current_goal = GOAL_NONE;
            updateParticles();
            return;
        }

        // Drive increments over time (~ every 12s)
        if (now - last_drive_tick > 12000) {
            last_drive_tick = now;
            hunger = min(100, hunger + 1);
            boredom = min(100, boredom + 2);
            loneliness = min(100, loneliness + 1);
            if (state == PET_SLEEP) {
                energy = min(100, energy + 6);
                happiness = min(100, happiness + 1);
            } else {
                energy = max(0, energy - 1);
                if (loneliness > 45) {
                    happiness = max(0, happiness - 1);
                }
            }
        }

        // 1. Shake / Startle Reaction
        if (is_shaken && state != PET_DIZZY && (now - last_dizzy_time > 4000)) {
            state = PET_DIZZY;
            last_dizzy_time = now;
            state_timer = now + 1800;
            vx = 0;
            M5.Speaker.tone(650, 40);
            vibrateFor(80, 40);
        }

        if (state == PET_DIZZY) {
            if (now > state_timer) {
                state = PET_IDLE;
            }
            updateParticles();
            return;
        }

        // 2. Directing via Tilt (Initial burst is fast, then smoothly converges to cruising speed!)
        bool user_tilting = (abs(tilt_x) > 0.02f);

        if (user_tilting) {
            // Track start of tilt or direction reversal for initial speed burst
            if (!was_user_tilting || (tilt_x > 0 && vx < -0.2f) || (tilt_x < 0 && vx > 0.2f)) {
                tilt_start_time = now;
            }
            was_user_tilting = true;

            // Cancel current autonomous plan if user manually directs him
            if (current_goal != GOAL_NONE && current_goal != GOAL_BEDTIME) {
                current_goal = GOAL_NONE;
                next_goal_eval_time = now + 5000;
            }

            if (state == PET_SLEEP && abs(tilt_x) > 0.18f) {
                state = PET_IDLE;
                y = 180;
                vibrateFor(40, 25);
            }

            if (state != PET_SLEEP) {
                state = PET_WALK;

                // Initial burst factor: starts at 2.25x and exponentially decays to 1.0x over ~650ms
                uint32_t elapsed = now - tilt_start_time;
                float burst_factor = 1.0f + 1.25f * expf(-((float)elapsed) / 280.0f);
                float max_speed = 2.0f * burst_factor; // ~4.5 px/frame initially, converges to 2.0 px/frame

                float target_vx = constrain(tilt_x * 5.2f * burst_factor, -max_speed, max_speed);
                vx = vx * 0.45f + target_vx * 0.55f;

                if (tilt_x > 0.01f) facing_right = true;
                if (tilt_x < -0.01f) facing_right = false;

                uint32_t step_interval = (uint32_t)(380 / constrain(burst_factor, 1.0f, 2.0f));
                if (abs(vx) > 0.8f && now - last_footstep_haptic > step_interval) {
                    last_footstep_haptic = now;
                    vibrateFor(16, 12);
                }
            }
        } else {
            was_user_tilting = false;
            tilt_start_time = 0;

            // Decelerate rapidly to full stop if not tilting
            if (current_goal == GOAL_NONE) {
                vx *= 0.55f;
                if (abs(vx) < 0.08f) {
                    vx = 0.0f;
                    if (state == PET_WALK) {
                        state = PET_IDLE;
                    }
                }
            }
        }

        // 3. Autonomous Goal Planning (When user is not actively tilting)
        if (!user_tilting && state != PET_PETTED && state != PET_EAT && state != PET_DIZZY && state != PET_CELEBRATE && state != PET_YAWN) {
            if (current_goal == GOAL_NONE) {
                if (now > next_goal_eval_time) {
                    evaluateGoals(hour, house, cam_x);
                }
            } else {
                executeGoal(house, cam_x, hour);
            }
        }

        // 4. State-specific logic
        if (state == PET_EAT) {
            is_active_action = true;
            if ((now / 200) % 2 == 0 && random(0, 3) == 0) {
                house.eatFood(1);
                spawnParticle(x + (facing_right ? 12 : -12), y + 4, 2);
                M5.Speaker.tone(480 + random(-30, 30), 20);
                vibrateFor(22, 14);
            }
            if (now > state_timer) {
                hunger = max(0, hunger - 45);
                happiness = min(100, happiness + 20);
                state = PET_CELEBRATE;
                state_timer = now + 1600;
                M5.Speaker.tone(680, 50);
                vibrateFor(35, 30);
            }
        } else if (state == PET_CELEBRATE) {
            is_active_action = true;
            if (random(0, 5) == 0) {
                spawnParticle(x + random(-16, 16), y - 18, 3);
            }
            if (now > state_timer) {
                state = PET_IDLE;
            }
        } else if (state == PET_YAWN) {
            is_active_action = true;
            if (now > state_timer) {
                state = PET_IDLE;
            }
        } else if (state == PET_SLEEP) {
            if (random(0, 50) == 0) {
                spawnParticle(x + 10, y - 18, 1);
            }
        } else if (state == PET_PETTED) {
            is_active_action = true;
            if (now < state_timer) {
                if ((now / 160) % 2 == 0) {
                    vibrateFor(40, 80);
                }
                if (random(0, 8) == 0) {
                    spawnParticle(x + random(-14, 14), y - 16, 0);
                }
            } else {
                if (happiness >= 90 && random(0, 2) == 0) {
                    state = PET_CELEBRATE;
                    state_timer = now + 1600;
                } else {
                    state = PET_IDLE;
                }
            }
        }

        // 5. Spontaneous Idle Behaviors (Yawning, Celebrating, Slumping)
        if (state == PET_IDLE && current_goal == GOAL_NONE && now - last_idle_anim_time > 15000) {
            last_idle_anim_time = now;
            if (energy < 25) {
                state = PET_YAWN;
                state_timer = now + 1800;
                M5.Speaker.tone(360, 45);
            } else if (happiness > 85 && random(0, 3) == 0) {
                state = PET_CELEBRATE;
                state_timer = now + 1600;
                M5.Speaker.tone(740, 40);
                spawnParticle(x, y - 18, 3);
            }
        }

        // 6. Physics integration
        x += vx;
        if (!user_tilting && current_goal == GOAL_NONE) {
            vx *= 0.70f;
            if (abs(vx) < 0.05f) {
                vx = 0.0f;
                if (state == PET_WALK) state = PET_IDLE;
            }
        }

        if (state == PET_WALK && abs(vx) > 0.08f) {
            walk_cycle += constrain(abs(vx) * 0.08f, 0.03f, 0.14f);
        } else {
            walk_cycle = 0.0f;
        }

        // Ball interaction on touch
        if (abs(x - house.ball.x) < 26.0f && abs(y - house.ball.y) < 20.0f && current_goal != GOAL_PLAY_BALL) {
            is_active_action = true;
            float kick_dir = (x < house.ball.x) ? 3.2f : -3.2f;
            house.ball.kick(kick_dir + vx * 1.5f);
            spawnParticle(house.ball.x, house.ball.y - 10, 3);
            M5.Speaker.tone(550, 30);
            happiness = min(100, happiness + 10);
            state = PET_CELEBRATE;
            state_timer = now + 1200;
        }

        // World Bounds
        if (x < 36.0f) { x = 36.0f; vx = 0; }
        if (x > WORLD_W - 36.0f) { x = WORLD_W - 36.0f; vx = 0; }

        updateParticles();
    }

    void petPet() {
        state = PET_PETTED;
        state_timer = millis() + 2000;
        happiness = min(100, happiness + 18);
        loneliness = 0;
        is_active_action = true;
        M5.Speaker.tone(587, 45);
        vibrateFor(50, 60);
        spawnParticle(x, y - 18, 0);
    }

    void updateParticles() {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                particles[i].x += particles[i].vx;
                particles[i].y += particles[i].vy;
                particles[i].life--;
            }
        }
    }

    // DRAW BUNNY PLUSH SPRITE & THOUGHT BUBBLE
    void draw(M5Canvas& canvas, float cam_x) {
        drawParticles(canvas, cam_x);

        int sx = (int)(x - cam_x);
        int sy = (int)y;

        if (sx < -AVATAR_W || sx > SCREEN_W + AVATAR_W) return;

        // Smooth plush bounce
        float bounce = 0.0f;
        if (state == PET_WALK) {
            bounce = abs(sin(walk_cycle)) * 3.0f;
        } else if (state == PET_CELEBRATE) {
            bounce = abs(sin(millis() / 140.0f)) * 7.0f;
        } else if (state == PET_IDLE) {
            bounce = sin(millis() / 520.0f) * 1.2f;
        }

        int draw_x = sx - AVATAR_W / 2;
        int draw_y = sy - (int)bounce - AVATAR_H / 2;

        // 1. Soft Realistic Ground Shadow
        if (state != PET_SLEEP) {
            int floor_y = 205; // Base floor plane
            if (state == PET_DRAGGED) {
                int height_lifted = max(0, floor_y - (sy + AVATAR_H / 2));
                int shadow_w = max(6, 22 - (int)(height_lifted * 0.25f));
                canvas.fillEllipse(sx, floor_y, shadow_w, 4, 0x528A);
            } else {
                int shadow_w = 20 - (int)(bounce * 0.4f);
                canvas.fillEllipse(sx, sy + AVATAR_H / 2 - 5, shadow_w, 5, 0x528A);
            }
        }

        // 2. Select PNG Frame (WITHOUT flipping the 'G' monogram!)
        const uint8_t* png_data = png_bunny_idle;
        size_t png_size = sizeof(png_bunny_idle);

        if (state == PET_DRAGGED) {
            png_data = png_bunny_dragged;
            png_size = sizeof(png_bunny_dragged);
            draw_x += (int)(sin(millis() / 140.0f) * 2.0f);
        } else if (state == PET_SLEEP) {
            png_data = png_bunny_sleep;
            png_size = sizeof(png_bunny_sleep);
        } else if (state == PET_PETTED) {
            png_data = png_bunny_happy;
            png_size = sizeof(png_bunny_happy);
        } else if (state == PET_EAT) {
            bool chew = ((millis() / 200) % 2 == 0);
            png_data = chew ? png_bunny_eat1 : png_bunny_eat2;
            png_size = chew ? sizeof(png_bunny_eat1) : sizeof(png_bunny_eat2);
        } else if (state == PET_CELEBRATE) {
            png_data = png_bunny_celebrate;
            png_size = sizeof(png_bunny_celebrate);
        } else if (state == PET_YAWN) {
            png_data = png_bunny_yawn;
            png_size = sizeof(png_bunny_yawn);
        } else if (state == PET_TIRED || (state == PET_IDLE && (hunger > 75 || energy < 20))) {
            png_data = png_bunny_tired;
            png_size = sizeof(png_bunny_tired);
        } else if (state == PET_DIZZY) {
            png_data = png_bunny_dizzy;
            png_size = sizeof(png_bunny_dizzy);
        } else if (state == PET_WALK) {
            bool step = (sin(walk_cycle) > 0);
            if (facing_right) {
                png_data = step ? png_bunny_walk1 : png_bunny_walk2;
                png_size = step ? sizeof(png_bunny_walk1) : sizeof(png_bunny_walk2);
            } else {
                png_data = step ? png_bunny_walk1_l : png_bunny_walk2_l;
                png_size = step ? sizeof(png_bunny_walk1_l) : sizeof(png_bunny_walk2_l);
            }
        }

        // 3. Render PNG with native true color & alpha blending
        canvas.drawPng(png_data, png_size, draw_x, draw_y);

        // Cute Nightcap when sleeping in bed
        if (state == PET_SLEEP) {
            int cap_x = sx - 6;
            int cap_y = draw_y - 2;
            canvas.fillTriangle(cap_x - 12, cap_y + 4, cap_x + 12, cap_y + 4, cap_x - 18, cap_y - 8, 0x535F);
            canvas.fillCircle(cap_x - 18, cap_y - 8, 4, TFT_WHITE);
        }

        // 4. Draw Floating Thought Bubble for Current Goal
        drawThoughtBubble(canvas, sx, draw_y);
    }

    void drawThoughtBubble(M5Canvas& canvas, int sx, int sy) {
        if (current_goal == GOAL_NONE || state == PET_SLEEP || state == PET_DIZZY || state == PET_DRAGGED || state == PET_CELEBRATE) return;

        float bubble_bob = sin(millis() / 320.0f) * 2.0f;
        int bx = sx + 18;
        int by = sy - 14 + (int)bubble_bob;

        // Connector dots
        canvas.fillCircle(sx + 8, sy - 2, 2, TFT_WHITE);
        canvas.drawCircle(sx + 8, sy - 2, 2, 0x31A6);

        canvas.fillCircle(sx + 13, sy - 6, 3, TFT_WHITE);
        canvas.drawCircle(sx + 13, sy - 6, 3, 0x31A6);

        // Main bubble
        canvas.fillRoundRect(bx - 10, by - 8, 22, 16, 6, TFT_WHITE);
        canvas.drawRoundRect(bx - 10, by - 8, 22, 16, 6, 0x31A6);

        // Bubble Icon based on goal
        if (current_goal == GOAL_HUNGER) {
            // Food bowl icon
            canvas.fillCircle(bx, by, 3, 0xFD20);
            canvas.drawFastHLine(bx - 3, by, 6, 0x9300);
        } else if (current_goal == GOAL_PLAY_BALL) {
            // Beach ball icon
            canvas.fillCircle(bx, by, 4, TFT_RED);
            canvas.fillCircle(bx, by, 2, TFT_YELLOW);
        } else if (current_goal == GOAL_WINDOW_GAZE) {
            // Sun / Star icon
            canvas.fillCircle(bx, by, 3, 0xFE40);
        } else if (current_goal == GOAL_COUCH_RELAX) {
            // Couch pillow icon
            canvas.fillRoundRect(bx - 4, by - 3, 8, 6, 2, 0x4477);
        } else if (current_goal == GOAL_ASK_ATTENTION) {
            // Heart icon
            canvas.fillCircle(bx - 2, by - 1, 2, COLOR_HEART_PINK);
            canvas.fillCircle(bx + 2, by - 1, 2, COLOR_HEART_PINK);
            canvas.fillTriangle(bx - 4, by, bx + 4, by, bx, by + 4, COLOR_HEART_PINK);
        }
    }

    void drawParticles(M5Canvas& canvas, float cam_x) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                int px = (int)(particles[i].x - cam_x);
                int py = (int)particles[i].y;
                if (px < -15 || px > SCREEN_W + 15) continue;

                if (particles[i].type == 0) {
                    canvas.fillCircle(px - 2, py - 2, 2, COLOR_HEART_PINK);
                    canvas.fillCircle(px + 2, py - 2, 2, COLOR_HEART_PINK);
                    canvas.fillTriangle(px - 4, py - 1, px + 4, py - 1, px, py + 4, COLOR_HEART_PINK);
                } else if (particles[i].type == 1) {
                    int size = (particles[i].life > 30) ? 1 : 2;
                    canvas.setTextSize(size);
                    canvas.setTextColor(0xBDF7);
                    canvas.drawString("z", px, py);
                } else if (particles[i].type == 2) {
                    canvas.fillCircle(px, py, 2, 0x9300);
                } else if (particles[i].type == 3) {
                    canvas.fillCircle(px, py, 2, TFT_YELLOW);
                    canvas.drawPixel(px - 3, py, TFT_YELLOW);
                    canvas.drawPixel(px + 3, py, TFT_YELLOW);
                    canvas.drawPixel(px, py - 3, TFT_YELLOW);
                    canvas.drawPixel(px, py + 3, TFT_YELLOW);
                }
            }
        }
    }
};
