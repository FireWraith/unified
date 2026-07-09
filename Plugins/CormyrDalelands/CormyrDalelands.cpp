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
#include "API/CNWSObject.hpp"
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
#include "API/CNWSItemPropertyHandler.hpp"
#include "API/CNWItemProperty.hpp"
#include "API/CNWRules.hpp"
#include "API/CNWVisibilityNode.hpp"
#include "API/CNetLayer.hpp"
#include "API/CServerExoApp.hpp"
#include "API/CServerExoAppInternal.hpp"
#include "API/CVirtualMachine.hpp"
#include "API/CTlkTable.hpp"
#include "API/CWorldTimer.hpp"
#include "API/CTwoDimArrays.hpp"
#include "API/C2DA.hpp"

#include <set>
#include <map>
#include <dlfcn.h>

using namespace NWNXLib;
using namespace NWNXLib::API;

namespace CormyrDalelands {

// Spell quickening - spells in this set will cast in 3000ms instead of 6000ms
static std::set<uint32_t> m_QuickenedSpells;

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

static std::set<std::uint8_t> m_SmiteEvilProgressingClasses = {
    Constants::ClassType::Paladin,
    Constants::ClassType::DivineChampion
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
        +[](CNWSCreature *pThis, CNWSObject *pTarget, BOOL bAttackHit = true) -> BOOL
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
                            BOOL retVal = s_ResolveDefensiveEffectsHook->CallOriginal<BOOL>(pThis, pTarget, bAttackHit);
                            s_InResolveDefensiveEffectsWithGhostTouchWeapon = false;
                            return retVal;
                        }
                    }
                }
            }

            return s_ResolveDefensiveEffectsHook->CallOriginal<BOOL>(pThis, pTarget, bAttackHit);
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

// Shared resolution logic for the ResolveSneakAttack/ResolveDeathAttack hooks.
// bDeathAttack selects which feat set qualifies and which attack data flag is set.
static void ResolveSneakOrDeathAttack(CNWSCreature *pThis, CNWSCreature *pTarget, bool bDeathAttack)
{
    static const float SNEAK_ATTACK_DISTANCE = std::pow(
        Globals::Rules()->GetRulesetFloatEntry(CRULES_HASHEDSTR("MAX_RANGED_SNEAK_ATTACK_DISTANCE"), 10.0f), 2);

    if (!pTarget)
        return;

    const auto &attackFeats = bDeathAttack ? m_DeathAttackFeats : m_SneakAttackFeats;

    CNWSCombatAttackData *pAttackData = pThis->m_pcCombatRound->GetAttack(pThis->m_pcCombatRound->m_nCurrentAttack);

    if (pAttackData->m_nAttackType == Constants::Feat::WhirlwindAttack ||
        pAttackData->m_nAttackType == Constants::Feat::ImprovedWhirlwind)
    {
        return;
    }

    bool hasAttackFeat = false;
    for (const auto &i : attackFeats)
    {
        if (pThis->m_pStats->HasFeat(i))
        {
            hasAttackFeat = true;
            break;
        }
    }
    if (!hasAttackFeat)
        return;

    if (pAttackData->m_bRangedAttack)
    {
        float fDistance = Vector::MagnitudeSquared(pThis->m_vPosition - pTarget->m_vPosition);
        if (fDistance >= SNEAK_ATTACK_DISTANCE)
            return;
    }

    bool isAttackValid = false;

    auto *pVisNode = pTarget->GetVisibleListElement(pThis->m_idSelf);
    if (!pVisNode || !pVisNode->m_bSeen || pTarget->GetFlatFooted())
    {
        isAttackValid = true;
    }
    else if (pThis->GetFlanked(pTarget))
    {
        isAttackValid = true;

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

            isAttackValid = attackerLevels - defenderLevels >= Globals::Rules()->GetRulesetIntEntry(CRULES_HASHEDSTR("FLANK_LEVEL_RANGE"), 4);
        }
    }

    if (isAttackValid)
    {
        if (pTarget->m_pStats->GetEffectImmunity(Constants::ImmunityType::SneakAttack, pThis, true) || pTarget->m_pStats->GetEffectImmunity(Constants::ImmunityType::CriticalHit, pThis, true))
        {
            CNWCCMessageData *pData = new CNWCCMessageData;
            pData->SetObjectID(0, pTarget->m_idSelf);
            pData->SetInteger(0, 134);
            pAttackData->m_alstPendingFeedback.Add(pData);
        }
        else if (bDeathAttack)
        {
            pAttackData->m_bDeathAttack = 1;
        }
        else
        {
            pAttackData->m_bSneakAttack = 1;
        }
    }
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
            ResolveSneakOrDeathAttack(pThis, pTarget, false);
        }, Hooks::Order::Final);
    
    static Hooks::Hook s_ResolveDeathAttackHook =
        Hooks::HookFunction(&CNWSCreature::ResolveDeathAttack,
        +[](CNWSCreature *pThis, CNWSCreature *pTarget) -> void
        {
            ResolveSneakOrDeathAttack(pThis, pTarget, true);
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
                return s_GetDamageRollHook->CallOriginal<int32_t>(pThis, pTarget, bOffHand, bCritical, bSneakAttack, bDeathAttack, bForceMax);
 
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

            std::set<std::uint16_t> calculatedFeats;
            
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
                        calculatedFeats.insert(it.first);
                        retval += it.second;
                    }
                }
            }

            return retval;
        }, Hooks::Order::Late);

    return {};
}

NWNX_EXPORT ArgumentStack SetClassProgressesSmiteEvil(ArgumentStack&& args)
{
    const auto nClassId = args.extract<int32_t>();
      ASSERT_OR_THROW(nClassId >= Constants::ClassType::MIN);
      ASSERT_OR_THROW(nClassId <= Constants::ClassType::MAX);

    CNWClass *pClass = nClassId < Globals::Rules()->m_nNumClasses ? &Globals::Rules()->m_lstClasses[nClassId] : nullptr;
      ASSERT_OR_THROW(pClass != nullptr);

    //Add the class to the list of Smite Evil classes
    m_SmiteEvilProgressingClasses.insert(nClassId);
    LOG_INFO("Class %s [%d] set as Smite Evil progressing class", pClass->GetNameText(), nClassId); 

    static Hooks::Hook s_GetDamageRollHook =
        Hooks::HookFunction(&CNWSCreatureStats::ResolveSpecialAttackDamageBonus,
        +[](CNWSCreatureStats *thisPtr, CNWSCreature *pTarget) -> int32_t
        {
            // Get the creature using this stats block
            CNWSCreature* pThis = thisPtr->m_pBaseCreature;
            if (!pThis)
                return s_GetDamageRollHook->CallOriginal<int32_t>(thisPtr, pTarget);

            CNWSCombatAttackData *pAttackData = pThis->m_pcCombatRound->GetAttack(pThis->m_pcCombatRound->m_nCurrentAttack);

            // Early exit if not a smite evil attack
            if (!pAttackData || pAttackData->m_nAttackType != Constants::Feat::SmiteEvil || !pTarget || pTarget->m_pStats->m_nAlignmentGoodEvil > 30)
                return s_GetDamageRollHook->CallOriginal<int32_t>(thisPtr, pTarget);

            // Calculate Epic Great Smiting rank - pre-hashed for performance
            int nSmiteRank = 1;
            auto nBaseFeat = Constants::Feat::EpicGreatSmiting1 - 1;
            
            static const uint64_t epicSmitingHashes[] = {
                0, // index 0 unused
                CRULES_HASHEDSTR("EPIC_GREAT_SMITING_1"), CRULES_HASHEDSTR("EPIC_GREAT_SMITING_2"), 
                CRULES_HASHEDSTR("EPIC_GREAT_SMITING_3"), CRULES_HASHEDSTR("EPIC_GREAT_SMITING_4"),
                CRULES_HASHEDSTR("EPIC_GREAT_SMITING_5"), CRULES_HASHEDSTR("EPIC_GREAT_SMITING_6"), 
                CRULES_HASHEDSTR("EPIC_GREAT_SMITING_7"), CRULES_HASHEDSTR("EPIC_GREAT_SMITING_8"),
                CRULES_HASHEDSTR("EPIC_GREAT_SMITING_9"), CRULES_HASHEDSTR("EPIC_GREAT_SMITING_10")
            };
            
            for (int i = 10; i > 0; --i)
            {
                if (thisPtr->HasFeat(nBaseFeat + i))
                {
                    nSmiteRank = Globals::Rules()->GetRulesetIntEntry(epicSmitingHashes[i], i);
                    break;
                }
            }

            // Sum levels from all configured smite evil progressing classes
            int nTotalLevels = 0;
            for (const auto &classId : m_SmiteEvilProgressingClasses)
                nTotalLevels += thisPtr->GetNumLevelsOfClass(classId);

            int nTotalDamage = nTotalLevels * nSmiteRank;
            return nTotalDamage;
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
                                    return;
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
                                    return;
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
                    
                    bool bFeatUsesFound = false;
                    for (int32_t i = 0; i < pThis->m_lstFeatUses.num; i++)
                    {
                        if (pThis->m_lstFeatUses.element[i]->m_nFeat == nFeat)
                        {
                            bFeatUsesFound = true;
                            break;
                        }
                    }
                    if (!bFeatUsesFound)
                        return 100;

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

                /* 
                //Commented out to make the stance infinite
                auto nNumUses = pThis->GetFeatTotalUses(nFeat);

                for (int32_t i = 0; i < pThis->m_lstFeatUses.num; i++)
                {
                    auto *pFeatUses = pThis->m_lstFeatUses.element[i];
                    if (pFeatUses->m_nFeat == nFeat)
                    {
                        nNumUses -= pFeatUses->m_nUsedToday;
                        break;
                    }
                }*/

                //return std::clamp<uint8_t>(nNumUses, 0, 100);
                return 100;
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
                    
                    bool bFeatUsesFound = false;
                    for (int32_t i = 0; i < pThis->m_lstFeatUses.num; i++)
                    {
                        if (pThis->m_lstFeatUses.element[i]->m_nFeat == nFeat)
                        {
                            bFeatUsesFound = true;
                            break;
                        }
                    }
                    if (!bFeatUsesFound)
                        return 100;

                    auto nBardSongClassLevels = 0;
                    for (const auto &it : m_BardSongUsesProgressingClasses)
                        nBardSongClassLevels += pThis->GetNumLevelsOfClass(it);

                    auto nNumUses = pFeat->m_nUsesPerDay + nBardSongClassLevels;
                    
                    // We combine Extra Music into Lingering Song
                    if (s_LingeringSongExtraMusic && (pThis->HasFeat(Constants::Feat::LingeringSong) || pThis->HasFeat(Constants::Feat::ExtraMusic)))
                    {
                        if (s_BardSongExtraMusicByCharismaModifier)
                            nNumUses += std::clamp<int>(pThis->m_nCharismaModifier, 0, nBardSongClassLevels);
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

                int nNumUses = pThis->GetFeatTotalUses(nFeat);

                for (int32_t i = 0; i < pThis->m_lstFeatUses.num; i++)
                {
                    auto *pFeatUses = pThis->m_lstFeatUses.element[i];
                    if (pFeatUses->m_nFeat == nFeat)
                    {
                        nNumUses -= pFeatUses->m_nUsedToday;
                        break;
                    }
                }

                return static_cast<uint8_t>(std::clamp(nNumUses, 0, 100));
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
            // Commented out the immobility effect since we now let people slowly walk with the stance active
            //auto *pSetAIState = new CGameEffect(pEffect, false);
            //pSetAIState->m_nType = Constants::EffectTrueType::SetAIState;
            //pSetAIState->SetInteger(0, -3);
            //pObject->ApplyEffect(pSetAIState, bLoadingGame, false);

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
                            (pEffect->GetInteger(5) != 4103 && (pEffect->GetInteger(5) & pCreature->GetDamageFlags()) == 0))
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

NWNX_EXPORT ArgumentStack SetSpellAutoQuicken(ArgumentStack&& args)
{
    const auto nSpellId = args.extract<int32_t>();
      ASSERT_OR_THROW(nSpellId >= 0);

    // Add spell ID to the list of auto quickened spells
    m_QuickenedSpells.insert(static_cast<uint32_t>(nSpellId));
    LOG_INFO("Spell %d set to auto quicken", nSpellId);

    // Hook AIActionCastSpell to get spell ID
    static Hooks::Hook s_AIActionCastSpellHook = Hooks::HookFunction(&CNWSCreature::AIActionCastSpell,
        +[](CNWSCreature *pCreature, CNWSObjectActionNode *pNode) -> uint32_t
        {
            pCreature->nwnxSet("QUICKEN_CURRENT_SPELL", static_cast<int>(pNode->m_pParameter[0]));

            return s_AIActionCastSpellHook->CallOriginal<uint32_t>(pCreature, pNode);
        }, Hooks::Order::Early);

    static Hooks::Hook s_StartCombatRoundCastHook = Hooks::HookFunction(&CNWSCombatRound::StartCombatRoundCast,
        +[](CNWSCombatRound *pThis, uint32_t nRoundLength) -> void
        {
            // Check if the current spell should be quickened
            if (auto spellId = pThis->m_pBaseCreature->nwnxGet<int>("QUICKEN_CURRENT_SPELL"))
            {
                if (m_QuickenedSpells.find(*spellId) != m_QuickenedSpells.end())
                    nRoundLength = 3000;
                pThis->m_pBaseCreature->nwnxRemove("QUICKEN_CURRENT_SPELL");
            }

            s_StartCombatRoundCastHook->CallOriginal<void>(pThis, nRoundLength);
        }, Hooks::Order::Late);

    return {};
}

void CustomHolyAvengerProperty() __attribute__((constructor));
void CustomHolyAvengerProperty()
{
    if (!Config::Get<bool>("ENABLE_CUSTOM_HOLY_AVENGER", false))
        return;

    LOG_INFO("Custom Holy Avenger property enabled");

    static Hooks::Hook s_ApplyHolyAvengerHook = Hooks::HookFunction(&CNWSItemPropertyHandler::ApplyHolyAvenger,
        +[](CNWSItemPropertyHandler *pThis, CNWSItem *pItem, CNWItemProperty* /*pItemProperty*/, CNWSCreature *pCreature, uint32_t nInventorySlot, BOOL bLoadingGame) -> int32_t
        {
            if (!pCreature || !pItem || !pCreature->m_pStats)
                return 0;

            auto *pStats = pCreature->m_pStats;
            auto nCharacterLevel = pStats->GetLevel(false);
            auto nItemID = pItem->m_idSelf;
            auto nBlackguardLevels = pStats->GetNumLevelsOfClass(Constants::ClassType::Blackguard);
            auto nPaladinLevels = pStats->GetNumLevelsOfClass(Constants::ClassType::Paladin);
            bool bIsUnholyAvenger = (nBlackguardLevels >= 10);
            bool bIsHolyAvenger = (nPaladinLevels >= 1);
            
            // Enhancement bonus requires any level of Blackguard or Paladin
            if (nBlackguardLevels < 1 && nPaladinLevels < 1)
                return 0; // Neither Blackguard nor Paladin - do not apply

            // 1. Level-based Enhancement Bonus (any Blackguard or Paladin level)
            int32_t nEnhancementBonus = (nCharacterLevel >= 25) ? 6 : 5;
            auto *pEnhancementProperty = new CNWItemProperty();
            pEnhancementProperty->m_nPropertyName = Constants::ItemProperty::EnhancementBonus;
            pEnhancementProperty->m_nSubType = 0;
            pEnhancementProperty->m_nCostTable = 0;
            pEnhancementProperty->m_nCostTableValue = nEnhancementBonus;
            pEnhancementProperty->m_nParam1 = 0;
            pEnhancementProperty->m_nParam1Value = 0;
            pEnhancementProperty->m_nChanceOfAppearing = 100;
            pEnhancementProperty->m_bUseable = true;
            pEnhancementProperty->m_nUsesPerDay = -1; // Unlimited uses

            // Apply the enhancement bonus
            pThis->ApplyEnhancementBonus(pItem, pEnhancementProperty, pCreature, nInventorySlot, bLoadingGame);
            delete pEnhancementProperty;

            // 2. Scaling Spell Resistance (10+ Blackguard OR any Paladin level)
            if (bIsUnholyAvenger || bIsHolyAvenger)
            {
                auto *pEffect = new CGameEffect(true);
                pEffect->m_nType = Constants::EffectTrueType::SpellResistanceIncrease;
                pEffect->SetDurationType(Constants::EffectDurationType::Equipped);
                pEffect->SetCreator(nItemID);
                int32_t nSpellResistance = 8 + nCharacterLevel;
                pEffect->SetInteger(0, nSpellResistance);
                pCreature->ApplyEffect(pEffect, bLoadingGame, false);
            }

            // 3. Damage Bonus (10+ Blackguard OR any Paladin level)
            // Blackguard (10+ levels): +1d6 Vile vs all targets
            // Paladin: +1d6 Radiant vs Evil creatures
            if (bIsUnholyAvenger || bIsHolyAvenger)
            {
                auto *pDamageBonusProperty = new CNWItemProperty();
                if (bIsUnholyAvenger)
                {
                    pDamageBonusProperty->m_nPropertyName = Constants::ItemProperty::DamageBonus;
                    pDamageBonusProperty->m_nSubType = 23; // Vile (23) from iprp_damagetypes.2da
                    pDamageBonusProperty->m_nCostTable = 4; // Points to IPRP_DAMAGECOST table
                    pDamageBonusProperty->m_nCostTableValue = 7; // 1d6 damage
                    pDamageBonusProperty->m_nChanceOfAppearing = 100;
                    pDamageBonusProperty->m_bUseable = true;
                    pDamageBonusProperty->m_nUsesPerDay = -1; // Unlimited uses
                }
                else
                {
                    pDamageBonusProperty->m_nPropertyName = Constants::ItemProperty::DamageBonusVSAlignmentGroup;
                    pDamageBonusProperty->m_nSubType = Constants::Alignment::Evil;
                    pDamageBonusProperty->m_nCostTable = 4; // Points to IPRP_DAMAGECOST table
                    pDamageBonusProperty->m_nCostTableValue = 7; // 1d6 damage
                    pDamageBonusProperty->m_nParam1 = -1; // Damage type table index
                    pDamageBonusProperty->m_nParam1Value = 21; // Radiant (21) from iprp_damagetypes.2da
                    pDamageBonusProperty->m_nChanceOfAppearing = 100;
                    pDamageBonusProperty->m_bUseable = true;
                    pDamageBonusProperty->m_nUsesPerDay = -1; // Unlimited uses     
                }

                // Apply the damage bonus
                pThis->ApplyDamageBonus(pItem, pDamageBonusProperty, pCreature, nInventorySlot, bLoadingGame);
                delete pDamageBonusProperty;
            }

            return 1; // Success - don't call original
        }, Hooks::Order::Final);
}

// Fix for prestige class spell progression
// This hooks GetSpellsKnownPerLevel to consider prestige class caster levels
// when determining how many spells a character should know, preventing spell
// removal during character loading.

static Hooks::Hook s_GetSpellsKnownPerLevelHook;
static Hooks::Hook s_ReadSpellsFromGffHook;
static CNWSCreatureStats* s_pCurrentCreatureStats = nullptr;

// Forward declarations
static uint8_t GetSpellsKnownPerLevelHook(CNWClass* pClass, uint8_t nLevel, uint8_t nSpellLevel, uint8_t nClass, uint16_t nRace, uint8_t nCastingAbilityBase);
static void ReadSpellsFromGffHook(CNWSCreatureStats* pCreatureStats, CResGFF* pRes, CResStruct* pGffStructWithCreatureStats, BOOL bDefaultUnsavedSpellsAsReadied);

void FixPrestigeClassSpellProgression() __attribute__((constructor));
void FixPrestigeClassSpellProgression()
{
    LOG_INFO("Spontaneous caster PRC GetSpellsKnownPerLevel spell progression override enabled");

    s_GetSpellsKnownPerLevelHook = Hooks::HookFunction(
        &CNWClass::GetSpellsKnownPerLevel,
        &GetSpellsKnownPerLevelHook, Hooks::Order::Early);

    // Hook ReadSpellsFromGff - this is where spells are loaded and validated from character files
    s_ReadSpellsFromGffHook = Hooks::HookFunction(
        &CNWSCreatureStats::ReadSpellsFromGff,
        &ReadSpellsFromGffHook, Hooks::Order::Early);
}

static void ReadSpellsFromGffHook(CNWSCreatureStats* pCreatureStats, CResGFF* pRes, CResStruct* pGffStructWithCreatureStats, BOOL bDefaultUnsavedSpellsAsReadied)
{
    // Set creature context for spell validation during spell reading
    s_pCurrentCreatureStats = pCreatureStats;
    
    s_ReadSpellsFromGffHook->CallOriginal<void>(pCreatureStats, pRes, pGffStructWithCreatureStats, bDefaultUnsavedSpellsAsReadied);
    
    // Clear context after spell reading
    s_pCurrentCreatureStats = nullptr;
}

// Helper function to get prestige class caster levels for a specific class
static int32_t GetPrestigeClassCasterLevels(uint8_t nClassId)
{
    if (!s_pCurrentCreatureStats)
        return 0;

    auto* pStats = s_pCurrentCreatureStats;
    auto* pRules = Globals::Rules();
    auto* p2DA = pRules->m_p2DArrays->GetCached2DA("classes", true);
    
    if (!p2DA)
        return 0;

    p2DA->Load2DArray();

    // Get the caster type (arcane/divine) of the target class
    int spellCaster, arcane;
    if (!p2DA->GetINTEntry(nClassId, "SpellCaster", &spellCaster) || !spellCaster)
        return 0;
    
    if (!p2DA->GetINTEntry(nClassId, "Arcane", &arcane))
        return 0;

    bool bIsArcane = arcane != 0;
    const char* modColumn = bIsArcane ? "ArcSpellLvlMod" : "DivSpellLvlMod";

    int32_t nBonusLevels = 0;
    
    // Look through all the character's classes for prestige classes that advance this caster type
    for (int i = 0; i < pStats->m_nNumMultiClasses; i++)
    {
        auto nCurrentClassId = pStats->m_ClassInfo[i].m_nClass;
        auto nClassLevel = pStats->m_ClassInfo[i].m_nLevel;
        
        if (nCurrentClassId == nClassId) // Skip the target class itself
            continue;

        int value;
        if (p2DA->GetINTEntry(nCurrentClassId, modColumn, &value))
        {
            if (value > 0 && nClassLevel > 0)
            {
                int bonus = (nClassLevel - 1) / value + 1;
                nBonusLevels += bonus;
            }
        }
    }
    
    return nBonusLevels;
}

static uint8_t GetSpellsKnownPerLevelHook(CNWClass* pClass, uint8_t nLevel, uint8_t nSpellLevel, uint8_t nClass, uint16_t nRace, uint8_t nCastingAbilityBase)
{
    auto retVal = s_GetSpellsKnownPerLevelHook->CallOriginal<uint8_t>(pClass, nLevel, nSpellLevel, nClass, nRace, nCastingAbilityBase);
    
    // If we have creature context, always check for prestige class bonuses
    if (s_pCurrentCreatureStats)
    {
        int32_t nPrestigeLevels = GetPrestigeClassCasterLevels(nClass);
        
        if (nPrestigeLevels > 0)
        {
            // Try with the effective caster level (base + prestige bonuses)
            uint8_t nEffectiveLevel = std::min(255, static_cast<int>(nLevel) + nPrestigeLevels);
            auto nPrestigeRetVal = s_GetSpellsKnownPerLevelHook->CallOriginal<uint8_t>(pClass, nEffectiveLevel, nSpellLevel, nClass, nRace, nCastingAbilityBase);
            
            if (nPrestigeRetVal > retVal)
            {
                LOG_DEBUG("Prestige class spell progression: Class %d L%d (+%d) spell level %d increased from %d to %d", 
                         nClass, nLevel, nPrestigeLevels, nSpellLevel, retVal, nPrestigeRetVal);
                retVal = nPrestigeRetVal;
            }
        }
    }

    return retVal;
}

// ==================== Familiar Level Scaling for Prestige Classes ====================
// Allows arcane prestige classes to contribute their levels to familiar level calculation

// Class IDs for arcane prestige classes that should contribute to familiar level
static const std::set<uint8_t> s_FamiliarLevelProgressingClasses = {
    51,  // Archmage
    60,  // Arcane Trickster
    80,  // Arcane Warrior
    55,  // Eldritch Knight
    63,  // Harper Mage
    64,  // Shadow Adept
    59,  // Mystic Theurge
    65,  // Red Wizard
    67,  // War Wizard
    52,  // Bladesinger
    34,  // Palemaster
    72,  // Demonic Servitor
    62,  // Dragonsong Lyrist
    90,  // Draconic Inheritor
    44,  // Wild Mage
};

static Hooks::Hook s_SummonFamiliarHook;

static void SummonFamiliarHook(CNWSCreature* pCreature);

void FamiliarLevelScaling() __attribute__((constructor));
void FamiliarLevelScaling()
{
    LOG_INFO("Familiar level scaling for arcane prestige classes enabled");

    s_SummonFamiliarHook = Hooks::HookFunction(
        &CNWSCreature::SummonFamiliar,
        &SummonFamiliarHook, Hooks::Order::Early);
}

static void SummonFamiliarHook(CNWSCreature* pCreature)
{
    auto* pStats = pCreature->m_pStats;
    if (!pStats)
    {
        s_SummonFamiliarHook->CallOriginal<void>(pCreature);
        return;
    }

    // Calculate bonus levels from arcane prestige classes
    int32_t nBonusLevels = 0;
    for (int i = 0; i < pStats->m_nNumMultiClasses; i++)
    {
        auto nClassId = pStats->m_ClassInfo[i].m_nClass;
        auto nClassLevel = pStats->m_ClassInfo[i].m_nLevel;

        if (s_FamiliarLevelProgressingClasses.count(nClassId))
        {
            nBonusLevels += nClassLevel;
        }
    }

    if (nBonusLevels > 0)
    {
        // Temporarily boost wizard or sorcerer level to affect familiar level
        // The game uses SpellCaster classes with MinAssociateLevel to calculate familiar level
        // We'll temporarily add levels to the first wizard/sorcerer class we find
        int nWizardIdx = -1;
        int nSorcererIdx = -1;
        
        for (int i = 0; i < pStats->m_nNumMultiClasses; i++)
        {
            auto nClassId = pStats->m_ClassInfo[i].m_nClass;
            if (nClassId == Constants::ClassType::Wizard && nWizardIdx < 0)
                nWizardIdx = i;
            else if (nClassId == Constants::ClassType::Sorcerer && nSorcererIdx < 0)
                nSorcererIdx = i;
        }

        int nTargetIdx = (nWizardIdx >= 0) ? nWizardIdx : nSorcererIdx;
        
        if (nTargetIdx >= 0)
        {
            uint8_t nOriginalLevel = pStats->m_ClassInfo[nTargetIdx].m_nLevel;
            uint8_t nBoostedLevel = static_cast<uint8_t>(std::min(255, static_cast<int>(nOriginalLevel) + nBonusLevels));
            
            pStats->m_ClassInfo[nTargetIdx].m_nLevel = nBoostedLevel;
            s_SummonFamiliarHook->CallOriginal<void>(pCreature);
            pStats->m_ClassInfo[nTargetIdx].m_nLevel = nOriginalLevel;
            return;
        }
    }

    s_SummonFamiliarHook->CallOriginal<void>(pCreature);
}

// ==================== Animal Companion Level Scaling ====================
// If Druid + Ranger levels are at least half the character's total level,
// all class levels contribute to animal companion level calculation

static Hooks::Hook s_SummonAnimalCompanionHook;

static void SummonAnimalCompanionHook(CNWSCreature* pCreature);

void AnimalCompanionLevelScaling() __attribute__((constructor));
void AnimalCompanionLevelScaling()
{
    LOG_INFO("Animal companion level scaling enabled (requires Druid+Ranger >= half total level)");

    s_SummonAnimalCompanionHook = Hooks::HookFunction(
        &CNWSCreature::SummonAnimalCompanion,
        &SummonAnimalCompanionHook, Hooks::Order::Early);
}

static void SummonAnimalCompanionHook(CNWSCreature* pCreature)
{
    auto* pStats = pCreature->m_pStats;
    if (!pStats)
    {
        s_SummonAnimalCompanionHook->CallOriginal<void>(pCreature);
        return;
    }

    // Calculate total level, druid+ranger levels, and other class levels
    int32_t nTotalLevel = 0;
    int32_t nDruidRangerLevels = 0;
    int32_t nOtherLevels = 0;
    int nDruidIdx = -1;
    int nRangerIdx = -1;

    for (int i = 0; i < pStats->m_nNumMultiClasses; i++)
    {
        auto nClassId = pStats->m_ClassInfo[i].m_nClass;
        auto nClassLevel = pStats->m_ClassInfo[i].m_nLevel;
        
        nTotalLevel += nClassLevel;

        if (nClassId == Constants::ClassType::Druid)
        {
            nDruidRangerLevels += nClassLevel;
            if (nDruidIdx < 0)
                nDruidIdx = i;
        }
        else if (nClassId == Constants::ClassType::Ranger)
        {
            nDruidRangerLevels += nClassLevel;
            if (nRangerIdx < 0)
                nRangerIdx = i;
        }
        else
        {
            nOtherLevels += nClassLevel;
        }
    }

    // Check if Druid + Ranger is at least half the total level
    // and there are other levels to add
    if (nDruidRangerLevels >= (nTotalLevel / 2) && nOtherLevels > 0)
    {
        // Find the target class to boost (prefer Druid over Ranger)
        int nTargetIdx = (nDruidIdx >= 0) ? nDruidIdx : nRangerIdx;

        if (nTargetIdx >= 0)
        {
            uint8_t nOriginalLevel = pStats->m_ClassInfo[nTargetIdx].m_nLevel;
            uint8_t nBoostedLevel = static_cast<uint8_t>(std::min(255, static_cast<int>(nOriginalLevel) + nOtherLevels));

            LOG_INFO("Animal companion level scaling: Boosting class %d from level %d to %d (bonus: %d, Druid+Ranger: %d, Total: %d)",
                     pStats->m_ClassInfo[nTargetIdx].m_nClass, nOriginalLevel, nBoostedLevel, nOtherLevels, nDruidRangerLevels, nTotalLevel);

            pStats->m_ClassInfo[nTargetIdx].m_nLevel = nBoostedLevel;
            s_SummonAnimalCompanionHook->CallOriginal<void>(pCreature);
            pStats->m_ClassInfo[nTargetIdx].m_nLevel = nOriginalLevel;
            return;
        }
    }

    s_SummonAnimalCompanionHook->CallOriginal<void>(pCreature);
}

// ============================================================================
// Network Decompression Stall Prevention
// ============================================================================
// Hooks CNetLayerInternal::UncompressMessage to rate-limit failed decompression
// attempts. A corrupted network packet can contain many sub-messages that each
// fail decompression, and each failure triggers expensive ExoLog::Emit calls
// (string formatting + I/O). This can block the main loop for minutes, causing
// Watchcat to kill the server.
//
// The fix: after MAX_DECOMPRESS_FAILURES_PER_TICK consecutive failures for a
// given player, skip remaining calls and return 0 (failure) immediately,
// avoiding the expensive logging in the original function.
// ============================================================================

static constexpr int MAX_DECOMPRESS_FAILURES_PER_TICK = 10;
static uint32_t s_DecompressFailPlayerId = 0;
static int s_DecompressFailCount = 0;

using UncompressMessageFunc = int32_t (*)(void* /*CNetLayerInternal*/, uint32_t /*playerId*/, uint8_t* /*data*/, uint32_t /*size*/);
static Hooks::Hook s_UncompressMessageHook;

static int32_t UncompressMessageHookFunc(void* pThis, uint32_t nPlayerId, uint8_t* pData, uint32_t dwSize)
{
    // Different player - reset the counter. This scopes suppression to a single
    // burst of bad sub-messages within one frame for one player.
    if (nPlayerId != s_DecompressFailPlayerId)
    {
        s_DecompressFailPlayerId = nPlayerId;
        s_DecompressFailCount = 0;
    }

    // If we've already hit the failure threshold for this player, skip the
    // call entirely. This prevents the logging storm that causes the stall.
    if (s_DecompressFailCount >= MAX_DECOMPRESS_FAILURES_PER_TICK)
    {
        return 0;
    }

    int32_t result = s_UncompressMessageHook->CallOriginal<int32_t>(pThis, nPlayerId, pData, dwSize);

    if (result == 0)
    {
        s_DecompressFailCount++;

        if (s_DecompressFailCount == MAX_DECOMPRESS_FAILURES_PER_TICK)
        {
            LOG_ERROR("Network decompression stall prevention: %d consecutive failures for player %u - disconnecting",
                        s_DecompressFailCount, nPlayerId);

            // Queue the disconnect on the main thread since we're in the network layer.
            // nStrRef 5838 = generic "has been disconnected" message.
            uint32_t playerId = nPlayerId;
            Tasks::QueueOnMainThread([playerId]()
            {
                auto* pNetLayer = Globals::AppManager()->m_pServerExoApp->GetNetLayer();
                if (pNetLayer)
                {
                    pNetLayer->DisconnectPlayer(playerId, 5838, false,
                        "Disconnected: corrupt network data detected. Please relog.");
                }
            });
        }
    }
    else
    {
        // Success - reset the counter
        s_DecompressFailCount = 0;
    }

    return result;
}

void NetworkDecompressionStallPrevention() __attribute__((constructor));
void NetworkDecompressionStallPrevention()
{
    if (!Config::Get<bool>("ENABLE_NETWORK_STALL_PREVENTION", true))
        return;

    // CNetLayerInternal::UncompressMessage is not in the NWNX typed API bindings,
    // so we resolve it by its mangled symbol name.
    void* pUncompressMessage = dlsym(RTLD_DEFAULT, "_ZN17CNetLayerInternal17UncompressMessageEjPhj");
    if (!pUncompressMessage)
    {
        LOG_ERROR("NetworkDecompressionStallPrevention: Failed to find CNetLayerInternal::UncompressMessage symbol");
        return;
    }

    LOG_INFO("Network decompression stall prevention enabled (max %d consecutive failures before suppression)",
             MAX_DECOMPRESS_FAILURES_PER_TICK);

    s_UncompressMessageHook = Hooks::HookFunction(
        pUncompressMessage,
        (void*)&UncompressMessageHookFunc,
        Hooks::Order::Earliest);
}

}