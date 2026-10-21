#pragma once
#include <Includes/Includes.hpp>
#include <Core/Lifecycle.hpp>
#include <Core/SDK/SDK.hpp>
#include "UpdateNames.hpp"
#include <Core/Features/auth.hpp>

namespace Core
{
    namespace Threads
    {
        namespace
        {
            inline bool IsSyncableLocalName(const std::string& name)
            {
                if (name.empty())
                    return false;

                std::string lower = name;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                return lower != "npc" &&
                    lower != "unknown" &&
                    lower != "unknown player";
            }

            inline void TrySyncLocalIngameName(CPed* localPlayer)
            {
                static std::string lastSyncedName;
                static std::mutex syncMutex;
                static ULONGLONG lastAttempt = 0;
                static std::atomic_bool requestInFlight{ false };

                if (!localPlayer)
                    return;

                const ULONGLONG now = GetTickCount64();
                if (now - lastAttempt < 30000 || requestInFlight.load(std::memory_order_acquire))
                    return;

                std::string localName = Core::Features::UpdateNames::GetName(localPlayer);
                if (!IsSyncableLocalName(localName))
                    return;

                {
                    std::lock_guard<std::mutex> lock(syncMutex);
                    if (localName == lastSyncedName)
                        return;
                }

                lastAttempt = now;
                requestInFlight.store(true, std::memory_order_release);

                std::thread([localName] {
                    if (auth::SendIngameName(localName)) {
                        std::lock_guard<std::mutex> lock(syncMutex);
                        lastSyncedName = localName;
                    }
                    requestInFlight.store(false, std::memory_order_release);
                    }).detach();
            }
        }

        class cLocalPlayerState
        {
        public:
            void Update()
            {
                while (!Core::IsShutDownRequested())
                {
                    auto* local_player = Core::SDK::Pointers::pLocalPlayer;
                    const bool has_local_player = Core::HasValidCoreOffsets() &&
                        local_player != nullptr &&
                        reinterpret_cast<uintptr_t>(local_player) >= 0x10000;

                    Core::SetLocalPlayerCached(has_local_player);
                    if (has_local_player)
                    {
                        TrySyncLocalIngameName(local_player);

                        // Semi God Mode - disabled to prevent conflicts with GodMode
                        // if (g_Config.Player->Heal)
                        // {
                        //     float health = local_player->GetHealth();
                        //     float armor = local_player->GetArmor();
                        //     if (health < 200.0f) {
                        //         local_player->SetHealth(200.0f);
                        //     }
                        //     if (armor < 100.0f) {
                        //         local_player->SetArmor(100.0f);
                        //     }
                        // }

                        // Infinite Stamina
                        if (g_Config.Player->InfiniteStamina)
                        {
                            uintptr_t player_info = Mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(local_player) + g_Offsets.m_PlayerInfo);
                            if (player_info) {
                                Mem.Write<float>(player_info + 0x0CF4, -1.0f);
                            }
                        }

                        // Regenerate Armor
                        if (g_Config.Player->RegenerateArmor)
                        {
                            float armor = local_player->GetArmor();
                            if (armor < 200.0f) {
                                local_player->SetArmor(200.0f);
                            }
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
        };

        inline cLocalPlayerState g_LocalPlayerState;
    }
}
