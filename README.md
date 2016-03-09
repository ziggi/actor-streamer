# ActorStreamer

Allows you to stream actors in your server.

Contributors:
- Emmet_
- Abagail
- Kar
- kvanningsen
- ziggi

# Functions

```Pawn
stock CreateDynamicActor(modelid, Float:x, Float:y, Float:z, Float:angle, worldid = -1, interiorid = -1, bool:invulnerable = true, Float:health = 100.0);
stock DestroyDynamicActor(actorid);

stock bool:IsDynamicActorStreamedIn(actorid, forplayerid);
stock bool:IsDynamicActorInStreamRange(actorid, forplayerid);
stock GetDynamicActorInternalID(actorid);

stock SetDynamicActorVirtualWorld(actorid, vworld);
stock GetDynamicActorVirtualWorld(actorid);

stock SetDynamicActorInterior(actorid, interior);
stock GetDynamicActorInterior(actorid);

stock SetDynamicActorSkin(actorid, skinid);
stock GetDynamicActorSkin(actorid);

stock ApplyDynamicActorAnimation(actorid, animlib[], animname[], Float:fDelta, loop, lockx, locky, freeze, time);
stock ClearDynamicActorAnimations(actorid);

stock SetDynamicActorPos(actorid, Float:x, Float:y, Float:z);
stock GetDynamicActorPos(actorid, &Float:x, &Float:y, &Float:z);

stock SetDynamicActorFacingAngle(actorid, Float:angle);
stock GetDynamicActorFacingAngle(actorid, &Float:angle);

stock SetDynamicActorHealth(actorid, Float:health);
stock GetDynamicActorHealth(actorid, &Float:health);

stock SetDynamicActorInvulnerable(actorid, bool:invulnerable = true);
stock bool:IsDynamicActorInvulnerable(actorid);
```

# Callbacks

```Pawn
forward OnDynamicActorStreamIn(actorid, forplayerid);
forward OnDynamicActorStreamOut(actorid, forplayerid);
forward OnPlayerGiveDamageDynamicActor(playerid, actorid, Float:amount, weaponid, bodypart);
```
