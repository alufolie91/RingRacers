// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2020 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  p_sight.c
/// \brief Line of sight/visibility checks, uses REJECT lookup table

#include "doomdef.h"
#include "doomstat.h"
#include "p_local.h"
#include "p_slopes.h"
#include "r_main.h"
#include "r_state.h"

#include "k_bot.h" // K_BotHatesThisSector
#include "k_kart.h" // K_TripwirePass

//
// P_CheckSight
//
// killough 4/19/98:
// Convert LOS info to struct for reentrancy and efficiency of data locality

typedef struct
{
	fixed_t sightzstart, t2x, t2y;		// eye z of looker
	divline_t strace;					// from t1 to t2
	fixed_t topslope, bottomslope;		// slopes to top and bottom of target
	fixed_t bbox[4];

	mobj_t *compareThing;				// Original thing
	boolean alreadyHates;				// For bot traversal, for if the bot is already in a sector it doesn't want to be
} los_t;

typedef boolean (*los_valid_t)(seg_t *, divline_t *, register los_t *);
typedef boolean (*los_valid_poly_t)(polyobj_t *, divline_t *, register los_t *);

typedef struct
{
	los_valid_t validate;				// Validation function. If true, continue iterating for possible success. If false, end early with failure.
	los_valid_poly_t validatePolyobj;	// If not NULL, then we will also check polyobject lines using this func.
} los_funcs_t;

static INT32 sightcounts[2];

//
// P_DivlineSide
//
// Returns side 0 (front), 1 (back), or 2 (on).
//
static INT32 P_DivlineSide(fixed_t x, fixed_t y, divline_t *node)
{
	fixed_t dx, dy, left, right;

	if (!node->dx)
	{
		if (x == node->x)
			return 2;

		if (x <= node->x)
			return (node->dy > 0);

		return (node->dy < 0);
	}

	if (!node->dy)
	{
		if (y == node->y)
			return 2;

		if (y <= node->y)
			return (node->dx < 0);

		return (node->dx > 0);
	}

	dx = x - node->x;
	dy = y - node->y;

	left = (node->dy>>FRACBITS) * (dx>>FRACBITS);
	right = (dy>>FRACBITS) * (node->dx>>FRACBITS);

	if (right < left)
		return 0; // front side

	if (left == right)
		return 2;

	return 1; // back side
}

//
// P_InterceptVector2
//
// Returns the fractional intercept point along the first divline.
// This is only called by the addthings and addlines traversers.
//
static fixed_t P_InterceptVector2(divline_t *v2, divline_t *v1)
{
	fixed_t frac, num, den;

	den = FixedMul(v1->dy>>8, v2->dx) - FixedMul(v1->dx>>8, v2->dy);

	if (!den)
		return 0;

	num = FixedMul((v1->x - v2->x)>>8, v1->dy) + FixedMul((v2->y - v1->y)>>8, v1->dx);
	frac = FixedDiv(num, den);

	return frac;
}

static boolean P_IsVisiblePolyObj(polyobj_t *po, divline_t *divl, register los_t *los)
{
	sector_t *polysec = po->lines[0]->backsector;
	fixed_t frac;
	fixed_t topslope, bottomslope;

	if (!(po->flags & POF_RENDERALL))
	{
		return true; // the polyobject isn't visible, so we can ignore it
	}

	// stop because it is not two sided
	/*
	if (!(po->flags & POF_TESTHEIGHT))
	{
		return false;
	}
	*/

	frac = P_InterceptVector2(&los->strace, divl);

	// get slopes of top and bottom of this polyobject line
	topslope = FixedDiv(polysec->ceilingheight - los->sightzstart , frac);
	bottomslope = FixedDiv(polysec->floorheight - los->sightzstart , frac);

	if (topslope >= los->topslope && bottomslope <= los->bottomslope)
	{
		// view completely blocked
		return false;
	}

	// TODO: figure out if it's worth considering partially blocked cases or not?
	// maybe to adjust los's top/bottom slopes if needed
	/*
	if (los->topslope <= los->bottomslope)
	{
		return false;
	}
	*/

	return true;
}

static boolean P_CrossSubsecPolyObj(polyobj_t *po, register los_t *los, register los_funcs_t *funcs)
{
	size_t i;

	for (i = 0; i < po->numLines; ++i)
	{
		line_t *line = po->lines[i];
		divline_t divl;
		const vertex_t *v1,*v2;

		// already checked other side?
		if (line->validcount == validcount)
			continue;

		line->validcount = validcount;

		// OPTIMIZE: killough 4/20/98: Added quick bounding-box rejection test
		if (line->bbox[BOXLEFT  ] > los->bbox[BOXRIGHT ] ||
			line->bbox[BOXRIGHT ] < los->bbox[BOXLEFT  ] ||
			line->bbox[BOXBOTTOM] > los->bbox[BOXTOP   ] ||
			line->bbox[BOXTOP]    < los->bbox[BOXBOTTOM])
			continue;

		v1 = line->v1;
		v2 = line->v2;

		// line isn't crossed?
		if (P_DivlineSide(v1->x, v1->y, &los->strace) ==
			P_DivlineSide(v2->x, v2->y, &los->strace))
			continue;

		divl.dx = v2->x - (divl.x = v1->x);
		divl.dy = v2->y - (divl.y = v1->y);

		// line isn't crossed?
		if (P_DivlineSide(los->strace.x, los->strace.y, &divl) ==
			P_DivlineSide(los->t2x, los->t2y, &divl))
			continue;

		if (funcs->validatePolyobj(po, &divl, los) == false)
		{
			return false;
		}
	}

	return true;
}

static boolean P_IsVisible(seg_t *seg, divline_t *divl, register los_t *los)
{
	line_t *line = seg->linedef;
	fixed_t popentop, popenbottom;
	const sector_t *front, *back;
	fixed_t frac;
	fixed_t fracx, fracy;
	fixed_t frontf, backf, frontc, backc;

	// stop because it is not two sided anyway
	if (!(line->flags & ML_TWOSIDED))
	{
		return false;
	}

	// calculate fractional intercept (how far along we are divided by how far we are from t2)
	frac = P_InterceptVector2(&los->strace, divl);

	front = seg->frontsector;
	back  = seg->backsector;
	// calculate position at intercept
	fracx = los->strace.x + FixedMul(los->strace.dx, frac);
	fracy = los->strace.y + FixedMul(los->strace.dy, frac);
	// calculate sector heights
	frontf = P_GetSectorFloorZAt  (front, fracx, fracy);
	frontc = P_GetSectorCeilingZAt(front, fracx, fracy);
	backf  = P_GetSectorFloorZAt  (back , fracx, fracy);
	backc  = P_GetSectorCeilingZAt(back , fracx, fracy);
	// crosses a two sided line
	// no wall to block sight with?
	if (frontf == backf && frontc == backc
	&& !front->ffloors & !back->ffloors) // (and no FOFs)
	{
		return true;
	}

	// possible occluder
	// because of ceiling height differences
	popentop = min(frontc, backc);

	// because of floor height differences
	popenbottom = max(frontf, backf);

	// quick test for totally closed doors
	if (popenbottom >= popentop)
	{
		return false;
	}

	if (frontf != backf)
	{
		fixed_t slope = FixedDiv(popenbottom - los->sightzstart , frac);
		if (slope > los->bottomslope)
			los->bottomslope = slope;
	}

	if (frontc != backc)
	{
		fixed_t slope = FixedDiv(popentop - los->sightzstart , frac);
		if (slope < los->topslope)
			los->topslope = slope;
	}

	if (los->topslope <= los->bottomslope)
	{
		return false;
	}

	// Monster Iestyn: check FOFs!
	if (front->ffloors || back->ffloors)
	{
		ffloor_t *rover;
		fixed_t topslope, bottomslope;
		fixed_t topz, bottomz;
		// check front sector's FOFs first
		for (rover = front->ffloors; rover; rover = rover->next)
		{
			if (!(rover->fofflags & FOF_EXISTS)
				|| !(rover->fofflags & FOF_RENDERSIDES) || (rover->fofflags & (FOF_TRANSLUCENT|FOF_FOG)))
			{
				continue;
			}

			topz    = P_GetFFloorTopZAt   (rover, fracx, fracy);
			bottomz = P_GetFFloorBottomZAt(rover, fracx, fracy);
			topslope    = FixedDiv(   topz - los->sightzstart, frac);
			bottomslope = FixedDiv(bottomz - los->sightzstart, frac);

			if (topslope >= los->topslope && bottomslope <= los->bottomslope)
			{
				return false; // view completely blocked
			}
		}
		// check back sector's FOFs as well
		for (rover = back->ffloors; rover; rover = rover->next)
		{
			if (!(rover->fofflags & FOF_EXISTS)
				|| !(rover->fofflags & FOF_RENDERSIDES) || (rover->fofflags & (FOF_TRANSLUCENT|FOF_FOG)))
			{
				continue;
			}

			topz    = P_GetFFloorTopZAt   (rover, fracx, fracy);
			bottomz = P_GetFFloorBottomZAt(rover, fracx, fracy);
			topslope    = FixedDiv(   topz - los->sightzstart, frac);
			bottomslope = FixedDiv(bottomz - los->sightzstart, frac);

			if (topslope >= los->topslope && bottomslope <= los->bottomslope)
			{
				return false; // view completely blocked
			}
		}
		// TODO: figure out if it's worth considering partially blocked cases or not?
		// maybe to adjust los's top/bottom slopes if needed
	}

	return true;
}

static boolean P_CanTraceBlockingLine(seg_t *seg, divline_t *divl, register los_t *los)
{
	line_t *line = seg->linedef;

	(void)divl;

	if (P_IsLineBlocking(line, los->compareThing) == true)
	{
		// This line will always block us
		return false;
	}

	if (los->compareThing->player != NULL)
	{
		if (P_IsLineTripWire(line) == true && K_TripwirePass(los->compareThing->player) == false)
		{
			// Can't go through trip wire.
			return false;
		}
	}

	return true;
}

static boolean P_CanBotTraverse(seg_t *seg, divline_t *divl, register los_t *los)
{
	line_t *line = seg->linedef;
	fixed_t maxstep = 0;

	if (P_CanTraceBlockingLine(seg, divl, los) == false)
	{
		// Blocked, so obviously can't traverse either.
		return false;
	}

	// set openrange, opentop, openbottom
	tm.x = los->compareThing->x;
	tm.y = los->compareThing->y;
	P_LineOpening(line, los->compareThing);
	maxstep = P_GetThingStepUp(los->compareThing, tm.x, tm.y);

	if ((openrange < los->compareThing->height) // doesn't fit
		|| (opentop - los->compareThing->z < los->compareThing->height) // mobj is too high
		|| (openbottom - los->compareThing->z > maxstep)) // too big a step up
	{
		// This line situationally blocks us
		return false;
	}

	if (los->compareThing->player != NULL && los->alreadyHates == false)
	{
		// Treat damage sectors like walls, if you're not already in a bad sector.
		vertex_t pos;
		P_ClosestPointOnLine(los->compareThing->x, los->compareThing->y, line, &pos);

		if (K_BotHatesThisSector(los->compareThing->player, line->frontsector, pos.x, pos.y)
			|| K_BotHatesThisSector(los->compareThing->player, line->backsector, pos.x, pos.y))
		{
			// This line does not block us, but we don't want to be in it.
			return false;
		}
	}

	return true;
}

static boolean P_CanWaypointTraverse(seg_t *seg, divline_t *divl, register los_t *los)
{
	line_t *line = seg->linedef;

	if (P_CanTraceBlockingLine(seg, divl, los) == false)
	{
		// Blocked, so obviously can't traverse either.
		return false;
	}

	if (line->special == 2001)
	{
		// Don't allow through the finish linedef.
		// Causes some janky behavior.
		return false;
	}

	return true;
}

//
// P_CrossSubsector
//
// Returns true if strace crosses the given subsector successfully.
//
static boolean P_CrossSubsector(size_t num, register los_t *los, register los_funcs_t *funcs)
{
	seg_t *seg;
	INT32 count;

#ifdef RANGECHECK
	if (num >= numsubsectors)
		I_Error("P_CrossSubsector: ss %s with numss = %s\n", sizeu1(num), sizeu2(numsubsectors));
#endif

	// haleyjd 02/23/06: this assignment should be after the above check
	seg = segs + subsectors[num].firstline;

	// haleyjd 02/23/06: check polyobject lines
	if (funcs->validatePolyobj != NULL)
	{
		polyobj_t *po;

		if ((po = subsectors[num].polyList))
		{
			while (po)
			{
				if (po->validcount != validcount)
				{
					po->validcount = validcount;
					if (!P_CrossSubsecPolyObj(po, los, funcs))
						return false;
				}
				po = (polyobj_t *)(po->link.next);
			}
		}
	}

	for (count = subsectors[num].numlines; --count >= 0; seg++)  // check lines
	{
		line_t *line = seg->linedef;
		const vertex_t *v1, *v2;
		divline_t divl;

		if (seg->glseg)
			continue;

		// already checked other side?
		if (line->validcount == validcount)
			continue;

		line->validcount = validcount;

		// OPTIMIZE: killough 4/20/98: Added quick bounding-box rejection test
		if (line->bbox[BOXLEFT  ] > los->bbox[BOXRIGHT ] ||
			line->bbox[BOXRIGHT ] < los->bbox[BOXLEFT  ] ||
			line->bbox[BOXBOTTOM] > los->bbox[BOXTOP   ] ||
			line->bbox[BOXTOP]    < los->bbox[BOXBOTTOM])
			continue;

		v1 = line->v1;
		v2 = line->v2;

		// line isn't crossed?
		if (P_DivlineSide(v1->x, v1->y, &los->strace) ==
			P_DivlineSide(v2->x, v2->y, &los->strace))
			continue;

		divl.dx = v2->x - (divl.x = v1->x);
		divl.dy = v2->y - (divl.y = v1->y);

		// line isn't crossed?
		if (P_DivlineSide(los->strace.x, los->strace.y, &divl) ==
			P_DivlineSide(los->t2x, los->t2y, &divl))
			continue;

		if (funcs->validate(seg, &divl, los) == false)
		{
			return false;
		}
	}

	// passed the subsector ok
	return true;
}

//
// P_CrossBSPNode
// Returns true
//  if strace crosses the given node successfully.
//
// killough 4/20/98: rewritten to remove tail recursion, clean up, and optimize

static boolean P_CrossBSPNode(INT32 bspnum, register los_t *los, register los_funcs_t *funcs)
{
	if (funcs->validate == NULL)
	{
		return false;
	}

	while (!(bspnum & NF_SUBSECTOR))
	{
		register node_t *bsp = nodes + bspnum;
		INT32 side = P_DivlineSide(los->strace.x, los->strace.y, (divline_t *)bsp) & 1;

		if (side == P_DivlineSide(los->t2x, los->t2y, (divline_t *) bsp))
		{
			bspnum = bsp->children[side]; // doesn't touch the other side
		}
		else         // the partition plane is crossed here
		{
			if (!P_CrossBSPNode(bsp->children[side], los, funcs))
				return false;  // cross the starting side
			else
				bspnum = bsp->children[side ^ 1];  // cross the ending side
		}
	}

	return P_CrossSubsector((bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR), los, funcs);
}

//
// P_CheckSight
//
// Returns true if a straight line between t1 and t2 is unobstructed.
// Uses REJECT.
//
boolean P_CheckSight(mobj_t *t1, mobj_t *t2)
{
	const sector_t *s1, *s2;
	size_t pnum;
	los_t los;
	los_funcs_t funcs;

	// First check for trivial rejection.
	if (!t1 || !t2)
		return false;

	I_Assert(!P_MobjWasRemoved(t1));
	I_Assert(!P_MobjWasRemoved(t2));

	if (!t1->subsector || !t2->subsector
	|| !t1->subsector->sector || !t2->subsector->sector)
		return false;

	s1 = t1->subsector->sector;
	s2 = t2->subsector->sector;
	pnum = (s1-sectors)*numsectors + (s2-sectors);

	if (rejectmatrix != NULL)
	{
		// Check in REJECT table.
		if (rejectmatrix[pnum>>3] & (1 << (pnum&7))) // can't possibly be connected
			return false;
	}

	// killough 11/98: shortcut for melee situations
	// same subsector? obviously visible
	// haleyjd 02/23/06: can't do this if there are polyobjects in the subsec
	if (!t1->subsector->polyList &&
		t1->subsector == t2->subsector)
		return true;

	// An unobstructed LOS is possible.
	// Now look from eyes of t1 to any part of t2.
	sightcounts[1]++;

	validcount++;

	los.compareThing = t1;
	los.alreadyHates = false;

	los.topslope =
		(los.bottomslope = t2->z - (los.sightzstart =
			t1->z + t1->height -
			(t1->height>>2))) + t2->height;
	los.strace.dx = (los.t2x = t2->x) - (los.strace.x = t1->x);
	los.strace.dy = (los.t2y = t2->y) - (los.strace.y = t1->y);

	if (t1->x > t2->x)
		los.bbox[BOXRIGHT] = t1->x, los.bbox[BOXLEFT] = t2->x;
	else
		los.bbox[BOXRIGHT] = t2->x, los.bbox[BOXLEFT] = t1->x;

	if (t1->y > t2->y)
		los.bbox[BOXTOP] = t1->y, los.bbox[BOXBOTTOM] = t2->y;
	else
		los.bbox[BOXTOP] = t2->y, los.bbox[BOXBOTTOM] = t1->y;

	// Prevent SOME cases of looking through 3dfloors
	//
	// This WILL NOT work for things like 3d stairs with monsters behind
	// them - they will still see you! TODO: Fix.
	//
	if (s1 == s2) // Both sectors are the same.
	{
		ffloor_t *rover;
		fixed_t topz1, bottomz1; // top, bottom heights at t1's position
		fixed_t topz2, bottomz2; // likewise but for t2

		for (rover = s1->ffloors; rover; rover = rover->next)
		{
			// Allow sight through water, fog, etc.
			/// \todo Improve by checking fog density/translucency
			/// and setting a sight limit.
			if (!(rover->fofflags & FOF_EXISTS)
				|| !(rover->fofflags & FOF_RENDERPLANES) /*|| (rover->fofflags & (FOF_TRANSLUCENT|FOF_FOG))*/)
			{
				continue;
			}

			topz1    = P_GetFFloorTopZAt   (rover, t1->x, t1->y);
			topz2    = P_GetFFloorTopZAt   (rover, t2->x, t2->y);
			bottomz1 = P_GetFFloorBottomZAt(rover, t1->x, t1->y);
			bottomz2 = P_GetFFloorBottomZAt(rover, t2->x, t2->y);

			// Check for blocking floors here.
			if ((los.sightzstart < bottomz1 && t2->z >= topz2)
				|| (los.sightzstart >= topz1 && t2->z + t2->height < bottomz2))
			{
				// no way to see through that
				return false;
			}

			if (rover->fofflags & FOF_SOLID)
				continue; // shortcut since neither mobj can be inside the 3dfloor

			if (rover->fofflags & FOF_BOTHPLANES || !(rover->fofflags & FOF_INVERTPLANES))
			{
				if (los.sightzstart >= topz1 && t2->z + t2->height < topz2)
					return false; // blocked by upper outside plane

				if (los.sightzstart < bottomz1 && t2->z >= bottomz2)
					return false; // blocked by lower outside plane
			}

			if (rover->fofflags & FOF_BOTHPLANES || rover->fofflags & FOF_INVERTPLANES)
			{
				if (los.sightzstart < topz1 && t2->z >= topz2)
					return false; // blocked by upper inside plane

				if (los.sightzstart >= bottomz1 && t2->z + t2->height < bottomz2)
					return false; // blocked by lower inside plane
			}
		}
	}

	funcs.validate = &P_IsVisible;
	funcs.validatePolyobj = &P_IsVisiblePolyObj;

	// the head node is the last node output
	return P_CrossBSPNode((INT32)numnodes - 1, &los, &funcs);
}

boolean P_TraceBlockingLines(mobj_t *t1, mobj_t *t2)
{
	const sector_t *s1, *s2;
	size_t pnum;
	los_t los;
	los_funcs_t funcs;

	// First check for trivial rejection.
	if (!t1 || !t2)
		return false;

	I_Assert(!P_MobjWasRemoved(t1));
	I_Assert(!P_MobjWasRemoved(t2));

	if (!t1->subsector || !t2->subsector
	|| !t1->subsector->sector || !t2->subsector->sector)
		return false;

	s1 = t1->subsector->sector;
	s2 = t2->subsector->sector;
	pnum = (s1-sectors)*numsectors + (s2-sectors);

	if (rejectmatrix != NULL)
	{
		// Check in REJECT table.
		if (rejectmatrix[pnum>>3] & (1 << (pnum&7))) // can't possibly be connected
			return false;
	}

	// killough 11/98: shortcut for melee situations
	// same subsector? obviously visible
	// haleyjd 02/23/06: can't do this if there are polyobjects in the subsec
	if (!t1->subsector->polyList &&
		t1->subsector == t2->subsector)
		return true;

	validcount++;

	los.strace.dx = (los.t2x = t2->x) - (los.strace.x = t1->x);
	los.strace.dy = (los.t2y = t2->y) - (los.strace.y = t1->y);

	if (t1->x > t2->x)
		los.bbox[BOXRIGHT] = t1->x, los.bbox[BOXLEFT] = t2->x;
	else
		los.bbox[BOXRIGHT] = t2->x, los.bbox[BOXLEFT] = t1->x;

	if (t1->y > t2->y)
		los.bbox[BOXTOP] = t1->y, los.bbox[BOXBOTTOM] = t2->y;
	else
		los.bbox[BOXTOP] = t2->y, los.bbox[BOXBOTTOM] = t1->y;

	los.compareThing = t1;
	los.alreadyHates = false;

	funcs.validate = &P_CanTraceBlockingLine;

	// the head node is the last node output
	return P_CrossBSPNode((INT32)numnodes - 1, &los, &funcs);
}

//
// ANOTHER version, this time for bot traversal.
//

boolean P_TraceBotTraversal(mobj_t *t1, mobj_t *t2)
{
	const sector_t *s1, *s2;
	size_t pnum;
	los_t los;
	los_funcs_t funcs;

	// First check for trivial rejection.
	if (!t1 || !t2)
		return false;

	I_Assert(!P_MobjWasRemoved(t1));
	I_Assert(!P_MobjWasRemoved(t2));

	if (!t1->subsector || !t2->subsector
	|| !t1->subsector->sector || !t2->subsector->sector)
		return false;

	s1 = t1->subsector->sector;
	s2 = t2->subsector->sector;
	pnum = (s1-sectors)*numsectors + (s2-sectors);

	if (rejectmatrix != NULL)
	{
		// Check in REJECT table.
		if (rejectmatrix[pnum>>3] & (1 << (pnum&7))) // can't possibly be connected
			return false;
	}

	// killough 11/98: shortcut for melee situations
	// same subsector? obviously visible
	// haleyjd 02/23/06: can't do this if there are polyobjects in the subsec
	if (!t1->subsector->polyList &&
		t1->subsector == t2->subsector)
		return true;

	validcount++;

	los.strace.dx = (los.t2x = t2->x) - (los.strace.x = t1->x);
	los.strace.dy = (los.t2y = t2->y) - (los.strace.y = t1->y);

	if (t1->x > t2->x)
		los.bbox[BOXRIGHT] = t1->x, los.bbox[BOXLEFT] = t2->x;
	else
		los.bbox[BOXRIGHT] = t2->x, los.bbox[BOXLEFT] = t1->x;

	if (t1->y > t2->y)
		los.bbox[BOXTOP] = t1->y, los.bbox[BOXBOTTOM] = t2->y;
	else
		los.bbox[BOXTOP] = t2->y, los.bbox[BOXBOTTOM] = t1->y;

	los.compareThing = t1;
	if (t1->player != NULL)
	{
		los.alreadyHates = K_BotHatesThisSector(
			t1->player, t1->subsector->sector,
			t1->x, t1->y
		);
	}
	else
	{
		los.alreadyHates = false;
	}

	funcs.validate = &P_CanBotTraverse;

	// the head node is the last node output
	return P_CrossBSPNode((INT32)numnodes - 1, &los, &funcs);
}

boolean P_TraceWaypointTraversal(mobj_t *t1, mobj_t *t2)
{
	const sector_t *s1, *s2;
	size_t pnum;
	los_t los;
	los_funcs_t funcs;

	// First check for trivial rejection.
	if (!t1 || !t2)
		return false;

	I_Assert(!P_MobjWasRemoved(t1));
	I_Assert(!P_MobjWasRemoved(t2));

	if (!t1->subsector || !t2->subsector
	|| !t1->subsector->sector || !t2->subsector->sector)
		return false;

	s1 = t1->subsector->sector;
	s2 = t2->subsector->sector;
	pnum = (s1-sectors)*numsectors + (s2-sectors);

	if (rejectmatrix != NULL)
	{
		// Check in REJECT table.
		if (rejectmatrix[pnum>>3] & (1 << (pnum&7))) // can't possibly be connected
			return false;
	}

	// killough 11/98: shortcut for melee situations
	// same subsector? obviously visible
	// haleyjd 02/23/06: can't do this if there are polyobjects in the subsec
	if (!t1->subsector->polyList &&
		t1->subsector == t2->subsector)
		return true;

	validcount++;

	los.strace.dx = (los.t2x = t2->x) - (los.strace.x = t1->x);
	los.strace.dy = (los.t2y = t2->y) - (los.strace.y = t1->y);

	if (t1->x > t2->x)
		los.bbox[BOXRIGHT] = t1->x, los.bbox[BOXLEFT] = t2->x;
	else
		los.bbox[BOXRIGHT] = t2->x, los.bbox[BOXLEFT] = t1->x;

	if (t1->y > t2->y)
		los.bbox[BOXTOP] = t1->y, los.bbox[BOXBOTTOM] = t2->y;
	else
		los.bbox[BOXTOP] = t2->y, los.bbox[BOXBOTTOM] = t1->y;

	los.compareThing = t1;
	los.alreadyHates = false;

	funcs.validate = &P_CanWaypointTraverse;

	// the head node is the last node output
	return P_CrossBSPNode((INT32)numnodes - 1, &los, &funcs);
}
