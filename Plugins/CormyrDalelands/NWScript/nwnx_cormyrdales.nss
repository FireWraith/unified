/// @addtogroup cormyrdalelands CormyrDalelands
/// @brief Cormyr and the Dalelands NWNX plugin
/// @{
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

/// @}

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
