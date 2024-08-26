#define OFF_GLOW_HIGHLIGHTS 0xb13c6a0
#define OFF_GLOW_ENABLE 0x28c
#define OFF_GLOW_THROUGH_WALL 0x26c
#define OFF_GLOW_HIGHLIGHT_ID 0x29C
#define OFF_HIGHLIGHT_TYPE_SIZE 0x34
#define OFF_GLOW_FIX 0x268
#define OFF_GLOW_DISTANCE 0x294 // Highlight_SetFarFadeDist [ 8B 81 ? ? ? ? 85 C0 75 ? 48 8D 0D ? ? ? ? E9 ]

// ITEM GLOW
if (conf->glow_items) {
        auto handle = mem.CreateScatterHandle(); bool ready = false;
        for (int highlightId = 15; highlightId < 66; highlightId++) {
                const GlowMode oldGlowMode = mem.Read<GlowMode>(highlightSettingsPtr + (OFF_HIGHLIGHT_TYPE_SIZE * highlightId) + 0);
                if (newGlowMode1 != oldGlowMode) {
                        ready = true;
                        mem.AddScatterWriteRequest(handle, highlightSettingsPtr + (OFF_HIGHLIGHT_TYPE_SIZE * highlightId) + 0, &newGlowMode1, sizeof(newGlowMode1));
                }
        }
        if (ready) mem.ExecuteWriteScatter(handle);
        mem.CloseScatterHandle(handle);
}
else {
        auto handle = mem.CreateScatterHandle();
        for (int highlightId = 15; highlightId < 66; highlightId++) {
                mem.AddScatterWriteRequest(handle, highlightSettingsPtr + (OFF_HIGHLIGHT_TYPE_SIZE * highlightId) + 0, &newGlowMode2, sizeof(newGlowMode2));
        }
        mem.ExecuteWriteScatter(handle);
        mem.CloseScatterHandle(handle);
}

if (conf->glow_player)
        {
            static const int contextId = 0; // Same as glow enable
            int settingIndex = 50;
            std::array<unsigned char, 4> highlightFunctionBits = {
                2,   // InsideFunction
                125, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE
                conf->glow_player_outline,  // OutlineRadius: size * 255 / 8
                64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7)
            };
            std::array<float, 3> glowColorRGB = { 0, 0, 0 };

            auto handle = mem.CreateScatterHandle();
            for (const auto p:*Players) {
                if (!p->IsValid() || !p->isHostile || !p->sure) continue;
                ready = true;

                if (p->isDown && conf->glow_player_down) {
                    settingIndex = 80;
                    glowColorRGB = { conf->glow_player_down_color[0], conf->glow_player_down_color[1],conf->glow_player_down_color[2] };
                }
                else {
                    if (!(p->isVisible && conf->glow_player_visible)) {
                        settingIndex = 75;
                        glowColorRGB = { conf->glow_player_disvisible_color[0], conf->glow_player_disvisible_color[1],conf->glow_player_disvisible_color[2] };
                    }
                    else {
                        settingIndex = 70;
                        glowColorRGB = { conf->glow_player_visible_color[0], conf->glow_player_visible_color[1],conf->glow_player_visible_color[2] };
                    }
                }

                //enable
                if (mem.Read<int>(p->base + OFF_GLOW_HIGHLIGHT_ID, true) != settingIndex) {
                    mem.Write<int>(p->base + OFF_GLOW_HIGHLIGHT_ID, 1);
                    //wall
                    mem.Write<int>(p->base + OFF_GLOW_THROUGH_WALL, 1);
                    //fix
                    mem.Write<int>(p->base + OFF_GLOW_FIX, 2);
                    mem.Write<int>(p->base + OFF_GLOW_DISTANCE, MAX_DIST);
                }
                mem.AddScatterWriteRequest(handle, p->base + OFF_GLOW_HIGHLIGHT_ID + contextId,&settingIndex,sizeof(settingIndex));
            }
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_HIGHLIGHT_TYPE_SIZE * settingIndex + 0, &highlightFunctionBits, sizeof(highlightFunctionBits));
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_HIGHLIGHT_TYPE_SIZE * settingIndex + 4, &glowColorRGB, sizeof(glowColorRGB));
            mem.ExecuteWriteScatter(handle);
            mem.CloseScatterHandle(handle);
        }
