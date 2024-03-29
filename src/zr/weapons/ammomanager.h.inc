/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          ammomanager.inc
 *  Description:   Ammo profile manager headers.
 *
 *  Copyright (C) 2009-2010  Greyscale, Richard Helgeby
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 */

#if defined _ammomanager_included
 #endinput
#endif
#define _ammomanager_included

/**
 * Specifies whether the ammo manager should apply settings to a weapon. This
 * setting is made for debug purposes so the ammo manager won't break anything
 * while testing other stuff in the plugin.
 *
 * Comment to not apply values to weapons.
 */
//#define AMMO_MANAGER_APPLY

/**
 * Specifies what slots that are managed and supported by the ammo manager.
 *
 * Note: Do not modify these values unless you're making support for a slot.
 */
new const bool:AmmoManagedSlots[WepLib_Slots] = 
{
    //false,  /* Dummy for invalid slot */
    true,   /* Primary */
    true,   /* Secondary */
    false,  /* Melee */
    false,  /* Projectile */
    false,  /* Explosive */
    false   /* NVGs */
};

/**
 * @section Number of managed slots. So far it's for primary and secondary
 *          weapons. The slot ID's should match WeaponsSlot.
 *
 *          Might not needed anymore.
 */
//#define AMMO_MANAGED_SLOTS 2
/**
 * @endsection
 */

/**
 * @section Maximum number of ammo managers. There must be one manager per
 *          client and per weapon slot.
 */
#define AMMO_MANAGER_MAX    (MAXPLAYERS + 1) * sizeof(g_WeaponsSlotDummy)
/**
 * @endsection
 */

/**
 * Ammo manager attributes.
 */
enum AmmoManager
{
    bool:AmmoManager_InUse,         /** Specifies whether the profile is used or not. */
    WepLib_Slots:AmmoManager_Slot,  /** What weapon slot that's managed.  TODO: Change tags to use WepLib_Slots when it's imported. */
    AmmoManager_Clip,               /** Current weapon clip value. */
    AmmoManager_Reserve,            /** Current weapon reserve value. */
    AmmoManager_ProfileIndex,       /** Index to a ammo profile. */
    Handle:AmmoManager_Timer        /** Timer used for refilling, if enabled. */
}


/**
 * All ammo managers.
 */
new AmmoManagers[AMMO_MANAGER_MAX][AmmoManager];

/**
 * Ammo manager index for assigning ammo managers to clients.
 */
new AmmoManagerIndex[MAXPLAYERS + 1][WepLib_Slots];
