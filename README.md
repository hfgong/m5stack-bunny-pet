# 🐰 M5Stack Bunny Virtual Desktop Pet

A full-featured, autonomous virtual desktop pet companion engineered for the **M5Stack Core2** and **M5Stack Core2 for AWS IoT Kit**.

Featuring an original plush stuffed-toy bunny character with a calligraphic embroidered **"G"** monogram, rich autonomic goal-directed AI, draggable touch physics, snappy tilt motion, and a continuous three-room house with dynamic day/night cycles.

<p align="center">
  <img src="previews/bunny_idle.png" width="90" alt="Idle Bunny" />
  <img src="previews/bunny_happy.png" width="90" alt="Happy Bunny" />
  <img src="previews/bunny_dragged.png" width="90" alt="Dragged Bunny" />
  <img src="previews/bunny_eat1.png" width="90" alt="Eating Bunny" />
  <img src="previews/bunny_celebrate.png" width="90" alt="Celebrating Bunny" />
</p>

---

## ✨ Features

### 🐾 1. Draggable Pet & Interactive Touch
- **Tap to Pet**: Tapping the bunny triggers floating love hearts, purring haptics, an affectionate chime, and boosts Happiness (+12).
- **Drag Across Rooms**: Pressing and holding onto the bunny lifts him off the ground with dangling paws (`bunny_dragged`), dynamically shrinking his floor shadow with height. Carrying him to the screen edges automatically scrolls the camera between rooms.
- **Smart Furniture Drop**:
  - Dropping near the **Food Bowl** in the Kitchen $\rightarrow$ starts eating immediately.
  - Dropping near the **Couch** in the Living Room $\rightarrow$ lounges and relaxes.
  - Dropping near the **Bed** in the Bedroom $\rightarrow$ curls up for bedtime.
  - Dropping onto the open floor $\rightarrow$ lands softly with star landing particles.

### 🎭 2. 14 High-Fidelity Plush Animations
Generated from a high-resolution plush sherpa wool model with soft lighting, pastel inner ears, and a calligraphically weighted embroidered "G" emblem that remains forward-facing when walking in either direction:

| State | Preview | Description |
|---|:---:|---|
| **Idle** | <img src="previews/bunny_idle.png" width="52"/> | Alert, curious bead eyes and gentle breathing |
| **Happy** | <img src="previews/bunny_happy.png" width="52"/> | Joyful squint eyes, soft rosy cheeks, open smile |
| **Sleep** | <img src="previews/bunny_sleep.png" width="52"/> | Downward curved lashes, peaceful smile |
| **Dragged** | <img src="previews/bunny_dragged.png" width="52"/> | Stretched body, dangling paws, surprised "o" mouth |
| **Eat (Munch)** | <img src="previews/bunny_eat1.png" width="52"/> | Head down, puffed chewing cheeks |
| **Eat (Chomp)** | <img src="previews/bunny_eat2.png" width="52"/> | Head up, happy chewing smile with crumb |
| **Celebrate** | <img src="previews/bunny_celebrate.png" width="52"/> | Joy hop with golden sparkles |
| **Yawn** | <img src="previews/bunny_yawn.png" width="52"/> | Sleepy droopy stretch with pink tongue |
| **Tired** | <img src="previews/bunny_tired.png" width="52"/> | Slumped posture, droopy eyes, pout |
| **Dizzy** | <img src="previews/bunny_dizzy.png" width="52"/> | Swirly cartoon eyes and squiggly mouth |

### 🍖💖⚡ 3. Vitals HUD & Autonomic Goal Planning
- **Live Status Gauges** on the top bar:
  - 🍖 **Fullness** (`100 - hunger`): Green $\to$ Yellow $\to$ Red meter.
  - 💖 **Joy** (`happiness`): Hot pink $\to$ Yellow $\to$ Red meter.
  - ⚡ **Energy**: Cyan $\to$ Yellow $\to$ Red meter.
- **Status Toast**: Tap anywhere on the top gauges to see exact percentages (`Full: 85% Joy: 92% Pwr: 78%`).
- **Autonomous AI**: The bunny monitors its own biological drives. When hungry, it journeys to the kitchen food bowl; when exhausted, it tucks into bed; when happy, it explores or hops excitedly.

### 🚀 4. Snappy Tilt Acceleration
- Tilting the device left or right provides an instant takeoff burst ($\sim 4.5\text{ px/frame}$) that smoothly converges over $\sim 650\text{ms}$ to steady cruising speed ($\sim 2.0\text{ px/frame}$).
- Reversing tilt direction resets the surge timer for crisp turnarounds.
- Upright resting calibration ($0.28\text{g}$ deadzone) ensures zero drift when stationary on a desk stand.

### 🏠 5. Continuous 3-Room World & Camera
- **Seamless 960px World**: Kitchen ($x=0\dots320$), Living Room ($x=320\dots640$), and Bedroom ($x=640\dots960$).
- **Dual Camera Tracking**:
  - Automatically frames the bunny as it walks or is carried across rooms.
  - Swipe anywhere on the room background to manually pan and explore.
- **Top Mini-Map Radar**: Displays room partitions, current viewport framing rectangle, and a live pastel marker dot tracking the bunny's position.

### ☀️🌙 6. Day / Night Cycle & SK6812 NeoPixel LEDs
- Realistic hourly daylight transitions (Dawn, Day, Sunset, Night, Twilight).
- 10-LED side illumination strips synchronize ambient colors with time of day and pulse gently when the bunny is actively engaged in an activity.

---

## 🎮 Controls

| Interaction | Action |
|---|---|
| **Tap Bunny** | Pet the bunny (hearts, purr haptics, +12 happiness) |
| **Drag Bunny** | Pick up and carry across rooms with dangling physics |
| **Drop on Furniture** | Contextual trigger (eat at bowl, sleep at bed, lounge at couch) |
| **Drag Room Floor** | Pan camera manually across the 3-room house |
| **Tilt Left / Right** | Accelerate bunny movement with responsive surge |
| **Tap Top Gauges** | Display exact numerical vitals toast |
| **Tap Mini-Map** | Jump camera instantly to that room |
| **Button A (Left)** | Feed treat (+25 Fullness) |
| **Button B (Center)** | Play / Bounce (+20 Joy) |
| **Button C (Right)** | Toggle sleep / wake |

---

## 🛠️ Hardware Requirements
- **Board**: [M5Stack Core2](https://docs.m5stack.com/en/core/core2) or [M5Stack Core2 for AWS IoT Kit](https://docs.m5stack.com/en/core/core2_for_aws).
- **Display**: 2.0" 320x240 ILI9342C capacitive touch LCD.
- **Sensors**: MPU6886 6-axis IMU (tilt detection).
- **Haptics & Audio**: Built-in NS4168 I2S power amplifier + vibrator motor.
- **LEDs**: 10x SK6812 side light strips (GPIO 25).

---

## 🚀 Building & Flashing

### Using PlatformIO

1. Clone this repository:
   ```bash
   git clone https://github.com/hfgong/m5stack-bunny-pet.git
   cd m5stack-bunny-pet
   ```

2. Build firmware:
   ```bash
   pio run -e m5stack-core2
   ```

3. Connect your M5Stack Core2 via USB and upload:
   ```bash
   pio run -e m5stack-core2 -t upload
   ```

4. Monitor serial logs (optional):
   ```bash
   pio device monitor -b 115200
   ```

---

## 🎨 Asset Regeneration
To regenerate the embedded PNG sprite buffers in `src/AvatarSprites.h` from `bunny_master.png`:
```bash
python3 generate_bunny_sprites.py
```

---

## 📄 License
This project is open-source and released under the [MIT License](LICENSE).
