/*********************************************************************
This file is a part of FItsSec project: Implementation of ETSI TS 103 097
Copyright (C) 2015  Denis Filatov (danya.filatov()gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed under GNU GPLv3 in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Fitsec.  If not, see <http://www.gnu.org/licenses/lgpl-3.0.txt>.
@license LGPL-3.0+ <http://www.gnu.org/licenses/lgpl-3.0.txt>

In particular cases this program can be distributed under other license
by simple request to the author.
*********************************************************************/

#include "fitsec_i.h"
#include "cserialize.h"
#include "cstr.h"
#include <errno.h>
#include <math.h>

#include "fitsec_unstats.h"

static uint32_t _TwoDLocation_Distance(const TwoDLocation * l, const TwoDLocation * p);
static void FN_THROW(RuntimeException) _TwoDLocation_ReadArray(TwoDLocation * points, int * pcount, const char** const ptr, const char* const end, int * const p_error);

static FSBOOL      _Circle_IsPointInside(const TwoDLocation * center, int radius, const TwoDLocation * l);
static FSBOOL      _Polygon_IsPointInside(const TwoDLocation * points, int rcount, const TwoDLocation * l);
static FSBOOL      _Polygon_IsCircleInside(const TwoDLocation * points, int rcount, const TwoDLocation * center, int radius);
static FSBOOL      _Rectangle_IsPointInside(const TwoDLocation * points, int rcount, const TwoDLocation * l);
static int         _Rectangular_ConvertToPolygon(const TwoDLocation * rpoints, int rcount, TwoDLocation * points, int pcount);
static FSBOOL      _Identified_IsPointInside(uint16_t identifier, int local, const TwoDLocation * l);
static FSBOOL	   _IdentifiedRegion_IsSubRegion(int parent, int child);

void FN_THROW(RuntimeException) GeographicRegion_Read(GeographicRegion * r, const char** const ptr, const char* const end, int * const perror)
{
	e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_REGION | FSERR_TYPE;
	r->type = cint8_read(ptr, end, perror);
	switch (r->type){
	case REGION_NONE:
		break;
	case REGION_CIRCLE:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_REGION | FSERR_POLYGON | FSERR_TWODLOCATION;
		TwoDLocation_Read(&r->u.circular.center, ptr, end, perror);
		r->u.circular.radius = cint16_read(ptr, end, perror);
		break;
	case REGION_RECTANGLE:
	case REGION_POLYGON:
		r->u.polygonal.count = 12; // max count
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_REGION | FSERR_POLYGON;
		_TwoDLocation_ReadArray(r->u.polygonal.points, &r->u.polygonal.count, ptr, end, perror);
		if (r->type == REGION_RECTANGLE) {
			r->u.rectangular.count = r->u.polygonal.count / 2;
		}
		break;
	case REGION_ID:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_REGION | FSERR_IDENTIFIED;
		r->u.identified.dictionary = cint8_read(ptr, end, perror);
		r->u.identified.identifier = cint16_read(ptr, end, perror);
		r->u.identified.local = (int)cintx_read(ptr, end, perror);
		break;
	default:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_REGION;
		cstr_read(NULL, ptr, end, perror); // skip it
	}
}


static void FN_THROW(RuntimeException) _TwoDLocation_ReadArray(TwoDLocation * points, int * pcount, const char** const ptr, const char* const end, int * const perror)
{
	int len;
	unsigned int maxcount, i;
	e4c.err.err = FSERR_SET_ELEMENT(e4c.err.err, FSERR_SIZE);
	len = cxsize_read(ptr, end, perror);
	const char * re = *ptr + len;
	if (re > end){
		throw(RuntimeException, FSERR_PARSEERROR, NULL);
	}
	maxcount = (unsigned int)*pcount;
	e4c.err.err = FSERR_SET_ELEMENT(e4c.err.err, FSERR_TWODLOCATION);
	for (i = 0; i < maxcount && *ptr < re; i++){
		TwoDLocation_Read(&points[i], ptr, end, perror);
	}
	e4c.err.err = FSERR_SET_ELEMENT(e4c.err.err, 0);
	if (*ptr < re){
		throw(RuntimeException, FSERR_NOSPACE, NULL);
	}
}

FSBOOL GeographicRegion_IsRegionInside(const GeographicRegion * r, const GeographicRegion * s)
{
	int i;
	if (r->type == REGION_NONE) return FSTRUE;
	if (s->type == REGION_NONE) return FSFALSE;
	switch (r->type){
	case REGION_CIRCLE:
		switch (s->type){
		case REGION_CIRCLE:
			return (_TwoDLocation_Distance(&r->u.circular.center, &s->u.circular.center) + s->u.circular.radius <= r->u.circular.radius) ? 1 : 0;
		case REGION_RECTANGLE:
			for (i = 0; i < s->u.rectangular.count; i++){
				TwoDLocation tmp;
				if (_TwoDLocation_Distance(&r->u.circular.center, &s->u.rectangular.r[i].nw) > r->u.circular.radius){
					return FSFALSE;
				}
				if (_TwoDLocation_Distance(&r->u.circular.center, &s->u.rectangular.r[i].se) > r->u.circular.radius){
					return FSFALSE;
				}
				tmp.latitude = s->u.rectangular.r[i].nw.latitude;
				tmp.longitude = s->u.rectangular.r[i].se.latitude;
				if (_TwoDLocation_Distance(&r->u.circular.center, &tmp) > r->u.circular.radius){
					return FSFALSE;
				}
				tmp.latitude = s->u.rectangular.r[i].se.latitude;
				tmp.longitude = s->u.rectangular.r[i].nw.latitude;
				if (_TwoDLocation_Distance(&r->u.circular.center, &tmp) > r->u.circular.radius){
					return FSFALSE;
				}
			}
			return FSTRUE;
		case REGION_POLYGON:
			for (i = 0; i < s->u.polygonal.count; i++){
				if (_TwoDLocation_Distance(&r->u.circular.center, &s->u.polygonal.points[i]) > r->u.circular.radius){
					return FSFALSE;
				}
			}
			return FSTRUE;
		case REGION_ID:
			// TODO:
		default:
			return FSFALSE;
		}
		break;
	case REGION_RECTANGLE:
		switch (s->type){
		case REGION_CIRCLE:
		{
			TwoDLocation p[24];
			int          pcount;
			pcount = _Rectangular_ConvertToPolygon(r->u.polygonal.points, r->u.rectangular.count, p, sizeof(p) / sizeof(p[0]));
			return (pcount > 4 && _Polygon_IsCircleInside(p, pcount, &s->u.circular.center, s->u.circular.radius)) ? FSTRUE : FSFALSE;
		}
		case REGION_RECTANGLE:
		case REGION_POLYGON:
			{
				// check that each point of s is inside in at least one of the rectangles of r
				int i, m;
				m = r->u.polygonal.count;
				if (s->type == REGION_RECTANGLE) m <<= 1;
				for (i = 0; i < m; i++) {
					if (0 == _Rectangle_IsPointInside(r->u.polygonal.points, r->u.rectangular.count, &s->u.polygonal.points[i])){
						return FSFALSE;
					}
				}
			}
			return FSTRUE;
		case REGION_ID:
			//TODO: 
		default:
			return FSFALSE;
		}
	case REGION_POLYGON:
		switch (s->type){
		case REGION_CIRCLE:
			return _Polygon_IsCircleInside(r->u.polygonal.points, r->u.polygonal.count, &s->u.circular.center, s->u.circular.radius);
		case REGION_RECTANGLE:
			for (i = 0; i < s->u.rectangular.count; i++){
				TwoDLocation tmp;
				if (0 == _Polygon_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, &s->u.rectangular.r[i].nw))
					return FSFALSE;
				if (0 == _Polygon_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, &s->u.rectangular.r[i].se))
					return FSFALSE;
				tmp.latitude = s->u.rectangular.r[i].nw.latitude;
				tmp.longitude = s->u.rectangular.r[i].se.latitude;
				if (0 == _Polygon_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, &tmp))
					return FSFALSE;
				tmp.latitude = s->u.rectangular.r[i].se.latitude;
				tmp.longitude = s->u.rectangular.r[i].nw.latitude;
				if (0 == _Polygon_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, &tmp))
					return FSFALSE;
			}
			return FSTRUE;
		case REGION_POLYGON:
			for (i = 0; i < s->u.polygonal.count; i++){
				if (0 == _Polygon_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, &s->u.polygonal.points[i]))
					return FSFALSE;
			}
			return FSTRUE;
		case REGION_ID:
			//TODO: 
		default:
			return FSFALSE;
		}
		return FSFALSE;

	case REGION_ID:
		switch (s->type){
		case REGION_ID:
			if (s->u.identified.identifier == r->u.identified.identifier){
				return (r->u.identified.local == 0 || r->u.identified.local == s->u.identified.local) ? FSTRUE : FSFALSE;
			}
			return (r->u.identified.dictionary == RDICT_UN_STATS && _IdentifiedRegion_IsSubRegion(r->u.identified.identifier, s->u.identified.identifier)) ? FSTRUE : FSFALSE;
		case REGION_CIRCLE:
			/* TODO: perform geodecoding */
			return FSTRUE;
		case REGION_RECTANGLE:
		case REGION_POLYGON:
			{
				// check that each point of s is inside the identified region
				int i, m;
				m = r->u.polygonal.count;
				if (s->type == REGION_RECTANGLE) m <<= 1;
				for (i = 0; i < m; i++) {
					if (0 == _Identified_IsPointInside(r->u.identified.identifier, r->u.identified.local, &s->u.polygonal.points[i])){
						return FSFALSE;
					}
				}
			}
			return FSTRUE;
		default: // unknown region
			return FSFALSE;
		}
	default:
		break;
	}
	return FSFALSE;
}

FSBOOL GeographicRegion_IsPointInside(const GeographicRegion * r, const TwoDLocation * l)
{
	switch (r->type){
	case REGION_NONE:
		return FSTRUE;
	case REGION_CIRCLE:
		return _Circle_IsPointInside(&r->u.circular.center, r->u.circular.radius, l);
	case REGION_RECTANGLE:
		return _Rectangle_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, l);
	case REGION_POLYGON:
		return _Polygon_IsPointInside(r->u.polygonal.points, r->u.polygonal.count, l);
	case REGION_ID:
		return _Identified_IsPointInside(r->u.identified.identifier, r->u.identified.local, l);
	}
	return FSFALSE;
}

static FSBOOL _IdentifiedRegion_IsSubRegion(int parent, int child)
{
	int n = _un_stats_regions[child];
	if (n == parent) return FSTRUE;
	if (n == 0)   return FSFALSE;
	return _IdentifiedRegion_IsSubRegion(parent, n);
}


static int _TwoDLocation_IsLeft(const TwoDLocation * P0, const TwoDLocation * P1, const TwoDLocation * P2)
{
	return
		((P1->longitude < P0->longitude) ? -1 : 1) *
		((P2->latitude  < P0->latitude) ? -1 : 1) *
		((P2->longitude < P0->longitude) ? -1 : 1) *
		((P1->latitude  < P0->latitude) ? -1 : 1);
}

static FSBOOL _Circle_IsPointInside(const TwoDLocation * center, int radius, const TwoDLocation * l)
{
	int distance = _TwoDLocation_Distance(center, l);
	return distance <= radius ? FSTRUE : FSFALSE;
}

static FSBOOL _Polygon_IsPointInside(const TwoDLocation * points, int pcount, const TwoDLocation * l)
{
	int i, wn = 0;    // the winding number counter

	// loop through all edges of the polygon
	for (i = 0; i <= pcount; i++)
	{// edge from V[i] to V[i+1]
		// loop through all edges of the polygon
		if (points[i].latitude <= l->latitude) {   // start y <= P.y
			if (points[(i + 1) % pcount].latitude > l->latitude)      // an upward crossing
				if (_TwoDLocation_IsLeft(&points[i], &points[(i + 1) % pcount], l) > 0) // P left of edge
					++wn;            // have a valid up intersect
		}
		else {   // start y > P.y (no test needed)
			if (points[(i + 1) % pcount].latitude <= l->latitude) // a downward crossing
				if (_TwoDLocation_IsLeft(&points[i], &points[(i + 1) % pcount], l) < 0) // P right of edge
					--wn;            // have a valid down intersect
		}
	}
	return (wn != 0) ? FSTRUE : FSFALSE;
}

static FSBOOL _Rectangle_IsPointInside(const TwoDLocation * points, int rcount, const TwoDLocation * l)
{
	int i;
	for (i = 0; i < rcount * 2; i += 2){
		const TwoDLocation * nw = &points[i];
		const TwoDLocation * se = &points[i + 1];

		if (nw->latitude >= l->latitude && se->latitude <= l->latitude) {
			int32_t lw, le, ll;
			if (nw->longitude < se->longitude){
				if (l->longitude >= nw->longitude && l->longitude <= se->longitude)
					return FSTRUE;
			}
			else{
				lw = nw->longitude - 1800000000;
				le = se->longitude + 1800000000;
				ll = (l->longitude > 0) ? l->longitude - 1800000000 : 1800000000 + l->longitude;
				if (ll >= lw && ll <= le)
					return FSTRUE;
			}
		}
	}
	return FSFALSE;
}

static FSBOOL      _Identified_IsPointInside(uint16_t identifier, int local, const TwoDLocation * l)
{
	// TODO: Check identified regions
	return FSTRUE;
}

static int _Rectangular_ConvertToPolygon(const TwoDLocation * rpoints, int rcount, TwoDLocation * points, int pcount)
{
	// TODO: Write this stupid think
	// return count of points
	return -1;
}

static const double _DEG_TO_RAD = 0.017453292519943295769236907684886;
static const double _EARTH_RADIUS_IN_METERS = 6372797.560856;
static const double _PI = 3.141592653589793;
static const double _2PI = 6.283185307179586;

static inline double its2degree(int32_t v){
	return ((double)v) / 10000000.0;
}

static uint32_t _TwoDLocation_Distance(const TwoDLocation * l, const TwoDLocation * p)
{
	double lat1, lon1, lat2, lon2;
	lat1 = its2degree(l->latitude)*_DEG_TO_RAD;
	lon1 = its2degree(l->longitude)*_DEG_TO_RAD;
	lat2 = its2degree(p->latitude)*_DEG_TO_RAD;
	lon2 = its2degree(p->longitude)*_DEG_TO_RAD;

	double latitudeArc = lat1 - lat2;
	double longitudeArc = lon1 - lon2;
//	if (longitudeArc > _PI)  longitudeArc = _2PI - longitudeArc;
//	if (longitudeArc < 0.0-_PI) longitudeArc = _2PI + longitudeArc;
	
	double latitudeH = sin(latitudeArc * 0.5);
	latitudeH *= latitudeH;
	double lontitudeH = sin(longitudeArc * 0.5);
	lontitudeH *= lontitudeH;
	double tmp = cos(lat1) * cos(lat2);
	return (uint32_t) (0.5 + 2.0 * _EARTH_RADIUS_IN_METERS * asin(sqrt(latitudeH + tmp*lontitudeH)));
}

static FSBOOL      _Polygon_IsCircleInside(const TwoDLocation * points, int rcount, const TwoDLocation * center, int radius)
{
	// TODO:
	return FSFALSE;
}

