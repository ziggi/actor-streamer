# actor-streamer

```
stock CreateDynamicActor(modelid, Float:x, Float:y, Float:z, Float:angle, worldid = -1, interiorid = -1);
stock DestroyDynamicActor(actorid);

stock IsDynamicActorStreamedIn(actorid, forplayerid);
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

native SetDynamicActorFacingAngle(actorid, Float:angle);
native GetDynamicActorFacingAngle(actorid, &Float:angle);

stock SetDynamicActorHealth(actorid, Float:health);
stock GetDynamicActorHealth(actorid, &Float:health);

stock SetDynamicActorInvulnerable(actorid, bool:invulnerable = true);
stock IsDynamicActorInvulnerable(actorid);
```

```
forward OnDynamicActorStreamIn(actorid, forplayerid);
forward OnDynamicActorStreamOut(actorid, forplayerid);
forward OnPlayerGiveDamageDynamicActor(playerid, actorid, Float:amount, weaponid, bodypart);
```
