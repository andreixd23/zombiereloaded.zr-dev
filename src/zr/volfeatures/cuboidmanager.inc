/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:           cuboidmanager.inc
 *  Type:           Module
 *  Description:    Cuboid managing API.
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

/**
 * Creates a new cuboid with the specified coordinates.
 *
 * @param min   Coordinates to first corner of cuboid.
 * @param max   Coordinates to diagonally oposite corner of cuboid.
 * @param name  Optional. Name of this cuboid.
 * @return      Cuboid index, or negative on error.
 *              -1 if no free cuboids
 *              -2 zero area cuboid
 */
VolCuboidAdd(Float:min[3], Float:max[3], String:name[] = "")
{
    new Float:x1 = min[0];
    new Float:y1 = min[1];
    new Float:z1 = min[2];
    new Float:x2 = max[0];
    new Float:y2 = max[1];
    new Float:z2 = max[2];
    
    // Check if both locations are equal.
    if (x1 == x2 &&
        y1 == y2 &&
        z1 == z2)
    {
        // Zero area cuboid.
        return -2;
    }
    
    // Get free cuboid index.
    new cuboid = VolGetFreeCuboid();
    
    // Validate index.
    if (cuboid < 0)
    {
        return -1;
    }
    
    // Sort out max and min values so 1-values are smaller.
    new Float:temp;
    if (x1 > x2)
    {
        // x1 is bigger than x2. Swap values.
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2)
    {
        // y1 is bigger than y2. Swap values.
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    if (z1 > z2)
    {
        // z1 is bigger than z2. Swap values.
        temp = z1;
        z1 = z2;
        z2 = temp;
    }
    
    // Apply coordinates to cuboid.
    Cuboids[cuboid][Cuboid_xMin] = x1;
    Cuboids[cuboid][Cuboid_yMin] = y1;
    Cuboids[cuboid][Cuboid_zMin] = z1;
    
    Cuboids[cuboid][Cuboid_xMax] = x2;
    Cuboids[cuboid][Cuboid_yMax] = y2;
    Cuboids[cuboid][Cuboid_zMax] = z2;
    
    TrimString(name);
    strcopy(Cuboids[cuboid][Cuboid_Name], VOL_NAME_LEN, name);
    
    return cuboid;
}

/**
 * Removes the specified cuboid.
 *
 * Note: Volumes and features that depend on the cuboid is also removed if
 *       recursive removal is enabled. Otherwise they are disabled.
 *
 * @param cuboid        Cuboid to remove.
 * @param recursive     Optional. Also remove volumes and features that depend
 *                      on the cuboid. Default is true.
 * @return              True if successful, false otherwise.
 */
bool:VolCuboidRemove(cuboid, bool:recursive = true)
{
    // Validate index.
    if (!VolCuboidIsValid(cuboid))
    {
        return false;
    }
    
    
    // Find volumes that use the cuboid and disable/remove them.
    for (new volumeIndex = 0; volumeIndex < ZR_VOLUMES_MAX; volumeIndex++)
    {
        // Validate volume.
        if (!VolIsValid(volumeIndex))
        {
            // Skip volume.
            continue;
        }
        
        // Check if the cuboid is used on the current volume.
        if (Volumes[volumeIndex][Vol_Shape] == VolShape_Cuboid &&
            Volumes[volumeIndex][Vol_ShapeData] == cuboid)
        {
            // Check if recursive removal is enabled.
            if (recursive)
            {
                // Remove volume.
                VolRemove(volumeIndex, recursive);
            }
            else
            {
                // Disable volume.
                VolDisableVolume(volumeIndex);
            }
        }
    }
    
    // Remove the cuboid.
    VolCuboidClearIndex(cuboid);
    
    return true;
}

/**
 * Removes all cuboids.
 *
 * Note: Volumes and features that depend on the cuboids are also removed if
 *       recursive removal is enabled. Otherwise they are disabled.
 *
 * @param recursive     Optional. Also remove volumes and features that depend
 *                      on the cuboids. Default is true.
 * @return              True if successful, false otherwise.
 */
stock VolCuboidRemoveAll(bool:recursive = true)
{
    for (new cuboid = 0; cuboid < ZR_CUBOIDS_MAX; cuboid++)
    {
        VolCuboidRemove(cuboid, recursive);
    }
}

/**
 * Search for a cuboid with the specified name.
 *
 * @param name              Name to search for.
 * @param caseSensitive     Optional. Use case sensitive search. Default is
 *                          false.
 * @return                  Cuboid index if found, -1 otherwise.
 */
VolCuboidFind(const String:name[], bool:caseSensitive = false)
{
    for (new cuboid = 0; cuboid < ZR_CUBOIDS_MAX; cuboid++)
    {
        if (VolCuboidInUse(cuboid))
        {
            if (StrEqual(Cuboids[cuboid][Cuboid_Name], name, caseSensitive))
            {
                return cuboid;
            }
        }
    }
    
    return -1;
}
