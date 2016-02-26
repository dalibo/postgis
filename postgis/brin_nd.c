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
#include "access/brin_tuple.h"
#include "utils/datum.h"
#include "gserialized_gist.h"

#define INCLUSION_UNION				0
#define INCLUSION_UNMERGEABLE		1
#define INCLUSION_CONTAINS_EMPTY	2

Datum gidx_brin_inclusion_add_value(BrinDesc *bdesc, BrinValues *column, Datum newval, bool isnull, int ndims);

PG_FUNCTION_INFO_V1(geom2d_brin_inclusion_add_value);
Datum
geom2d_brin_inclusion_add_value(PG_FUNCTION_ARGS)
{
	BrinDesc   *bdesc = (BrinDesc *) PG_GETARG_POINTER(0);
	BrinValues *column = (BrinValues *) PG_GETARG_POINTER(1);
	Datum newval = PG_GETARG_DATUM(2);
	bool		isnull = PG_GETARG_BOOL(3);
	PG_RETURN_DATUM(gidx_brin_inclusion_add_value(bdesc, column, newval, isnull, 2));
}

PG_FUNCTION_INFO_V1(geom3d_brin_inclusion_add_value);
Datum
geom3d_brin_inclusion_add_value(PG_FUNCTION_ARGS)
{
	BrinDesc   *bdesc = (BrinDesc *) PG_GETARG_POINTER(0);
	BrinValues *column = (BrinValues *) PG_GETARG_POINTER(1);
	Datum newval = PG_GETARG_DATUM(2);
	bool		isnull = PG_GETARG_BOOL(3);
	PG_RETURN_DATUM(gidx_brin_inclusion_add_value(bdesc, column, newval, isnull, 3));
}

/*
 * As we index geometries but store a GIDX, we need to overload the original
 * brin_inclusion_add_value() function to be able to do this. Other original
 * mandatory support functions doesn't need to be overloaded.
 *
 * The previous limitation might be lifted, but we also eliminate some overhead
 * by doing it this way, namely calling different functions through the
 * FunctionCallInvoke machinery for each heap tuple.
 */
Datum
gidx_brin_inclusion_add_value(BrinDesc *bdesc, BrinValues *column, Datum newval, bool isnull, int ndims)
{
	GIDX * gidx_geom, *gidx_index;
	int dims_geom, i;

	Assert(ndims <= GIDX_MAX_DIM);

	if (isnull)
	{
		if (column->bv_hasnulls)
			PG_RETURN_BOOL(false);

		column->bv_hasnulls = true;
		PG_RETURN_BOOL(true);
	}

	/*
	 * Always allocate memory for max GIDX dimension, for safety and further
	 * evolution up to max GIDX dimension geometries
	 */
	gidx_geom = gidx_new(GIDX_MAX_DIM);

	if(gserialized_datum_get_gidx_p(newval, gidx_geom) == LW_FAILURE)
		elog(ERROR, "Error while extracting the gidx from the geom");

	dims_geom = GIDX_NDIMS(gidx_geom);

	if (column->bv_allnulls)
	{
		/* For clarity, set extraneous dimensions to 0 */
		for(i=ndims; i < dims_geom; i++)
		{
			GIDX_SET_MIN(gidx_geom, i, (int) 0);
			GIDX_SET_MAX(gidx_geom, i, (int) 0);
		}
		column->bv_values[INCLUSION_UNION] = datumCopy((Datum) gidx_geom, false, GIDX_SIZE(dims_geom));
		column->bv_values[INCLUSION_UNMERGEABLE] = BoolGetDatum(false);
		column->bv_values[INCLUSION_CONTAINS_EMPTY] = BoolGetDatum(false);
		column->bv_allnulls = false;
		PG_RETURN_BOOL(true);
	}

	gidx_index = (GIDX *) column->bv_values[INCLUSION_UNION];

	for ( i = 0; i < ndims; i++ )
	{
		/* Adjust minimums */
		GIDX_SET_MIN(gidx_index, i, Min(GIDX_GET_MIN(gidx_index,i),GIDX_GET_MIN(gidx_geom,i)));
		/* Adjust maximums */
		GIDX_SET_MAX(gidx_index, i, Max(GIDX_GET_MAX(gidx_index,i),GIDX_GET_MAX(gidx_geom,i)));
	}
	pfree(gidx_geom);

	PG_RETURN_BOOL(false);
}
