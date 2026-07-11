/// @addtogroup cormyrdalelands CormyrDalelands
/// @brief Cormyr and the Dalelands NWNX plugin
/// @file nwnx_cormyrdales.nss

const string NWNX_CormyrDalelands = "NWNX_CormyrDalelands"; ///< @private

/// @brief Gets whether a creature is incorporeal for use with ghost touch weapons.
/// @param oCreature The creature.
/// @return Whether a creature is incorporeal.
int NWNX_CormyrDalelands_GetCreatureIncorporealFlag(object oCreature);

/// @brief Sets a creature as incorporeal for use with ghost touch weapons.
/// @param oCreature The creature.
/// @param bIsIncorporeal Whether the creature is incorporeal.
void NWNX_CormyrDalelands_SetCreatureIncorporealFlag(object oCreature, int bIsIncorporeal);

/// @brief Sets a class as Uncanny Dodge II class for sneak/death attacks.
/// @param nClassID The class.
/// @param bIsUncannyDodgeClass Whether the class is considered an uncanny dodge class or not.
void NWNX_CormyrDalelands_SetClassIsSneakAttackUncannyDodgeClass(int nClassID, int bIsUncannyDodgeClass = TRUE);

/// @brief Set a feat as sneak attack feat that individually adds 1d6 sneak attack damage.
/// @param nFeat The feat.
/// @param bIsSneakAttackFeat Whether the feat is considered a sneak attack or not.
void NWNX_CormyrDalelands_SetFeatIsSneakAttackFeat(int nFeat, int bIsSneakAttackFeat = TRUE);

/// @brief Set a feat as death attack feat that individually adds 1d6 death attack damage.
/// @param nFeat The feat.
/// @param bIsDeathAttackFeat Whether the feat is considered a death attack or not.
void NWNX_CormyrDalelands_SetFeatIsDeathAttackFeat(int nFeat, int bIsDeathAttackFeat = TRUE);

/// @brief Sets a feat to modify creature's natural base AC by nModifier amount.
/// @param nFeatID The feat.
/// @param nModifier AC modifier.
void NWNX_CormyrDalelands_SetNaturalBaseACModifierFeat(int nFeatID, int nModifier);

/// @brief Sets a class to add its class levels to bard song's uses per day.
/// @param nClassID The class.
void NWNX_CormyrDalelands_SetClassProgressesBardSongUses(int nClassID, int bProgressesUses = TRUE);

/// @brief Retrieves a ruleset.2da entry.
/// @param sEntry The ruleset entry name.
/// @param fDefault Default value to return if sEntry is missing.
/// @return sEntry's value or fDefault.
float NWNX_CormyrDalelands_GetRulesetFloatEntry(string sEntry, float fDefault);

/// @brief Retrieves a ruleset.2da entry.
/// @param sEntry The ruleset entry name.
/// @param nDefault Default value to return if sEntry is missing.
/// @return sEntry's value or nDefault.
int NWNX_CormyrDalelands_GetRulesetIntEntry(string sEntry, int nDefault);

/// @brief Retrieves a ruleset.2da entry.
/// @param sEntry The ruleset entry name.
/// @param sDefault Default value to return if sEntry is missing.
/// @return sEntry's value or sDefault.
string NWNX_CormyrDalelands_GetRulesetStringEntry(string sEntry, string sDefault);

/// @brief Updates oCreature's combat information.
/// @param oCreature The creature whose's combat information to update.
void NWNX_CormyrDalelands_UpdateCombatInformation(object oCreature);

/// @brief Updates oCreature's age.
/// @param oCreature The creature whose age to edit.
/// @param nAge The new age to set.
void NWNX_CormyrDalelands_SetCreatureAge(object oCreature, int nAge);

/// @brief Sets whether nBaseItemType is allowed to be used while unequipped.
/// @param nBaseItemType The BASE_ITEM_* type.
/// @param bAllow Whether to allow the item type to be useable while unequipped.
void NWNX_CormyrDalelands_SetUseBaseItemTypeUnequippedAllowed(int nBaseItemType, int bAllow);

/// @brief Set whether a spell should be quickened (3 second cast time)
/// @param nSpellId The spell ID to modify
void NWNX_CormyrDalelands_SetSpellQuicken(int nSpellId);

/// @brief Sets a class to add its class levels to smite evil damage calculation.
/// @brief Compatible with Great Smiting ruleset entries
/// @param nClassID The class.
void NWNX_CormyrDalelands_SetClassProgressesSmiteEvil(int nClassID);

/// @brief Result of NWNX_CormyrDalelands_ResolveWeaponStrike().
struct NWNX_CormyrDalelands_WeaponStrike
{
    /// 1=hit, 2=parried, 3=critical hit, 4=miss, 5=resisted, 7=automatic hit,
    /// 8=concealed, 9=miss chance, 10=devastating crit. 0 on error.
    int nAttackResult;
    /// Total damage dealt on a hit, before the target's resistances/immunities.
    int nTotalDamage;
    int nToHitRoll;   ///< The d20 attack roll.
    int nToHitMod;    ///< The attack modifier used against the target.
    int bSneakAttack; ///< TRUE if the strike was a sneak attack.
    int bKillingBlow; ///< TRUE if the strike killed the target.
};

/// @brief Resolves one melee attack (at best attack bonus) against oTarget using the
/// engine's full combat resolution. Attack roll, damage, crits, sneak attack, feedback
/// and on-hit effects all behave like a real attack.
/// @warning Do not call from NWNX attack/damage event scripts (e.g. cdx_d_onattack /
/// cdx_d_ondamage): those run inside the engine's own attack resolution.
/// @param oAttacker The attacking creature.
/// @param oTarget The target (creature, door or placeable).
/// @param nStrModOverride Substitute STR modifier for attack and damage rolls. 255 = use real STR.
/// @param nAttackBonusMod Flat attack bonus modifier for this strike.
/// @param bOffHand TRUE = strike with the off-hand weapon instead of the main hand.
/// @param bAutoHit TRUE = a miss/parry/concealment result becomes an automatic hit.
/// @param nCritOverride 0 = normal, 1 = never crit, 2 = hits become critical hits.
/// @param nSneakOverride 0 = normal, 1 = never sneak, 2 = force sneak (bypasses the engine's own eligibility checks).
/// @param nBonusDamageType DAMAGE_TYPE_* for extra damage on hit (0 = none).
/// @param nBonusDamage Extra damage amount; roll any dice script-side.
/// @return The attack result, see NWNX_CormyrDalelands_WeaponStrike.
struct NWNX_CormyrDalelands_WeaponStrike NWNX_CormyrDalelands_ResolveWeaponStrike(object oAttacker, object oTarget, int nStrModOverride = 255, int nAttackBonusMod = 0, int bOffHand = FALSE, int bAutoHit = FALSE, int nCritOverride = 0, int nSneakOverride = 0, int nBonusDamageType = 0, int nBonusDamage = 0);

int NWNX_CormyrDalelands_GetCreatureIncorporealFlag(object oCreature)
{
    NWNXPushObject(oCreature);
    NWNXCall(NWNX_CormyrDalelands, "GetCreatureIncorporealFlag");

    return NWNXPopInt();
}

void NWNX_CormyrDalelands_SetCreatureIncorporealFlag(object oCreature, int bIsIncorporeal)
{
    NWNXPushInt(bIsIncorporeal);
    NWNXPushObject(oCreature);
    NWNXCall(NWNX_CormyrDalelands, "SetCreatureIncorporealFlag");
}

void NWNX_CormyrDalelands_SetClassIsSneakAttackUncannyDodgeClass(int nClassID, int bIsUncannyDodgeClass = TRUE)
{
    NWNXPushInt(bIsUncannyDodgeClass);
    NWNXPushInt(nClassID);
    NWNXCall(NWNX_CormyrDalelands, "SetClassIsSneakAttackUncannyDodgeClass");
}

void NWNX_CormyrDalelands_SetFeatIsSneakAttackFeat(int nFeat, int bIsSneakAttackFeat = TRUE)
{
    NWNXPushInt(bIsSneakAttackFeat);
    NWNXPushInt(nFeat);
    NWNXCall(NWNX_CormyrDalelands, "SetFeatIsSneakAttackFeat");
}

void NWNX_CormyrDalelands_SetFeatIsDeathAttackFeat(int nFeat, int bIsDeathAttackFeat = TRUE)
{
    NWNXPushInt(bIsDeathAttackFeat);
    NWNXPushInt(nFeat);
    NWNXCall(NWNX_CormyrDalelands, "SetFeatIsDeathAttackFeat");
}

void NWNX_CormyrDalelands_SetNaturalBaseACModifierFeat(int nFeatID, int nModifier)
{
    NWNXPushInt(nModifier);
    NWNXPushInt(nFeatID);
    NWNXCall(NWNX_CormyrDalelands, "SetNaturalBaseACModifierFeat");
}

void NWNX_CormyrDalelands_SetClassProgressesBardSongUses(int nClassID, int bProgressesUses = TRUE)
{
    NWNXPushInt(bProgressesUses);
    NWNXPushInt(nClassID);
    NWNXCall(NWNX_CormyrDalelands, "SetClassProgressesBardSongUses");
}

float NWNX_CormyrDalelands_GetRulesetFloatEntry(string sEntry, float fDefault)
{
    NWNXPushFloat(fDefault);
    NWNXPushString(sEntry);
    NWNXCall(NWNX_CormyrDalelands, "GetRulesetFloatEntry");

    return NWNXPopFloat();
}

int NWNX_CormyrDalelands_GetRulesetIntEntry(string sEntry, int nDefault)
{
    NWNXPushInt(nDefault);
    NWNXPushString(sEntry);
    NWNXCall(NWNX_CormyrDalelands, "GetRulesetIntEntry");

    return NWNXPopInt();
}

string NWNX_CormyrDalelands_GetRulesetStringEntry(string sEntry, string sDefault)
{
    NWNXPushString(sDefault);
    NWNXPushString(sEntry);
    NWNXCall(NWNX_CormyrDalelands, "GetRulesetStringEntry");

    return NWNXPopString();
}

void NWNX_CormyrDalelands_UpdateCombatInformation(object oCreature)
{
    NWNXPushObject(oCreature);
    NWNXCall(NWNX_CormyrDalelands, "UpdateCombatInformation");
}

void NWNX_CormyrDalelands_SetCreatureAge(object oCreature, int nAge)
{
    NWNXPushInt(nAge);
    NWNXPushObject(oCreature);
    NWNXCall(NWNX_CormyrDalelands, "SetCreatureAge");
}

void NWNX_CormyrDalelands_SetUseBaseItemTypeUnequippedAllowed(int nBaseItemType, int bAllow)
{
    NWNXPushInt(bAllow);
    NWNXPushInt(nBaseItemType);
    NWNXCall(NWNX_CormyrDalelands, "SetUseBaseItemTypeUnequippedAllowed");
}

void NWNX_CormyrDalelands_SetSpellAutoQuicken(int nSpellId)
{
    NWNXPushInt(nSpellId);
    NWNXCall(NWNX_CormyrDalelands, "SetSpellAutoQuicken");
}

void NWNX_CormyrDalelands_SetClassProgressesSmiteEvil(int nClassID)
{
    NWNXPushInt(nClassID);
    NWNXCall(NWNX_CormyrDalelands, "SetClassProgressesSmiteEvil");
}

struct NWNX_CormyrDalelands_WeaponStrike NWNX_CormyrDalelands_ResolveWeaponStrike(object oAttacker, object oTarget, int nStrModOverride = 255, int nAttackBonusMod = 0, int bOffHand = FALSE, int bAutoHit = FALSE, int nCritOverride = 0, int nSneakOverride = 0, int nBonusDamageType = 0, int nBonusDamage = 0)
{
    NWNXPushInt(nBonusDamage);
    NWNXPushInt(nBonusDamageType);
    NWNXPushInt(nSneakOverride);
    NWNXPushInt(nCritOverride);
    NWNXPushInt(bAutoHit);
    NWNXPushInt(bOffHand);
    NWNXPushInt(nAttackBonusMod);
    NWNXPushInt(nStrModOverride);
    NWNXPushObject(oTarget);
    NWNXPushObject(oAttacker);
    NWNXCall(NWNX_CormyrDalelands, "ResolveWeaponStrike");

    struct NWNX_CormyrDalelands_WeaponStrike strike;
    strike.nAttackResult = NWNXPopInt();
    strike.nTotalDamage = NWNXPopInt();
    strike.nToHitRoll = NWNXPopInt();
    strike.nToHitMod = NWNXPopInt();
    strike.bSneakAttack = NWNXPopInt();
    strike.bKillingBlow = NWNXPopInt();
    return strike;
}
