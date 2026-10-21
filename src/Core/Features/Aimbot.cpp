#include "Aimbot.hpp"
#include "CMetaData.h"
#include "TargetHighlight.hpp"
#include <Core/Lifecycle.hpp>

#include <algorithm>
#include <random>
#include <chrono>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define SKEL_Neck_1 0x9995
#define SKEL_Head   0x796e
#define SKEL_Spine3 0x60f2

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(-0.01f, 0.01f);

static CPed* g_StickyTarget = nullptr;
// Frame-rate independent smoothing step (ported from SunketsWare-FiveM):
// the Smoothness slider maps onto a time constant instead of a per-frame
// fraction, so the feel does not depend on tick rate.
//   slider 0   -> snap instantly
//   slider 100 -> ~1.0s to close most of the remaining angle
static float SmoothStep(float Slider, float DeltaSeconds)
{
    if (Slider <= 0.0f)
        return 1.0f;

    const float t = std::clamp(Slider / 100.0f, 0.0f, 1.0f);
    const float TimeConstant = 0.02f + t * t * 0.98f;

    float step = 1.0f - std::exp(-DeltaSeconds / TimeConstant);
    if (step > 1.0f) step = 1.0f;
    if (step < 0.0001f) step = 0.0001f;
    return step;
}
std::vector<uint16_t> GetSelectedBoneGroupsABOT()
{
    std::vector<uint16_t> bones;
    auto* cfg = g_Config.Aimbot;
    if (cfg->BoneGroup_Head) bones.push_back(SKEL_Head);
    if (cfg->BoneGroup_Neck) bones.push_back(SKEL_Neck_1);
    if (cfg->BoneGroup_Chest)
    {
        bones.push_back(SKEL_Spine3);
        bones.push_back(SKEL_Spine2);
        bones.push_back(SKEL_Spine1);
    }
    if (cfg->BoneGroup_Arms)
    {
        bones.insert(bones.end(), {
            SKEL_L_UpperArm, SKEL_L_Forearm, SKEL_L_Hand,
            SKEL_R_UpperArm, SKEL_R_Forearm, SKEL_R_Hand
            });
    }
    if (cfg->BoneGroup_Legs)
    {
        bones.insert(bones.end(), {
            SKEL_L_Thigh, SKEL_L_Calf,
            SKEL_R_Thigh, SKEL_R_Calf
            });
    }
    if (cfg->BoneGroup_Feet)
    {
        bones.insert(bones.end(), {
            SKEL_L_Foot,
            SKEL_R_Foot
            });
    }
    if (bones.empty())
        bones.push_back(SKEL_Head);
    return bones;
}

D3DXVECTOR3 PredictPosition2(CPed* ped, D3DXVECTOR3 bonePos)
{
    if (!ped || !Core::SDK::Pointers::pLocalPlayer)
        return bonePos;

    CPed* local = Core::SDK::Pointers::pLocalPlayer;
    D3DXVECTOR3 targetVel{};

    if (ped->HasFlag(ePedConfigFlag::InVehicle))
    {
        uintptr_t veh = Mem.Read<uintptr_t>((uintptr_t)ped + 0x0D30);
        if (veh) targetVel = Mem.Read<D3DXVECTOR3>(veh + 0x0D40);
    }
    else
    {
        targetVel = Mem.Read<D3DXVECTOR3>((uintptr_t)ped + 0x300);
    }

    static D3DXVECTOR3 lastVel{};
    float speed = D3DXVec3Length(&targetVel);
    float smooth = (speed < 0.1f) ? 0.9f : 0.5f;
    targetVel = lastVel + (targetVel - lastVel) * smooth;
    lastVel = targetVel;

    auto* wm = local->GetWeaponManager();
    if (!wm) return bonePos;
    auto* wi = wm->GetWeaponInfo();
    if (!wi) return bonePos;

    float bulletSpeed = wi->GetWeaponBulletSpeed();
    if (bulletSpeed < 10.0f) return bonePos;

    if (bulletSpeed != 1300.0f)
    {
        uintptr_t wpM = (g_Offsets.CurrentBuild >= 2802) ? 0x10B8ULL : 0x10D8ULL;
        uintptr_t wpmng = Mem.Read<uintptr_t>((uintptr_t)local + wpM);
        if (wpmng)
        {
            uintptr_t wepI = Mem.Read<uintptr_t>(wpmng + 0x20);
            if (wepI) Mem.Write<float>(wepI + 0xE0, 1300.0f);
        }
    }

    D3DXVECTOR3 localPos = Mem.Read<D3DXVECTOR3>((uintptr_t)local + 0x90);
    D3DXVECTOR3 dir = bonePos - localPos;
    float distSq = D3DXVec3LengthSq(&dir);
    float dist = std::sqrt(distSq);
    if (dist < 1.0f) return bonePos;

    float a = D3DXVec3Dot(&targetVel, &targetVel) - (bulletSpeed * bulletSpeed);
    float b = 2.0f * D3DXVec3Dot(&dir, &targetVel);
    float c = distSq;
    float travelTime = 0.0f;

    if (std::fabs(a) < 0.001f)
    {
        if (std::fabs(b) > 0.001f)
            travelTime = -c / b;
    }
    else
    {
        float disc = b * b - 4.0f * a * c;
        if (disc >= 0.0f)
        {
            float sd = std::sqrt(disc);
            float t1 = (-b + sd) / (2.0f * a);
            float t2 = (-b - sd) / (2.0f * a);
            travelTime = (t1 > 0.0f && t2 > 0.0f) ? std::min(t1, t2) : (t1 > 0.0f ? t1 : (t2 > 0.0f ? t2 : 0.0f));
        }
    }

    if (travelTime <= 0.001f || travelTime > 5.0f)
        travelTime = dist / bulletSpeed;

    D3DXVECTOR3 predicted = bonePos + targetVel * travelTime;
    if (dist > 80.0f)
        predicted.z += (dist - 80.0f) / 400.0f * 0.4f;

    return predicted;
}

void Core::Features::cAimbot::SetViewAngles(CPed* Ped, D3DXVECTOR3 BonePos)
{
    uintptr_t CamFollowPedCamera = Mem.Read<uintptr_t>(Core::SDK::Pointers::pCamGamePlayDirector + 0x2C0);
    if (!CamFollowPedCamera) return;

    D3DXVECTOR3 CamPos = Mem.Read<D3DXVECTOR3>(CamFollowPedCamera + 0x60);

    ::SDK::CMetaData* metadata = (::SDK::CMetaData*)Mem.Read<uintptr_t>(CamFollowPedCamera + 0x10);
    bool isFirstPerson = metadata ? metadata->bIsFristPerson() : false;

    uintptr_t rotationOffset = isFirstPerson ? 0x40 : 0x3D0;

    D3DXVECTOR3 CurrentViewAngles = Mem.Read<D3DXVECTOR3>(CamFollowPedCamera + rotationOffset);
    D3DXVec3Normalize(&CurrentViewAngles, &CurrentViewAngles);

    if (metadata)
    {
        if (isFirstPerson)
        {
            float currentAlt = Mem.Read<float>((uintptr_t)metadata + 0x88);
            if (std::abs(currentAlt) > 0.001f)
                metadata->fSetAltitude(0.0f);
        }

        if (Core::SDK::Pointers::pLocalPlayer)
            metadata->SetVectorFix(Core::SDK::Pointers::pLocalPlayer->InVehicle());
    }

    D3DXVECTOR3 TargetViewAngles = BonePos - CamPos;
    D3DXVec3Normalize(&TargetViewAngles, &TargetViewAngles);

    // Frame-rate independent smoothing (ported from SunketsWare-FiveM):
    // the Smoothness slider (one value shared by horizontal and vertical)
    // maps onto a time constant, and Aimbot Speed stays the base per-tick
    // convergence rate. They combine multiplicatively, so Smoothness 0
    // reproduces the old Aimbot Speed feel exactly.
    static auto s_lastAimTick = std::chrono::steady_clock::now();
    const auto nowAimTick = std::chrono::steady_clock::now();
    float AimDeltaSeconds = std::chrono::duration<float>(nowAimTick - s_lastAimTick).count();
    s_lastAimTick = nowAimTick;
    AimDeltaSeconds = std::clamp(AimDeltaSeconds, 0.0005f, 0.05f);

    float BaseFactor;
    if (g_Config.Aimbot->LegitMode)
        BaseFactor = 1.0f / std::max(1.0f, (float)g_Config.Aimbot->AimbotSpeed * 2.0f);
    else
        BaseFactor = 1.0f / std::max(1.0f, (float)g_Config.Aimbot->AimbotSpeed);

    const float SmoothFactor = std::clamp(BaseFactor * SmoothStep((float)g_Config.Aimbot->SmoothHorizontal, AimDeltaSeconds), 0.0001f, 1.0f);

    D3DXVECTOR3 FinalAngles;
    if (SmoothFactor >= 1.0f)
    {
        FinalAngles = TargetViewAngles;
    }
    else
    {
        FinalAngles = CurrentViewAngles + (TargetViewAngles - CurrentViewAngles) * SmoothFactor;
        D3DXVec3Normalize(&FinalAngles, &FinalAngles);
    }

    Mem.Write<D3DXVECTOR3>(CamFollowPedCamera + rotationOffset, FinalAngles);

    if (!isFirstPerson)
    {
        D3DXVECTOR3 Cam3DAngles = Mem.Read<D3DXVECTOR3>(CamFollowPedCamera + 0x3D0);
        D3DXVECTOR3 ThirdPersonAngles = FinalAngles;
        ThirdPersonAngles.z = FinalAngles.z - (CurrentViewAngles.z - Cam3DAngles.z);
        Mem.Write<D3DXVECTOR3>(CamFollowPedCamera + 0x3D0, ThirdPersonAngles);
    }
}
#include <Includes/CustomWidgets/Notify.hpp>

bool IsAimbotActive()
{
    auto* cfg = g_Config.Aimbot;
    HWND ActiveWindow = GetForegroundWindow();
    const bool InGameContext = !g_MenuInfo.IsOpen && ActiveWindow == g_Variables.g_hGameWindow;

    if (!InGameContext)
    {
        cfg->KeyBindToggled = false;
        cfg->KeyBindLast = false;
        return false;
    }

    if (!cfg->Enabled || cfg->KeyBind == 0)
    {
        cfg->KeyBindToggled = false;
        cfg->KeyBindLast = false;
        return false;
    }

    bool down = (GetAsyncKeyState(cfg->KeyBind) & 0x8000) != 0;

    if (cfg->KeyBindMode == 1) // Hold
    {
        cfg->KeyBindToggled = down;
        cfg->KeyBindLast = down;
        return down;
    }

    // Toggle
    if (down && !cfg->KeyBindLast)
    {
        cfg->KeyBindToggled = !cfg->KeyBindToggled;

        if (cfg->KeyBindToggled)
            NotifyManager::Send("Aimbot Enabled", 200.0f);
        else
            NotifyManager::Send("Aimbot Disabled", 200.0f);
    }

    cfg->KeyBindLast = down;

    return cfg->KeyBindToggled;
}
void Core::Features::cAimbot::Start(uint64_t worker_epoch)
{
    constexpr int IDLE_SLEEP = 24;
    constexpr int ACTIVE_SLEEP = 10;

    while (Core::ShouldGameThreadRun(worker_epoch))
    {
        if (!IsAimbotActive())
        {
            Core::Features::TargetHighlight::Clear(Core::Features::TargetHighlight::Source::Aimbot);

            if (g_Config.Aimbot->StickyAim)
                g_StickyTarget = nullptr;

            std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_SLEEP));
            continue;
        }

        CPed* local = Core::SDK::Pointers::pLocalPlayer;
        if (!local)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_SLEEP));
            continue;
        }

        D3DXVECTOR3 localPos = Mem.Read<D3DXVECTOR3>((uintptr_t)local + 0x90);

        uintptr_t wepI = 0;
        {
            uintptr_t wpM = (g_Offsets.CurrentBuild >= 2802) ? 0x10B8ULL : 0x10D8ULL;
            uintptr_t wpmng = Mem.Read<uintptr_t>((uintptr_t)local + wpM);
            if (wpmng)
                wepI = Mem.Read<uintptr_t>(wpmng + 0x20);
        }

        CPed* TargetPed = nullptr;

        if (g_Config.Aimbot->StickyAim && g_StickyTarget)
        {
            if (Core::SDK::Game::IsValidPed(g_StickyTarget) &&
                Core::SDK::Game::GetDistanceToPed(g_StickyTarget) <= g_Config.Aimbot->MaxDistance)
            {
                TargetPed = g_StickyTarget;
            }
            else
            {
                g_StickyTarget = nullptr;
            }
        }

        if (!TargetPed)
        {
            TargetPed = Core::SDK::Game::GetClosestPed(
                g_Config.Aimbot->MaxDistance,
                g_Config.Aimbot->IgnoreNPCs,
                g_Config.Aimbot->IgnoreDead,
                g_Config.Aimbot->OnlyVisible,
                true,
                g_Config.Aimbot->ExcludeGod
            );

            if (TargetPed && g_Config.Aimbot->StickyAim)
                g_StickyTarget = TargetPed;
        }

        if (!TargetPed)
        {
            Core::Features::TargetHighlight::Clear(Core::Features::TargetHighlight::Source::Aimbot);
            std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_SLEEP));
            continue;
        }

        uintptr_t frag = Mem.Read<uintptr_t>((uintptr_t)TargetPed + g_Offsets.m_FragInst);
        uintptr_t v9 = Mem.Read<uintptr_t>(frag + 0x68);
        if (!v9)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(ACTIVE_SLEEP));
            continue;
        }

        Core::SDK::Game::cSkeleton_t Skeleton;

        Skeleton.m_pSkeleton = Mem.Read<uintptr_t>(v9 + 0x178);

        Skeleton.crSkeletonData.Ptr = Mem.Read<uintptr_t>(Skeleton.m_pSkeleton);
        Skeleton.crSkeletonData.m_Used = Mem.Read<unsigned int>(Skeleton.crSkeletonData.Ptr + 0x1A);
        Skeleton.crSkeletonData.m_NumBones = Mem.Read<unsigned int>(Skeleton.crSkeletonData.Ptr + 0x5E);
        Skeleton.crSkeletonData.m_BoneIdTable_Slots = Mem.Read<unsigned short>(Skeleton.crSkeletonData.Ptr + 0x18);

        if (!Skeleton.crSkeletonData.m_BoneIdTable_Slots)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(ACTIVE_SLEEP));
            continue;
        }

        Skeleton.crSkeletonData.m_BoneIdTable = Mem.Read<uintptr_t>(Skeleton.crSkeletonData.Ptr + 0x10);

        Skeleton.Arg1 = Mem.Read<D3DXMATRIX>(Mem.Read<uintptr_t>(Skeleton.m_pSkeleton + 0x8));
        Skeleton.Arg2 = Mem.Read<uintptr_t>(Skeleton.m_pSkeleton + 0x18);

        auto bones = GetSelectedBoneGroupsABOT();

        float bestDistSq = FLT_MAX;
        D3DXVECTOR3 bestPos{};
        bool found = false;

        for (uint16_t id : bones)
        {
            D3DXVECTOR3 pos = Core::SDK::Game::GetBonePosComplex(TargetPed, id, Skeleton);

            D3DXVECTOR3 diff = pos - localPos;
            float distance = D3DXVec3Length(&diff);

            if (g_Config.Aimbot->Prediction)
                pos = Core::SDK::Game::AplPredict((uintptr_t)TargetPed, distance, pos, wepI);

            D3DXVECTOR2 screen = Core::SDK::Game::WorldToScreen(pos);

            if (!Core::SDK::Game::IsOnScreen(screen))
                continue;

            float dx = screen.x - g_Variables.g_vGameWindowCenter.x;
            float dy = screen.y - g_Variables.g_vGameWindowCenter.y;

            const float distSq = Core::FovUtils::GetNormalizedDistanceSq(
                dx,
                dy,
                static_cast<float>(g_Config.Aimbot->FOVX),
                static_cast<float>(g_Config.Aimbot->FOVY));

            if (distSq < bestDistSq && distSq <= 1.0f)
            {
                bestDistSq = distSq;
                bestPos = pos;
                found = true;
            }
        }

        if (!found)
        {
            Core::Features::TargetHighlight::Clear(Core::Features::TargetHighlight::Source::Aimbot);
            std::this_thread::sleep_for(std::chrono::milliseconds(ACTIVE_SLEEP));
            continue;
        }

        D3DXVECTOR2 ScreenPos = Core::SDK::Game::WorldToScreen(bestPos);

        if (Core::SDK::Game::IsOnScreen(ScreenPos))
        {
            float dx = ScreenPos.x - g_Variables.g_vGameWindowCenter.x;
            float dy = ScreenPos.y - g_Variables.g_vGameWindowCenter.y;

            const float distSq = Core::FovUtils::GetNormalizedDistanceSq(
                dx,
                dy,
                static_cast<float>(g_Config.Aimbot->FOVX),
                static_cast<float>(g_Config.Aimbot->FOVY));

            if (distSq <= 1.0f)
            {
                Core::Features::TargetHighlight::Mark(Core::Features::TargetHighlight::Source::Aimbot, TargetPed);
                SetViewAngles(TargetPed, bestPos);
            }
            else
            {
                Core::Features::TargetHighlight::Clear(Core::Features::TargetHighlight::Source::Aimbot);
            }
        }
        else
        {
            Core::Features::TargetHighlight::Clear(Core::Features::TargetHighlight::Source::Aimbot);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(ACTIVE_SLEEP));
    }

    Core::Features::TargetHighlight::Clear(Core::Features::TargetHighlight::Source::Aimbot);
    g_StickyTarget = nullptr;
}
