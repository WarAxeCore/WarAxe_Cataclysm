/*
 * Copyright (C) 2010-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2005-2019 MaNGOS <https://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PATH_INFO_H
#define _PATH_INFO_H

#include <ace/RW_Mutex.h>

#include "SharedDefines.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "MoveSplineInitArgs.h"

using Movement::Vector3;
using Movement::PointsArray;

class Unit;

// 74*4.0f=296y  number_of_points*interval = max_path_len
// this is way more than actual evade range
// I think we can safely cut those down even more
#define MAX_PATH_LENGTH         74
#define MAX_POINT_PATH_LENGTH   74

#define SMOOTH_PATH_STEP_SIZE   4.0f
#define SMOOTH_PATH_SLOP        0.3f

#define VERTEX_SIZE       3
#define INVALID_POLYREF   0

enum PathType
{
	PATHFIND_BLANK = 0x00,   // path not built yet
	PATHFIND_NORMAL = 0x01,   // normal path
	PATHFIND_SHORTCUT = 0x02,   // travel through obstacles, terrain, air, etc (old behavior)
	PATHFIND_INCOMPLETE = 0x04,   // we have partial path to follow - getting closer to target
	PATHFIND_NOPATH = 0x08,   // no valid path at all or error in generating one
	PATHFIND_NOT_USING_PATH = 0x10,   // used when we are either flying/swiming or on map w/o mmaps
	PATHFIND_SHORT = 0x20,   // path is longer or equal to its limited path length
};

class PathFinderMovementGenerator
{
public:
	PathFinderMovementGenerator(Unit const* owner);
	~PathFinderMovementGenerator();

	// Calculate the path from owner to given destination
	// return: true if new path was calculated, false otherwise (no change needed)
	bool CalculatePath(float destX, float destY, float destZ, bool forceDest = false);

	// option setters - use optional
	void SetUseStraightPath(bool useStraightPath) { _useStraightPath = useStraightPath; };
	void SetPathLengthLimit(float distance) { _pointPathLimit = std::min<uint32>(uint32(distance / SMOOTH_PATH_STEP_SIZE), MAX_POINT_PATH_LENGTH); };

	// result getters
	Vector3 const& GetStartPosition()      const { return _startPosition; }
	Vector3 const& GetEndPosition()        const { return _endPosition; }
	Vector3 const& GetActualEndPosition()  const { return _actualEndPosition; }

	float GetPathLength()
	{
		float len = 0.0f;
		for (uint32 idx = 1; idx < _pathPoints.size(); ++idx)
		{
			Vector3 const& node = _pathPoints[idx];
			Vector3 const& prev = _pathPoints[idx - 1];
			float xd = node.x - prev.x;
			float yd = node.y - prev.y;
			float zd = node.z - prev.z;
			len += sqrtf(xd*xd + yd * yd + zd * zd);
		}
		return len;
	}

	PointsArray& GetPath() { return _pathPoints; }
	PathType GetPathType() const { return _type; }

private:

	dtPolyRef      _pathPolyRefs[MAX_PATH_LENGTH];   // array of detour polygon references
	uint32         _polyLength;                      // number of polygons in the path

	PointsArray    _pathPoints;       // our actual (x,y,z) path to the target
	PathType       _type;             // tells what kind of path this is

	bool           _useStraightPath;  // type of path will be generated
	bool           _forceDestination; // when set, we will always arrive at given point
	uint32         _pointPathLimit;   // limit point path size; min(this, MAX_POINT_PATH_LENGTH)

	Vector3        _startPosition;    // {x, y, z} of current location
	Vector3        _endPosition;      // {x, y, z} of the destination
	Vector3        _actualEndPosition;// {x, y, z} of the closest possible point to given destination

	Unit const* const       _sourceUnit;       // the unit that is moving
	dtNavMesh const*        _navMesh;          // the nav mesh
	dtNavMeshQuery const*   _navMeshQuery;     // the nav mesh query used to find the path

	dtQueryFilter _filter;                     // use single filter for all movements, update it when needed

	void SetStartPosition(Vector3 Point) { _startPosition = Point; }
	void SetEndPosition(Vector3 Point) { _actualEndPosition = Point; _endPosition = Point; }
	void SetActualEndPosition(Vector3 Point) { _actualEndPosition = Point; }
	void NormalizePath();

	void Clear()
	{
		_polyLength = 0;
		_pathPoints.clear();
	}

	bool InRange(Vector3 const& p1, Vector3 const& p2, float r, float h) const;
	float Dist3DSqr(Vector3 const& p1, Vector3 const& p2) const;
	bool InRangeYZX(float const* v1, float const* v2, float r, float h) const;

	dtPolyRef GetPathPolyByPosition(dtPolyRef const* polyPath, uint32 polyPathSize, float const* Point, float* Distance = NULL) const;
	dtPolyRef GetPolyByLocation(float const* Point, float* Distance) const;
	bool HaveTile(Vector3 const& p) const;

	void BuildPolyPath(Vector3 const& startPos, Vector3 const& endPos);
	void BuildPointPath(float const* startPoint, float const* endPoint);
	void BuildShortcut();

	NavTerrain GetNavTerrain(float x, float y, float z);
	void CreateFilter();
	void UpdateFilter();

	// smooth path aux functions
	uint32 FixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath, dtPolyRef const* visited, uint32 nvisited);
	bool GetSteerTarget(float const* startPos, float const* endPos, float minTargetDist, dtPolyRef const* path, uint32 pathSize, float* steerPos,
		unsigned char& steerPosFlag, dtPolyRef& steerPosRef);
	dtStatus FindSmoothPath(float const* startPos, float const* endPos,
		dtPolyRef const* polyPath, uint32 polyPathSize,
		float* smoothPath, int* smoothPathSize, uint32 smoothPathMaxSize);
};

#endif