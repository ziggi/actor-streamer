/*
 *  ActorStreamer.inc
 *  Allows you to stream actors in your server.
 *
 *  Contributors:
 *    Emmet_
 *    Abagail
 *    Kar
 *    kvanningsen
 *    ziggi
 *
 *  Created on April 30, 2015 @ 7:21 AM.
 *  Last updated on mar 9, 2016.
*/

// Maximum amount of allocated slots.
#if !defined MAX_DYNAMIC_ACTORS
	#define MAX_DYNAMIC_ACTORS 2000
#endif

// Maximum stream distance.
#if !defined MAX_ACTOR_STREAM_DISTANCE
	#define MAX_ACTOR_STREAM_DISTANCE 200.0
#endif

// Tick rate. Lower values mean more server power!
#if !defined ACTOR_STREAM_TICK_RATE
	#define ACTOR_STREAM_TICK_RATE 500
#endif

// Internal defines
#define MAX_DYNAMIC_ACTOR_ANIM_LIB_NAME 32
#define MAX_DYNAMIC_ACTOR_ANIM_NAME 32

// Alternative names.
#define GetDynamicActorModel GetDynamicActorSkin
#define SetDynamicActorModel SetDynamicActorSkin

enum e_DynamicActors {
	e_iSlotUsed,
	e_iActorModel,
	Float:e_fActorX,
	Float:e_fActorY,
	Float:e_fActorZ,
	Float:e_fActorA,
	e_iActorWorld,
	e_iActorInterior,
	e_iActorTimer,
	e_iActorID,
	Float:e_fActorHealth,
	bool:e_bInvulnerable,
};

enum e_AnimationInfo {
	bool:e_bAnimation,
	e_sAnimLibrary[MAX_DYNAMIC_ACTOR_ANIM_LIB_NAME char],
	e_sAnimName[MAX_DYNAMIC_ACTOR_ANIM_NAME char],
	Float:e_fAnimDelta,
	e_iAnimLockX,
	e_iAnimLockY,
	e_iAnimFreeze,
	e_iAnimTime,
};

static
	s_DynamicActors[MAX_DYNAMIC_ACTORS][e_DynamicActors],
	s_ActorAnimation[MAX_DYNAMIC_ACTORS][e_AnimationInfo],
	s_DynamicActorsMap[MAX_ACTORS]
;

forward OnDynamicActorStreamIn(actorid, forplayerid);
forward OnDynamicActorStreamOut(actorid, forplayerid);
forward OnPlayerGiveDamageDynamicActor(playerid, actorid, Float:amount, weaponid, bodypart);

public OnFilterScriptExit()
{
	for (new i = 0; i < MAX_DYNAMIC_ACTORS; i ++) {
		if (s_DynamicActors[i][e_iSlotUsed]) {
			// Destroy all actors before unloading the filterscript.
			DestroyDynamicActor(i);
		}
	}

	#if defined AS_OnFilterScriptExit
		return AS_OnFilterScriptExit();
	#else
		return 1;
	#endif
}

public OnGameModeExit()
{
	for (new i = 0; i < MAX_DYNAMIC_ACTORS; i ++) {
		if (s_DynamicActors[i][e_iSlotUsed]) {
			// Destroy all actors before gamemode exit.
			DestroyDynamicActor(i);
		}
	}

	#if defined AS_OnGameModeExit
		return AS_OnGameModeExit();
	#else
		return 1;
	#endif
}

stock CreateDynamicActor(modelid, Float:x, Float:y, Float:z, Float:angle, worldid = -1, interiorid = -1, bool:invulnerable = true, Float:health = 100.0)
{
	new
		slot = INVALID_ACTOR_ID;

	for (new i = 0; i < MAX_DYNAMIC_ACTORS; i ++) {
		if (!s_DynamicActors[i][e_iSlotUsed]) {
			slot = i;
			break;
		}
	}

	if (slot == INVALID_ACTOR_ID) {
		return INVALID_ACTOR_ID;
	}

	s_DynamicActors[slot][e_iSlotUsed] = 1;
	s_DynamicActors[slot][e_iActorModel] = modelid;
	s_DynamicActors[slot][e_fActorX] = x;
	s_DynamicActors[slot][e_fActorY] = y;
	s_DynamicActors[slot][e_fActorZ] = z;
	s_DynamicActors[slot][e_fActorA] = angle;
	s_DynamicActors[slot][e_iActorWorld] = worldid;
	s_DynamicActors[slot][e_iActorInterior] = interiorid;
	s_DynamicActors[slot][e_bInvulnerable] = invulnerable;
	s_DynamicActors[slot][e_fActorHealth] = health;
	s_DynamicActors[slot][e_iActorID] = INVALID_ACTOR_ID;

	// Timer used for update checking.
	s_DynamicActors[slot][e_iActorTimer] = SetTimerEx("UpdateActorStream", ACTOR_STREAM_TICK_RATE, true, "i", i);

	return slot;
}

stock IsValidDynamicActor(actorid)
{
	if (!(0 <= actorid < MAX_DYNAMIC_ACTORS) || (!s_DynamicActors[actorid][e_iSlotUsed])) {
		return 0;
	}
	return 1;
}

stock GetDynamicActorInternalID(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return INVALID_ACTOR_ID;
	}
	return s_DynamicActors[actorid][e_iActorID];
}

stock DestroyDynamicActor(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	// Destroy the actor if it is streamed.
	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		DestroyActor(s_DynamicActors[actorid][e_iActorID]);
	}

	// Clear the animation info.
	ClearDynamicActorAnimations(actorid);

	// Free up the slot.
	s_DynamicActors[actorid][e_iSlotUsed] = 0;
	return 1;
}

stock bool:IsDynamicActorStreamedIn(actorid, forplayerid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}
	return s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID;
}

stock bool:IsDynamicActorInStreamRange(actorid, forplayerid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	// Checks if the player is in range of the actor's position.
	if (!IsPlayerInRangeOfPoint(forplayerid, MAX_ACTOR_STREAM_DISTANCE,
			s_DynamicActors[actorid][e_fActorX],
			s_DynamicActors[actorid][e_fActorY],
			s_DynamicActors[actorid][e_fActorZ])) {
		return 0;
	}

	// An additional check for interiors and virtual worlds.
	new
		interior,
		world;

	interior = s_DynamicActors[actorid][e_iActorInterior];
	world = s_DynamicActors[actorid][e_iActorWorld];

	if (interior != -1 && GetPlayerInterior(forplayerid) != interior) {
		return 0;
	}

	if (world != -1 && GetPlayerVirtualWorld(forplayerid) != world) {
		return 0;
	}
	return 1;
}

stock SetDynamicActorVirtualWorld(actorid, worldid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	s_DynamicActors[actorid][e_iActorWorld] = worldid;

	if (worldid != INVALID_ACTOR_ID) {
		SetActorVirtualWorld(s_DynamicActors[actorid][e_iActorID], worldid);
	}
	return 1;
}

stock GetDynamicActorVirtualWorld(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}
	return s_DynamicActors[actorid][e_iActorWorld];
}

stock SetDynamicActorInterior(actorid, interiorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}
	s_DynamicActors[actorid][e_iActorInterior] = interiorid;
	return 1;
}

stock GetDynamicActorInterior(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}
	return s_DynamicActors[actorid][e_iActorInterior];
}

stock ApplyDynamicActorAnimation(actorid, animlib[], animname[], Float:fDelta, loop, lockx, locky, freeze, time)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	// We want to save the looping animation so it plays when a player streams in.
	if (loop) {
		strpack(s_ActorAnimation[actorid][e_sAnimLibrary], animlib, MAX_DYNAMIC_ACTOR_ANIM_LIB_NAME);
		strpack(s_ActorAnimation[actorid][e_sAnimName], animname, MAX_DYNAMIC_ACTOR_ANIM_NAME);

		s_ActorAnimation[actorid][e_bAnimation] = true;
		s_ActorAnimation[actorid][e_fAnimDelta] = fDelta;
		s_ActorAnimation[actorid][e_iAnimLockX] = lockx;
		s_ActorAnimation[actorid][e_iAnimLockY] = locky;
		s_ActorAnimation[actorid][e_iAnimFreeze] = freeze;
		s_ActorAnimation[actorid][e_iAnimTime] = time;
	}

	// If the actor is streamed, apply the animation.
	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		ApplyActorAnimation(s_DynamicActors[actorid][e_iActorID], animlib, animname, fDelta, loop, lockx, locky, freeze, time);
	}
	return 1;
}

stock ClearDynamicActorAnimations(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	// Reset the animation info.
	if (s_ActorAnimation[actorid][e_bAnimation]) {
		s_ActorAnimation[actorid][e_bAnimation] = false;
		s_ActorAnimation[actorid][e_fAnimDelta] = 0.0;
		s_ActorAnimation[actorid][e_iAnimLockX] = 0;
		s_ActorAnimation[actorid][e_iAnimLockY] = 0;
		s_ActorAnimation[actorid][e_iAnimFreeze] = 0;
		s_ActorAnimation[actorid][e_iAnimTime] = 0;
	}

	// If the actor is streamed, clear the animations.
	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		ClearActorAnimations(s_DynamicActors[actorid][e_iActorID]);
	}
	return 1;
}

stock SetDynamicActorPos(actorid, Float:x, Float:y, Float:z)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	s_DynamicActors[actorid][e_fActorX] = x;
	s_DynamicActors[actorid][e_fActorY] = y;
	s_DynamicActors[actorid][e_fActorZ] = z;

	// If the actor is streamed, update the position.
	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		SetActorPos(s_DynamicActors[actorid][e_iActorID], x, y, z);
	}
	return 1;
}

stock GetDynamicActorPos(actorid, &Float:x, &Float:y, &Float:z)
{
	if (!IsValidDynamicActor(actorid)) {
		x = 0.0;
		y = 0.0;
		z = 0.0;
		return 0;
	}

	x = s_DynamicActors[actorid][e_fActorX];
	y = s_DynamicActors[actorid][e_fActorY];
	z = s_DynamicActors[actorid][e_fActorZ];
	return 1;
}

stock SetDynamicActorFacingAngle(actorid, Float:angle)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	s_DynamicActors[actorid][e_fActorA] = angle;

	// If the actor is streamed, update the angle.
	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		SetActorFacingAngle(s_DynamicActors[actorid][e_iActorID], angle);
	}
	return 1;
}

stock GetDynamicActorFacingAngle(actorid, &Float:angle)
{
	if (!IsValidDynamicActor(actorid)) {
		angle = 0.0;
		return 0;
	}

	angle = s_DynamicActors[actorid][e_fActorA];
	return 1;
}

stock SetDynamicActorSkin(actorid, skinid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	s_DynamicActors[actorid][e_iActorModel] = skinid;

	// Re-stream the actor for the changes to occur.
	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		DestroyActor(s_DynamicActors[actorid][e_iActorID]);
		s_DynamicActors[actorid][e_iActorID] = INVALID_ACTOR_ID;
		UpdateActorStream(actorid);
	}
	return 1;
}

stock GetDynamicActorSkin(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}
	return s_DynamicActors[actorid][e_iActorModel];
}

stock SetDynamicActorHealth(actorid, Float:health)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	s_DynamicActors[actorid][e_fActorHealth] = health;

	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		SetActorHealth(s_DynamicActors[actorid][e_iActorID], health);
	}
	return 1;
}

stock GetDynamicActorHealth(actorid, &Float:health)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		GetActorHealth(actorid, health);
	} else {
		health = s_DynamicActors[actorid][e_fActorHealth];
	}

	return 1;
}

stock SetDynamicActorInvulnerable(actorid, bool:invulnerable = true)
{
	if (!IsValidDynamicActor(actorid)) {
		return 0;
	}

	s_DynamicActors[actorid][e_bInvulnerable] = invulnerable;

	if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID) {
		SetActorInvulnerable(actorid, invulnerable);
	}
	return 1;
}

stock bool:IsDynamicActorInvulnerable(actorid)
{
	if (!IsValidDynamicActor(actorid)) {
		return false;
	}
	return s_DynamicActors[actorid][e_bInvulnerable];
}

forward UpdateActorStream(actorid);
public UpdateActorStream(actorid)
{
	new
		internal_actorid,
		bool:is_in_range;

#if defined _inc_foreach
	foreach (new i : Player)
#else
	for (new i = GetPlayerPoolSize(); i != -1; i--)
#endif
	{
		// If the player is close to the actor...
		if (IsDynamicActorInStreamRange(actorid, i)) {
			// Indicate that this actor is streamed in.
			is_in_range = true;
			break;
		}
	}

	internal_actorid = s_DynamicActors[actorid][e_iActorID];

	// If the actor is streamed
	if (internal_actorid != INVALID_ACTOR_ID) {
		// The actor is no longer streamed in for any players... so delete it then.
		if (!is_in_range) {
			DestroyActor(internal_actorid);
			s_DynamicActorsMap[internal_actorid] = INVALID_ACTOR_ID;
			s_DynamicActors[internal_actorid][e_iActorID] = INVALID_ACTOR_ID;
		}
		return;
	}

	// If the actor is not streamed yet, create it.
	internal_actorid = CreateActor(s_DynamicActors[actorid][e_iActorModel], s_DynamicActors[actorid][e_fActorX], s_DynamicActors[actorid][e_fActorY], s_DynamicActors[actorid][e_fActorZ], s_DynamicActors[actorid][e_fActorA]);

	if (internal_actorid == INVALID_ACTOR_ID) {
		printf("[Error] Could not stream dynamic actor %d. The SA-MP limit has been reached.", actorid);
		return;
	}
	s_DynamicActorsMap[internal_actorid] = internal_actorid;

	// Applies the virtual world.
	if (s_DynamicActors[actorid][e_iActorWorld] != -1) {
		SetActorVirtualWorld(internal_actorid, s_DynamicActors[actorid][e_iActorWorld]);
	}

	// Applies the saved animation data.
	if (s_ActorAnimation[actorid][e_bAnimation]) {
		ApplyActorAnimation(internal_actorid, s_ActorAnimation[actorid][e_sAnimLibrary], s_ActorAnimation[actorid][e_sAnimName], s_ActorAnimation[actorid][e_fAnimDelta], true, s_ActorAnimation[actorid][e_iAnimLockX], s_ActorAnimation[actorid][e_iAnimLockY], s_ActorAnimation[actorid][e_iAnimFreeze], s_ActorAnimation[actorid][e_iAnimTime]);
	}

	SetActorInvulnerable(internal_actorid, s_DynamicActors[actorid][e_bInvulnerable]);
	SetActorHealth(internal_actorid, s_DynamicActors[actorid][e_fActorHealth]);
}

public OnActorStreamIn(actorid, forplayerid)
{
	new dyn_actorid = s_DynamicActorsMap[actorid];

	if (IsValidDynamicActor(dyn_actorid) && s_DynamicActors[dyn_actorid][e_iActorID] == actorid) {
		CallLocalFunction("OnDynamicActorStreamIn", "ii", dyn_actorid, forplayerid);
	}

	#if defined AS_OnActorStreamIn
		return AS_OnActorStreamIn(actorid, forplayerid);
	#else
		return 1;
	#endif
}

public OnActorStreamOut(actorid, forplayerid)
{
	new dyn_actorid = s_DynamicActorsMap[actorid];

	if (IsValidDynamicActor(dyn_actorid) && s_DynamicActors[dyn_actorid][e_iActorID] == actorid) {
		CallLocalFunction("OnDynamicActorStreamOut", "ii", dyn_actorid, forplayerid);
	}

	#if defined AS_OnActorStreamOut
		return AS_OnActorStreamOut(actorid, forplayerid);
	#else
		return 1;
	#endif
}

public OnPlayerGiveDamageActor(playerid, damaged_actorid, Float: amount, weaponid, bodypart)
{
	new dyn_actorid = s_DynamicActorsMap[damaged_actorid];

	if (IsValidDynamicActor(dyn_actorid) && s_DynamicActors[dyn_actorid][e_iActorID] == damaged_actorid) {
		CallLocalFunction("OnPlayerGiveDamageDynamicActor", "iifii", playerid, dyn_actorid, amount, weaponid, bodypart);
	}

	#if defined AS_OnPlayerGiveDamageActor
		return AS_OnPlayerGiveDamageActor(playerid, damaged_actorid, amount, weaponid, bodypart);
	#else
		return 1;
	#endif
}

#if defined _ALS_OnFilterScriptExit
	#undef OnFilterScriptExit
#else
	#define _ALS_OnFilterScriptExit
#endif

#define OnFilterScriptExit AS_OnFilterScriptExit

#if defined AS_OnFilterScriptExit
	forward AS_OnFilterScriptExit();
#endif

#if defined _ALS_OnGameModeExit
	#undef OnGameModeExit
#else
	#define _ALS_OnGameModeExit
#endif

#define OnGameModeExit AS_OnGameModeExit

#if defined AS_OnGameModeExit
	forward AS_OnGameModeExit();
#endif

#if defined _ALS_OnActorStreamIn
	#undef OnActorStreamIn
#else
	#define _ALS_OnActorStreamIn
#endif

#define OnActorStreamIn AS_OnActorStreamIn

#if defined AS_OnActorStreamIn
	forward AS_OnActorStreamIn(actorid, forplayerid);
#endif

#if defined _ALS_OnActorStreamOut
	#undef OnActorStreamOut
#else
	#define _ALS_OnActorStreamOut
#endif

#define OnActorStreamOut AS_OnActorStreamOut

#if defined AS_OnActorStreamOut
	forward AS_OnActorStreamOut(actorid, forplayerid);
#endif

#if defined _ALS_OnPlayerGiveDamageActor
	#undef OnPlayerGiveDamageActor
#else
	#define _ALS_OnPlayerGiveDamageActor
#endif

#define OnPlayerGiveDamageActor AS_OnPlayerGiveDamageActor

#if defined AS_OnPlayerGiveDamageActor
	forward AS_OnPlayerGiveDamageActor(playerid, damaged_actorid, Float: amount, weaponid, bodypart);
#endif
