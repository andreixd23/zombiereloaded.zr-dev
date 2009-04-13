/**
 * ====================
 *   Zombie:Reloaded
 *   File: restriction.inc
 *   Author: Greyscale
 * ==================== 
 */

/**
 * Array to store restricted weapon names.
 */
new Handle:gRestrictedWeapons = INVALID_HANDLE;

/**
 * Array to store keyvalue data.
 */
new Handle:kvWeaponGroups = INVALID_HANDLE;

/**
 * Array that stores the "HookID" to be later unhooked on player disconnect.
 */
new gCanUseHookID[MAXPLAYERS+1];

/**
 * Query results returned when (un)restricting a weapon.
 */
enum WpnRestrictQuery
{
    Successful_Weapon,  /** Weapon (un)restrict query was successful. */
    Successful_Group,  /** Group (un)restrict query was successful. */
    Failed_Weapon, /** Weapon (un)restrict was unsuccessful */
    Failed_Group, /** Group (un)restrict was unsuccessful */
    Invalid,  /** Weapon/Group invalid */
}

/**
 * Initialize data and hook commands.
 */
WeaponRestrictInit()
{
    // Initialize weapon restrict array
    gRestrictedWeapons = CreateArray(32, 0);
    
    // Hook buy command
    RegConsoleCmd("buy", WeaponRestrictBuyHook);
}

/**
 * Loads weapon data from file.
 */
WeaponRestrictOnMapStart()
{
    // Clear restricted weapons
    RestrictWeaponUnrestrictAll();
    
    // Load weapon group data
    if (kvWeaponGroups != INVALID_HANDLE)
    {
        CloseHandle(kvWeaponGroups);
    }
    
    kvWeaponGroups = CreateKeyValues("weapongroups");
    
    decl String:path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/zr/weapons/weapongroups.txt");
    
    // If file isn't found, stop plugin
    if (!FileToKeyValues(kvWeaponGroups, path))
    {
        SetFailState("\"%s\" missing from server", path);
    }
    
    KvRewind(kvWeaponGroups);
    
    if (KvGotoFirstSubKey(kvWeaponGroups))
    {
        do
        {
            if (KvGotoFirstSubKey(kvWeaponGroups))
            {
                decl String:groupweapon[32];
                
                do
                {
                    
                    KvGetSectionName(kvWeaponGroups, groupweapon, sizeof(groupweapon));
                    
                    if (!WeaponsIsValidWeapon(groupweapon))
                    {
                        // TODO: LOG INVALID WEAPON
                    }
                } while (KvGotoNextKey(kvWeaponGroups));
                
                KvGoBack(kvWeaponGroups);
            }
        } while (KvGotoNextKey(kvWeaponGroups));
    }
}

/**
 * Clears restricted weapon array.
 */
RestrictWeaponUnrestrictAll()
{
    ClearArray(gRestrictedWeapons);
}

/**
 * Hook Weapon_CanUse function on a client.
 * 
 * @param client    The client index.
 */
WeaponRestrictClientInit(client)
{
    gCanUseHookID[client] = Hacks_Hook(client, HACKS_HTYPE_WEAPON_CANUSE, WeaponRestrictCanUse, false);
}

/**
 * Unhook Weapon_CanUse function on a client.
 * 
 * @param client    The client index.
 */
WeaponRestrictClientDisconnect(client)
{
    Hacks_Unhook(gCanUseHookID[client]);
}

/**
 * Command callback function for the "buy" command
 * Used to block use of this command under certain conditions.
 * 
 * @param client    The client index.
 * @param argc      Argument count.
 */
public Action:WeaponRestrictBuyHook(client, argc)
{
    // If plugin is disabled then stop
    new bool:enabled = GetConVarBool(gCvars[CVAR_ENABLE]);
    if (!enabled)
    {
        // Allow command
        return Plugin_Continue;
    }
    
    // If player is a zombie then block command
    if (IsPlayerZombie(client))
    {
        ZR_PrintToChat(client, "Zombie cant use weapon");
        
        // Block command
        return Plugin_Handled;
    }
    
    decl String:weapon[64];
    GetCmdArg(1, weapon, sizeof(weapon));
    
    ReplaceString(weapon, sizeof(weapon), "weapon_", "");
    
    // Check if the weapon is restricted, if so then block command
    if (WeaponRestrictIsRestricted(weapon))
    {
        ZR_PrintToChat(client, "Weapon is restricted", weapon);
        
        // Block command
        return Plugin_Handled;
    }
    
    // Allow command
    return Plugin_Continue;
}

/**
 * Restricts a weapon.
 * 
 * @param weapon    The weapon/group name.
 * @return          Successful_Weapon: The call successfully restricted a weapon.
 *                  Successful_Group: The call successfully restricted a weapon group.
 *                  Failed_Weapon: The call failed to restrict a weapon.
 *                  Failed_Group: The call failed to restrict a weapon group.
 *                  Invalid: The call was unsuccessful due to invalid weapon.
 */
WpnRestrictQuery:WeaponRestrictRestrict(const String:weapon[])
{
    if (WeaponRestrictIsCustomGroup(weapon))
    {
        KvRewind(kvWeaponGroups);
        KvJumpToKey(kvWeaponGroups, weapon);
        
        if (KvGotoFirstSubKey(kvWeaponGroups))
        {
            decl String:groupweapon[32];
            
            do
            {
                
                KvGetSectionName(kvWeaponGroups, groupweapon, sizeof(groupweapon));
                
                // If weapon is invalid, then skip
                if (!WeaponsIsValidWeapon(groupweapon))
                {
                    continue;
                }
                
                if (!WeaponRestrictIsRestricted(groupweapon))
                {
                    PushArrayString(gRestrictedWeapons, groupweapon);
                }
            } while (KvGotoNextKey(kvWeaponGroups));
        }
        
        return Successful_Group;
    }
    
    if (!WeaponsIsValidWeapon(weapon))
    {
        return Invalid;
    }
    
    if (WeaponRestrictIsRestricted(weapon))
    {
        return Failed_Weapon;
    }
    
    PushArrayString(gRestrictedWeapons, weapon);
    
    return Successful_Weapon;
}

/**
 * Unrestricts a weapon.
 * 
 * @param weapon    The weapon/group name.
 * @return          Successful_Weapon: The call successfully restricted a weapon.
 *                  Successful_Group: The call successfully restricted a weapon group.
 *                  Failed_Weapon: The call failed to restrict a weapon.
 *                  Failed_Group: The call failed to restrict a weapon group. 
 *                  Invalid: The call was unsuccessful due to invalid weapon.
 */
WpnRestrictQuery:WeaponRestrictUnrestrict(const String:weapon[])
{
    if (WeaponRestrictIsCustomGroup(weapon))
    {
        KvRewind(kvWeaponGroups);
        KvJumpToKey(kvWeaponGroups, weapon);
        
        if (KvGotoFirstSubKey(kvWeaponGroups))
        {
            decl String:groupweapon[WEAPONS_MAX_LENGTH];
            
            do
            {
                KvGetSectionName(kvWeaponGroups, groupweapon, sizeof(groupweapon));
                
                // If weapon is invalid, then skip
                if (!WeaponsIsValidWeapon(groupweapon))
                {
                    continue;
                }
                
                if (WeaponRestrictIsRestricted(groupweapon))
                {
                    new weaponindex = WeaponRestrictGetIndex(groupweapon);
                    if (weaponindex > -1)
                    {
                        RemoveFromArray(gRestrictedWeapons, weaponindex);
                    }
                }
            } while (KvGotoNextKey(kvWeaponGroups));
        }
        
        return Successful_Group;
    }
    
    if (!WeaponsIsValidWeapon(weapon))
    {
        return Invalid;
    }
    
    if (!WeaponRestrictIsRestricted(weapon))
    {
        return Failed_Weapon;
    }
    
    new weaponindex = WeaponRestrictGetIndex(weapon);
    if (weaponindex > -1)
    {
        RemoveFromArray(gRestrictedWeapons, weaponindex);
    }
    
    return Successful_Weapon;
}

/**
 * Checks if a weapon is restricted.
 * 
 * @param weapon    The weapon name.
 */
bool:WeaponRestrictIsRestricted(const String:weapon[])
{
    new size = GetArraySize(gRestrictedWeapons);
    for (new x = 0; x < size; x++)
    {
        decl String:restrictedweapon[WEAPONS_MAX_LENGTH];
        GetArrayString(gRestrictedWeapons, x, restrictedweapon, sizeof(restrictedweapon));
        
        if (StrEqual(weapon, restrictedweapon, false))
        {
            return true;
        }
    }
    
    return false;
}

/**
 * Returns the array index of the restricted weapon.
 * 
 * @param weapon    The weapon name.
 */
WeaponRestrictGetIndex(const String:weapon[])
{
    new size = GetArraySize(gRestrictedWeapons);
    for (new x = 0; x < size; x++)
    {
        decl String:restrictedweapon[WEAPONS_MAX_LENGTH];
        GetArrayString(gRestrictedWeapons, x, restrictedweapon, sizeof(restrictedweapon));
        
        if (StrEqual(weapon, restrictedweapon, false))
        {
            return x;
        }
    }
    
    return -1;
}

/**
 * Checks if the provided name is a custom group.
 * 
 * @param groupname     Name of the group to check.
 * @return              True if it's a group, false if not.
 */
bool:WeaponRestrictIsCustomGroup(const String:groupname[])
{
    // Reset the traversal stack
    KvRewind(kvWeaponGroups);
    
    // Returns true if groupname is listed in the custom groups file
    return KvJumpToKey(kvWeaponGroups, groupname);
}

/**
 * Returns a string of all weapons in a custom weapon group separated
 * by the provided character. 
 * 
 * @param groupname     Name of the group to get weapon list from.
 * @param weaponlist    Variable to store weapon list string in.
 * @param maxlen        Maximum length of the weapon list, the rest is truncated.
 * @param separator     Separator character between weapon names. 
 */
WeaponRestrictGetWeaponList(const String:groupname[], String:weaponlist[], maxlen, const String:separator[])
{
    KvRewind(kvWeaponGroups);
    KvJumpToKey(kvWeaponGroups, groupname);
    
    if (KvGotoFirstSubKey(kvWeaponGroups))
    {
        decl String:groupweapon[WEAPONS_MAX_LENGTH];
        
        strcopy(weaponlist, maxlen, "");
        
        do
        {
            KvGetSectionName(kvWeaponGroups, groupweapon, sizeof(groupweapon));
            
            // If weapon is invalid, then skip
            if (!WeaponsIsValidWeapon(groupweapon))
            {
                continue;
            }
            
            if (!weaponlist[0])
            {
                strcopy(weaponlist, maxlen, groupweapon);
            }
            else
            {
                Format(weaponlist, maxlen, "%s%s%s", weaponlist, separator, groupweapon);
            }
        } while (KvGotoNextKey(kvWeaponGroups));
    }
}

/**
 * Hook callback, called when a player is trying to pick up a weapon.
 * @param client    The client index.
 * @param weapon    The weapon index.
 * @return          0 to block weapon pickup, Hacks_Continue to allow.
 */
public WeaponRestrictCanUse(client, weapon, dummy1, dummy2, dummy3, dummy4)
{
    // If plugin is disabled then stop
    new bool:enabled = GetConVarBool(gCvars[CVAR_ENABLE]);
    if (!enabled)
    {
        return Hacks_Continue;
    }
    
    new String:weaponname[WEAPONS_MAX_LENGTH];
    GetEdictClassname(weapon, weaponname, sizeof(weaponname));
    
    // Strip "weapon_" from entity name
    ReplaceString(weaponname, sizeof(weaponname), "weapon_", "");
    
    // If the weapon is restricted then prevent pickup
    if (WeaponRestrictIsRestricted(weaponname))
    {
        return 0;
    }
    
    // If the player is a zombie and the weapon isn't a knife then prevent pickup
    if (IsPlayerZombie(client) && !StrEqual(weaponname, "knife"))
    {
        return 0;
    }
    
    return Hacks_Continue;
}