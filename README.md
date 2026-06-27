# Send Martin Tower to Mars! — C / Raylib port

A C + [raylib](https://www.raylib.com/) rewrite of the original LÖVE/Lua game.
Physics use raylib's single-header [`physac`](https://github.com/raysan5/physac)
(vendored in `vendor/`) instead of Box2D.

## Layout

```
src/
  main.c        entry point, game loop, global state, shared helpers, capture mode
  game.h        shared types (System, Tower), screen enum, prototypes
  state.c       screen registry + ChangeScreen + fade-through-black transitions
  assets.c/.h   one-time load/unload of textures, fonts, audio, sprite anims
  physics.c/.h  thin wrapper over physac (single PHYSAC_IMPLEMENTATION TU)
  input.c/.h    action mapping over keyboard / pointer / scripted demo
  fx.c/.h       juice layer: particles, sprite explosions, trauma screen-shake
  screens/      one file per screen (intro, menu, intro02, stage, goal1-3, level1-3)
assets/         images, audio (.ogg), font (copied from ../game/assets)
vendor/physac.h vendored physics single-header
```

The flow mirrors the original:
`intro → menu → intro02 → level1 → goal1 → level2 → goal2 → level3 → goal3 → stage → (loop, level++)`.

## Build & run (desktop)

Requires `raylib` installed (e.g. `pkg-config --modversion raylib`) and a C compiler.

```
make          # builds ./martin
make run      # build + run
```

Controls: **Left/Right** fill fuel (L1) / steer (L2,L3) · **Up** thrust (L3) ·
**A/D** rotate (L3) · **Space** advance/restart · **M** toggle music · **Esc** quit.
Touch / mouse: left third, right third, center act as Left / Right / Thrust.

### Gameplay mechanics

- **Lives** — you start with 3 ships (shown top-right). A crash spends one and
  replays *that level*; running out resets to the start. (`NextLifeOrGameOver`
  in `main.c`.)
- **Level 1 launch** — the rhythm fills a tank, then the tower climbs at a
  visible, fuel-burning rate. Bank too little and it **stalls and falls back**
  before reaching orbit (the fuel bar turns amber→red as it drains).
- **Level 3 HUD** — descent / drift / tilt readouts, green when within the
  safe-landing envelope and red ("UNSAFE") when not.
- **Level 3 landing pads** — flat terrain is marked with teal strips; touching
  down on one multiplies the landing bonus (narrower pad = bigger multiplier).
- **Level 2 grazes + combo** — slipping a meteoroid past you within a hair
  (`L2_GRAZE` px) scores a "GRAZE!" + fuel. Chained grazes build a **combo
  multiplier** (up to x5) that scales the payout; taking a hit resets it. Grazes,
  hits, and landing bonuses spawn floating score popups.
- **High score** — the best final score persists (`highscore.dat`) and shows on
  the menu and the results screen ("NEW BEST!" on a record).

### Presentation / juice ([fx.c](src/fx.c))

- Particles + sprite explosions on every crash, engine exhaust + smoke trails,
  meteoroid debris, graze sparks, landing dust, and a celebratory landing burst.
- Trauma-based **screen shake** kicks the whole frame on impacts (a global shake
  camera in the main loop; the HUD rides with it briefly).
- **Fade-through-black transitions** between every screen (`state.c`), with a
  fade-up from black on launch.
- **Level 1 launch sequence**: an engine "charge" glow that flares with your
  mashing, venting steam, a build-up shudder, an ignition blast, a thick exhaust
  plume + smoke trail, and a sky that darkens to space (with stars) as you climb.
- **Level 2** has a parallax starfield for depth.
- **Floating score popups** (`FxPopup`) on grazes/combos, meteoroid kills, and
  landing bonuses.
- The plain text / goal / round screens get a slow, twinkling **ambient
  starfield** backdrop; the **menu** has a bobbing title, a smoothly pulsing
  prompt, and warm drifting embers.

## Web (WebAssembly)

Needs the [emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
on `PATH` and a raylib built for web (`libraylib.web.a`).

```
make PLATFORM=PLATFORM_WEB RAYLIB_WEB_LIB=/path/to/libraylib.web.a
# serve build/index.html, e.g.:  emrun build/index.html
```

## Verification / tuning helpers (debug only)

- `./martin debug 01|02|03` — jump straight into a level.
- `MARTIN_CAP="screen:seconds[:out.png]" ./martin` — jump to a screen, run N
  seconds, save a screenshot, then quit (used for automated visual checks).
- `MARTIN_DEMO="thrust,rotl,left,right,confirm"` — hold the listed actions
  (scripted input for capture runs).
- Per-level constants are overridable via env vars for tuning. Level 1 launch:
  `L1_CLIMB` (climb px/s), `L1_BURN` (fuel/s), `L1_FALL` (stall gravity),
  `L1_CD` (fill countdown s), `L1_FUEL` (preset tank, for testing the stall).
  Level 3: `L3_MAIN` (thrust), `L3_G` (gravity), `L3_LANDV` (safe-landing speed).
  Level 2: `L2_SCROLL` (scroll speed), `L2_SPAWN` (meteoroid density), `L2_SIDE`
  (steering thrust), `L2_DAMP` (steering damping - lower = more momentum),
  `L2_GRAZE` (near-miss px). Defaults are baked in.

## Notes on the port

- physac has one global gravity (set per level), no chain shapes, and no contact
  callbacks. Level 3 keeps its own terrain polyline and detects touchdown by
  testing the lander's lower corners against it. Level 2 (an on-rails scroller)
  uses lightweight manual motion + manual circle/tower collision.
- Physics force constants were re-tuned by feel (physac's units differ from
  Box2D); they are not numerically identical to the original.
