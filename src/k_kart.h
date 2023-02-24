// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.h
/// \brief SRB2kart stuff.

#ifndef __K_KART__
#define __K_KART__

#include "doomdef.h"
#include "d_player.h" // Need for player_t
#include "command.h" // Need for player_t

#ifdef __cplusplus
extern "C" {
#endif

#define KART_FULLTURN 800

/*
Number of animations for the invincibility sparkles
If states are ever added or removed
Make sure this matches the actual number of states
*/
#define KART_NUMINVSPARKLESANIM 12

#define MAXHITLAGTICS 18 //12
#define HITLAGJITTERS (FRACUNIT / 20)

#define GROW_SCALE (2*FRACUNIT)
#define SHRINK_SCALE (FRACUNIT/2)

#define GROW_PHYSICS_SCALE (3*FRACUNIT/2)
#define SHRINK_PHYSICS_SCALE (3*FRACUNIT/4)

#define RR_PROJECTILE_FUSE (8*TICRATE)

#define STUMBLE_STEEP_VAL ANG60
#define STUMBLE_STEEP_VAL_AIR (ANG30 + ANG10)

boolean K_PodiumSequence(void);

player_t *K_GetItemBoxPlayer(mobj_t *mobj);
angle_t K_ReflectAngle(angle_t angle, angle_t against, fixed_t maxspeed, fixed_t yourspeed);

void K_RegisterKartStuff(void);

boolean K_IsDuelItem(mobjtype_t type);
boolean K_DuelItemAlwaysSpawns(mapthing_t *mt);

void K_TimerReset(void);
void K_TimerInit(void);

UINT32 K_GetPlayerDontDrawFlag(player_t *player);
void K_ReduceVFX(mobj_t *mo, player_t *owner);

boolean K_IsPlayerLosing(player_t *player);
fixed_t K_GetKartGameSpeedScalar(SINT8 value);

INT32 K_GetShieldFromItem(INT32 item);
SINT8 K_ItemResultToType(SINT8 getitem);
UINT8 K_ItemResultToAmount(SINT8 getitem);
tic_t K_GetItemCooldown(SINT8 itemResult);
void K_SetItemCooldown(SINT8 itemResult, tic_t time);
void K_RunItemCooldowns(void);

boolean K_TimeAttackRules(void);
boolean K_CapsuleTimeAttackRules(void);

fixed_t K_GetMobjWeight(mobj_t *mobj, mobj_t *against);
boolean K_KartBouncing(mobj_t *mobj1, mobj_t *mobj2);
boolean K_KartSolidBounce(mobj_t *bounceMobj, mobj_t *solidMobj);
void K_KartPainEnergyFling(player_t *player);
void K_FlipFromObject(mobj_t *mo, mobj_t *master);
void K_MatchGenericExtraFlags(mobj_t *mo, mobj_t *master);
void K_GenericExtraFlagsNoZAdjust(mobj_t *mo, mobj_t *master);
void K_SpawnDashDustRelease(player_t *player);
void K_SpawnDriftBoostClip(player_t *player);
void K_SpawnDriftBoostClipSpark(mobj_t *clip);
void K_SpawnNormalSpeedLines(player_t *player);
void K_SpawnGardenTopSpeedLines(player_t *player);
void K_SpawnInvincibilitySpeedLines(mobj_t *mo);
void K_SpawnBumpEffect(mobj_t *mo);
void K_KartMoveAnimation(player_t *player);
void K_KartPlayerHUDUpdate(player_t *player);
void K_KartResetPlayerColor(player_t *player);
void K_KartPlayerThink(player_t *player, ticcmd_t *cmd);
void K_KartPlayerAfterThink(player_t *player);
angle_t K_MomentumAngle(mobj_t *mo);
void K_AddHitLag(mobj_t *mo, INT32 tics, boolean fromDamage);
void K_SetHitLagForObjects(mobj_t *mo1, mobj_t *mo2, INT32 tics, boolean fromDamage);
void K_AwardPlayerRings(player_t *player, INT32 rings, boolean overload);
void K_DoInstashield(player_t *player);
void K_DoPowerClash(player_t *t1, player_t *t2);
void K_BattleAwardHit(player_t *player, player_t *victim, mobj_t *inflictor, UINT8 bumpersRemoved);
void K_RemoveGrowShrink(player_t *player);
boolean K_IsBigger(mobj_t *compare, mobj_t *other);
void K_SpinPlayer(player_t *player, mobj_t *inflictor, mobj_t *source, INT32 type);
void K_TumblePlayer(player_t *player, mobj_t *inflictor, mobj_t *source);
void K_TumbleInterrupt(player_t *player);
angle_t K_StumbleSlope(angle_t angle, angle_t pitch, angle_t roll);
void K_StumblePlayer(player_t *player);
boolean K_CheckStumble(player_t *player, angle_t oldPitch, angle_t oldRoll, boolean fromAir);
void K_InitStumbleIndicator(player_t *player);
void K_UpdateStumbleIndicator(player_t *player);
INT32 K_ExplodePlayer(player_t *player, mobj_t *inflictor, mobj_t *source);
void K_DebtStingPlayer(player_t *player, mobj_t *source);
void K_HandleBumperChanges(player_t *player, UINT8 prevBumpers);
void K_DestroyBumpers(player_t *player, UINT8 amount);
void K_TakeBumpersFromPlayer(player_t *player, player_t *victim, UINT8 amount);
void K_MineFlashScreen(mobj_t *source);
void K_SpawnMineExplosion(mobj_t *source, UINT8 color, tic_t delay);
void K_RunFinishLineBeam(void);
UINT16 K_DriftSparkColor(player_t *player, INT32 charge);
void K_SpawnBoostTrail(player_t *player);
void K_SpawnSparkleTrail(mobj_t *mo);
void K_SpawnWipeoutTrail(mobj_t *mo);
void K_SpawnDraftDust(mobj_t *mo);
void K_SpawnMagicianParticles(mobj_t *mo, int spread);
void K_DriftDustHandling(mobj_t *spawner);
void K_Squish(mobj_t *mo);
mobj_t *K_ThrowKartItem(player_t *player, boolean missile, mobjtype_t mapthing, INT32 defaultDir, INT32 altthrow, angle_t angleOffset);
void K_PuntMine(mobj_t *mine, mobj_t *punter);
void K_DoSneaker(player_t *player, INT32 type);
void K_DoPogoSpring(mobj_t *mo, fixed_t vertispeed, UINT8 sound);
void K_DoInvincibility(player_t *player, tic_t time);
void K_KillBananaChain(mobj_t *banana, mobj_t *inflictor, mobj_t *source);
void K_UpdateHnextList(player_t *player, boolean clean);
void K_DropHnextList(player_t *player, boolean keepshields);
void K_RepairOrbitChain(mobj_t *orbit);
void K_CalculateBananaSlope(mobj_t *mobj, fixed_t x, fixed_t y, fixed_t z, fixed_t radius, fixed_t height, boolean flip, boolean player);
mobj_t *K_FindJawzTarget(mobj_t *actor, player_t *source, angle_t range);
INT32 K_GetKartRingPower(player_t *player, boolean boosted);
void K_UpdateDistanceFromFinishLine(player_t *const player);
boolean K_CheckPlayersRespawnColliding(INT32 playernum, fixed_t x, fixed_t y);
INT16 K_UpdateSteeringValue(INT16 inputSteering, INT16 destSteering);
INT16 K_GetKartTurnValue(player_t *player, INT16 turnvalue);
INT32 K_GetUnderwaterTurnAdjust(player_t *player);
INT32 K_GetKartDriftSparkValue(player_t *player);
INT32 K_StairJankFlip(INT32 value);
INT32 K_GetKartDriftSparkValueForStage(player_t *player, UINT8 stage);
void K_SpawnDriftBoostExplosion(player_t *player, int stage);
void K_SpawnDriftElectricSparks(player_t *player, int color, boolean shockwave);
void K_KartUpdatePosition(player_t *player);
SINT8 K_GetTotallyRandomResult(UINT8 useodds);
mobj_t *K_CreatePaperItem(fixed_t x, fixed_t y, fixed_t z, angle_t angle, SINT8 flip, UINT8 type, UINT8 amount);
void K_DropItems(player_t *player);
void K_DropRocketSneaker(player_t *player);
void K_DropKitchenSink(player_t *player);
void K_StripItems(player_t *player);
void K_StripOther(player_t *player);
void K_MomentumToFacing(player_t *player);
boolean K_ApplyOffroad(player_t *player);
boolean K_SlopeResistance(player_t *player);
tripwirepass_t K_TripwirePassConditions(player_t *player);
boolean K_TripwirePass(player_t *player);
boolean K_MovingHorizontally(mobj_t *mobj);
boolean K_WaterRun(mobj_t *mobj);
boolean K_WaterSkip(mobj_t *mobj);
void K_SpawnWaterRunParticles(mobj_t *mobj);
boolean K_IsRidingFloatingTop(player_t *player);
boolean K_IsHoldingDownTop(player_t *player);
mobj_t *K_GetGardenTop(player_t *player);
void K_ApplyTripWire(player_t *player, tripwirestate_t state);
INT16 K_GetSpindashChargeTime(player_t *player);
fixed_t K_GetSpindashChargeSpeed(player_t *player);
fixed_t K_GrowShrinkSpeedMul(player_t *player);
fixed_t K_GetKartSpeedFromStat(UINT8 kartspeed);
fixed_t K_GetKartSpeed(player_t *player, boolean doboostpower, boolean dorubberbanding);
fixed_t K_GetKartAccel(player_t *player);
UINT16 K_GetKartFlashing(player_t *player);
boolean K_PlayerShrinkCheat(player_t *player);
void K_UpdateShrinkCheat(player_t *player);
boolean K_KartKickstart(player_t *player);
UINT16 K_GetKartButtons(player_t *player);
SINT8 K_GetForwardMove(player_t *player);
fixed_t K_GetNewSpeed(player_t *player);
fixed_t K_3dKartMovement(player_t *player);
boolean K_PlayerEBrake(player_t *player);
SINT8 K_Sliptiding(player_t *player);
boolean K_FastFallBounce(player_t *player);
fixed_t K_PlayerBaseFriction(fixed_t original);
void K_AdjustPlayerFriction(player_t *player);
void K_MoveKartPlayer(player_t *player, boolean onground);
void K_CheckSpectateStatus(void);
UINT8 K_GetInvincibilityItemFrame(void);
UINT8 K_GetOrbinautItemFrame(UINT8 count);
boolean K_IsSPBInGame(void);
void K_KartEbrakeVisuals(player_t *p);
void K_HandleDirectionalInfluence(player_t *player);
fixed_t K_DefaultPlayerRadius(player_t *player);

// sound stuff for lua
void K_PlayAttackTaunt(mobj_t *source);
void K_PlayBoostTaunt(mobj_t *source);
void K_PlayOvertakeSound(mobj_t *source);
void K_PlayPainSound(mobj_t *source, mobj_t *other);
void K_PlayHitEmSound(mobj_t *source, mobj_t *other);
void K_TryHurtSoundExchange(mobj_t *victim, mobj_t *attacker);
void K_PlayPowerGloatSound(mobj_t *source);

fixed_t K_ItemScaleForPlayer(player_t *player);
void K_SetItemOut(player_t *player);
void K_UnsetItemOut(player_t *player);

void K_UpdateMobjItemOverlay(mobj_t *part, SINT8 itemType, UINT8 itemCount);

void K_EggmanTransfer(player_t *source, player_t *victim);

tic_t K_TimeLimitForGametype(void);
UINT32 K_PointLimitForGametype(void);

#ifdef __cplusplus
} // extern "C"
#endif

// =========================================================================
#endif  // __K_KART__
