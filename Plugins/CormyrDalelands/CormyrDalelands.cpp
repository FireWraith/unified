#include "nwnx.hpp"

#include "API/Globals.hpp"
#include "API/CAppManager.hpp"
#include "API/CTlkTable.hpp"
#include "API/CFeatUseListEntry.hpp"
#include "API/CGameEffect.hpp"
#include "API/CNWClass.hpp"
#include "API/CNWFeat.hpp"
#include "API/CNWSVirtualMachineCommands.hpp"
#include "API/CNWSArea.hpp"
#include "API/CNWSCreature.hpp"
#include "API/CNWSCreatureStats.hpp"
#include "API/CNWSCombatAttackData.hpp"
#include "API/CNWSCombatRound.hpp"
#include "API/CNWSEffectListHandler.hpp"
#include "API/CNWSInventory.hpp"
#include "API/CNWSPlayer.hpp"
#include "API/CNWBaseItemArray.hpp"
#include "API/CNWBaseItem.hpp"
#include "API/CNWCCMessageData.hpp"
#include "API/CNWSItem.hpp"
#include "API/CNWRules.hpp"
#include "API/CNWVisibilityNode.hpp"
#include "API/CServerExoApp.hpp"
#include "API/CVirtualMachine.hpp"
#include "API/CTlkTable.hpp"
#include "API/CWorldTimer.hpp"

#include <set>
#include <map>

using namespace NWNXLib;
using namespace NWNXLib::API;

namespace CormyrDalelands {

const static int32_t ITEM_PROPERTY_GHOST_TOUCH_WEAPON = 213;

const static uint8_t CLASS_TYPE_TEMPEST = 86;
const static uint16_t FEAT_TEMPEST_AMBIDEXTERITY_1 = 2483;
const static uint16_t FEAT_TEMPEST_AMBIDEXTERITY_2 = 2486;

static std::unordered_map<std::uint16_t, int8_t> m_ACNaturalBaseModifierFeats;

static std::set<std::uint16_t> m_DefaultSneakAttackFeats = {
    Constants::Feat::SneakAttack,
    Constants::Feat::SneakAttack2,
    Constants::Feat::SneakAttack3,
    Constants::Feat::SneakAttack4,
    Constants::Feat::SneakAttack5,
    Constants::Feat::SneakAttack6,
    Constants::Feat::SneakAttack7,
    Constants::Feat::SneakAttack8,
    Constants::Feat::SneakAttack9,
    Constants::Feat::SneakAttack10,
    Constants::Feat::SneakAttack11,
    Constants::Feat::SneakAttack12,
    Constants::Feat::SneakAttack13,
    Constants::Feat::SneakAttack14,
    Constants::Feat::SneakAttack15,
    Constants::Feat::SneakAttack16,
    Constants::Feat::SneakAttack17,
    Constants::Feat::SneakAttack18,
    Constants::Feat::SneakAttack19,
    Constants::Feat::SneakAttack20,
    Constants::Feat::BlackguardSneakAttack1d6,
    Constants::Feat::BlackguardSneakAttack2d6,
    Constants::Feat::BlackguardSneakAttack3d6,
    Constants::Feat::BlackguardSneakAttack4d6,
    Constants::Feat::BlackguardSneakAttack5d6,
    Constants::Feat::BlackguardSneakAttack6d6,
    Constants::Feat::BlackguardSneakAttack7d6,
    Constants::Feat::BlackguardSneakAttack8d6,
    Constants::Feat::BlackguardSneakAttack9d6,
    Constants::Feat::BlackguardSneakAttack10d6,
    Constants::Feat::BlackguardSneakAttack11d6,
    Constants::Feat::BlackguardSneakAttack12d6,
    Constants::Feat::BlackguardSneakAttack13d6,
    Constants::Feat::BlackguardSneakAttack14d6,
    Constants::Feat::BlackguardSneakAttack15d6,
    Constants::Feat::EpicImprovedSneakAttack1,
    Constants::Feat::EpicImprovedSneakAttack2,
    Constants::Feat::EpicImprovedSneakAttack3,
    Constants::Feat::EpicImprovedSneakAttack4,
    Constants::Feat::EpicImprovedSneakAttack5,
    Constants::Feat::EpicImprovedSneakAttack6,
    Constants::Feat::EpicImprovedSneakAttack7,
    Constants::Feat::EpicImprovedSneakAttack8,
    Constants::Feat::EpicImprovedSneakAttack9,
    Constants::Feat::EpicImprovedSneakAttack10
};
static std::set<std::uint16_t> m_SneakAttackFeats = m_DefaultSneakAttackFeats;

static std::set<std::uint16_t> m_DefaultDeathAttackFeats = {
    Constants::Feat::PrestigeDeathAttack1,
    Constants::Feat::PrestigeDeathAttack2,
    Constants::Feat::PrestigeDeathAttack3,
    Constants::Feat::PrestigeDeathAttack4,
    Constants::Feat::PrestigeDeathAttack5,
    Constants::Feat::PrestigeDeathAttack6,
    Constants::Feat::PrestigeDeathAttack7,
    Constants::Feat::PrestigeDeathAttack8,
    Constants::Feat::PrestigeDeathAttack9,
    Constants::Feat::PrestigeDeathAttack10,
    Constants::Feat::PrestigeDeathAttack11,
    Constants::Feat::PrestigeDeathAttack12,
    Constants::Feat::PrestigeDeathAttack13,
    Constants::Feat::PrestigeDeathAttack14,
    Constants::Feat::PrestigeDeathAttack15,
    Constants::Feat::PrestigeDeathAttack16,
    Constants::Feat::PrestigeDeathAttack17,
    Constants::Feat::PrestigeDeathAttack18,
    Constants::Feat::PrestigeDeathAttack19,
    Constants::Feat::PrestigeDeathAttack20
};
static std::set<std::uint16_t> m_DeathAttackFeats = m_DefaultDeathAttackFeats;

static std::set<std::uint8_t> m_SneakAttackUncannyDodgeClasses = {
    Constants::ClassType::Barbarian,
    Constants::ClassType::Rogue,
    Constants::ClassType::Assassin,
    Constants::ClassType::Shadowdancer
};

static std::set<std::uint8_t> m_BardSongUsesProgressingClasses = {
    Constants::ClassType::Bard
};

static std::set<std::uint32_t> m_BaseItemsAllowUseUnequipped;

static bool s_InResolveDefensiveEffectsWithGhostTouchWeapon;
static bool s_OverrideSneakAttackDamageRoll;
static bool s_InSneakAttackRollDice;
static bool s_OverrideDeathAttackDamageRoll;
static bool s_InDeathAttackRollDice;
static int32_t s_TempestAmbidexterityModifier;
static bool s_InUseItemAllowUnequipped;
static bool s_BardSongExtraMusicByCharismaModifier = Config::Get<bool>("BARD_SONG_EXTRA_MUSIC_BY_CHARISMA_MOD", false);
static bool s_LingeringSongExtraMusic = Config::Get<bool>("BARD_SONG_EXTRA_MUSIC_USE_LINGERING_SONG_FEAT", false);

static CNWSCreatureStats *s_SneakAttackDamageRollCreatureStats = nullptr;
static CNWSCreatureStats *s_DeathAttackDamageRollCreatureStats = nullptr;

void GhostTouchWeaponProperty() __attribute__((constructor));
void GhostTouchWeaponProperty()
{
    if (!Config::Get<bool>("ENABLE_GHOST_TOUCH_WEAPON_PROPERTY", false))
        return;

    LOG_INFO("Concealment of creatures flagged incorporeal with NWNX_CormyrDalelands_SetCreatureIncorporealFlag() will be ignored by weapons with the Ghost Touch property");

    static Hooks::Hook s_ResolveDefensiveEffectsHook =
        Hooks::HookFunction(&CNWSCreature::ResolveDefensiveEffects,
        +[](CNWSCreature *pThis, CNWSObject *pTarget, BOOL bAttackHit = true) -> bool
        {
            if (pTarget->nwnxGet<int>("CDX_INCORPOREAL_CREATURE"))
            {
                if (auto *pCombatRound = pThis->m_pcCombatRound)
                {
                    if (auto *pWeapon = pCombatRound->GetCurrentAttackWeapon(pCombatRound->GetWeaponAttackType()))
                    {
                        if (pWeapon->GetPropertyByTypeExists(ITEM_PROPERTY_GHOST_TOUCH_WEAPON))
                        {
                            s_InResolveDefensiveEffectsWithGhostTouchWeapon = true;
                            s_ResolveDefensiveEffectsHook->CallOriginal<bool>(pThis, pTarget, bAttackHit);
                            s_InResolveDefensiveEffectsWithGhostTouchWeapon = false;
                        }
                    }
                }
            }

            return s_ResolveDefensiveEffectsHook->CallOriginal<bool>(pThis, pTarget, bAttackHit);
        }, Hooks::Order::Earliest);

    static Hooks::Hook s_GetEffectIntegerHook =
        Hooks::HookFunction(&CGameEffect::GetInteger,
        +[](CGameEffect *pThis, int32_t nStorageLocation) -> int32_t
        {
            if (s_InResolveDefensiveEffectsWithGhostTouchWeapon)
            {
                if (pThis->m_nType == Constants::EffectTrueType::Concealment && nStorageLocation == 0)
                    return 0;
            }

            return s_GetEffectIntegerHook->CallOriginal<int32_t>(pThis, nStorageLocation);
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetRulesetIntEntryHook =
        Hooks::HookFunction(&CNWRules::GetRulesetIntEntry,
        +[](CNWRules *pThis, uint64_t hashedLabel, int32_t whenMissing) -> int32_t
        {
            if (s_InResolveDefensiveEffectsWithGhostTouchWeapon)
            {
                if (hashedLabel == CRULES_HASHEDSTR("INVISIBILITY_CONCEALMENT_CHANCE") ||
                    hashedLabel == CRULES_HASHEDSTR("EPIC_SELF_CONCEALMENT_10") ||
                    hashedLabel == CRULES_HASHEDSTR("EPIC_SELF_CONCEALMENT_20") ||
                    hashedLabel == CRULES_HASHEDSTR("EPIC_SELF_CONCEALMENT_30") ||
                    hashedLabel == CRULES_HASHEDSTR("EPIC_SELF_CONCEALMENT_40") ||
                    hashedLabel == CRULES_HASHEDSTR("EPIC_SELF_CONCEALMENT_50"))
                    return 0;
            }

            return s_GetRulesetIntEntryHook->CallOriginal<int32_t>(pThis, hashedLabel, whenMissing);
        }, Hooks::Order::Late);
}

NWNX_EXPORT ArgumentStack GetCreatureIncorporealFlag(ArgumentStack&& args)
{
    int32_t retval = false;

    if (auto *pCreature = Utils::PopCreature(args))
    {
        if (auto incorporeal = pCreature->nwnxGet<int>("CDX_INCORPOREAL_CREATURE"))
        {
            retval = !!*incorporeal;
        }
    }

    return retval;
}

NWNX_EXPORT ArgumentStack SetCreatureIncorporealFlag(ArgumentStack&& args)
{
    if (auto *pCreature = Utils::PopCreature(args))
    {
        auto bIncorporealFlag = !!args.extract<int32_t>();

        if (bIncorporealFlag == false)
            pCreature->nwnxRemove("CDX_INCORPOREAL_CREATURE");
        else
            pCreature->nwnxSet("CDX_INCORPOREAL_CREATURE", bIncorporealFlag, false);
    }

    return {};
}

NWNX_EXPORT ArgumentStack SetClassIsSneakAttackUncannyDodgeClass(ArgumentStack&& args)
{
    const auto nClassId = args.extract<int32_t>();
      ASSERT_OR_THROW(nClassId >= Constants::ClassType::MIN);
      ASSERT_OR_THROW(nClassId <= Constants::ClassType::MAX);

    CNWClass *pClass = nClassId < Globals::Rules()->m_nNumClasses ? &Globals::Rules()->m_lstClasses[nClassId] : nullptr;
      ASSERT_OR_THROW(pClass != nullptr);

    auto bSet = !!args.extract<int32_t>();

    if (bSet)
    {
        m_SneakAttackUncannyDodgeClasses.insert(nClassId);
        LOG_INFO("Class %s [%d] set as sneak attack uncanny dodge class", pClass->GetNameText(), nClassId);
    }
    else
    {
        m_SneakAttackUncannyDodgeClasses.erase(nClassId);
        LOG_INFO("Class %s [%d] unset as sneak attack uncanny dodge class", pClass->GetNameText(), nClassId);
    }

    static Hooks::Hook s_ResolveSneakAttackHook =
        Hooks::HookFunction(&CNWSCreature::ResolveSneakAttack,
        +[](CNWSCreature *pThis, CNWSCreature *pTarget) -> void
        {
            static const float SNEAK_ATTACK_DISTANCE = std::pow(
                Globals::Rules()->GetRulesetFloatEntry(CRULES_HASHEDSTR("MAX_RANGED_SNEAK_ATTACK_DISTANCE"), 10.0f), 2);

            if (!pTarget)
                return;

            CNWSCombatAttackData* pAttackData = pThis->m_pcCombatRound->GetAttack(pThis->m_pcCombatRound->m_nCurrentAttack);

            if (pAttackData->m_nAttackType == Constants::Feat::WhirlwindAttack ||
                pAttackData->m_nAttackType == Constants::Feat::ImprovedWhirlwind)
            {
                return;
            }

            bool hasSneakAttack = false;
            for (const auto &i : m_SneakAttackFeats)
            {
                if (pThis->m_pStats->HasFeat(i))
                {
                    hasSneakAttack = true;
                    break;
                }
            }
            if (!hasSneakAttack)
                return;

            float fDistance = 0.0;
            if (pAttackData->m_bRangedAttack)
            {
                fDistance = Vector::MagnitudeSquared(pThis->m_vPosition - pTarget->m_vPosition);
                if (fDistance >= SNEAK_ATTACK_DISTANCE)
                    return;
            }

            bool isSneakAttack = false;

            auto* pVisNode = pTarget->GetVisibleListElement(pThis->m_idSelf);
            if (!pVisNode || !pVisNode->m_bSeen || pTarget->GetFlatFooted())
            {
                isSneakAttack = true;
            }
            else if (pThis->GetFlanked(pTarget))
            {
                isSneakAttack = true;

                if (pTarget->m_pStats->HasFeat(Constants::Feat::UncannyDodge2))
                {
                    int attackerLevels = 0, defenderLevels = 0;

                    for (uint8_t i = 0; i < pThis->m_pStats->m_nNumMultiClasses; i++)
                    {
                        uint8_t attackerClass = pThis->m_pStats->GetClass(i);
                        
                        if (m_SneakAttackUncannyDodgeClasses.find(attackerClass) != m_SneakAttackUncannyDodgeClasses.end())
                            attackerLevels += pThis->m_pStats->GetClassLevel(i, false);
                    }

                    for (uint8_t i = 0; i < pTarget->m_pStats->m_nNumMultiClasses; i++)
                    {
                        uint8_t defenderClass = pTarget->m_pStats->GetClass(i);

                        if (m_SneakAttackUncannyDodgeClasses.find(defenderClass) != m_SneakAttackUncannyDodgeClasses.end())
                            defenderLevels += pTarget->m_pStats->GetClassLevel(i, false);
                    }

                    isSneakAttack = attackerLevels - defenderLevels >= Globals::Rules()->GetRulesetIntEntry(CRULES_HASHEDSTR("FLANK_LEVEL_RANGE"), 4);
                }
            }

            if (isSneakAttack)
            {
                if (pTarget->m_pStats->GetEffectImmunity(Constants::ImmunityType::SneakAttack, pThis, true) || pTarget->m_pStats->GetEffectImmunity(Constants::ImmunityType::CriticalHit, pThis, true))
                {
                    CNWCCMessageData* pData = new CNWCCMessageData;
                    pData->SetObjectID(0, pTarget->m_idSelf);
                    pData->SetInteger(0, 134);
                    pAttackData->m_alstPendingFeedback.Add(pData);
                }
                else
                {
                    pAttackData->m_bSneakAttack = 1;
                }
            }
        }, Hooks::Order::Final);
    
    static Hooks::Hook s_ResolveDeathAttackHook =
        Hooks::HookFunction(&CNWSCreature::ResolveDeathAttack,
        +[](CNWSCreature *pThis, CNWSCreature *pTarget) -> void
        {
            static const float SNEAK_ATTACK_DISTANCE = std::pow(
                Globals::Rules()->GetRulesetFloatEntry(CRULES_HASHEDSTR("MAX_RANGED_SNEAK_ATTACK_DISTANCE"), 10.0f), 2);
            if (!pTarget)
                return;

            CNWSCombatAttackData *pAttackData = pThis->m_pcCombatRound->GetAttack(pThis->m_pcCombatRound->m_nCurrentAttack);

            if (pAttackData->m_nAttackType == Constants::Feat::WhirlwindAttack ||
                pAttackData->m_nAttackType == Constants::Feat::ImprovedWhirlwind)
            {
                return;
            }

            bool hasDeathAttack = false;
            for (const auto &i : m_DeathAttackFeats)
            {
                if (pThis->m_pStats->HasFeat(i))
                {
                    hasDeathAttack = true;
                    break;
                }
            }
            if (!hasDeathAttack)
                return;

            float fDistance = 0.0;
            if (pAttackData->m_bRangedAttack)
            {
                fDistance = Vector::MagnitudeSquared(pThis->m_vPosition - pTarget->m_vPosition);
                if (fDistance >= SNEAK_ATTACK_DISTANCE)
                    return;
            }

            bool isDeathAttack = false;

            auto* pVisNode = pTarget->GetVisibleListElement(pThis->m_idSelf);
            if (!pVisNode || !pVisNode->m_bSeen || pTarget->GetFlatFooted())
            {
                isDeathAttack = true;
            }
            else if (pThis->GetFlanked(pTarget))
            {
                isDeathAttack = true;

                if (pTarget->m_pStats->HasFeat(Constants::Feat::UncannyDodge2))
                {
                    int attackerLevels = 0, defenderLevels = 0;

                    for (uint8_t i = 0; i < pThis->m_pStats->m_nNumMultiClasses; i++)
                    {
                        uint8_t attackerClass = pThis->m_pStats->GetClass(i);
                        
                        if (m_SneakAttackUncannyDodgeClasses.find(attackerClass) != m_SneakAttackUncannyDodgeClasses.end())
                            attackerLevels += pThis->m_pStats->GetClassLevel(i, false);
                    }

                    for (uint8_t i = 0; i < pTarget->m_pStats->m_nNumMultiClasses; i++)
                    {
                        uint8_t defenderClass = pTarget->m_pStats->GetClass(i);

                        if (m_SneakAttackUncannyDodgeClasses.find(defenderClass) != m_SneakAttackUncannyDodgeClasses.end())
                            defenderLevels += pTarget->m_pStats->GetClassLevel(i, false);
                    }

                    isDeathAttack = attackerLevels - defenderLevels >= Globals::Rules()->GetRulesetIntEntry(CRULES_HASHEDSTR("FLANK_LEVEL_RANGE"), 4);
                }
            }

            if (isDeathAttack)
            {
                if (pTarget->m_pStats->GetEffectImmunity(Constants::ImmunityType::SneakAttack, pThis, true) || pTarget->m_pStats->GetEffectImmunity(Constants::ImmunityType::CriticalHit, pThis, true))
                {
                    CNWCCMessageData* pData = new CNWCCMessageData;
                    pData->SetObjectID(0, pTarget->m_idSelf);
                    pData->SetInteger(0, 134);
                    pAttackData->m_alstPendingFeedback.Add(pData);
                }
                else
                {
                    pAttackData->m_bDeathAttack = 1;
                }
            }
        }, Hooks::Order::Final);

    return {};
}

NWNX_EXPORT ArgumentStack SetFeatIsSneakAttackFeat(ArgumentStack&& args)
{
    const auto nFeat = args.extract<int32_t>();
      ASSERT_OR_THROW(nFeat >= Constants::Feat::MIN);
      ASSERT_OR_THROW(nFeat <= Constants::Feat::MAX);

    CNWFeat *pFeat = Globals::Rules()->GetFeat(static_cast<uint16_t>(nFeat));
      ASSERT_OR_THROW(pFeat);

    auto bSet = !!args.extract<int32_t>();

    if (bSet)
    {
        m_SneakAttackFeats.insert(nFeat);
        LOG_INFO("Feat %s [%d] set as Sneak Attack feat", pFeat->GetNameText(), nFeat);
    }
    else
    {
        m_SneakAttackFeats.erase(nFeat);
        LOG_INFO("Feat %s [%d] unset as Sneak Attack feat", pFeat->GetNameText(), nFeat);
    }

    static Hooks::Hook s_GetDamageRollHook =
        Hooks::HookFunction(&CNWSCreatureStats::GetDamageRoll,
        +[](CNWSCreatureStats *pThis, CNWSObject *pTarget, BOOL bOffHand, BOOL bCritical, BOOL bSneakAttack, BOOL bDeathAttack, BOOL bForceMax = false) -> int32_t
        {
            if (!bSneakAttack)
                return s_GetDamageRollHook->CallOriginal<int32_t>(pThis, pTarget, bOffHand, bCritical, bSneakAttack, bDeathAttack, bForceMax);;

            s_SneakAttackDamageRollCreatureStats = pThis;
            auto retval = s_GetDamageRollHook->CallOriginal<int32_t>(pThis, pTarget, bOffHand, bCritical, bSneakAttack, bDeathAttack, bForceMax);
            s_SneakAttackDamageRollCreatureStats = nullptr;

            return retval;
        }, Hooks::Order::Late);

    static Hooks::Hook s_HasFeatHook =
        Hooks::HookFunction(&CNWSCreatureStats::HasFeat,
        +[](CNWSCreatureStats *pThis, uint16_t nFeat) -> bool
        {
            if (!s_InSneakAttackRollDice && s_SneakAttackDamageRollCreatureStats && m_DefaultSneakAttackFeats.find(nFeat) != m_DefaultSneakAttackFeats.end())
            {
                s_OverrideSneakAttackDamageRoll = true;
                return false;
            }
            
            return s_HasFeatHook->CallOriginal<bool>(pThis, nFeat);
        }, Hooks::Order::Late);

    static Hooks::Hook s_RollDiceHook =
        Hooks::HookFunction(&CNWRules::RollDice,
        +[](CNWRules *pThis, uint8_t nNumberOfDice, uint8_t nSides) -> uint16_t
        {
            if (s_OverrideSneakAttackDamageRoll)
            {
                s_InSneakAttackRollDice = true;

                if (s_SneakAttackDamageRollCreatureStats)
                {
                    nNumberOfDice = 0;
                    for (const auto &i : m_SneakAttackFeats)
                    {
                        if (s_SneakAttackDamageRollCreatureStats->HasFeat(i))
                            nNumberOfDice++;
                    }
                }

                s_InSneakAttackRollDice = false;
                s_OverrideSneakAttackDamageRoll = false;
            }
                
            return s_RollDiceHook->CallOriginal<uint16_t>(pThis, nNumberOfDice, nSides);
        }, Hooks::Order::Late);

    return {};
}

NWNX_EXPORT ArgumentStack SetFeatIsDeathAttackFeat(ArgumentStack&& args)
{
    const auto nFeat = args.extract<int32_t>();
      ASSERT_OR_THROW(nFeat >= Constants::Feat::MIN);
      ASSERT_OR_THROW(nFeat <= Constants::Feat::MAX);

    CNWFeat *pFeat = Globals::Rules()->GetFeat(static_cast<uint16_t>(nFeat));
      ASSERT_OR_THROW(pFeat);

    auto bSet = !!args.extract<int32_t>();

    if (bSet)
    {
        m_DeathAttackFeats.insert(nFeat);
        LOG_INFO("Feat %s [%d] set as Death Attack feat", pFeat->GetNameText(), nFeat);
    }
    else
    {
        m_DeathAttackFeats.erase(nFeat);
        LOG_INFO("Feat %s [%d] unset as Death Attack feat", pFeat->GetNameText(), nFeat);
    }

    static Hooks::Hook s_GetDamageRollHook =
        Hooks::HookFunction(&CNWSCreatureStats::GetDamageRoll,
        +[](CNWSCreatureStats *pThis, CNWSObject *pTarget, BOOL bOffHand, BOOL bCritical, BOOL bSneakAttack, BOOL bDeathAttack, BOOL bForceMax = false) -> int32_t
        {
            if (!bDeathAttack)
                return s_GetDamageRollHook->CallOriginal<int32_t>(pThis, pTarget, bOffHand, bCritical, bSneakAttack, bDeathAttack, bForceMax);;

            s_DeathAttackDamageRollCreatureStats = pThis;
            auto retval = s_GetDamageRollHook->CallOriginal<int32_t>(pThis, pTarget, bOffHand, bCritical, bSneakAttack, bDeathAttack, bForceMax);
            s_DeathAttackDamageRollCreatureStats = nullptr;

            return retval;
        }, Hooks::Order::Late);

    static Hooks::Hook s_HasFeatHook =
        Hooks::HookFunction(&CNWSCreatureStats::HasFeat,
        +[](CNWSCreatureStats *pThis, uint16_t nFeat) -> bool
        {
            if (!s_InDeathAttackRollDice && s_DeathAttackDamageRollCreatureStats && m_DefaultDeathAttackFeats.find(nFeat) != m_DefaultDeathAttackFeats.end())
            {
                s_OverrideDeathAttackDamageRoll = true;
                return false;
            }
            
            return s_HasFeatHook->CallOriginal<bool>(pThis, nFeat);
        }, Hooks::Order::Late);

    static Hooks::Hook s_RollDiceHook =
        Hooks::HookFunction(&CNWRules::RollDice,
        +[](CNWRules *pThis, uint8_t nNumberOfDice, uint8_t nSides) -> uint16_t
        {
            if (s_OverrideDeathAttackDamageRoll)
            {
                s_InDeathAttackRollDice = true;

                if (s_DeathAttackDamageRollCreatureStats)
                {
                    nNumberOfDice = 0;
                    for (const auto &i : m_DeathAttackFeats)
                    {
                        if (s_DeathAttackDamageRollCreatureStats->HasFeat(i))
                            nNumberOfDice++;
                    }
                }

                s_InDeathAttackRollDice = false;
                s_OverrideDeathAttackDamageRoll = false;
            }
                
            return s_RollDiceHook->CallOriginal<uint16_t>(pThis, nNumberOfDice, nSides);
        }, Hooks::Order::Late);

    return {};
}

NWNX_EXPORT ArgumentStack SetNaturalBaseACModifierFeat(ArgumentStack&& args)
{
    const auto nFeat = args.extract<int32_t>();
      ASSERT_OR_THROW(nFeat >= Constants::Feat::MIN);
      ASSERT_OR_THROW(nFeat <= Constants::Feat::MAX);

    const auto nModifier = args.extract<int32_t>();

    CNWFeat *pFeat = Globals::Rules()->GetFeat(static_cast<uint16_t>(nFeat));
      ASSERT_OR_THROW(pFeat);

    m_ACNaturalBaseModifierFeats[static_cast<uint16_t>(nFeat)] = static_cast<int8_t>(nModifier);

    LOG_INFO("Feat %s [%d] set as %+d base natural AC modifier feat", pFeat->GetNameText(), nFeat, nModifier);

    static Hooks::Hook s_GetACNaturalBaseHook =
        Hooks::HookFunction(&CNWSCreatureStats::GetACNaturalBase,
        +[](CNWSCreatureStats *pThis, BOOL bVsTouchAttack = false) -> char
        {
            char retval = s_GetACNaturalBaseHook->CallOriginal<char>(pThis, bVsTouchAttack);

            if (bVsTouchAttack)
                return retval;

            std::set<std::uint8_t> calculatedFeats;
            
            for (const auto &it : m_ACNaturalBaseModifierFeats)
            {
                if (pThis->HasFeat(it.first))
                {
                    auto itHighestFeat = m_ACNaturalBaseModifierFeats.find(pThis->GetHighestLevelOfFeat(it.first));
                    if (itHighestFeat != m_ACNaturalBaseModifierFeats.end())
                    {
                        if (calculatedFeats.find(itHighestFeat->first) == calculatedFeats.end())
                        {
                            calculatedFeats.insert(itHighestFeat->first);
                            retval += itHighestFeat->second;
                        }
                    }
                    else
                    {
                        calculatedFeats.insert(it.second);
                        retval += it.second;
                    }
                }
            }

            return retval;
        }, Hooks::Order::Late);

    return {};
}

void RangedWeaponsUseOnHitEffectItemProperties() __attribute__((constructor));
void RangedWeaponsUseOnHitEffectItemProperties()
{
    if (!Config::Get<bool>("RANGED_WEAPONS_USE_ON_HIT_EFFECT_ITEM_PROPERTIES", false))
        return;

    LOG_INFO("Ranged weapons will use On Hit: Effect item properties when the ammo does not have any On Hit or On Hit Cast Spell properties");

    static Hooks::Hook s_ResolveOnHitEffectHook = Hooks::HookFunction(&CNWSCreature::ResolveOnHitEffect,
        +[](CNWSCreature *pCreature, CNWSObject *pTarget, BOOL bOffHandAttack, BOOL bCritical) -> void
        {
            if (auto *pCombatRound = pCreature->m_pcCombatRound)
            {
                if (auto *pWeapon = pCombatRound->GetCurrentAttackWeapon(pCombatRound->GetWeaponAttackType()))
                {
                    auto nBaseItem = pWeapon->m_nBaseItem;

                    if (nBaseItem == Constants::BaseItem::HeavyCrossbow ||
                        nBaseItem == Constants::BaseItem::LightCrossbow ||
                        nBaseItem == Constants::BaseItem::Longbow ||
                        nBaseItem == Constants::BaseItem::Shortbow ||
                        nBaseItem == Constants::BaseItem::Sling)
                    {
                        auto nEquipmentSlot = Constants::EquipmentSlot::None;
                        switch (nBaseItem)
                        {
                            case Constants::BaseItem::HeavyCrossbow:
                            case Constants::BaseItem::LightCrossbow:
                                nEquipmentSlot = Constants::EquipmentSlot::Bolts;
                                break;
                            case Constants::BaseItem::Longbow:
                            case Constants::BaseItem::Shortbow:
                                nEquipmentSlot = Constants::EquipmentSlot::Arrows;
                                break;
                            case Constants::BaseItem::Sling:
                                nEquipmentSlot = Constants::EquipmentSlot::Bullets;
                                break;
                        }
         
                        if (nEquipmentSlot)
                        {
                            if (auto *pAmmunition = pCreature->m_pInventory->GetItemInSlot(nEquipmentSlot))
                            {
                                auto bIgnoreWeapon = false;

                                for (int32_t i = 0; i < pAmmunition->m_lstPassiveProperties.num; i++)
                                {
                                    if (auto *pItemProperty = pAmmunition->GetPassiveProperty(i))
                                    {
                                        if (pItemProperty->m_nPropertyName == Constants::ItemProperty::OnHitCastSpell || pItemProperty->m_nPropertyName == Constants::ItemProperty::OnHitProperties)
                                        {
                                            if (pItemProperty->m_nDurationType == Constants::EffectDurationType::Permanent)
                                            {
                                                bIgnoreWeapon = true;
                                                break;
                                            }
                                        }
                                    }
                                }

                                if (!bIgnoreWeapon)
                                {
                                    pWeapon->m_nBaseItem = Constants::BaseItem::Shortsword;
                                    s_ResolveOnHitEffectHook->CallOriginal<void>(pCreature, pTarget, bOffHandAttack, bCritical);
                                    pWeapon->m_nBaseItem = nBaseItem;
                                }
                            }
                        }
                    }
                }
            }

            s_ResolveOnHitEffectHook->CallOriginal<void>(pCreature, pTarget, bOffHandAttack, bCritical);
            
        }, Hooks::Order::Earliest);
}

void RangedWeaponsUseOnHitCastSpellItemProperties() __attribute__((constructor));
void RangedWeaponsUseOnHitCastSpellItemProperties()
{
    if (!Config::Get<bool>("RANGED_WEAPONS_USE_ON_HIT_CAST_SPELL_ITEM_PROPERTIES", false))
        return;

    LOG_INFO("Ranged weapons will use On Hit: Cast Spell item properties when the ammo does not have any On Hit or On Hit Cast Spell properties");

    static Hooks::Hook s_ResolveItemCastSpellHook = Hooks::HookFunction(&CNWSCreature::ResolveItemCastSpell,
        +[](CNWSCreature *pCreature, CNWSObject *pTarget) -> void
        {
            if (auto *pCombatRound = pCreature->m_pcCombatRound)
            {
                if (auto *pWeapon = pCombatRound->GetCurrentAttackWeapon(pCombatRound->GetWeaponAttackType()))
                {
                    auto nBaseItem = pWeapon->m_nBaseItem;

                    if (nBaseItem == Constants::BaseItem::HeavyCrossbow ||
                        nBaseItem == Constants::BaseItem::LightCrossbow ||
                        nBaseItem == Constants::BaseItem::Longbow ||
                        nBaseItem == Constants::BaseItem::Shortbow ||
                        nBaseItem == Constants::BaseItem::Sling)
                    {
                        auto nEquipmentSlot = Constants::EquipmentSlot::None;
                        switch (nBaseItem)
                        {
                            case Constants::BaseItem::HeavyCrossbow:
                            case Constants::BaseItem::LightCrossbow:
                                nEquipmentSlot = Constants::EquipmentSlot::Bolts;
                                break;
                            case Constants::BaseItem::Longbow:
                            case Constants::BaseItem::Shortbow:
                                nEquipmentSlot = Constants::EquipmentSlot::Arrows;
                                break;
                            case Constants::BaseItem::Sling:
                                nEquipmentSlot = Constants::EquipmentSlot::Bullets;
                                break;
                        }
         
                        if (nEquipmentSlot)
                        {
                            if (auto *pAmmunition = pCreature->m_pInventory->GetItemInSlot(nEquipmentSlot))
                            {
                                auto bIgnoreWeapon = false;

                                for (int32_t i = 0; i < pAmmunition->m_lstPassiveProperties.num; i++)
                                {
                                    if (auto *pItemProperty = pAmmunition->GetPassiveProperty(i))
                                    {
                                        if (pItemProperty->m_nPropertyName == Constants::ItemProperty::OnHitCastSpell || pItemProperty->m_nPropertyName == Constants::ItemProperty::OnHitProperties)
                                        {
                                            if (pItemProperty->m_nDurationType == Constants::EffectDurationType::Permanent)
                                            {
                                                bIgnoreWeapon = true;
                                                break;
                                            }
                                        }
                                    }
                                }

                                if (!bIgnoreWeapon)
                                {
                                    pWeapon->m_nBaseItem = Constants::BaseItem::Shortsword;
                                    s_ResolveItemCastSpellHook->CallOriginal<void>(pCreature, pTarget);
                                    pWeapon->m_nBaseItem = nBaseItem;
                                }
                            }
                        }
                    }
                }
            }

            s_ResolveItemCastSpellHook->CallOriginal<void>(pCreature, pTarget);
            
        }, Hooks::Order::Earliest);
}

void FixDefensiveStanceTotalUses() __attribute__((constructor));
void FixDefensiveStanceTotalUses()
{
    if (!Config::Get<bool>("FIX_DEFENSIVE_STANCE_TOTAL_USES", false))
        return;

    LOG_INFO("Defensive Stance's total uses will be set according to feats.2da UsesPerDay");

    static Hooks::Hook s_GetFeatTotalUsesHook = Hooks::HookFunction(&CNWSCreatureStats::GetFeatTotalUses,
        +[](CNWSCreatureStats *pThis, uint16_t nFeat) -> uint8_t
        {
            if (nFeat == Constants::Feat::DwarvenDefenderDefensiveStance)
            {
                if (auto *pFeat = Globals::Rules()->GetFeat(nFeat))
                {
                    if (!pThis->HasFeat(nFeat))
                        return 0;
                    
                    if (pThis->m_lstFeatUses.num < 1)
                        return 100;

                    auto *pFeatUses = pThis->m_lstFeatUses.element[0];
                    for (int32_t i = 0; pFeatUses->m_nFeat != nFeat; ++i)
                    {
                        if (i > pThis->m_lstFeatUses.num)
                            return 100;

                        pFeatUses = pThis->m_lstFeatUses.element[i];
                    }

                    auto nNumUses = pFeat->m_nUsesPerDay;
                    if (nNumUses > 100)
                        return 100;

                    return nNumUses;
                }
            }

            return s_GetFeatTotalUsesHook->CallOriginal<uint8_t>(pThis, nFeat);
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetFeatRemainingUsesHook = Hooks::HookFunction(&CNWSCreatureStats::GetFeatRemainingUses,
        +[](CNWSCreatureStats *pThis, uint16_t nFeat) -> uint8_t
        {
            if (nFeat == Constants::Feat::DwarvenDefenderDefensiveStance)
            {
                if (!Globals::Rules()->GetFeat(nFeat))
                    return 0;
                
                if (pThis->GetIsDM())
                    return 100;

                if (!pThis->HasFeat(nFeat))
                    return 0;

                auto nNumUses = pThis->GetFeatTotalUses(nFeat);

                for (int32_t i = 0; i < pThis->m_lstFeatUses.num; i++)
                {
                    auto *pFeatUses = pThis->m_lstFeatUses.element[i];
                    if (pFeatUses->m_nFeat == nFeat)
                    {
                        nNumUses -= pFeatUses->m_nUsedToday;
                        break;
                    }
                }

                return std::clamp<uint8_t>(nNumUses, 0, 100);
            }

            return s_GetFeatRemainingUsesHook->CallOriginal<uint8_t>(pThis, nFeat);
        }, Hooks::Order::Late);
}

NWNX_EXPORT ArgumentStack SetClassProgressesBardSongUses(ArgumentStack&& args)
{
    const auto nClassId = args.extract<int32_t>();
      ASSERT_OR_THROW(nClassId >= Constants::ClassType::MIN);
      ASSERT_OR_THROW(nClassId <= Constants::ClassType::MAX);

    CNWClass *pClass = nClassId < Globals::Rules()->m_nNumClasses ? &Globals::Rules()->m_lstClasses[nClassId] : nullptr;
      ASSERT_OR_THROW(pClass != nullptr);

    auto bSet = !!args.extract<int32_t>();

    if (bSet)
    {
        m_BardSongUsesProgressingClasses.insert(nClassId);
        LOG_INFO("Class %s [%d] set as Bard Song uses progressing class", pClass->GetNameText(), nClassId);
    }
    else
    {
        m_BardSongUsesProgressingClasses.erase(nClassId);
        LOG_INFO("Class %s [%d] unset as Bard Song uses progressing class", pClass->GetNameText(), nClassId);
    }

    static Hooks::Hook s_GetFeatTotalUsesHook = Hooks::HookFunction(&CNWSCreatureStats::GetFeatTotalUses,
        +[](CNWSCreatureStats *pThis, uint16_t nFeat) -> uint8_t
        {
            if (nFeat == Constants::Feat::BardSongs || (nFeat >= Constants::Feat::BardSongs2 && nFeat <= Constants::Feat::BardSongs20))
            {
                if (auto *pFeat = Globals::Rules()->GetFeat(nFeat))
                {
                    if (!pThis->HasFeat(nFeat))
                        return 0;
                    
                    if (pThis->m_lstFeatUses.num < 1)
                        return 100;

                    auto *pFeatUses = pThis->m_lstFeatUses.element[0];
                    for (int32_t i = 0; pFeatUses->m_nFeat != nFeat; ++i)
                    {
                        if (i > pThis->m_lstFeatUses.num)
                            return 100;

                        pFeatUses = pThis->m_lstFeatUses.element[i];
                    }

                    auto nBardSongClassLevels = 0;
                    for (const auto &it : m_BardSongUsesProgressingClasses)
                        nBardSongClassLevels += pThis->GetNumLevelsOfClass(it);

                    auto nNumUses = pFeat->m_nUsesPerDay + nBardSongClassLevels;
                    
                    // We combine Extra Music into Lingering Song
                    if (s_LingeringSongExtraMusic && (pThis->HasFeat(Constants::Feat::LingeringSong) || pThis->HasFeat(Constants::Feat::ExtraMusic)))
                    {
                        if (s_BardSongExtraMusicByCharismaModifier)
                            nNumUses += std::clamp<uint8_t>(pThis->m_nCharismaModifier, 0, nBardSongClassLevels);
                        else
                            nNumUses += Globals::Rules()->GetRulesetIntEntry(CRULES_HASHEDSTR("EXTRA_MUSIC_BONUS_USES"), 4);
                    }

                    if (m_BardSongUsesProgressingClasses.find(Constants::ClassType::Bard) != m_BardSongUsesProgressingClasses.end())
                        nNumUses -= std::min(pThis->GetNumLevelsOfClass(Constants::ClassType::Bard), 20);

                    if (nNumUses > 100)
                        return 100;

                    return nNumUses;
                
                }
            }

            return s_GetFeatTotalUsesHook->CallOriginal<uint8_t>(pThis, nFeat);
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetFeatRemainingUsesHook = Hooks::HookFunction(&CNWSCreatureStats::GetFeatRemainingUses,
        +[](CNWSCreatureStats *pThis, uint16_t nFeat) -> uint8_t
        {
            if (nFeat == Constants::Feat::BardSongs || (nFeat >= Constants::Feat::BardSongs2 && nFeat <= Constants::Feat::BardSongs20))
            {
                if (!Globals::Rules()->GetFeat(nFeat))
                    return 0;

                if (pThis->GetIsDM())
                    return 100;

                if (!pThis->HasFeat(nFeat))
                    return 0;

                auto nNumUses = pThis->GetFeatTotalUses(nFeat);

                for (int32_t i = 0; i < pThis->m_lstFeatUses.num; i++)
                {
                    auto *pFeatUses = pThis->m_lstFeatUses.element[i];
                    if (pFeatUses->m_nFeat == nFeat)
                    {
                        nNumUses -= pFeatUses->m_nUsedToday;
                        break;
                    }
                }

                return std::clamp<uint8_t>(nNumUses, 0, 100);
            }

            return s_GetFeatRemainingUsesHook->CallOriginal<uint8_t>(pThis, nFeat);
        }, Hooks::Order::Late);

    return {};
}

void ReplaceDefensiveStanceEffects() __attribute__((constructor));
void ReplaceDefensiveStanceEffects()
{
    if (!Config::Get<bool>("DEFENSIVE_STANCE_REPLACE_WITH_SPELL_SCRIPT", false))
        return;

    LOG_INFO("Defensive Stance will be replaced by EffectRunScript events in x2_s2_defstance");

    static Hooks::Hook s_OnApplyDefensiveStanceHook = Hooks::HookFunction(&CNWSEffectListHandler::OnApplyDefensiveStance,
        +[](CNWSEffectListHandler*, CNWSObject *pObject, CGameEffect *pEffect, BOOL bLoadingGame = false) -> int32_t
        {
            auto *pSetAIState = new CGameEffect(pEffect, false);
            pSetAIState->m_nType = Constants::EffectTrueType::SetAIState;
            pSetAIState->SetInteger(0, -3);
            pObject->ApplyEffect(pSetAIState, bLoadingGame, false);

            auto *pRunScript = new CGameEffect(pEffect, false);
            pRunScript->m_nType = Constants::EffectTrueType::RunScript;
            pRunScript->SetInteger(0, 6000);
            // Unnecessary, gets set in CNWSEffectListHandler::OnApplyRunScript()
            /*
            auto *pWorldTimer = Globals::AppManager()->m_pServerExoApp->GetWorldTimer();
            uint32_t currentCalendarDay, currentTimeOfDay;
            pWorldTimer->GetWorldTime(&currentCalendarDay, &currentTimeOfDay);
            pRunScript->SetInteger(2, currentCalendarDay);
            pRunScript->SetInteger(3, currentTimeOfDay);
            */
            pRunScript->SetString(0, CExoString(""));
            pRunScript->SetString(1, CExoString("x2_s2_defstance"));
            pRunScript->SetString(2, CExoString("x2_s2_defstance"));
            pRunScript->SetString(3, CExoString("x2_s2_defstance"));
            pObject->ApplyEffect(pRunScript, bLoadingGame, false);

            return 0;
        }, Hooks::Order::Final);
}

void ExtendEffectACBonusTypes() __attribute__((constructor));
void ExtendEffectACBonusTypes()
{
    if (!Config::Get<bool>("EXTEND_EFFECTAC_BONUS_TYPES", false))
        return;

    LOG_INFO("Enabled AC_BASE_BONUS=5 AC bonus type for EffectACIncrease and EffectACDecrease");

    static Hooks::Hook s_ExecuteCommandEffectACIncreaseHook = Hooks::HookFunction(&CNWSVirtualMachineCommands::ExecuteCommandEffectACIncrease,
        +[](CNWSVirtualMachineCommands *pThis, int32_t nCommandId, int32_t nParameters) -> int32_t
        {
            auto *pVM = Globals::VirtualMachine();
            int32_t nValue, nModifyType, nDamageType;

            if (!pVM->StackPopInteger(&nValue) ||
                !pVM->StackPopInteger(&nModifyType) ||
                (nParameters >= 3 && !pVM->StackPopInteger(&nDamageType)))
                return Constants::VMError::StackUnderflow;

            if (nModifyType == 5)
            {
                auto *pEffect = new CGameEffect(true);
                  SCOPEGUARD(delete pEffect);
                pEffect->m_nType = Constants::EffectTrueType::ACIncrease;
                pEffect->SetSubType_Magical();
                pEffect->SetInteger(0, nModifyType);
                pEffect->SetInteger(1, nValue);
                pEffect->SetInteger(2, Constants::RacialType::All);
                pEffect->SetInteger(5, nDamageType);

                if (auto *pGameObject = Utils::GetGameObject(pThis->m_oidObjectRunScript))
                    pEffect->SetCreator(pGameObject->m_idSelf);

                if (!pVM->StackPushEngineStructure(Constants::VMStructure::Effect, (void*)pEffect))
                    return Constants::VMError::StackOverflow;

                return 0;
            }

            if ((nParameters >= 3 && !pVM->StackPushInteger(nDamageType)) ||
                !pVM->StackPushInteger(nModifyType) ||
                !pVM->StackPushInteger(nValue))
                return Constants::VMError::StackOverflow;

            return s_ExecuteCommandEffectACIncreaseHook->CallOriginal<int32_t>(pThis, nCommandId, nParameters);
        }, Hooks::Order::Late);

    static Hooks::Hook s_ExecuteCommandEffectACDecreaseHook = Hooks::HookFunction(&CNWSVirtualMachineCommands::ExecuteCommandEffectACDecrease,
        +[](CNWSVirtualMachineCommands *pThis, int32_t nCommandId, int32_t nParameters) -> int32_t
        {
            auto *pVM = Globals::VirtualMachine();
            int32_t nValue, nModifyType, nDamageType;

            if (!pVM->StackPopInteger(&nValue) ||
                !pVM->StackPopInteger(&nModifyType) ||
                (nParameters >= 3 && !pVM->StackPopInteger(&nDamageType)))
                return Constants::VMError::StackUnderflow;

            if (nModifyType == 5)
            {
                auto *pEffect = new CGameEffect(true);
                  SCOPEGUARD(delete pEffect);
                pEffect->m_nType = Constants::EffectTrueType::ACDecrease;
                pEffect->SetSubType_Magical();
                pEffect->SetInteger(0, nModifyType);
                pEffect->SetInteger(1, nValue);
                pEffect->SetInteger(2, Constants::RacialType::All);
                pEffect->SetInteger(5, nDamageType);

                if (auto *pGameObject = Utils::GetGameObject(pThis->m_oidObjectRunScript))
                    pEffect->SetCreator(pGameObject->m_idSelf);

                if (!pVM->StackPushEngineStructure(Constants::VMStructure::Effect, (void*)pEffect))
                    return Constants::VMError::StackOverflow;

                return 0;
            }

            if ((nParameters >= 3 && !pVM->StackPushInteger(nDamageType)) ||
                !pVM->StackPushInteger(nModifyType) ||
                !pVM->StackPushInteger(nValue))
                return Constants::VMError::StackOverflow;

            return s_ExecuteCommandEffectACDecreaseHook->CallOriginal<int32_t>(pThis, nCommandId, nParameters);
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetACNaturalBaseHook =
        Hooks::HookFunction(&CNWSCreatureStats::GetACNaturalBase,
        +[](CNWSCreatureStats *pThis, BOOL bVsTouchAttack = false) -> char
        {
            auto retval = s_GetACNaturalBaseHook->CallOriginal<char>(pThis, bVsTouchAttack);

            if (bVsTouchAttack)
                return retval;

            if (auto *pGameObject = Utils::AsNWSObject(pThis->m_pBaseCreature))
            {
                for (auto *pEffect : pGameObject->m_appliedEffects)
                {
                    if ((pEffect->m_nType == Constants::EffectTrueType::ACIncrease || pEffect->m_nType == Constants::EffectTrueType::ACDecrease) && pEffect->GetInteger(0) == 5)
                    {
                        if (pEffect->GetInteger(2) != Constants::RacialType::All ||
                            pEffect->GetInteger(3) != 0 ||
                            pEffect->GetInteger(4) != 0 ||
                            pEffect->GetInteger(5) != 4103)
                                continue;
                        
                        if (pEffect->m_nType == Constants::EffectTrueType::ACIncrease)
                            retval += pEffect->GetInteger(1);
                        else
                            retval -= pEffect->GetInteger(1);
                    }
                }
            }

            return retval;
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetArmorClassVersusHook =
        Hooks::HookFunction(&CNWSCreatureStats::GetArmorClassVersus,
        +[](CNWSCreatureStats *pThis, CNWSCreature *pCreature = nullptr, BOOL bVsTouchAttack = false) -> int16_t
        {
            auto retval = s_GetArmorClassVersusHook->CallOriginal<int16_t>(pThis, pCreature, bVsTouchAttack);

            if (bVsTouchAttack || !pCreature)
                return retval;

            if (auto *pGameObject = Utils::AsNWSObject(pThis->m_pBaseCreature))
            {
                for (auto *pEffect : pGameObject->m_appliedEffects)
                {
                    if ((pEffect->m_nType == Constants::EffectTrueType::ACIncrease || pEffect->m_nType == Constants::EffectTrueType::ACDecrease) && pEffect->GetInteger(0) == 5)
                    {
                        if ((pEffect->GetInteger(2) != Constants::RacialType::All && pEffect->GetInteger(2) != pCreature->m_pStats->m_nRace) ||
                            (pEffect->GetInteger(3) != 0 && pEffect->GetInteger(3) != pCreature->m_pStats->GetSimpleAlignmentLawChaos()) ||
                            (pEffect->GetInteger(4) != 0 && pEffect->GetInteger(4) != pCreature->m_pStats->GetSimpleAlignmentGoodEvil()) ||
                            (pEffect->GetInteger(5) != 4103 && (pEffect->GetInteger(5) & pCreature->GetDamageFlags())) != 0)
                                continue;

                        if (pEffect->m_nType == Constants::EffectTrueType::ACIncrease)
                            retval += pEffect->GetInteger(1);
                        else
                            retval -= pEffect->GetInteger(1);
                    }
                }
            }

            return retval;
        }, Hooks::Order::Late);
}

void DisableItemRestrictionProperties() __attribute__((constructor));
void DisableItemRestrictionProperties()
{
    if (!Config::Get<bool>("DISABLE_ITEM_RESTRICTION_PROPERTIES", false))
        return;

    LOG_INFO("Disabled all item restriction properties");

    static Hooks::Hook s_CheckItemClassRestrictionsHook = Hooks::HookFunction(&CNWSCreature::CheckItemClassRestrictions,
        +[](CNWSCreature*, CNWSItem*) -> bool
        {
            return true;
        }, Hooks::Order::Final);

    static Hooks::Hook s_CheckItemAlignmentRestrictionsHook = Hooks::HookFunction(&CNWSCreature::CheckItemAlignmentRestrictions,
        +[](CNWSCreature*, CNWSItem*) -> bool
        {
            return true;
        }, Hooks::Order::Final);

    static Hooks::Hook s_CheckItemRaceRestrictionsHook = Hooks::HookFunction(&CNWSCreature::CheckItemRaceRestrictions,
        +[](CNWSCreature*, CNWSItem*) -> bool
        {
            return true;
        }, Hooks::Order::Final);
}

NWNX_EXPORT ArgumentStack GetRulesetFloatEntry(ArgumentStack&& args)
{
    const auto sEntry = args.extract<std::string>();
      ASSERT_OR_THROW(!sEntry.empty());

    const auto fDefault = args.extract<float>();

    return Globals::Rules()->GetRulesetFloatEntry(CRULES_HASHEDSTR(sEntry.c_str()), fDefault);
}

NWNX_EXPORT ArgumentStack GetRulesetIntEntry(ArgumentStack&& args)
{
    const auto sEntry = args.extract<std::string>();
      ASSERT_OR_THROW(!sEntry.empty());

    const auto nDefault = args.extract<int32_t>();

    return Globals::Rules()->GetRulesetIntEntry(CRULES_HASHEDSTR(sEntry.c_str()), nDefault);
}

NWNX_EXPORT ArgumentStack GetRulesetStringEntry(ArgumentStack&& args)
{
    const auto sEntry = args.extract<std::string>();
      ASSERT_OR_THROW(!sEntry.empty());

    const auto sDefault = args.extract<std::string>();

    return Globals::Rules()->GetRulesetStringEntry(CRULES_HASHEDSTR(sEntry.c_str()), sDefault.c_str());
}

void TempestAmbidexterity() __attribute__((constructor));
void TempestAmbidexterity()
{
    if (!Config::Get<bool>("ENABLE_TEMPEST_AMBIDEXTERITY", false))
        return;

    LOG_INFO("Enabled Tempest Ambidexterity feats");

    static Hooks::Hook s_GetRulesetIntEntryHook =
        Hooks::HookFunction(&CNWRules::GetRulesetIntEntry,
        +[](CNWRules *pThis, uint64_t hashedLabel, int32_t whenMissing) -> int32_t
        {
            auto retval = s_GetRulesetIntEntryHook->CallOriginal<int32_t>(pThis, hashedLabel, whenMissing);

            if (hashedLabel == CRULES_HASHEDSTR("TWO_WEAPON_FIGHTING_BONUS") && s_TempestAmbidexterityModifier > 0)
                retval += s_TempestAmbidexterityModifier;

            return retval;
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetAttackModifierVersusHook = Hooks::HookFunction(&CNWSCreatureStats::GetAttackModifierVersus,
        +[](CNWSCreatureStats *pStats, CNWSCreature *pCreature) -> int32_t
        {
            if (!pStats->GetNumLevelsOfClass(CLASS_TYPE_TEMPEST))
                return s_GetAttackModifierVersusHook->CallOriginal<int32_t>(pStats, pCreature);

            if (auto *pArmor = pStats->m_pBaseCreature->m_pInventory->GetItemInSlot(Constants::EquipmentSlot::Chest))
            {
                if (pArmor->m_nArmorValue >= 6 && pArmor->m_nArmorValue != 9)
                    return s_GetAttackModifierVersusHook->CallOriginal<int32_t>(pStats, pCreature);
            }

            if (pStats->HasFeat(FEAT_TEMPEST_AMBIDEXTERITY_2))
                s_TempestAmbidexterityModifier = 2;
            else if (pStats->HasFeat(FEAT_TEMPEST_AMBIDEXTERITY_1))
                s_TempestAmbidexterityModifier = 1;
            
            auto retval = s_GetAttackModifierVersusHook->CallOriginal<int32_t>(pStats, pCreature);
            s_TempestAmbidexterityModifier = 0;

            return retval;
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetMeleeAttackBonusHook = Hooks::HookFunction(&CNWSCreatureStats::GetMeleeAttackBonus,
        +[](CNWSCreatureStats *pStats, int32_t bOffHand, int32_t bIncludeBase, int32_t bTouchAttack) -> int32_t
        {
            if (bTouchAttack || !pStats->GetNumLevelsOfClass(CLASS_TYPE_TEMPEST))
                return s_GetMeleeAttackBonusHook->CallOriginal<int32_t>(pStats, bOffHand, bIncludeBase, bTouchAttack);

            if (auto *pArmor = pStats->m_pBaseCreature->m_pInventory->GetItemInSlot(Constants::EquipmentSlot::Chest))
            {
                if (pArmor->m_nArmorValue >= 6 && pArmor->m_nArmorValue != 9)
                    return s_GetMeleeAttackBonusHook->CallOriginal<int32_t>(pStats, bOffHand, bIncludeBase, bTouchAttack);
            }

            if (pStats->HasFeat(FEAT_TEMPEST_AMBIDEXTERITY_2))
                s_TempestAmbidexterityModifier = 2;
            else if (pStats->HasFeat(FEAT_TEMPEST_AMBIDEXTERITY_1))
                s_TempestAmbidexterityModifier = 1;
            
            auto retval = s_GetMeleeAttackBonusHook->CallOriginal<int32_t>(pStats, bOffHand, bIncludeBase, bTouchAttack);
            s_TempestAmbidexterityModifier = 0;

            return retval;
        }, Hooks::Order::Late);

}

NWNX_EXPORT ArgumentStack UpdateCombatInformation(ArgumentStack&& args)
{
    if (auto *pCreature = Utils::PopCreature(args))
        pCreature->m_pStats->UpdateCombatInformation();

    return {};
}

NWNX_EXPORT ArgumentStack SetCreatureAge(ArgumentStack&& args)
{
    if (auto *pCreature = Utils::PopCreature(args))
        pCreature->m_pStats->m_nAge = args.extract<int32_t>();
    
    return {};
}

NWNX_EXPORT ArgumentStack SetUseBaseItemTypeUnequippedAllowed(ArgumentStack&& args)
{
    const auto nItemType = args.extract<int32_t>();
      ASSERT_OR_THROW(nItemType >= Constants::BaseItem::MIN);
      ASSERT_OR_THROW(nItemType <= Constants::BaseItem::MAX);

    auto bAllow = !!args.extract<int32_t>();

    CNWBaseItem *pBaseItem  = Globals::Rules()->m_pBaseItemArray->GetBaseItem(nItemType);
      ASSERT_OR_THROW(pBaseItem);

    LOG_INFO("Item type %s [%d] set to allow using spell properties while unequipped: %s", pBaseItem->GetNameText(), nItemType, bAllow ? "true" : "false");

    if (bAllow)
        m_BaseItemsAllowUseUnequipped.insert(nItemType);
    else
        m_BaseItemsAllowUseUnequipped.erase(nItemType);
    
    static Hooks::Hook s_UseItemHook =
        Hooks::HookFunction(&CNWSCreature::UseItem,
        +[](CNWSCreature *pThis, ObjectID oidItem, uint8_t nActivePropertyIndex, uint8_t nSubPropertyIndex, ObjectID oidTarget, Vector vTargetPosition, ObjectID oidArea, int32_t bUseCharges) -> int32_t
        {
            if (auto *pItem = Utils::AsNWSItem(Utils::GetGameObject(oidItem)))
            {
                if (m_BaseItemsAllowUseUnequipped.find(pItem->m_nBaseItem) != m_BaseItemsAllowUseUnequipped.end())
                {
                    s_InUseItemAllowUnequipped = true;
                    int32_t retval = s_UseItemHook->CallOriginal<int32_t>(pThis, oidItem, nActivePropertyIndex, nSubPropertyIndex, oidTarget, vTargetPosition, oidArea, bUseCharges);
                    s_InUseItemAllowUnequipped = false;

                    return retval;
                }
            }
            
            return s_UseItemHook->CallOriginal<int32_t>(pThis, oidItem, nActivePropertyIndex, nSubPropertyIndex, oidTarget, vTargetPosition, oidArea, bUseCharges);
        }, Hooks::Order::Late);

    static Hooks::Hook s_GetSlotFromItem =
        Hooks::HookFunction(&CNWSInventory::GetSlotFromItem,
        +[](CNWSInventory *pThis, CNWSItem *pItem) -> uint32_t
    {
        if (s_InUseItemAllowUnequipped)
            return 1;

        return s_GetSlotFromItem->CallOriginal<uint32_t>(pThis, pItem);
    }, Hooks::Order::Late);


    return {};
}

}