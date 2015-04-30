/*
 *  Actor streamer by Emmet
 *  Created on April 30, 2015 @ 7:21 AM.
*/

// Maximum amount of allocated slots.
#if !defined MAX_DYNAMIC_ACTORS
	#define MAX_DYNAMIC_ACTORS      2000
#endif

// Maximum stream distance.
#if !defined MAX_ACTOR_STREAM_DISTANCE
	#define MAX_ACTOR_STREAM_DISTANCE   200.0
#endif

// Tick rate. Lower values mean more server power!
#if !defined ACTOR_STREAM_TICK_RATE
	#define ACTOR_STREAM_TICK_RATE      500
#endif

enum e_DynamicActors
{
	e_iSlotUsed,
	e_iActorModel,
	Float:e_fActorX,
	Float:e_fActorY,
	Float:e_fActorZ,
	Float:e_fActorA,
	e_iActorVirtual,
	e_iActorInterior,
	e_iActorTimer,
	e_iActorID
};

enum e_AnimationInfo
{
	bool:e_bAnimation,
	e_sAnimLibrary[32 char],
	e_sAnimName[32 char],
	Float:e_fAnimDelta,
	e_iAnimLockX,
	e_iAnimLockY,
	e_iAnimFreeze,
	e_iAnimTime
};

static
	s_DynamicActors[MAX_DYNAMIC_ACTORS][e_DynamicActors],
	s_ActorAnimation[MAX_DYNAMIC_ACTORS][e_AnimationInfo]
;

public OnFilterScriptExit()
{
	for (new i = 0; i < MAX_DYNAMIC_ACTORS; i ++)
	{
		if (s_DynamicActors[i][e_iSlotUsed])
		{
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
	for (new i = 0; i < MAX_DYNAMIC_ACTORS; i ++)
	{
		if (s_DynamicActors[i][e_iSlotUsed])
		{
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

stock CreateDynamicActor(modelid, Float:x, Float:y, Float:z, Float:angle, worldid = -1, interiorid = -1)
{
	for (new i = 0; i < MAX_DYNAMIC_ACTORS; i ++)
	{
		if (!s_DynamicActors[i][e_iSlotUsed])
		{
		    s_DynamicActors[i][e_iSlotUsed] = 1;
		    s_DynamicActors[i][e_iActorModel] = modelid;
		    s_DynamicActors[i][e_fActorX] = x;
			s_DynamicActors[i][e_fActorY] = y;
		    s_DynamicActors[i][e_fActorZ] = z;
			s_DynamicActors[i][e_fActorA] = angle;
		    s_DynamicActors[i][e_iActorVirtual] = worldid;
		    s_DynamicActors[i][e_iActorInterior] = interiorid;
		    s_DynamicActors[i][e_iActorID] = INVALID_ACTOR_ID;
		    
		    // Timer used for update checking.
		    s_DynamicActors[i][e_iActorTimer] = SetTimerEx("OnActorStreamUpdate", ACTOR_STREAM_TICK_RATE, true, "i", i);
		    return i;
		}
	}
	return INVALID_ACTOR_ID;
}

stock IsValidDynamicActor(actorid)
{
    if (!(0 <= actorid < MAX_DYNAMIC_ACTORS) || (!s_DynamicActors[actorid][e_iSlotUsed]))
	{
	    return 0;
	}
	return 1;
}

stock DestroyDynamicActor(actorid)
{
	if (IsValidDynamicActor(actorid))
	{
		if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
		{
		    // Destroy the actor if it is streamed.
		    DestroyActor(s_DynamicActors[actorid][e_iActorID]);
		}
		// Clear the animation info.
		ClearDynamicActorAnimations(actorid);
		
		// Free up the slot.
		s_DynamicActors[actorid][e_iSlotUsed] = 0;
		return 1;
	}
	return 0;
}

stock IsDynamicActorStreamedIn(actorid, forplayerid)
{
	if (IsValidDynamicActor(actorid))
	{
	    // Checks if the player is in range of the actor's position.
	    if (IsPlayerInRangeOfPoint(forplayerid, MAX_ACTOR_STREAM_DISTANCE, s_DynamicActors[actorid][e_fActorX], s_DynamicActors[actorid][e_fActorY], s_DynamicActors[actorid][e_fActorZ]))
		{
		    // An additional check for interiors and virtual worlds.
		    if ((s_DynamicActors[actorid][e_iActorInterior] != -1 && GetPlayerInterior(forplayerid) != s_DynamicActors[actorid][e_iActorInterior]) || (s_DynamicActors[actorid][e_iActorVirtual] != -1 && GetPlayerVirtualWorld(forplayerid) != s_DynamicActors[actorid][e_iActorVirtual]))
		    {
		        return 0;
			}
			return 1;
		}
	}
	return 0;
}

stock SetDynamicActorVirtualWorld(actorid, worldid)
{
	if (IsValidDynamicActor(actorid))
	{
	    s_DynamicActors[actorid][e_iActorVirtual] = worldid;
	    
	    if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
	    {
	        // If the actor is streamed, update their virtual world.
	        SetActorVirtualWorld(s_DynamicACtors[actorid][e_iActorID], worldid);
		}
	    return 1;
	}
	return 0;
}

stock GetDynamicActorVirtualWorld(actorid)
{
	if (IsValidDynamicActor(actorid))
	{
	    return s_DynamicActors[actorid][e_iActorVirtual];
	}
	return 0;
}

stock SetDynamicActorInterior(actorid, interiorid)
{
	if (IsValidDynamicActor(actorid))
	{
	    s_DynamicActors[actorid][e_iActorInterior] = interiorid;
	    return 1;
	}
	return 0;
}

stock GetDynamicActorInterior(actorid)
{
	if (IsValidDynamicActor(actorid))
	{
	    return s_DynamicActors[actorid][e_iActorInterior];
	}
	return 0;
}

stock ApplyDynamicActorAnimation(actorid, animlib[], animname[], Float:fDelta, loop, lockx, locky, freeze, time)
{
	if (IsValidDynamicActor(actorid))
	{
	    if (loop)
	    {
	        // We want to save the looping animation so it plays when a player streams in.
			strpack(s_ActorAnimation[actorid][e_sAnimLibrary], animlib, 32);
			strpack(s_ActorAnimation[actorid][e_sAnimName], animname, 32);
		
			s_ActorAnimation[actorid][e_bAnimation] = true;
			s_ActorAnimation[actorid][e_fAnimDelta] = fDelta;
			s_ActorAnimation[actorid][e_iAnimLockX] = lockx;
			s_ActorAnimation[actorid][e_iAnimLockY] = locky;
			s_ActorAnimation[actorid][e_iAnimFreeze] = freeze;
			s_ActorAnimation[actorid][e_iAnimTime] = time;
		}
		if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
		{
			// If the actor is streamed, apply the animation.
			ApplyActorAnimation(s_DynamicActors[actorid][e_iActorID], animlib, animname, fDelta, loop, lockx, locky, freeze, time);
		}
		return 1;
	}
	return 0;
}

stock ClearDynamicActorAnimations(actorid)
{
	if (IsValidDynamicActor(actorid))
	{
		if (s_ActorAnimation[actorid][e_bAnimation])
		{
		    // Reset the animation info.
		    s_ActorAnimation[actorid][e_bAnimation] = false;
		    s_ActorAnimation[actorid][e_fAnimDelta] = 0.0;
		    s_ActorAnimation[actorid][e_iAnimLockX] = 0;
		    s_ActorAnimation[actorid][e_iAnimLockY] = 0;
		    s_ActorAnimation[actorid][e_iAnimFreeze] = 0;
		    s_ActorAnimation[actorid][e_iAnimTime] = 0;
		}
		if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
		{
		    // If the actor is streamed, clear the animations.
		    ClearActorAnimations(s_DynamicActors[actorid][e_iActorID]);
		}
		return 1;
	}
	return 0;
}

stock SetDynamicActorPos(actorid, Float:x, Float:y, Float:z)
{
	if (IsValidDynamicActor(actorid))
	{
	    s_DynamicActors[actorid][e_fActorX] = x;
	    s_DynamicActors[actorid][e_fActorY] = y;
	    s_DynamicActors[actorid][e_fActorZ] = z;
	    
	    if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
	    {
	        // If the actor is streamed, update the position.
	        SetActorPos(s_DynamicActors[actorid][e_iActorID], x, y, z);
		}
	    return 1;
	}
	return 0;
}

stock GetDynamicActorPos(actorid, &Float:x, &Float:y, &Float:z)
{
	if (IsValidDynamicActor(actorid))
	{
	    x = s_DynamicActors[actorid][e_fActorX];
	    y = s_DynamicActors[actorid][e_fActorY];
	    z = s_DynamicActors[actorid][e_fActorZ];
	    return 1;
	}
	x = 0.0;
	y = 0.0;
	z = 0.0;
	return 0;
}

stock SetDynamicActorFacingAngle(actorid, Float:angle)
{
	if (IsValidDynamicActor(actorid))
	{
	    s_DynamicActors[actorid][e_fActorA] = angle;
	    
	    if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
	    {
	        // If the actor is streamed, update the angle.
	        SetActorFacingAngle(s_DynamicActors[actorid][e_iActorID], angle);
		}
	    return 1;
	}
	return 0;
}

stock GetDynamicActorFacingAngle(actorid, &Float:angle)
{
	if (IsValidDynamicActor(actorid))
	{
	    angle = s_DynamicActors[actorid][e_fActorA];
	    return 1;
	}
	angle = 0.0;
	return 0;
}

stock SetDynamicActorSkin(actorid, skinid)
{
	if (IsValidDynamicActor(actorid))
	{
	    s_DynamicActors[actorid][e_iActorModel] = actorid;
	    
	    if (s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
	    {
	        // Re-stream the actor for the changes to occur.
	        DestroyActor(s_DynamicActors[actorid][e_iActorID]);
	        
			s_DynamicActors[actorid][e_iActorID] = INVALID_ACTOR_ID;
	        
	        OnActorStreamUpdate(actorid);
		}
		return 1;
	}
	return 0;
}

stock GetDynamicActorSkin(actorid)
{
	if (IsValidDynamicActor(actorid))
	{
	    return s_DynamicActors[actorid][e_iActorModel];
	}
	return 0;
}

forward OnActorStreamUpdate(actorid);
public OnActorStreamUpdate(actorid)
{
	new
	    bool:bStreamed = false;

	#if defined _inc_foreach
	foreach (new i : Player)
	#else
	for (new i = 0; i <= GetPlayerPoolSize(); i ++)
	#endif
	{
	    // If the player is close to the actor...
		if (IsDynamicActorStreamedIn(actorid, i))
		{
		    // Indicate that this actor is streamed in.
			bStreamed = true;
			
			// If the actor is not streamed yet, create it.
   			if (s_DynamicActors[actorid][e_iActorID] == INVALID_ACTOR_ID)
			{
                s_DynamicActors[actorid][e_iActorID] = CreateActor(s_DynamicActors[actorid][e_iActorModel], s_DynamicActors[actorid][e_fActorX], s_DynamicActors[actorid][e_fActorY], s_DynamicActors[actorid][e_fActorZ], s_DynamicActors[actorid][e_fActorA]);

                if (s_DynamicActors[actorid][e_iActorVirtual] != -1)
				{
				    // Applies the virtual world.
					SetActorVirtualWorld(s_DynamicActors[actorid][e_iActorID], s_DynamicActors[actorid][e_iActorVirtual]);
                }
                if (s_ActorAnimation[actorid][e_bAnimation])
                {
                    // Applies the saved animation data.
                    ApplyActorAnimation(s_DynamicActors[actorid][e_iActorID], s_ActorAnimation[actorid][e_sAnimLibrary], s_ActorAnimation[actorid][e_sAnimName], s_ActorAnimation[actorid][e_fAnimDelta], true, s_ActorAnimation[actorid][e_iAnimLockX], s_ActorAnimation[actorid][e_iAnimLockY], s_ActorAnimation[actorid][e_iAnimFreeze], s_ActorAnimation[actorid][e_iAnimTime]);
				}
		    }
		    break;
		}
	}
	
	if (!bStreamed && s_DynamicActors[actorid][e_iActorID] != INVALID_ACTOR_ID)
	{
	    // The actor is no longer streamed in for any players... so delete it then.
	    DestroyActor(s_DynamicActors[actorid][e_iActorID]);
	    
	    s_DynamicActors[actorid][e_iActorID] = INVALID_ACTOR_ID;
	}
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
