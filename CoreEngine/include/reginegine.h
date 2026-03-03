/* ========================================================
 * @file        reginegine.h
 * @brief       Single-include umbrella header for game projects.
 *
 * Include this ONE header in your game's main.cpp instead of
 * listing every engine system individually. It pulls in all
 * CoreEngine systems, utilities, and third-party dependencies
 * that a game entry point needs.
 *
 * Usage in your game's main.cpp:
 *     #include "reginegine.h"
 *     // then add any game-specific system headers below
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once

// ── Third-party / platform ────────────────────────────────────────────────
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <crtdbg.h>

// ── Engine entry point & window ───────────────────────────────────────────
#include "glapp.h"
#include "CoreEngine.h"

// ── Engine utilities ──────────────────────────────────────────────────────
#include "FilePath.h"
#include "JsonLoad.h"
#include "CrashHandler.h"

// ── Engine systems ────────────────────────────────────────────────────────
#include "AudioSystem.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "MovementSystem.h"
#include "LogicSystem.h"
#include "GuiSystem.h"
#include "MonoManager.h"
#include "MonoScriptComponent.h"
#include "MonoSystem.h"

// ── Editor (excluded in shipping builds) ─────────────────────────────────
#ifndef NO_IMGUI
#include "ImguiSystem.h"
#endif
