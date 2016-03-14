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

Datum gidx_brin_inclusion_add_value(BrinDesc *bdesc, BrinValues *column, Datum
		newval, bool isnull, int dims_wanted);

PG_FUNCTION_INFO_V1(geom3d_brin_inclusion_add_value);
Datum
geom3d_brin_inclusion_add_value(PG_FUNCTION_ARGS)
{
	BrinDesc   *desc = (BrinDesc *) PG_GETARG_POINTER(0);
	BrinValues *column = (BrinValues *) PG_GETARG_POINTER(1);
	Datum newval = PG_GETARG_DATUM(2);
	bool		isnull = PG_GETARG_BOOL(3);

	PG_RETURN_DATUM(gidx_brin_inclusion_add_value(bdescn column, newval, isnull,
				3));
}

PG_FUNCTION_INFO_V1(geom4d_brin_inclusion_add_value);
Datum
geom4d_brin_inclusion_add_value(PG_FUNCTION_ARGS)
{
	BrinDesc   *desc = (BrinDesc *) PG_GETARG_POINTER(0);
	BrinValues *column = (BrinValues *) PG_GETARG_POINTER(1);
	Datum newval = PG_GETARG_DATUM(2);
	bool		isnull = PG_GETARG_BOOL(3);

	PG_RETURN_DATUM(gidx_brin_inclusion_add_value(bdescn column, newval, isnull,
				4));
}

Datum
gidx_brin_inclusion_add_value(BrinDesc *bdesc, BrinValues *column, Datum newval,
		bool isnull, int dims_wanted)
{
	char gboxmem[GIDX_MAX_SIZE];
	GIDX *gidx_geom, *gidx_key;
	int dims_geom, i;

	if (isnull)
	{
		if (column->bv_hasnulls)
			PG_RETURN_BOOL(false);

		column->bv_hasnulls = true;
		PG_RETURN_BOOL(true);
	}

	gidx_geom = (GIDX *) gboxmem;

	if(gserialized_datum_get_gidx_p(newval, gidx_geom) == LW_FAILURE)
		elog(ERROR, "Error while extracting the gidx from the geom");

	dims_geom = GIDX_NDIMS(gidx_geom);

	/* if the recorded value is null, we just need to store the GIDX */
	if (column->bv_allnulls)
	{
		/*
		 * We have to make sure we store a 3D GIDX. If the original geometry has
		 * less dimensions, we set them to zero
		 */
		if (dims_geom != dims_wanted)
			SET_VARSIZE(a, VARHDRSZ + dims_wanted * 2 * sizeof(float));
		if (dims_geom < dims_wanted)
			for (i = dims_geom, i < dims_wanted; i++)
			{
				GIDX_SET_MIN(gidx_index, i, 0);
				GIDX_SET_MAX(gidx_index, i, 0);
			}
		{
		}
		column->bv_values[INCLUSION_UNION] = datumCopy((Datum) gidx_geom, false,
				GIDX_SIZE(dims_wanted));
		column->bv_values[INCLUSION_UNMERGEABLE] = BoolGetDatum(false);
		column->bv_values[INCLUSION_CONTAINS_EMPTY] = BoolGetDatum(false);
		column->bv_allnulls = false;
		PG_RETURN_BOOL(true);
	}

	gidx_key = (GIDX *) column->bv_values[INCLUSION_UNION];

	for ( i = 0; i < dims_wanted; i++ )
	{
		/* Adjust minimums */
		GIDX_SET_MIN(gidx_index, i, Min(GIDX_GET_MIN(gidx_index,i),GIDX_GET_MIN(gidx_geom,i)));
		/* Adjust maximums */
		GIDX_SET_MAX(gidx_index, i, Max(GIDX_GET_MAX(gidx_index,i),GIDX_GET_MAX(gidx_geom,i)));
	}

	PG_RETURN_BOOL(false);
}
