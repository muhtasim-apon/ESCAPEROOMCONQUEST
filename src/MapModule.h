#pragma once

#include "GameContext.h"
#include "GameState.h"

namespace MapModule {
    /// Runs the world‐map camera demo (after “New Game”) then returns next state.
    GameState run(GameContext& ctx);
}
