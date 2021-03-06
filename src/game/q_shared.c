/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include "q_shared.h"

#define DEG2RAD(x) ((x * M_PI )/180.0F)

vec3_t		vec3_origin = {0, 0, 0};


void
RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees)
{
	float		m[3][3];
	float		im[3][3];
	float		zrot[3][3];
	float		tmpmat[3][3];
	float		rot[3][3];
	int		i;
	vec3_t		vr, vup, vf;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector(vr, dir);
	CrossProduct(vr, vf, vup);

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy(im, m, sizeof(im));

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset(zrot, 0, sizeof(zrot));
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0;

	zrot[0][0] = cos(DEG2RAD(degrees));
	zrot[0][1] = sin(DEG2RAD(degrees));
	zrot[1][0] = -zrot[0][1];
	zrot[1][1] = zrot[0][0];

	R_ConcatRotations(m, zrot, tmpmat);
	R_ConcatRotations(tmpmat, im, rot);

	for (i = 0; i < 3; i++) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

void
AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	static float	sr, sp, sy, cr, cp, cy;

	/* static to help MS compiler fp bugs */

	angle = angles[YAW] * (M_PI * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	
	if (right || up) {
		angle = angles[ROLL] * (M_PI * 2 / 360);
		sr = sin(angle);
		cr = cos(angle);
	}

	if (forward) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if (right) {
		right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
		right[2] = -1 * sr * cp;
	}
	if (up) {
		up[0] = (cr * sp * cy + -sr * -sy);
		up[1] = (cr * sp * sy + -sr * cy);
		up[2] = cr * cp;
	}
}


void
ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float		d;
	vec3_t		n;
	float		inv_denom;

	inv_denom = 1.0 / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
 * * assumes "src" is normalized
 */
void
PerpendicularVector(vec3_t dst, const vec3_t src)
{
	int		pos;
	int		i;
	float		minelem = 1.0;
	vec3_t		tempvec;

	/*
	 * * find the smallest magnitude axially aligned vector
	 */
	for (pos = 0, i = 0; i < 3; i++) {
		if (fabs(src[i]) < minelem) {
			pos = i;
			minelem = fabs(src[i]);
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0;
	tempvec[pos] = 1.0;

	/*
	 * * project the point onto the plane defined by src
	 */
	ProjectPointOnPlane(dst, tempvec, src);

	/*
	 * * normalize the result
	 */
	VectorNormalize(dst);
}



/*
 * ================ R_ConcatRotations ================
 */
void
R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
	in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
	    in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
	    in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
	    in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
	    in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
	    in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
	    in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
	    in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
	    in1[2][2] * in2[2][2];
}


/*
 * ================ R_ConcatTransforms ================
 */
void
R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
	in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
	    in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
	    in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
	    in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
	    in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
	    in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
	    in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
	    in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
	    in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
	    in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
	    in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
	    in1[2][2] * in2[2][3] + in1[2][3];
}


/*
 * ===========================================================================
 * =
 */

/*
 * =============== LerpAngle
 *
 * ===============
 */
float
LerpAngle(float a2, float a1, float frac)
{
	if (a1 - a2 > 180)
		a1 -= 360;
	if (a1 - a2 < -180)
		a1 += 360;
	return a2 + frac * (a1 - a2);
}


float
anglemod(float a)
{
	a = (360.0 / 65536) * ((int)(a * (65536 / 360.0)) & 65535);
	return a;
}

/* this is the slow, general version */
int
BoxOnPlaneSide2(vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int		i;
	float		dist1   , dist2;
	int		sides;
	vec3_t		corners [2];

	for (i = 0; i < 3; i++) {
		if (p->normal[i] < 0) {
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		} else {
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct(p->normal, corners[0]) - p->dist;
	dist2 = DotProduct(p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}

/*
 * ==================
 * BoxOnPlaneSide
 *
 * Returns 1, 2, or 1 + 2
 * ==================
 */
int
BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	switch(p->signbits) {
	default:
	case 0:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) >= p->dist) |
		        (((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) < p->dist) << 1));
	case 1:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) >= p->dist) | 
		        (((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) < p->dist) << 1));
	case 2:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) >= p->dist) |
		        (((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) < p->dist) << 1));
	case 3:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) >= p->dist) |
		        (((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) < p->dist) << 1));
	case 4:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) >= p->dist) |
		        (((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));
	case 5:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) >= p->dist) |
		        (((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));
	case 6:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) >= p->dist) |
		        (((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));
	case 7:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) >= p->dist) |
		        (((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));
	}
}

/*
 * ================= PlaneTypeForNormal =================
 */

int
PlaneTypeForNormal(const vec3_t normal)
{
	vec_t		ax, ay, az;

	/* NOTE: should these have an epsilon around 1.0? */
	if (normal[0] >= 1.0)
		return PLANE_X;
	if (normal[1] >= 1.0)
		return PLANE_Y;
	if (normal[2] >= 1.0)
		return PLANE_Z;

	ax = fabs(normal[0]);
	ay = fabs(normal[1]);
	az = fabs(normal[2]);

	if (ax >= ay && ax >= az)
		return PLANE_ANYX;
	if (ay >= ax && ay >= az)
		return PLANE_ANYY;
	return PLANE_ANYZ;
}

void
ClearBounds(vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void
AddPointToBounds(vec3_t v, vec3_t mins, vec3_t maxs)
{
	int		i;
	vec_t		val;

	for (i = 0; i < 3; i++) {
		val = v[i];
		if (val < mins[i])
			mins[i] = val;
		if (val > maxs[i])
			maxs[i] = val;
	}
}


vec_t
VectorNormalize(vec3_t v)
{
	float		length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

	if (length) {
		length = sqrt(length);	/* FIXME */
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
	return length;
}

vec_t
VectorNormalize2(vec3_t v, vec3_t out)
{
	float		length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

	if (length) {
		length = sqrt(length);	/* FIXME */
		ilength = 1 / length;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
	}
	return length;
}

vec_t
_DotProduct(vec3_t v1, vec3_t v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void
_VectorSubtract(vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}

void
_VectorAdd(vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

void
_VectorCopy(vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

int
Q_log2(int val)
{
	int		answer = 0;

	while (val >>= 1)
		answer++;
	return answer;
}

/*
 * ===========================================================================
 * =========
 */

/*
 * ============ COM_SkipPath ============
 */
char           *
COM_SkipPath(char *pathname)
{
	char           *last;

	last = (char *)pathname;
	while (*pathname) {
		if (*pathname == '/')
			last = (char *)pathname + 1;
		pathname++;
	}
	return last;
}

/*
 *============ COM_FixPath
 *
 * Change '\\' to '/', removes ./ 
 * and leading/ending '/'
 * "something/a/../b" -> "something/b" ============
 *
*/
void 
COM_FixPath(char *path)
{
	int	i, j, len = 0, lastLash = -1;

	for (i = 0; path[i]; i++) {
		switch (path[i]) {
		case '\\':
		case '/':
			if(!len)
				break;

			if (path[len-1] == '/') /* remove multiple "/" */
				break;

			if(path[len-1] == '.')
			{
				if(len == 1 || (len >= 2 && path[len-2] != '.'))
				{	/* remove "./" */
					len--;
					break;
				}
			}

			lastLash = len;
			path[len++] = '/';
			break;
		case '.':
			if(len >= 2 && path[len-1] == '.')
			{
				if(lastLash > 0 && path[lastLash-1] != '.')
				{	/* theres lastlash and its not "../" */
					for (j = lastLash-1; j >= 0; j--)
					{
						if(path[j] == '/')
							break;
					}
					lastLash = j;
					len = lastLash+1;			
					break;
				}
				if(path[len-2] == '.')
					break;
			}
			/* fallthrough */
		default:
			path[len++] = path[i];
			break;
		}
	}
	path[len] = '\0';

	if (len && path[len-1] == '/')
		path[len-1] = '\0';

}

/*
 * ============ COM_StripExtension ============
 */
void
COM_StripExtension(char *in, char *out)
{
	char *dot;

	if (!(dot = strrchr(in, '.'))) {
		//Q_strncpyz(out, in, strlen(in) + 1);
		strcpy(out, in);
		return;
	}
	while (*in && in != dot)
		*out++ = *in++;
	*out = 0;
}

/*
 * ============ COM_FileExtension ============
 */
char *
COM_FileExtension(char *in)
{
	static char	exten[8];
	int		i;

	while (*in && *in != '.')
		in++;
	if (!*in)
		return "";
	in++;
	for (i = 0; i < 7 && *in; i++, in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

/*
 * ============ COM_FileBase ============
 */
void
COM_FileBase(char *in, char *out)
{
	char           *s, *s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
		s--;

	for (s2 = s; s2 != in && *s2 != '/'; s2--);

	if (s - s2 < 2)
		out[0] = 0;
	else {
		s--;
		strncpy(out, s2 + 1, s - s2);
		out[s - s2] = 0;
	}
}

/*
 * ============ COM_FilePath
 *
 * Returns the path up to, but not including the last / ============
 */
void
COM_FilePath(char *in, char *out)
{
	char           *s;

	s = in + strlen(in) - 1;

	while (s != in && *s != '/')
		s--;

	strncpy(out, in, s - in);
	out[s - in] = 0;
}


/*
 * ================== COM_DefaultExtension ==================
 */
void
COM_DefaultExtension(char *path, char *extension)
{
	char           *src;

	/* 
	 * If path doesn't have a .EXT, append extension
	 * (extension should include the '.') 
	*/
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path) {
		if (*src == '.')
			return;	/* it has an extension */
		src--;
	}

	strcat(path, extension);
}

/*
 * ================== COM_MakePrintable ==================
 */

void
COM_MakePrintable(char *s)
{
	char *string = s;
	int	c;

	while((c = *string) != 0) {
		if ( c >= 0x20 && c <= 0x7E )
			*s++ = c;

		*string++;
	}
	*s = '\0';
}


/*
 *
 * ============================================================================
 *
 * BYTE ORDER FUNCTIONS
 *
 * ============================================================================
 */

qboolean	bigendien;

/* can't just use function pointers, or dll linkage can */
/* mess up when qcommon is included in multiple places */
short           (*_BigShort) (short l);
short           (*_LittleShort) (short l);
int             (*_BigLong) (int l);
int             (*_LittleLong) (int l);
float           (*_BigFloat) (float l);
float           (*_LittleFloat) (float l);

short
BigShort (short l){
	return _BigShort(l);
}

short
LittleShort(short l){
	return _LittleShort(l);
}

int
BigLong    (int l){
	return _BigLong(l);
}

int
LittleLong (int l){
	return _LittleLong(l);
}

float
BigFloat (float l){
	return _BigFloat(l);
}

float
LittleFloat(float l){
	return _LittleFloat(l);
}

short
ShortSwap(short l)
{
	byte		b1       , b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short
ShortNoSwap(short l)
{
	return l;
}

int
LongSwap(int l)
{
	byte		b1       , b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int
LongNoSwap(int l)
{
	return l;
}

float
FloatSwap(float f)
{
	union {
		float		f;
		byte		b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float
FloatNoSwap(float f)
{
	return f;
}

/*
 * ================ Swap_Init ================
 */
void
Swap_Init(void)
{
	byte		swaptest[2] = {1, 0};

	/* set the byte swapping variables in a portable manner	 */
	if (*(short *)swaptest == 1) {
		bigendien = false;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	} else {
		bigendien = true;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}

}



/*
 * ============ va
 *
 * does a varargs printf into a temp buffer, so I don't need to have varargs
 * versions of all text functions. FIXME: make this buffer size safe someday
 * ============
 */
char           *
va(char *format,...)
{
	va_list		argptr;
	static char	string[2048];

	va_start(argptr, format);
	vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	return string;
}

/*
 * ============== COM_Parse
 *
 * Parse a token out of a string ==============
 */
char    *
COM_Parse (char **data_p)
{
	int		c, len = 0;
	char	*data;
	static char	com_token[MAX_TOKEN_CHARS];

	data = *data_p;
	com_token[0] = 0;
	
	if (!data)
	{
		*data_p = NULL;
		return com_token;
	}
		
/* skip whitespace */
	do
	{
		while ((c = *data) <= ' ')
		{
			if (c == 0)
			{
				*data_p = NULL;
				return com_token;
			}
			data++;
		}
		
		/* skip // comments */
		if (c == '/' && data[1] == '/')
		{
			data += 2;

			while (*data && *data != '\n')
				data++;
		}
		else
			break;
	} while(1);


	/* handle quoted strings specially */
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c == '\"' || !c)
				break;

			if (len < MAX_TOKEN_CHARS)
				com_token[len++] = c;
		}
	}
	else
	{
		/* parse a regular word */
		do
		{
			if (len < MAX_TOKEN_CHARS)
				com_token[len++] = c;

			data++;
			c = *data;
		} while (c>32);
	}

	if (len == MAX_TOKEN_CHARS)
		len = 0;

	com_token[len] = 0;

	*data_p = data;
	return com_token;
}

/*
 * =============== Com_PageInMemory
 *
 * ===============
 */
int		paged_total;

void
Com_PageInMemory(byte * buffer, int size)
{
	int		i;

	for (i = size - 1; i > 0; i -= 4096)
		paged_total += buffer[i];
}



/*
 *
 * ============================================================================
 *
 * LIBRARY REPLACEMENT FUNCTIONS
 *
 * ============================================================================
 */

/* PATCH: matt */
/* use our own strncasecmp instead of this implementation */
#ifdef sun

#define Q_strncasecmp(s1, s2, n) (strncasecmp(s1, s2, n))

int
Q_stricmp(char *s1, char *s2)
{
	return strcasecmp(s1, s2);
}

#else

/* FIXME: replace all Q_stricmp with Q_strcasecmp */
int
Q_stricmp(char *s1, char *s2)
{
	return strcasecmp(s1, s2);
}


int
Q_strncasecmp(char *s1, char *s2, int n)
{
	int		c1        , c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;	/* strings are equal until end point */

		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;	/* strings not equal */
		}
	} while (c1);

	return 0;		/* strings are equal */
}

#endif

int
Q_strcasecmp(char *s1, char *s2)
{
	return Q_strncasecmp(s1, s2, 99999);
}


void
Q_strncpyz(char *dest, const char *src, size_t size)
{
	while (--size && (*dest++ = *src++));
	*dest = '\0';
}

char           *
Q_strlwr(char *s)
{
	char *p;

	p = s;
	while (*p) {
		*p = tolower(*p);
		p++;
	}
	return s;
}

void
Com_sprintf(char *dest, int size, char *fmt,...)
{
	int		len;
	va_list		argptr;
	static char	bigbuffer[0x10000];

	va_start(argptr, fmt);
	len = vsnprintf(bigbuffer, sizeof(bigbuffer), fmt, argptr);
	va_end(argptr);
#if 0
	if (len >= size)
		Com_Printf("Com_sprintf: overflow of %i in %i\n", len, size);
#endif
	strncpy(dest, bigbuffer, size - 1);
}

/*
 * =====================================================================
 *
 * INFO STRINGS
 *
 * =====================================================================
 */

/*
 * =============== Info_ValueForKey
 *
 * Searches the string for the given key and returns the associated value, or an
 * empty string. ===============
 */
char           *
Info_ValueForKey(char *s, char *key)
{
	char		pkey      [512];
	static char	value[2][512];	/* use two buffers so compares */

	/* work without stomping on each other */
	static int	valueindex;
	char           *o;

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1) {
		o = pkey;
		while (*s != '\\') {
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s) {
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp(key, pkey))
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

void
Info_RemoveKey(char *s, char *key)
{
	char           *start;
	char		pkey      [512];
	char		value     [512];
	char           *o;

	if (strstr(key, "\\")) {
		/* Com_Printf ("Can't use a key with a \\\n"); */
		return;
	}
	while (1) {
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\') {
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s) {
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp(key, pkey)) {
			strcpy(start, s);	/* remove this part */
			return;
		}
		if (!*s)
			return;
	}

}


/*
 * ================== Info_Validate
 *
 * Some characters are illegal in info strings because they can mess up the
 * server's parsing ==================
 */
qboolean
Info_Validate(char *s)
{
	if (strstr(s, "\""))
		return false;
	if (strstr(s, ";"))
		return false;
	return true;
}

void
Info_SetValueForKey(char *s, char *key, char *value)
{
	char		newi      [MAX_INFO_STRING], *v;
	int		c;
	int		maxsize = MAX_INFO_STRING;

	if (strstr(key, "\\") || strstr(value, "\\")) {
		Com_Printf("Can't use keys or values with a \\\n");
		return;
	}
	if (strstr(key, ";")) {
		Com_Printf("Can't use keys or values with a semicolon\n");
		return;
	}
	if (strstr(key, "\"") || strstr(value, "\"")) {
		Com_Printf("Can't use keys or values with a \"\n");
		return;
	}
	if (strlen(key) > MAX_INFO_KEY - 1 || strlen(value) > MAX_INFO_KEY - 1) {
		Com_Printf("Keys and values must be < 64 characters.\n");
		return;
	}
	Info_RemoveKey(s, key);
	if (!value || !strlen(value))
		return;

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) > maxsize) {
		Com_Printf("Info string length exceeded\n");
		return;
	}
	/* only copy ascii values */
	s += strlen(s);
	v = newi;
	while (*v) {
		c = *v++;
		c &= 127;	/* strip high bits */
		if (c >= 32 && c < 127)
			*s++ = c;
	}
	*s = 0;
}

/* Like glob_match, but match PATTERN against any final segment of TEXT.  */
int
glob_match_after_star(char *pattern, char *text)
{
	register char  *p = pattern, *t = text;
	register char	c, c1;

	while ((c = *p++) == '?' || c == '*')
		if (c == '?' && *t++ == '\0')
			return 0;

	if (c == '\0')
		return 1;

	if (c == '\\')
		c1 = *p;
	else
		c1 = c;

	while (1) {
		if ((c == '[' || *t == c1) && glob_match(p - 1, t))
			return 1;
		if (*t++ == '\0')
			return 0;
	}
}

/*
 * Match the pattern PATTERN against the string TEXT; return 1 if it matches,
 * 0 otherwise.
 *
 * A match means the entire string TEXT is used up in matching.
 *
 * In the pattern string, `*' matches any sequence of characters, `?' matches
 * any character, [SET] matches any character in the specified set, [!SET]
 * matches any character not in the specified set.
 *
 * A set is composed of characters or ranges; a range looks like character
 * hyphen character (as in 0-9 or A-Z). [0-9a-zA-Z_] is the set of characters
 * allowed in C identifiers. Any other character in the pattern must be
 * matched exactly.
 *
 * To suppress the special syntactic significance of any of `[]*?!-\', and match
 * the character exactly, precede it with a `\'.
 */

int
glob_match(char *pattern, char *text)
{
	register char  *p = pattern, *t = text;
	register char	c;

	while ((c = *p++) != '\0')
		switch (c) {
		case '?':
			if (*t == '\0')
				return 0;
			else
				++t;
			break;

		case '\\':
			if (*p++ != *t++)
				return 0;
			break;

		case '*':
			return glob_match_after_star(p, t);

		case '[':
			{
				register char	c1 = *t++;
				int		invert;

				if (!c1)
					return (0);

				invert = ((*p == '!') || (*p == '^'));
				if (invert)
					p++;

				c = *p++;
				while (1) {
					register char	cstart = c, cend = c;

					if (c == '\\') {
						cstart = *p++;
						cend = cstart;
					}
					if (c == '\0')
						return 0;

					c = *p++;
					if (c == '-' && *p != ']') {
						cend = *p++;
						if (cend == '\\')
							cend = *p++;
						if (cend == '\0')
							return 0;
						c = *p++;
					}
					if (c1 >= cstart && c1 <= cend)
						goto match;
					if (c == ']')
						break;
				}
				if (!invert)
					return 0;
				break;

		match:

				/*
				 * Skip the rest of the [...] construct that
				 * already matched.
				 */
				while (c != ']') {
					if (c == '\0')
						return 0;
					c = *p++;
					if (c == '\0')
						return 0;
					else if (c == '\\')
						++p;
				}
				if (invert)
					return 0;
				break;
			}

		default:
			if (c != *t++)
				return 0;
		}

	return *t == '\0';
}


/* ==================================================================== */
