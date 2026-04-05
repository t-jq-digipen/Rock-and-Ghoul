const pptxgen = require("pptxgenjs");

const pres = new pptxgen();
pres.layout = "LAYOUT_16x9";
pres.title = "Personal Contributions - Yasmin";

// ── Palette ──────────────────────────────────────────────────────────────────
const BG       = "0D1117";
const PANEL    = "161B22";
const CARD     = "1C2333";
const PURPLE   = "7C3AED";
const GOLD     = "D4A017";
const WHITE    = "FFFFFF";
const SUBTEXT  = "94A3B8";
const LIGHTPUR = "A78BFA";
const DARK_PUR = "4C1D95";
const TEAL     = "0D9488";
const W = 10, H = 5.625;

// ── Footer ───────────────────────────────────────────────────────────────────
function addFooter(slide) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: H - 0.28, w: W, h: 0.28,
    fill: { color: DARK_PUR }, line: { color: DARK_PUR }
  });
  slide.addText("Rock & Ghoul", {
    x: 0.15, y: H - 0.27, w: 2.5, h: 0.25,
    fontSize: 8, color: LIGHTPUR, bold: true, valign: "middle", margin: 0
  });
  slide.addText("CSD2401F25 | Team AvidDivas | Yasmin", {
    x: W - 4.5, y: H - 0.27, w: 4.3, h: 0.25,
    fontSize: 8, color: SUBTEXT, align: "right", valign: "middle", margin: 0
  });
}

// ── Section header ────────────────────────────────────────────────────────────
function addSectionHeader(slide, title, subtitle, accentColor) {
  const col = accentColor || GOLD;
  slide.addShape(pres.shapes.RECTANGLE, {
    x: 0.4, y: 0.3, w: 0.07, h: 0.55,
    fill: { color: col }, line: { color: col }
  });
  slide.addText(title, {
    x: 0.6, y: 0.28, w: 9, h: 0.38,
    fontSize: 22, bold: true, color: WHITE, valign: "middle", margin: 0
  });
  if (subtitle) {
    slide.addText(subtitle, {
      x: 0.6, y: 0.66, w: 8, h: 0.26,
      fontSize: 11, color: col, italic: true, valign: "top", margin: 0
    });
  }
}

// ── Bullet panel ──────────────────────────────────────────────────────────────
function makeBulletSlide(title, subtitle, bullets, noteText, accentColor) {
  const s = pres.addSlide();
  s.background = { color: BG };
  addFooter(s);
  addSectionHeader(s, title, subtitle, accentColor);

  s.addShape(pres.shapes.RECTANGLE, {
    x: 0.35, y: 1.05, w: 9.3, h: 4.18,
    fill: { color: PANEL }, line: { color: PANEL }
  });
  s.addShape(pres.shapes.RECTANGLE, {
    x: 0.35, y: 1.05, w: 0.06, h: 4.18,
    fill: { color: accentColor || PURPLE }, line: { color: accentColor || PURPLE }
  });

  const textRuns = bullets.map((b, i) => ({
    text: b,
    options: {
      bullet: { code: "2022" },
      color: WHITE,
      fontSize: 13,
      paraSpaceAfter: 5,
      breakLine: i < bullets.length - 1
    }
  }));

  s.addText(textRuns, {
    x: 0.6, y: 1.1, w: 8.9, h: noteText ? 3.5 : 4.0,
    valign: "top", margin: [8, 8, 8, 8]
  });

  if (noteText) {
    s.addShape(pres.shapes.RECTANGLE, {
      x: 0.35, y: 4.75, w: 9.3, h: 0.45,
      fill: { color: DARK_PUR }, line: { color: DARK_PUR }
    });
    s.addText(noteText, {
      x: 0.5, y: 4.75, w: 9.1, h: 0.45,
      fontSize: 9, color: LIGHTPUR, italic: true, valign: "middle", margin: 0
    });
  }
  return s;
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE 1 — TITLE
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };

  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 3.8, h: H, fill: { color: PURPLE }, line: { color: PURPLE } });
  s.addShape(pres.shapes.RECTANGLE, { x: 3.8, y: 0, w: 6.2, h: 0.08, fill: { color: GOLD }, line: { color: GOLD } });

  s.addText("PERSONAL", { x: 0.1, y: 1.5, w: 3.6, h: 0.6, fontSize: 28, bold: true, color: WHITE, align: "center", charSpacing: 4 });
  s.addText("CONTRIBUTIONS", { x: 0.05, y: 2.08, w: 3.7, h: 0.6, fontSize: 21, bold: true, color: GOLD, align: "center", charSpacing: 2 });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.6, y: 2.72, w: 2.6, h: 0.04, fill: { color: WHITE }, line: { color: WHITE } });
  s.addText("Yasmin", { x: 0.1, y: 2.86, w: 3.6, h: 0.45, fontSize: 18, color: WHITE, align: "center", italic: true });
  s.addText("yasmin.t@digipen.edu", { x: 0.1, y: 3.3, w: 3.6, h: 0.3, fontSize: 9.5, color: "DDD6FE", align: "center" });

  s.addText("Rock & Ghoul", { x: 4.2, y: 1.1, w: 5.5, h: 0.75, fontSize: 38, bold: true, color: WHITE });
  s.addShape(pres.shapes.RECTANGLE, { x: 4.2, y: 1.88, w: 4.5, h: 0.05, fill: { color: GOLD }, line: { color: GOLD } });
  s.addText("A C++ Game Engine & Game", { x: 4.2, y: 2.02, w: 5.5, h: 0.36, fontSize: 14, color: SUBTEXT });

  // Rubric chips
  const chips = [
    ["What I Learned", 4.2], ["My Roles", 5.78], ["Tools Used", 7.2]
  ];
  for (const [lbl, cx] of chips) {
    s.addShape(pres.shapes.ROUNDED_RECTANGLE, { x: cx, y: 2.65, w: 1.3, h: 0.36, fill: { color: DARK_PUR }, line: { color: PURPLE }, rectRadius: 0.08 });
    s.addText(lbl, { x: cx, y: 2.65, w: 1.3, h: 0.36, fontSize: 8, color: LIGHTPUR, align: "center", valign: "middle", bold: true });
  }

  s.addShape(pres.shapes.RECTANGLE, { x: 3.8, y: H - 0.28, w: 6.2, h: 0.28, fill: { color: DARK_PUR }, line: { color: DARK_PUR } });
  s.addText("CSD2401F25 | Team AvidDivas", { x: 4.0, y: H - 0.27, w: 5.8, h: 0.25, fontSize: 8, color: SUBTEXT, align: "right", valign: "middle", margin: 0 });
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE 2 — WHAT DID I LEARN?
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  addFooter(s);
  addSectionHeader(s, "1. What Did I Learn?", "Skills & knowledge gained over both trimesters", TEAL);

  const cols = [
    {
      title: "Engine Programming",
      items: ["Custom math library (Remath)", "JSON serialization & data-driven design", "Spatial grid & broad-phase collision", "Physics & forces simulation", "Scene/prefab save-load systems"],
    },
    {
      title: "Gameplay Programming",
      items: ["Full combat system design (C++)", "Elemental spell mechanics (Fire/Ice/Lightning/Wind)", "Player input: keyboard + gamepad", "Status effects & cooldowns", "Integrating gameplay across engine systems"],
    },
    {
      title: "Soft Skills",
      items: ["Collaborating in a 6-person team", "Reading & contributing to others' code", "Version control discipline (Git branching)", "Communicating technical decisions", "Scoping features under time pressure"],
    }
  ];

  const cw = 2.8, ch = 3.8, startX = 0.35, y = 1.1, gap = 0.2;
  for (let i = 0; i < cols.length; i++) {
    const x = startX + i * (cw + gap);
    const col = cols[i];

    s.addShape(pres.shapes.RECTANGLE, { x, y, w: cw, h: ch, fill: { color: CARD }, line: { color: TEAL, width: 0.5 } });
    s.addShape(pres.shapes.RECTANGLE, { x, y, w: cw, h: 0.42, fill: { color: TEAL }, line: { color: TEAL } });
    s.addText(col.title, { x: x + 0.08, y, w: cw - 0.1, h: 0.42, fontSize: 10.5, bold: true, color: WHITE, valign: "middle", margin: 0 });

    const runs = col.items.map((t, j) => ({
      text: t,
      options: { bullet: { code: "2022" }, color: WHITE, fontSize: 10, paraSpaceAfter: 4, breakLine: j < col.items.length - 1 }
    }));
    s.addText(runs, { x: x + 0.08, y: y + 0.5, w: cw - 0.15, h: ch - 0.55, valign: "top", margin: [4, 4, 4, 4] });
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE 3 — WHAT WERE MY ROLES?
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  addFooter(s);
  addSectionHeader(s, "2. What Were My Roles?", "Responsibilities across both trimesters", GOLD);

  const rows = [
    { role: "Engine Programmer", period: "Trim 1", desc: "Built core systems: math library, JSON serialization, spatial grid, collision, physics, prefab/scene system" },
    { role: "Combat System Owner", period: "Trim 2 early", desc: "Designed and coded CombatSystem from scratch — elemental types, AOE spells, status effects, damage pipeline" },
    { role: "Player System", period: "Trim 2", desc: "Dash system, elemental cooldowns, shield reset, element-select UI, and player–combat integration" },
    { role: "Input / Controller", period: "Trim 2 late", desc: "Added full gamepad support to the engine — binding controller input to all player actions" },
  ];

  const rh = 0.78, gy = 1.1;
  for (let i = 0; i < rows.length; i++) {
    const r = rows[i];
    const y = gy + i * (rh + 0.08);

    s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y, w: 9.3, h: rh, fill: { color: CARD }, line: { color: DARK_PUR } });
    // Period badge
    s.addShape(pres.shapes.ROUNDED_RECTANGLE, { x: 0.42, y: y + 0.17, w: 1.0, h: 0.36, fill: { color: DARK_PUR }, line: { color: PURPLE }, rectRadius: 0.06 });
    s.addText(r.period, { x: 0.42, y: y + 0.17, w: 1.0, h: 0.36, fontSize: 8, color: LIGHTPUR, align: "center", valign: "middle", bold: true });
    // Role title
    s.addText(r.role, { x: 1.55, y: y + 0.06, w: 3.2, h: 0.3, fontSize: 11.5, bold: true, color: WHITE, valign: "middle", margin: 0 });
    // Description
    s.addText(r.desc, { x: 1.55, y: y + 0.38, w: 7.9, h: 0.32, fontSize: 9.5, color: SUBTEXT, valign: "top", margin: 0 });
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE 4 — ACTUAL WORK: OVERVIEW
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  addFooter(s);
  addSectionHeader(s, "3. Show My Actual Work", "Feature breakdown across the full project", PURPLE);

  const areas = [
    { icon: "∑", label: "Custom Math Library",              sub: "Remath — Vec2/Vec3, trig, AABB" },
    { icon: "{}", label: "JSON Serialization & Scenes",      sub: "JsonLoad, prefabs, save/load" },
    { icon: "♫", label: "Fretboard & Spell Slot Logic",     sub: "Guitar rhythm mechanic" },
    { icon: "⊞", label: "Collision & Spatial Grid",         sub: "Broad-phase, layers, debug vis" },
    { icon: "⚡", label: "Physics & Dash System",            sub: "Forces, momentum, player dash" },
    { icon: "✦", label: "Combat & Elemental Spells",        sub: "CombatSystem, AOE, 4 elements" },
    { icon: "◈", label: "Spell UI & Elemental Barriers",    sub: "Power logos, barriers, cooldowns" },
    { icon: "🎮", label: "Controller / Gamepad Input",       sub: "Full gamepad support (Input.cpp)" },
  ];

  const cw = 4.3, ch = 0.72, gx = 0.35, gy = 1.1;
  for (let i = 0; i < areas.length; i++) {
    const col = i % 2, row = Math.floor(i / 2);
    const x = gx + col * (cw + 0.22);
    const y = gy + row * (ch + 0.1);
    const a = areas[i];

    s.addShape(pres.shapes.RECTANGLE, { x, y, w: cw, h: ch, fill: { color: CARD }, line: { color: PURPLE, width: 0.5 } });
    s.addShape(pres.shapes.RECTANGLE, { x, y, w: 0.06, h: ch, fill: { color: PURPLE }, line: { color: PURPLE } });
    s.addShape(pres.shapes.OVAL, { x: x + 0.14, y: y + 0.14, w: 0.42, h: 0.42, fill: { color: DARK_PUR }, line: { color: DARK_PUR } });
    s.addText(a.icon, { x: x + 0.14, y: y + 0.14, w: 0.42, h: 0.42, fontSize: 13, color: LIGHTPUR, align: "center", valign: "middle" });
    s.addText(a.label, { x: x + 0.64, y: y + 0.08, w: cw - 0.72, h: 0.3, fontSize: 10.5, bold: true, color: WHITE, valign: "middle", margin: 0 });
    s.addText(a.sub, { x: x + 0.64, y: y + 0.38, w: cw - 0.72, h: 0.25, fontSize: 8.5, color: SUBTEXT, valign: "middle", margin: 0 });
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDES 5–11 — DEEP DIVE (actual work per area)
// ═══════════════════════════════════════════════════════════════════════════════

makeBulletSlide(
  "Math Library & JSON System",
  "Sep – Oct 2025  |  Core engine foundations",
  [
    "Remath.h/cpp: Vec2, Vec3, trig functions, circle-AABB collision — no <cmath> dependency",
    "JsonLoad.h/cpp: full JSON entity/component loading system (data-driven engine architecture)",
    "Prefab system: entities instantiated from reusable .json templates at runtime",
    "Scene save/load: serialises live entity state back to JSON for editor persistence",
    "FilePath utility: cross-platform file resolution; Settings system (settings.json)",
  ],
  "Files: Remath.h/cpp  |  JsonLoad.h/cpp  |  FilePath.h/cpp  |  EntityCreation.h/cpp",
  TEAL
);

makeBulletSlide(
  "Fretboard Logic & Collision System",
  "Sep – Nov 2025  |  Gameplay mechanic + engine performance",
  [
    "Fretboard system: guitar-rhythm mechanic mapping player input sequences to spell triggers",
    "Spell prototype: SpellManager refactored into Fretboard + CombatSystem",
    "Spatial Grid (SpatialGrid.h/cpp): broad-phase collision detection — major performance win",
    "Collision layers: trigger vs victim entity classification; scale-aware collision components",
    "CollisionManager refactor + ImGui debug visualisation of collision shapes and grid cells",
  ],
  "Files: Fretboard.h/cpp  |  SpatialGrid.h/cpp  |  CollisionManager.h/cpp  |  Component.h",
  PURPLE
);

makeBulletSlide(
  "Physics & Dash System",
  "Nov 2025  |  Player movement foundation",
  [
    "Physics system (Physics.h/cpp): velocity, acceleration, mass — integrated into engine loop",
    "ForcesManager: applies forces to entities per frame; PhysicsSystem drives movement",
    "Player Dash: momentum-based burst with directional input, integrated with ForcesManager",
    "Custom matrix math added to Remath for physics transformations",
    "Resolved major frame-rate lag in physics + collision loop (performance debugging)",
  ],
  "Files: Physics.h/cpp  |  PhysicsSystem.h/cpp  |  ForcesManager.h/cpp  |  MovementSystem.cpp",
  GOLD
);

makeBulletSlide(
  "Combat System & Elemental Spells",
  "Jan – Feb 2026  |  Core combat feature",
  [
    "Designed & implemented CombatSystem (CombatSystem.h/cpp) as a new standalone engine system",
    "Four elemental types: Fire, Ice, Lightning, Wind — each with distinct damage and debuff",
    "AOE damage system with per-element VFX prefabs (Fire/Ice/Lightning/Wind AOE spritesheets)",
    "Status effect logic: elemental debuffs cannot stack — override-only semantics",
    "Integrated elemental spells with EnemySystem and PlayerSystem; extended Component.h",
  ],
  "Files: CombatSystem.h/cpp  |  PlayerSystem.cpp  |  EnemySystem.cpp  |  Component.h",
  TEAL
);

makeBulletSlide(
  "Spell UI, Barriers & Controller Input",
  "Feb – Apr 2026  |  Player experience polish + accessibility",
  [
    "Power Logo UI: Fire, Ice, Lightning, Wind icons; element-select red circle indicator",
    "Elemental Barrier system: Fire/Ice/Lightning/Wind barrier prefabs + CombatSystem integration",
    "Dash cooldown + per-element spell cooldown tracking and reset in PlayerSystem",
    "Full gamepad/controller input (Input.h/cpp): axis, buttons, dead-zones",
    "Controller bindings wired into PlayerSystem; gamepad integrated into glapp + CoreEngine loop",
  ],
  "Files: Input.h/cpp  |  PlayerSystem.cpp  |  CombatSystem.cpp  |  prefabs/Barrier_*.json",
  PURPLE
);

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE — TOOLS I USED
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  addFooter(s);
  addSectionHeader(s, "4. Tools I Used", "Development environment & libraries", GOLD);

  const tools = [
    { cat: "IDE & Compiler",    items: "Visual Studio 2022  |  MSVC (C++17)" },
    { cat: "Build System",      items: "CMake  |  cmake/ImportDependencies.cmake" },
    { cat: "Version Control",   items: "Git  |  GitHub Desktop  |  DigiPen GitLab" },
    { cat: "Graphics / Window", items: "OpenGL  |  GLFW  |  GLAD  |  Dear ImGui" },
    { cat: "Audio",             items: "FMOD (integration with AudioSystem)" },
    { cat: "Scripting Runtime", items: "Mono (.NET) — MonoManager, MonoSystem, MonoBindings" },
    { cat: "Data Format",       items: "JSON (nlohmann/json) — for all scene & asset data" },
    { cat: "AI Tools",          items: "Claude Code — used for code review & explanation" },
  ];

  const rh = 0.48, gy = 1.1;
  for (let i = 0; i < tools.length; i++) {
    const t = tools[i];
    const y = gy + i * (rh + 0.06);

    s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y, w: 9.3, h: rh, fill: { color: CARD }, line: { color: DARK_PUR } });
    s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y, w: 0.06, h: rh, fill: { color: GOLD }, line: { color: GOLD } });
    s.addText(t.cat, { x: 0.55, y, w: 2.0, h: rh, fontSize: 10, bold: true, color: GOLD, valign: "middle", margin: [0, 0, 0, 6] });
    s.addText(t.items, { x: 2.65, y, w: 6.9, h: rh, fontSize: 10, color: WHITE, valign: "middle", margin: [0, 0, 0, 0] });
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE — REFLECT ON EXPERIENCE
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  addFooter(s);
  addSectionHeader(s, "5. Reflect on My Experience", "Personal post-mortem — what I'd do differently", TEAL);

  // Two-column: What went well | What I'd improve
  const leftItems = [
    "Building the JSON system early gave the whole team data-driven flexibility",
    "Spatial grid made collision fast enough for many enemies on screen",
    "CombatSystem design was extensible — elements added cleanly later",
    "Good Git discipline kept merges manageable",
    "Enjoyed deep-diving into systems I built from scratch",
  ];
  const rightItems = [
    "Start designing systems with team-wide APIs in mind earlier",
    "Write more comments during crunch — future-me suffered reading old code",
    "Plan cooldown/status systems before combat features (retrofitting was messy)",
    "Dedicate time to profiling sooner — lag fix came late in Trim 1",
    "Test on hardware without a gamepad earlier in controller development",
  ];

  const ch = 3.72, y = 1.12;

  s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y, w: 4.4, h: ch, fill: { color: CARD }, line: { color: TEAL, width: 0.5 } });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y, w: 4.4, h: 0.38, fill: { color: TEAL }, line: { color: TEAL } });
  s.addText("What Went Well", { x: 0.42, y, w: 4.2, h: 0.38, fontSize: 11, bold: true, color: WHITE, valign: "middle", margin: 0 });
  s.addText(leftItems.map((t, i) => ({ text: t, options: { bullet: { code: "2022" }, color: WHITE, fontSize: 10, paraSpaceAfter: 5, breakLine: i < leftItems.length - 1 } })),
    { x: 0.42, y: y + 0.45, w: 4.2, h: ch - 0.52, valign: "top", margin: [4, 4, 4, 4] });

  s.addShape(pres.shapes.RECTANGLE, { x: 5.22, y, w: 4.4, h: ch, fill: { color: CARD }, line: { color: GOLD, width: 0.5 } });
  s.addShape(pres.shapes.RECTANGLE, { x: 5.22, y, w: 4.4, h: 0.38, fill: { color: GOLD }, line: { color: GOLD } });
  s.addText("What I'd Do Differently", { x: 5.29, y, w: 4.2, h: 0.38, fontSize: 11, bold: true, color: BG, valign: "middle", margin: 0 });
  s.addText(rightItems.map((t, i) => ({ text: t, options: { bullet: { code: "2022" }, color: WHITE, fontSize: 10, paraSpaceAfter: 5, breakLine: i < rightItems.length - 1 } })),
    { x: 5.29, y: y + 0.45, w: 4.2, h: ch - 0.52, valign: "top", margin: [4, 4, 4, 4] });
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLIDE — THANK YOU / CLOSING
// ═══════════════════════════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };

  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: W, h: 1.5, fill: { color: PURPLE }, line: { color: PURPLE } });
  s.addText("Thank You", { x: 0, y: 0.2, w: W, h: 1.1, fontSize: 44, bold: true, color: WHITE, align: "center", valign: "middle" });

  // 5 questions recap
  const recap = [
    ["1 — What did I learn?",     "Math, serialization, physics, combat systems; team collaboration & Git"],
    ["2 — What were my roles?",   "Engine programmer (Trim 1) → Combat & player systems (Trim 2)"],
    ["3 — Show actual work",      "8 major systems built from scratch — all visible in the codebase"],
    ["4 — Tools I used",          "Visual Studio, CMake, Git, OpenGL, FMOD, Mono, ImGui, Claude Code"],
    ["5 — Reflection",            "Design APIs for teammates early; comment during crunch; profile sooner"],
  ];

  const rh = 0.56, gy = 1.65;
  for (let i = 0; i < recap.length; i++) {
    const [q, a] = recap[i];
    const y = gy + i * (rh + 0.06);
    s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y, w: 9.3, h: rh, fill: { color: CARD }, line: { color: DARK_PUR } });
    s.addText(q, { x: 0.5, y, w: 3.2, h: rh, fontSize: 10, bold: true, color: LIGHTPUR, valign: "middle", margin: [0, 0, 0, 6] });
    s.addText(a, { x: 3.8, y, w: 5.7, h: rh, fontSize: 10, color: WHITE, valign: "middle", margin: 0 });
  }

  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: H - 0.5, w: W, h: 0.5, fill: { color: DARK_PUR }, line: { color: DARK_PUR } });
  s.addText("Rock & Ghoul  |  CSD2401F25  |  Team AvidDivas  |  Yasmin", {
    x: 0, y: H - 0.5, w: W, h: 0.5, fontSize: 9, color: LIGHTPUR, align: "center", valign: "middle", bold: true
  });
}

// ── Save ──────────────────────────────────────────────────────────────────────
pres.writeFile({
  fileName: "C:\\Users\\yasmi\\source\\repos\\aviddivas\\csd2401f25_team_aviddivas\\.claude\\worktrees\\frosty-yonath\\yasmin_contributions.pptx"
}).then(() => {
  console.log("SUCCESS: yasmin_contributions.pptx saved");
}).catch(e => {
  console.error("Error:", e);
  process.exit(1);
});
