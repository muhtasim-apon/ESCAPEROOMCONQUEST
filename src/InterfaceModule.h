#pragma once
#include "GameContext.h"
#include "GameState.h"

namespace InterfaceModule {
    /// Show your splash screen / main menu,
    /// then return GameState::MAP when you’re ready.
    GameState run(GameContext& ctx);
}
