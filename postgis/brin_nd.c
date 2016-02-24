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

Datum is_contained_box3d_geom(PG_FUNCTION_ARGS);

static double BOX3D_xmin(BOX3D *box)
{
        return Min(box->xmin, box->xmax);
}

static double BOX3D_ymin(BOX3D *box)
{
        return Min(box->ymin, box->ymax);
}

static double BOX3D_zmin(BOX3D *box)
{
        return Min(box->zmin, box->zmax);
}

static double BOX3D_xmax(BOX3D *box)
{
        return Max(box->xmin, box->xmax);
}

static double BOX3D_ymax(BOX3D *box)
{
        return Max(box->ymin, box->ymax);
}

static double BOX3D_zmax(BOX3D *box)
{
        return Max(box->zmin, box->zmax);
}

PG_FUNCTION_INFO_V1(is_contained_box3d_geom);
Datum is_contained_box3d_geom(PG_FUNCTION_ARGS)
{
	/* take the first argument - a box3d */
        BOX3D *box_a = (BOX3D *)PG_GETARG_POINTER(0);

	/* take the second argument - a geometry - and retrieve its box3d */	
        GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(1);
        LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
        GBOX gbox;
        BOX3D *box_b;
        int rv = lwgeom_calculate_gbox(lwgeom, &gbox);

        if ( rv == LW_FAILURE )
                PG_RETURN_NULL();

        box_b = box3d_from_gbox(&gbox);
        box_b->srid = lwgeom->srid;

        lwgeom_free(lwgeom);

	/* check if the box3d is contained in the bounding box of the geometry */
        if (BOX3D_xmin(box_a) > BOX3D_xmin(box_b) && BOX3D_ymin(box_a) > BOX3D_ymin(box_b) && BOX3D_zmin(box_a) > BOX3D_zmin(box_b)) {
                if (BOX3D_xmax(box_a) < BOX3D_xmax(box_b) && BOX3D_ymax(box_a) < BOX3D_ymax(box_b) && BOX3D_zmax(box_a) < BOX3D_zmax(box_b)) {
                        PG_RETURN_BOOL(TRUE);
                }
        }

        PG_RETURN_BOOL(FALSE);
}
