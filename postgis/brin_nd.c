#include "postgres.h"
#include "fmgr.h"

#include "../postgis_config.h"

/*#define POSTGIS_DEBUG_LEVEL 4*/

#include "liblwgeom.h"         /* For standard geometry types. */
#include "lwgeom_pg.h"       /* For debugging macros. */

#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>

/*
 ** For debugging
 */
#if POSTGIS_DEBUG_LEVEL > 0
static int geog_counter_leaf = 0;
static int geog_counter_internal = 0;
#endif

Datum contains_box3d_geom(PG_FUNCTION_ARGS);
Datum contains_box2d_geom(PG_FUNCTION_ARGS);
Datum is_contained_box2d_geom(PG_FUNCTION_ARGS);
Datum overlaps_box2d_geom(PG_FUNCTION_ARGS);

#define BOX_xmin(a) (Min(a->xmin, a->xmax))
#define BOX_xmax(a) (Max(a->xmin, a->xmax))
#define BOX_ymin(a) (Min(a->ymin, a->ymax))
#define BOX_ymax(a) (Max(a->ymin, a->ymax))
#define BOX_zmin(a) (Min(a->zmin, a->zmax))
#define BOX_zmax(a) (Max(a->zmin, a->zmax))



PG_FUNCTION_INFO_V1(contains_box3d_geom);
Datum contains_box3d_geom(PG_FUNCTION_ARGS)
{
	/* take the first argument - a box3d */
	BOX3D *box_a = (BOX3D *)PG_GETARG_POINTER(0);

	/* take the second argument - a geometry - and retrieve its box3d */
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(1);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	const GBOX *gbox = lwgeom_get_bbox(lwgeom);
	bool result = TRUE;
	if((BOX_xmin(box_a) > gbox->xmin) || (BOX_xmax(box_a) < gbox->xmax) ||
			(BOX_ymin(box_a) > gbox->ymin) || (BOX_ymax(box_a) < gbox->ymax) ||
			(BOX_zmin(box_a) > gbox->zmin) || (BOX_zmax(box_a) < gbox->zmax)){
		result = FALSE;
	}
	lwgeom_free(lwgeom);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_box2d_geom);
Datum contains_box2d_geom(PG_FUNCTION_ARGS)
{
	GBOX *box_a = (GBOX *)PG_GETARG_POINTER(0);

	/* take the second argument - a geometry - and retrieve its box3d */
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(1);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	const GBOX *gbox = lwgeom_get_bbox(lwgeom);
	bool result = TRUE;
	if((BOX_xmin(box_a) > gbox->xmin) || (BOX_xmax(box_a) < gbox->xmax) ||
			(BOX_ymin(box_a) > gbox->ymin) || (BOX_ymax(box_a) < gbox->ymax) ||
			(BOX_zmin(box_a) > gbox->zmin) || (BOX_zmax(box_a) < gbox->zmax)){
		result = FALSE;
	}
	lwgeom_free(lwgeom);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(is_contained_box2d_geom);
Datum is_contained_box2d_geom(PG_FUNCTION_ARGS)
{
	GBOX *box_a = (GBOX *)PG_GETARG_POINTER(0);
	/* take the second argument - a geometry - and retrieve its box3d */
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(1);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	const GBOX *gbox = lwgeom_get_bbox(lwgeom);
	bool result = TRUE;
	if((BOX_xmin(box_a) < gbox->xmin) || (BOX_xmax(box_a) > gbox->xmax) ||
			(BOX_ymin(box_a) < gbox->ymin) || (BOX_ymax(box_a) > gbox->ymax) ||
			(BOX_zmin(box_a) < gbox->zmin) || (BOX_zmax(box_a) > gbox->zmax)){
		result = FALSE;
	}
	lwgeom_free(lwgeom);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_box2d_geom);
Datum overlaps_box2d_geom(PG_FUNCTION_ARGS)
{
	GBOX *box_a = (GBOX *)PG_GETARG_POINTER(0);
	/* take the second argument - a geometry - and retrieve its box3d */
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(1);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	const GBOX *gbox = lwgeom_get_bbox(lwgeom);
	bool result = TRUE;
	if((BOX_xmin(box_a) > gbox->xmax) || (BOX_xmax(box_a) < gbox->xmin) ||
			(BOX_ymin(box_a) > gbox->ymax) || (BOX_ymax(box_a) < gbox->ymin) ||
			(BOX_zmin(box_a) > gbox->zmin) || (BOX_zmax(box_a) < gbox->zmin)){
		result = FALSE;
	}
	lwgeom_free(lwgeom);
	PG_RETURN_BOOL(result);
}
