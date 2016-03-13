#include "postgis_brin.h"

/*
 * As we index geometries but store either a BOX2DF or GIDX according to the
 * operator class, we need to overload the original brin_inclusion_add_value()
 * function to be able to do this. Other original mandatory support functions
 * doesn't need to be overloaded.
 *
 * The previous limitation might be lifted, but we also eliminate some overhead
 * by doing it this way, namely calling different functions through the
 * FunctionCallInvoke machinery for each heap tuple.
 */

PG_FUNCTION_INFO_V1(geomnd_brin_inclusion_add_value);
Datum
geomnd_brin_inclusion_add_value(PG_FUNCTION_ARGS)
{
	BrinValues *column = (BrinValues *) PG_GETARG_POINTER(1);
	Datum newval = PG_GETARG_DATUM(2);
	bool		isnull = PG_GETARG_BOOL(3);
	GIDX * gidx_geom, *gidx_index;
	int dims_geom, i;
	int ndims = 4; /* FIXME */

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
