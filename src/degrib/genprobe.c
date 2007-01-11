/*****************************************************************************
 * genprobe.c
 *
 * DESCRIPTION
 *    This file contains the generic probing routines.
 *
 * HISTORY
 *   12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myerror.h"
#include "myutil.h"
#include "genprobe.h"
#include "probe.h"
#include "cube.h"
#include "myassert.h"
#include "mymapf.h"
#include "scan.h"
#include "interp.h"
#include "database.h"
#include "tendian.h"
#include "weather.h"
#include "sector.h"
#include "grpprobe.h"
#ifdef _DWML_
#include "xmlparse.h"
#endif
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/* *INDENT-OFF* */
/* Problems using MISSING to denote all possible, since subcenter = Missing
 * is defined for NDFD. */
static const genElemDescript NdfdElements[] = {
   {NDFD_MAX,2, 8,MISSING_2,0, 8,0,0,4,12, 1,0.0,0.0 },
   {NDFD_MIN,2, 8,MISSING_2,0, 8,0,0,5,12, 1,0.0,0.0 },
   {NDFD_POP,2, 8,MISSING_2,0, 9,0,1,8,12, 1,0.0,0.0 },
   {NDFD_TEMP,2, 8,MISSING_2,0, 0,0,0,0,0, 1,0.0,0.0 },
   {NDFD_WD,2, 8,MISSING_2,0, 0,0,2,0,0, 1,0.0,0.0 },
   {NDFD_WS,2, 8,MISSING_2,0, 0,0,2,1,0, 1,0.0,0.0 },
   {NDFD_TD,2, 8,MISSING_2,0, 0,0,0,6,0, 1,0.0,0.0 },
   {NDFD_SKY,2, 8,MISSING_2,0, 0,0,6,1,0, 1,0.0,0.0 },
   {NDFD_QPF,2, 8,MISSING_2,0, 8,0,1,8,6, 1,0.0,0.0 },
   {NDFD_SNOW,2, 8,MISSING_2,0, 8,0,1,29,6, 1,0.0,0.0 },
   {NDFD_WX,2, 8,MISSING_2,0, 0,0,1,192,0, 1,0.0,0.0 },
   {NDFD_WH,2, 8,MISSING_2,0, 0,10,0,5,0, 1,0.0,0.0 },
   {NDFD_AT,2, 8,MISSING_2,0, 0,0,0,193,0, 1,0.0,0.0 },
   {NDFD_RH,2, 8,MISSING_2,0, 0,0,1,1,0, 1,0.0,0.0 },
/*
   {NDFD_WG,2, 8,MISSING_2,0, 0,0,2,22,0, 1,0.0,0.0 },
   {NDFD_INC34,2, 8,MISSING_2,0, 9,0,2,1,6, 103,10.0,0.0 },
   {NDFD_INC50,2, 8,MISSING_2,0, 9,0,2,1,6, 103,10.0,0.0 },
   {NDFD_INC64,2, 8,MISSING_2,0, 9,0,2,1,6, 103,10.0,0.0 },
   {NDFD_CUM34,2, 8,MISSING_2,0, 9,0,2,1,6, 103,10.0,0.0 },
   {NDFD_CUM50,2, 8,MISSING_2,0, 9,0,2,1,6, 103,10.0,0.0 },
   {NDFD_CUM64,2, 8,MISSING_2,0, 9,0,2,1,6, 103,10.0,0.0 },
*/
   {NDFD_UNDEF,2, MISSING_2,MISSING_2,MISSING_1,
                  MISSING_2,MISSING_1,MISSING_1,MISSING_1,0,
                  MISSING_1,0.0,0.0 },
   {NDFD_MATCHALL,0, MISSING_2,MISSING_2,MISSING_1,
                     MISSING_2,MISSING_1,MISSING_1,MISSING_1,0,
                     MISSING_1,0.0,0.0 },
};

static const uChar NdfdElementsLen = (sizeof (NdfdElements) /
                                      sizeof (NdfdElements[0]));

/* *INDENT-ON* */

/*****************************************************************************
 * genNdfdVar_Lookup() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Return the NDFD enumeration of the given character string (or UNDEF).
 *
 * ARGUMENTS
 *          str = A pointer to the char string to look up. (Input)
 *    f_toLower = Perform strToLower in this procedure (Input)
 * f_ndfdConven = 0 => use short name conventions (see metaname.c)
 *                1 => use standard NDFD file naming convention
 *                2 => use verification NDFD file naming convention (Input)
 *
 * RETURNS: int
 *   The desired NDFD enumeration value.
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *   see meta.h for the following:
 *   enum { NDFD_MAX, NDFD_MIN, NDFD_POP, NDFD_TEMP, NDFD_WD, NDFD_WS,
 *          NDFD_TD, NDFD_SKY, NDFD_QPF, NDFD_SNOW, NDFD_WX, NDFD_WH,
 *          NDFD_AT, NDFD_RH, NDFD_UNDEF, NDFD_MATCHALL };
 *****************************************************************************
 */
/*
static char *NDFD_Type[] = { "maxt", "mint", "pop12", "t", "winddir",
   "windspd", "td", "sky", "qpf", "snowamt", "wx", "waveheight",
   "apparentt", "rh", "windgust", "probwindspd34i", "probwindspd50c",
   "probwindspd64c", "probwindspd34c", "probwindspd50c", "probwindspd64c",
   NULL
};
static char *NDFD_File[] = { "maxt", "mint", "pop12", "temp", "wdir",
   "wspd", "td", "sky", "qpf", "snow", "wx", "waveh", "apt", "rhm",
   "wgust", "tcwspdabv34i", "tcwspdabv50i", "tcwspdabv64i",
   "tcwspdabv34c", "tcwspdabv50c", "tcwspdabv64c", NULL
};
static char *NDFD_File2[] = { "mx", "mn", "po", "tt", "wd",
   "ws", "dp", "cl", "qp", "sn", "wx", "wh", "at", "rh", "wg", "i3",
   "i5", "i6", "c3", "c5", "c6", NULL
};
*/
static char *NDFD_Type[] = { "maxt", "mint", "pop12", "t", "winddir",
   "windspd", "td", "sky", "qpf", "snowamt", "wx", "waveheight",
   "apparentt", "rh", NULL
};
static char *NDFD_File[] = { "maxt", "mint", "pop12", "temp", "wdir",
   "wspd", "td", "sky", "qpf", "snow", "wx", "waveh", "apt", "rhm", NULL
};
static char *NDFD_File2[] = { "mx", "mn", "po", "tt", "wd",
   "ws", "dp", "cl", "qp", "sn", "wx", "wh", "at", "rh", NULL
};

uChar genNdfdVar_Lookup (char *str, char f_toLower, char f_ndfdConven)
{
   int index;
   uChar elemNum;       /* The index into the table that matches str. */

   if (f_toLower)
      strToLower (str);
   if (f_ndfdConven == 0) {
      if (GetIndexFromStr (str, NDFD_Type, &index) < 0) {
         elemNum = NDFD_UNDEF;
      } else {
         elemNum = (uChar) index;
      }
   } else if (f_ndfdConven == 1) {
      if (GetIndexFromStr (str, NDFD_File, &index) < 0) {
         elemNum = NDFD_UNDEF;
      } else {
         elemNum = (uChar) index;
      }
   } else if (f_ndfdConven == 2) {
      if (GetIndexFromStr (str, NDFD_File2, &index) < 0) {
         elemNum = NDFD_UNDEF;
      } else {
         elemNum = (uChar) index;
      }
   } else {
      elemNum = NDFD_UNDEF;
   }
   return elemNum;
}

/*****************************************************************************
 * genNdfdEnumToStr() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Return a pointer to the string that matches the NDFD enumeration, or
 * NULL for NDFD_MATCHALL, NDFD_UNDEF, or error.
 *
 * ARGUMENTS
 *     ndfdEnum = The NDFD Enumeration to look up.
 * f_ndfdConven = 0 => use short name conventions (see metaname.c)
 *                1 => use standard NDFD file naming convention
 *                2 => use verification NDFD file naming convention (Input)
 *
 * RETURNS: const char *
 *   The desired string associated with the given NDFD enumeration.
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *   see meta.h for the following:
 *   enum { NDFD_MAX, NDFD_MIN, NDFD_POP, NDFD_TEMP, NDFD_WD, NDFD_WS,
 *          NDFD_TD, NDFD_SKY, NDFD_QPF, NDFD_SNOW, NDFD_WX, NDFD_WH,
 *          NDFD_AT, NDFD_RH, NDFD_UNDEF, NDFD_MATCHALL };
 *****************************************************************************
 */
const char *genNdfdEnumToStr (uChar ndfdEnum, char f_ndfdConven)
{
   if (ndfdEnum >= NDFD_UNDEF)
      return NULL;
   if (f_ndfdConven == 0) {
      return (NDFD_Type[ndfdEnum]);
   } else if (f_ndfdConven == 1) {
      return (NDFD_File[ndfdEnum]);
   } else if (f_ndfdConven == 2) {
      return (NDFD_File2[ndfdEnum]);
   } else {
      return NULL;
   }
}

/*****************************************************************************
 * genElemInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Initialize an element structure to the NDFD_UNDEF value.
 *
 * ARGUMENTS
 * elem = A pointer to the element to init. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genElemInit (genElemDescript * elem)
{
   memset (elem, 0, sizeof (genElemDescript));
   elem->ndfdEnum = NDFD_UNDEF;
}

/*****************************************************************************
 * genElemFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free the data assocoated with an element.  This procedure currently is
 * just a place holder for future developments.
 *
 * ARGUMENTS
 * elem = A pointer to the element to free. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genElemFree (genElemDescript * elem)
{
   return;
}

/*****************************************************************************
 * genElemListInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Make sure that the elem data structure is completed correctly. If
 * f_validNDFD is true, then it inits it based on the value in ndfdEnum,
 * otherwise it sets ndfdEnum to NDFD_UNDEF, and inits the values to
 * Missing.  Presumably the user would then init the other values themself.
 * If they didn't then they'd get all matches.
 *
 * ARGUMENTS
 *     numElem = Number of elements to init. (Input)
 *        elem = The element array to init. (Input/Output)
 * f_validNDFD = 1 ndfdEnum is already value, 0 ndfdEnum is not (Input)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *   Being phased out...
 *****************************************************************************
 */
#ifdef OLD_176
void genElemListInit (size_t numElem, genElemDescript * elem,
                      uChar f_validNDFD)
{
   size_t i;            /* Loop variable over number of elements. */
   uShort2 j;           /* Loop variable over number of NdfdElements */

   if (!f_validNDFD) {
      for (i = 0; i < numElem; i++) {
         genElemInit (elem + i);
      }
      return;
   }
   for (i = 0; i < numElem; i++) {
      /* Try guessing where it is in the table. */
      if ((elem[i].ndfdEnum < NdfdElementsLen) &&
          (NdfdElements[elem[i].ndfdEnum].ndfdEnum == elem[i].ndfdEnum)) {
         memcpy (&(elem[i]), &(NdfdElements[elem[i].ndfdEnum]),
                 sizeof (genElemDescript));
      } else {
         /* Guess Failed, search for it. */
         for (j = 0; j < NdfdElementsLen; j++) {
            if (NdfdElements[j].ndfdEnum == elem[i].ndfdEnum) {
               memcpy (&(elem[i]), &(NdfdElements[j]),
                       sizeof (genElemDescript));
               break;
            }
         }
         /* Couldn't find it. */
         if (j == NdfdElementsLen) {
#ifdef DEBUG
            printf ("Couldn't find %d\n", elem->ndfdEnum);
#endif
            genElemInit (elem + i);
         }
      }
   }
}
#endif

/*****************************************************************************
 * genElemListInit2() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Takes the set of flags for variables that the procedure cared about,
 * adjusts them based on the user choices, allocates memory for elem, and
 * copies the NDFD var structure to elem, for variables that scored at least
 * a "2" (1 interest from procedure + 1 interest from user), or (2 vital from
 * procedure + 0/1 from user).
 *
 * ARGUMENTS
 *   varFilter = A set of flags for each NDFD variable already set to calling
 *               procedure's interest level.  (1 means interested but ok if
 *               user wants to drop, 2 means vital, 0 means don't care)
 *               (Input/Output)
 * numNdfdVars = Number of user selected ndfdVars (Input)
 *    ndfdVars = The user selected NDFD variables. (Input)
 *     numElem = length of elem. (Input)
 *        elem = The element array to init. (Output)
 *
 * RETURNS: void
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *  5/2006 AAT: Modified to choose all only if calling procedure didn't select
 *              something.
 *
 * NOTES:
 *****************************************************************************
 */
void genElemListInit2 (uChar varFilter[NDFD_MATCHALL + 1],
                       size_t numNdfdVars, const uChar *ndfdVars,
                       size_t *numElem, genElemDescript ** elem)
{
   size_t i;            /* Loop variable over number of elements. */
   size_t valElem;      /* The current location in elem to copy to. */
   sChar f_force;       /* Flag whether the caller forced an element. */

   myAssert (NDFD_MATCHALL + 1 == NdfdElementsLen);
   myAssert (*elem == NULL);
   myAssert (*numElem == 0);

   for (i = 0; i < numNdfdVars; i++) {
      if (ndfdVars[i] < NDFD_MATCHALL + 1) {
         varFilter[ndfdVars[i]]++;
      }
   }
   /* If the user provided no elements, then treat it as if user had set all
    * of them. */
   if (numNdfdVars == 0) {
      /* Check if program forced a choice. */
      f_force = 0;
      for (i = 0; i < NDFD_MATCHALL + 1; i++) {
         if (varFilter[i] > 1) {
            f_force = 1;
         }
      }
      if (!f_force) {
         for (i = 0; i < NDFD_MATCHALL + 1; i++) {
            varFilter[i]++;
         }
      }
   }

   *numElem = NDFD_MATCHALL + 1;
   *elem = (genElemDescript *) malloc (*numElem * sizeof (genElemDescript));
   /* Walk through varFilter, when it is >= 2, copy it to valElem location. */
   valElem = 0;
   for (i = 0; i < NDFD_MATCHALL + 1; i++) {
      if (varFilter[i] >= 2) {
         memcpy (&((*elem)[valElem]), &(NdfdElements[i]),
                 sizeof (genElemDescript));
         valElem++;
      }
   }
   /* Reduce numElem to correct valElem */
   *numElem = valElem;
}

/*****************************************************************************
 * genMatchInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Initialize a match structure.
 *
 * ARGUMENTS
 * match = A pointer to the match to init. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genMatchInit (genMatchType * match)
{
   genElemInit (&(match->elem));
   match->value = NULL;
   match->numValue = 0;
   match->unit = NULL;
}

/*****************************************************************************
 * genValueFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free the data assocoated with a value.
 *
 * ARGUMENTS
 * value = A pointer to the value to free. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void genValueFree (genValueType * value)
{
   if ((value->valueType == 1) || (value->valueType == 2)) {
      if (value->str != NULL) {
         free (value->str);
         value->str = NULL;
      }
   }
}

/*****************************************************************************
 * genMatchFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free the data assocoated with a match.
 *
 * ARGUMENTS
 * match = A pointer to the match to free. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genMatchFree (genMatchType * match)
{
   size_t i;            /* Loop variable over number of values. */

   genElemFree (&(match->elem));
   if (match->value != NULL) {
      for (i = 0; i < match->numValue; i++) {
         genValueFree (match->value + i);
      }
      free (match->value);
      match->value = NULL;
   }
   if (match->unit != NULL) {
      free (match->unit);
   }
   match->numValue = 0;
   return;
}

/*****************************************************************************
 * genElemMatchMeta() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Determine if the meta data matches what the element is looking for.
 *
 * ARGUMENTS
 * elem = An element description structure to try to match. (Input)
 * meta = The meta data associated with the current grid. (Input)
 *
 * RETURNS: int
 *   1 if they match, 0 if they don't match.
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static int genElemMatchMeta (const genElemDescript * elem,
                             const grib_MetaData *meta)
{
   if ((elem->center != MISSING_2) && (elem->center != meta->center))
      return 0;
   if ((elem->subcenter != MISSING_2) && (elem->subcenter != meta->subcenter))
      return 0;
   if ((elem->version != 0) && (elem->version != meta->GribVersion))
      return 0;
   /* Those are all the current checks for non-GRIB2 data. */
   if (meta->GribVersion != 2)
      return 1;

   if ((elem->genID != MISSING_1) && (elem->genID != meta->pds2.sect4.genID))
      return 0;
   if ((elem->prodType != MISSING_1) &&
       (elem->prodType != meta->pds2.prodType))
      return 0;
   if ((elem->templat != MISSING_2) &&
       (elem->templat != meta->pds2.sect4.templat))
      return 0;
   if ((elem->cat != MISSING_1) && (elem->cat != meta->pds2.sect4.cat))
      return 0;
   if ((elem->subcat != MISSING_1) &&
       (elem->subcat != meta->pds2.sect4.subcat))
      return 0;
   if (((meta->pds2.sect4.templat == 8) ||
        (meta->pds2.sect4.templat == 9)) &&
       (meta->pds2.sect4.numInterval == 1)) {
      if ((elem->lenTime != 0) &&
          (elem->lenTime != meta->pds2.sect4.Interval[0].lenTime))
         return 0;
   }
   if (elem->surfType != MISSING_1) {
      if (elem->surfType != meta->pds2.sect4.fstSurfType)
         return 0;
      if (elem->value != meta->pds2.sect4.fstSurfValue)
         return 0;
      if (elem->sndValue != meta->pds2.sect4.sndSurfValue)
         return 0;
   }
   return 1;
}
#endif

/*****************************************************************************
 * setGenElem() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Set an element descriptor based on what is in the meta data from the
 * current grid.
 *
 * ARGUMENTS
 * elem = The element description structure to set. (Output)
 * meta = The meta data associated with the current grid. (Input)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void setGenElem (genElemDescript * elem, const grib_MetaData *meta)
{
   uShort2 i;           /* Used to help find the ndfdEnum value. */

   elem->center = meta->center;
   elem->subcenter = meta->subcenter;
   elem->version = meta->GribVersion;
   elem->ndfdEnum = NDFD_UNDEF;
   /* Those are all the current variables to set for non-GRIB2 data. */
   if (meta->GribVersion != 2)
      return;

   elem->genID = meta->pds2.sect4.genID;
   elem->prodType = meta->pds2.prodType;
   elem->templat = meta->pds2.sect4.templat;
   elem->cat = meta->pds2.sect4.cat;
   elem->subcat = meta->pds2.sect4.subcat;
   if (((meta->pds2.sect4.templat == 8) ||
        (meta->pds2.sect4.templat == 9)) &&
       (meta->pds2.sect4.numInterval == 1)) {
      elem->lenTime = meta->pds2.sect4.Interval[0].lenTime;
   } else {
      elem->lenTime = 0;
   }
   elem->surfType = meta->pds2.sect4.fstSurfType;
   elem->value = meta->pds2.sect4.fstSurfValue;
   elem->sndValue = meta->pds2.sect4.sndSurfValue;

   /* Determine the ndfdEnum for this element. Note, ndfdEnum is already
    * initialized to NDFD_UNDEF. */
   for (i = 0; i < NdfdElementsLen; i++) {
      if ((NdfdElements[i].version == elem->version) &&
          (NdfdElements[i].center == elem->center) &&
          (NdfdElements[i].subcenter == elem->subcenter) &&
          (NdfdElements[i].genID == elem->genID) &&
          (NdfdElements[i].prodType == elem->prodType) &&
          (NdfdElements[i].templat == elem->templat) &&
          (NdfdElements[i].cat == elem->cat) &&
          (NdfdElements[i].subcat == elem->subcat) &&
          (NdfdElements[i].lenTime == elem->lenTime) &&
          (NdfdElements[i].surfType == elem->surfType) &&
          (NdfdElements[i].value == elem->value) &&
          (NdfdElements[i].sndValue == elem->sndValue)) {
         elem->ndfdEnum = NdfdElements[i].ndfdEnum;
         break;
      }
   }
#ifdef DEBUG
   if (i == NdfdElementsLen) {
      printf ("%d %d %d %d %d %d %d %d %ld %d %f %f\n", elem->version,
              elem->center, elem->subcenter, elem->genID, elem->prodType,
              elem->templat, elem->cat, elem->subcat, elem->lenTime,
              elem->surfType, elem->value, elem->sndValue);
   }
#endif
}
#endif

/*****************************************************************************
 * getValAtPnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a grid and a point, determine the value at that point.
 *
 * ARGUMENTS
 * gribDataLen = Length of gribData. (Input)
 *    gribData = The current grid. (Input)
 *         map = The current map transformation (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *        pntX = The point in question (X component) (Input)
 *        pntY = The point in question (Y component) (Input)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *      f_miss = Missing management: 0 none, 1 pri, 2 pri & sec. (Input)
 *     missPri = Primary missing value. (Input)
 *     missSec = Secondary missing value. (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *         ans = The desired value.
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void getValAtPnt (sInt4 gribDataLen, const double *gribData,
                         myMaparam *map, sChar f_pntType, double pntX,
                         double pntY, sInt4 Nx, sInt4 Ny, uChar f_miss,
                         double missPri, double missSec, uChar f_interp,
                         double *ans)
{
   double newX, newY;   /* The location of point on the grid. */
   sInt4 x1, y1;        /* f_interp=0, The nearest grid point, Otherwise
                         * corners of bounding box around point */
   sInt4 x2, y2;        /* Corners of bounding box around point. */
   sInt4 row;           /* The index into gribData for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   double d11, d12, d21, d22; /* values of bounding box corners. */
   double d_temp1, d_temp2; /* Temp storage during interpolation. */

   myAssert (Nx * Ny >= gribDataLen);

   /* Get point on the grid. */
   myAssert ((f_pntType == 0) || (f_pntType == 1));
   if (f_pntType == 0) {
      myCll2xy (map, pntY, pntX, &newX, &newY);
   } else {
      newX = pntX;
      newY = pntY;
   }

   /* Find value at the nearest grid cell. */
   if (!f_interp) {
      /* make sure x1 and y1 are in bounds. */
      x1 = (sInt4) (newX + .5);
      if ((x1 < 1) || (x1 > Nx)) {
         *ans = missPri;
         return;
      }
      y1 = (sInt4) (newY + .5);
      if ((y1 < 1) || (y1 > Ny)) {
         *ans = missPri;
         return;
      }

      /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
      row = (x1 - 1) + (y1 - 1) * Nx;
      myAssert ((row >= 0) && (row < gribDataLen));
      *ans = gribData[row];
      return;
   }

   /* Perform bi-linear interpolation. */
   x1 = (sInt4) newX;
   x2 = x1 + 1;
   y1 = (sInt4) newY;
   y2 = y1 + 1;
   if ((x1 < 1) || (x2 > Nx) || (y1 < 1) || (y2 > Ny)) {
      if (map->f_latlon) {
         /* Find out if we can do a border interpolation. */
         *ans = BiLinearBorder (gribData, map, newX, newY, Nx, Ny, f_miss,
                                missPri, missSec);
      } else {
         *ans = missPri;
      }
      return;
   }

   /* Get the first (1,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x1 - 1) + (y1 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d11 = gribData[row];
   if ((d11 == missPri) || ((f_miss == 2) && (d11 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Get the second (1,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x1 - 1) + (y2 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d12 = gribData[row];
   if ((d12 == missPri) || ((f_miss == 2) && (d12 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Get the third (2,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x2 - 1) + (y1 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d21 = gribData[row];
   if ((d21 == missPri) || ((f_miss == 2) && (d21 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Get the fourth (2,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x2 - 1) + (y2 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d22 = gribData[row];
   if ((d22 == missPri) || ((f_miss == 2) && (d22 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Do Bi-linear interpolation to get value. */
   d_temp1 = d11 + (newX - x1) * (d11 - d12) / (x1 - x2);
   d_temp2 = d21 + (newX - x1) * (d21 - d22) / (x1 - x2);
   *ans = (d_temp1 + (newY - y1) * (d_temp1 - d_temp2) / (y1 - y2));
}
#endif

/*****************************************************************************
 * getCubeValAtPnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a cube file and a point, determine the value at that point.
 *
 * ARGUMENTS
 *        data = The opened data cube to read from. (Input)
 *  dataOffset = The starting offset in the data cube file. (Input)
 *        scan = The scan mode of the data cube file (0 or 64) (Input)
 * f_bigEndian = Endian'ness of the data cube file (1=Big, 0=Lit) (Input)
 *         map = The current map transformation (Input)
 *        pntX = The point in question (in grid cell space) (Input)
 *        pntY = The point in question (in grid cell space) (Input)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *         ans = The desired value. (Output)
 *
 * RETURNS: void
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 * Doesn't handle border interpolation exception for lat/lon grids.
 *****************************************************************************
 */
static void getCubeValAtPnt (FILE *data, sInt4 dataOffset, uChar scan,
                             uChar f_bigEndian, myMaparam *map, double pntX,
                             double pntY, sInt4 Nx, sInt4 Ny, uChar f_interp,
                             float *ans)
{
   sInt4 offset;        /* Where the current data is in the data file. */
   sInt4 x1, y1;        /* f_interp=0, The nearest grid point, Otherwise
                         * corners of bounding box around point */
   sInt4 x2, y2;        /* Corners of bounding box around point. */
   float missPri = 9999; /* Missing value to use with cube's is always 9999 */
   float d11, d12, d21, d22; /* values of bounding box corners. */
   double d_temp1, d_temp2; /* Temp storage during interpolation. */

   /* Find value at the nearest grid cell. */
   if (!f_interp) {
      /* make sure newX and newY are in bounds. */
      x1 = (sInt4) (pntX + .5);
      if ((x1 < 1) || (x1 > Nx)) {
         *ans = missPri;
         return;
      }
      y1 = (sInt4) (pntY + .5);
      if ((y1 < 1) || (y1 > Ny)) {
         *ans = missPri;
         return;
      }

      if (scan == 0) {
         offset = dataOffset + (((x1 - 1) + ((Ny - 1) - (y1 - 1)) * Nx) *
                                sizeof (float));
      } else {
         offset = dataOffset + ((x1 - 1) + (y1 - 1) * Nx) * sizeof (float);
      }
      fseek (data, offset, SEEK_SET);
      if (f_bigEndian) {
         FREAD_BIG (ans, sizeof (float), 1, data);
      } else {
         FREAD_LIT (ans, sizeof (float), 1, data);
      }
      return;
   }

   /* Perform bi-linear interpolation. */
   x1 = (sInt4) pntX;
   x2 = x1 + 1;
   y1 = (sInt4) pntY;
   y2 = y1 + 1;
   if ((x1 < 1) || (x2 > Nx) || (y1 < 1) || (y2 > Ny)) {
      *ans = missPri;
      myAssert (!map->f_latlon);
      /* For latlon grids, would try BiLinearBorder here.  We assume that
       * since this is a cube routine, that the cubes would not contain
       * lat/lon grids where the deltaX is such that 1 cell from the right
       * edge of the grid is on the left edge of the grid.  If they did,
       * then_ handling the probes that are outside the grid wasn't worth the
       * effort, and one could just return missing. Technically,
       * BiLinearBorder could be rewritten so that it doesn't need a map, as
       * we already have x1, y1, Nx, Ny, and from those could calcualate the
       * appropriate points. */
      return;
   }

   /* Get the (1,1) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x1 - 1) + ((Ny - 1) - (y1 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x1 - 1) + (y1 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d11, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d11, sizeof (float), 1, data);
   }
   if (d11 == missPri) {
      *ans = missPri;
      return;
   }

   /* Get the (1,2) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x1 - 1) + ((Ny - 1) - (y2 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x1 - 1) + (y2 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d12, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d12, sizeof (float), 1, data);
   }
   if (d12 == missPri) {
      *ans = missPri;
      return;
   }

   /* Get the (2,1) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x2 - 1) + ((Ny - 1) - (y1 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x2 - 1) + (y1 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d21, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d21, sizeof (float), 1, data);
   }
   if (d21 == missPri) {
      *ans = missPri;
      return;
   }

   /* Get the (2,2) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x2 - 1) + ((Ny - 1) - (y2 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x2 - 1) + (y2 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d22, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d22, sizeof (float), 1, data);
   }
   if (d21 == missPri) {
      *ans = missPri;
      return;
   }

   /* Do Bi-linear interpolation to get value. */
   d_temp1 = d11 + (pntX - x1) * (d11 - d12) / (x1 - x2);
   d_temp2 = d21 + (pntX - x1) * (d21 - d22) / (x1 - x2);
   *ans = (d_temp1 + (pntY - y1) * (d_temp1 - d_temp2) / (y1 - y2));
}

/*****************************************************************************
 * getWxString() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a value at a point, look up the weather information in the weather
 * table, and store it in str.
 *
 * ARGUMENTS
 *       str = Where to store the answer (Output)
 *   wxIndex = The index into the wx table (as a double) (Input)
 *        wx = The weather table associated with this grid (Input)
 * f_wxParse = 0 => store ugly string, 1 => store English Translation,
 *             2 => store -SimpleWx code. (Input)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void getWxString (char **str, sInt4 wxIndex, const sect2_WxType *wx,
                         sChar f_WxParse)
{
   int j;               /* loop counter over the weather keys. */

   if ((wxIndex < 0) || (wxIndex >= (sInt4) wx->dataLen)) {
      mallocSprintf (str, "%ld", wxIndex);
      return;
   }
   /* Print out the weather string according to f_WxParse. */
   switch (f_WxParse) {
      case 0:
         *str = (char *) malloc (strlen (wx->data[wxIndex]) + 1);
         strcpy ((*str), wx->data[wxIndex]);
         return;
      case 1:
         *str = NULL;
         for (j = 0; j < NUM_UGLY_WORD; j++) {
            if (wx->ugly[wxIndex].english[j] == NULL) {
               if (j == 0) {
                  reallocSprintf (str, "No Weather");
               }
               return;
            }
            if (j != 0) {
               if (j == wx->ugly[wxIndex].numValid - 1) {
                  reallocSprintf (str, " and ");
               } else {
                  reallocSprintf (str, ", ");
               }
            }
            reallocSprintf (str, "%s", wx->ugly[wxIndex].english[j]);
         }
         return;
      case 2:
         mallocSprintf (str, "%d", wx->ugly[wxIndex].SimpleCode);
         return;
   }
}
#endif

/*****************************************************************************
 * genFillValue() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a grid and a set of points, determine the values for those points.
 *
 * ARGUMENTS
 * gribDataLen = Length of gribData. (Input)
 *    gribData = The current grid. (Input)
 *      grdAtt = Attributes about the current grid (ie missing values). (In)
 *         map = Map projection to use to convert from lat/lon to x/y (In)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *          wx = The weather table associated with this grid (Input)
 *   f_wxParse = 0 => store ugly string, 1 => store English Translation,
 *               2 => store -SimpleWx code. (Input)
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe. (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *       value = The values at the points (already alloced to numPnts. (Out)
 *
 * RETURNS: void
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void genFillValue (sInt4 gribDataLen, const double *gribData,
                          const gridAttribType *grdAtt, myMaparam *map,
                          sInt4 Nx, sInt4 Ny, uChar f_interp,
                          const sect2_WxType *wx, sChar f_WxParse,
                          size_t numPnts, const Point * pnts,
                          sChar f_pntType, genValueType * value)
{
   double missing;      /* Missing value to use. */
   size_t i;            /* loop counter over number of points. */
   double ans;          /* The grid value at the current point. */

   /* getValAtPnt does not allow f_pntType == 2 */
   myAssert (f_pntType != 2);

   /* Figure out a missing value, to assist with interpolation. */
   if (grdAtt->f_miss == 0) {
      missing = 9999;
      if (grdAtt->f_maxmin) {
         if ((missing <= grdAtt->max) && (missing >= grdAtt->min)) {
            missing = grdAtt->max + 1;
         }
      }
   } else {
      missing = grdAtt->missPri;
   }

   /* Loop over the points. */
   for (i = 0; i < numPnts; i++) {
      /* make sure that !(f_interp && (wx != NULL)) */
      myAssert ((!f_interp) || (wx == NULL));
      getValAtPnt (gribDataLen, gribData, map, f_pntType, pnts[i].X,
                   pnts[i].Y, Nx, Ny, grdAtt->f_miss, missing,
                   grdAtt->missSec, f_interp, &ans);
      if (ans == missing) {
         value[i].valueType = 2;
         value[i].data = ans;
         if (wx == NULL) {
            value[i].str = NULL;
         } else {
            mallocSprintf (&(value[i].str), "%.0f", ans);
         }
      } else {
         if (wx == NULL) {
            value[i].valueType = 0;
            value[i].data = ans;
            value[i].str = NULL;
         } else {
            value[i].valueType = 1;
            value[i].data = 0;
            getWxString (&(value[i].str), (sInt4) ans, wx, f_WxParse);
         }
      }
   }
}
#endif

/*****************************************************************************
 * genCubeFillValue() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a cube file and a set of points, determine the values for those
 * points.
 *
 * ARGUMENTS
 *        data = The opened data cube to read from. (Input)
 *  dataOffset = The starting offset in the data cube file. (Input)
 *        scan = The scan mode of the data cube file (0 or 64) (Input)
 * f_bigEndian = Endian'ness of the data cube file (1=Big, 0=Lit) (Input)
 *         map = The current map transformation (Input)
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe (at this point in grid cell units). (In)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *    elemEnum = The NDFD element enumeration for this grid (Input)
 *    numTable = Number of strings in the table (Input)
 *       table = Table of strings associated with this PDS (Input)
 *   f_wxParse = 0 => store ugly string, 1 => store English Translation,
 *               2 => store -SimpleWx code. (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *       value = The values at the points (already alloced to numPnts. (Out)
 *
 * RETURNS: void
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void genCubeFillValue (FILE *data, sInt4 dataOffset, uChar scan,
                              uChar f_bigEndian, myMaparam *map,
                              size_t numPnts, const Point * pnts, sInt4 Nx,
                              sInt4 Ny, uChar f_interp, uChar elemEnum,
                              uShort2 numTable, char **table,
                              sChar f_WxParse, sChar f_SimpleVer,
                              genValueType * value)
{
   size_t i;            /* loop counter over number of points. */
   float ans;           /* The current cell value. */
   uShort2 wxIndex;     /* 'value' cast to an integer for table lookup. */
   size_t j;            /* Counter used to print "english" weather. */
   UglyStringType ugly; /* Used to 'translate' the weather keys. */

   myAssert ((scan == 0) || (scan == 64));
   myAssert (sizeof (float) == 4);
   myAssert (((elemEnum == NDFD_WX) && (numTable != 0)) ||
             ((elemEnum != NDFD_WX) && (numTable == 0)));

   /* Check if the user is trying to bi-linear interpolate weather. */
   if (elemEnum == NDFD_WX) {
      f_interp = 0;
   }

   for (i = 0; i < numPnts; i++) {
      getCubeValAtPnt (data, dataOffset, scan, f_bigEndian, map, pnts[i].X,
                       pnts[i].Y, Nx, Ny, f_interp, &ans);

      /* 9999 is the missing value for data cubes */
      if (ans == 9999) {
         value[i].valueType = 2;
         value[i].data = ans;
         if (elemEnum != NDFD_WX) {
            value[i].str = NULL;
         } else {
            value[i].str = (char *) malloc (4 + 1);
            strcpy (value[i].str, "9999");
         }
      } else {
         if (elemEnum != NDFD_WX) {
            value[i].valueType = 0;
            value[i].data = ans;
            value[i].str = NULL;
         } else {
            wxIndex = (uShort2) ans;
            if ((numTable == 0) || (wxIndex >= numTable)) {
               value[i].valueType = 2;
               value[i].data = wxIndex;
               mallocSprintf (&(value[i].str), "%ld", wxIndex);
            } else {
               value[i].valueType = 1;
               value[i].data = 0;
               switch (f_WxParse) {
                  case 0:
                     value[i].str =
                           (char *) malloc (strlen (table[wxIndex]) + 1);
                     strcpy (value[i].str, table[wxIndex]);
                     break;
                  case 1:
                     ParseUglyString (&ugly, table[wxIndex], f_SimpleVer);
                     value[i].str = NULL;
                     for (j = 0; j < NUM_UGLY_WORD; j++) {
                        if (ugly.english[j] == NULL) {
                           if (j == 0) {
                              reallocSprintf (&(value[i].str), "No Weather");
                           }
                           break;
                        }
                        if (j != 0) {
                           if (j + 1 == ugly.numValid) {
                              reallocSprintf (&(value[i].str), " and ");
                           } else {
                              reallocSprintf (&(value[i].str), ", ");
                           }
                        }
                        reallocSprintf (&(value[i].str), "%s",
                                        ugly.english[j]);
                     }
                     FreeUglyString (&ugly);
                     break;
                  case 2:
                     ParseUglyString (&ugly, table[wxIndex], f_SimpleVer);
                     mallocSprintf (&(value[i].str), "%d", ugly.SimpleCode);
                     FreeUglyString (&ugly);
               }
            }
         }
      }
   }
}

/*****************************************************************************
 * genProbeGrib() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probe a given GRIB file for all messages that match the element criteria,
 * and return the values of the given set of points inside the match
 * structure.
 *
 * ARGUMENTS
 *          fp = Opened GRIB file ready to be read. (Input)
 *                      POINT FILTERING INFO
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe. (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *                      ELEMENT FILTERING INFO
 *     numElem = Number of elements in element filter list. (Input)
 *        elem = Return only data found in this list.
 *               Use NDFD_MATCHALL, to get all the elements. (Input)
 *                      TIME FILTERING INFO
 *   f_valTime = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *               3 both f_validStartTime, and f_validEndTime (Input)
 *   startTime = first valid time that we are interested in. (Input)
 *     endTime = last valid time that we are interested in. (Input)
 *                      SPECIAL FLAGS
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *      f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m (Input)
 *    majEarth = Use this to override the majEarth (< 6000 ignored) (Input)
 *    minEarth = Use this to override the minEarth (< 6000 ignored) (Input)
 *   f_wxParse = 0 => ugly string, 1 => English Translation,
 *               2 => -SimpleWx code. (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *                      OUTPUT
 *    numMatch = Number of matches found. (Output)
 *       match = Matches. (Output)
 *
 * RETURNS: int
 *   -1 = problems reading a GRIB message
 *   -2 = problems with the Grid Definition Section.
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static int genProbeGrib (FILE *fp, size_t numPnts, const Point * pnts,
                         sChar f_pntType, size_t numElem,
                         const genElemDescript * elem, sChar f_valTime,
                         double startTime, double endTime, uChar f_interp,
                         sChar f_unit, double majEarth, double minEarth,
                         sChar f_WxParse, sChar f_SimpleVer,
                         size_t *numMatch, genMatchType ** match)
{
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
   int subgNum;         /* Subgrid in the msg that we are interested in. */
   uInt4 gribDataLen;   /* Current length of gribData. */
   double *gribData;    /* Holds the grid retrieved from a GRIB2 message. */
   int c;               /* Determine if end of the file without fileLen. */
   sInt4 f_lstSubGrd;   /* 1 if we read the last subGrid in a message */
   LatLon lwlf;         /* ReadGrib2Record allows subgrids.  We want entire
                         * grid, so set the lat to -100. */
   LatLon uprt;         /* ReadGrib2Record allows subgrids.  We want entire
                         * grid, so set the lat to -100. */
   genMatchType *curMatch; /* The current match */
   size_t i;            /* Loop counter while searching for a matching elem */
   double validTime;    /* The current grid's valid time. */
   myMaparam map;       /* The current grid's map parameter. */

   /* getValAtPnt does not currently allow f_pntType == 2 */
   myAssert (f_pntType != 2);

   /* Initialize data and structures used when unpacking a message */
   IS_Init (&is);
   MetaInit (&meta);
   f_lstSubGrd = 1;
   subgNum = 0;
   lwlf.lat = -100;
   uprt.lat = -100;
   gribDataLen = 0;
   gribData = NULL;

   /* Start loop for all messages. */
   while ((c = fgetc (fp)) != EOF) {
      ungetc (c, fp);

      /* Improve this by reading the message into memory, parsing key peices
       * out of it, and returning them to the caller.  Then if caller still
       * interested call ReadGrib2Record to break up the message. */

      /* Read the GRIB message. */
      if (ReadGrib2Record (fp, f_unit, &gribData, &gribDataLen, &meta,
                           &is, subgNum, majEarth, minEarth, f_SimpleVer,
                           &f_lstSubGrd, &(lwlf), &(uprt)) != 0) {
         preErrSprintf ("ERROR: In call to ReadGrib2Record.\n");
         free (gribData);
         IS_Free (&is);
         MetaFree (&meta);
         return -1;
      }
      if (!f_lstSubGrd) {
         subgNum++;
      } else {
         subgNum = 0;
      }

      /* Check if we're interested in this data based on validTime. */
      if (meta.GribVersion == 2) {
         validTime = meta.pds2.sect4.validTime;
      } else if (meta.GribVersion == 1) {
         validTime = meta.pds1.validTime;
      } else if (meta.GribVersion == -1) {
         validTime = meta.pdsTdlp.refTime + meta.pdsTdlp.project;
      } else {
         MetaFree (&meta);
         continue;
      }
      if ((f_valTime & 1) && (validTime < startTime)) {
         MetaFree (&meta);
         continue;
      }
      if ((f_valTime & 2) && (validTime > endTime)) {
         MetaFree (&meta);
         continue;
      }

      /* Check if we're interested in this data based on an element match. */
      for (i = 0; i < numElem; i++) {
         if (genElemMatchMeta (&(elem[i]), &meta) == 1) {
            break;
         }
      }
      if (i == numElem) {
         MetaFree (&meta);
         continue;
      }

      /* Check that gds is valid before setting up map projection. */
      if ((GDSValid (&(meta.gds)) != 0) ||
          (meta.gds.Nx * meta.gds.Ny < gribDataLen)) {
         preErrSprintf ("ERROR: Sect3 was not Valid.\n");
         free (gribData);
         IS_Free (&is);
         MetaFree (&meta);
         return -2;
      }
      SetMapParam (&map, &(meta.gds));

      /* Have determined that this is a good match, allocate memory */
      *numMatch = *numMatch + 1;
      *match =
            (genMatchType *) realloc (*match,
                                      (*numMatch) * sizeof (genMatchType));
      curMatch = &((*match)[*numMatch - 1]);

      /* Might try to use genElemMatchMeta info to help with the enum type.
       * Note: Can't just init the elem type since the data could be
       * NDFD_UNDEF, so we need to call setGenElem. */
      setGenElem (&(curMatch->elem), &meta);
#ifdef DEBUG
      if (curMatch->elem.ndfdEnum != elem[i].ndfdEnum) {
         printf ("%d %d\n", curMatch->elem.ndfdEnum, elem[i].ndfdEnum);
      }
      myAssert (curMatch->elem.ndfdEnum == elem[i].ndfdEnum);
#endif

      /* Set other meta info about the match. */
      if (meta.GribVersion == 2) {
         curMatch->refTime = meta.pds2.refTime;
      } else if (meta.GribVersion == 1) {
         curMatch->refTime = meta.pds1.refTime;
      } else if (meta.GribVersion == -1) {
         curMatch->refTime = meta.pdsTdlp.refTime;
      }
      curMatch->validTime = validTime;
      curMatch->unit = (char *) malloc (strlen (meta.unitName) + 1);
      strcpy (curMatch->unit, meta.unitName);

      /* fill in the value structure. */
      curMatch->numValue = numPnts;
      curMatch->value =
            (genValueType *) malloc (numPnts * sizeof (genValueType));
      if ((meta.GribVersion != 2) || (strcmp (meta.element, "Wx") != 0)) {
         genFillValue (gribDataLen, gribData, &(meta.gridAttrib), &map,
                       meta.gds.Nx, meta.gds.Ny, f_interp, NULL, f_WxParse,
                       numPnts, pnts, f_pntType, curMatch->value);
      } else {
         genFillValue (gribDataLen, gribData, &(meta.gridAttrib), &map,
                       meta.gds.Nx, meta.gds.Ny, f_interp,
                       &(meta.pds2.sect2.wx), f_WxParse, numPnts, pnts,
                       f_pntType, curMatch->value);
      }

      MetaFree (&meta);
   }
   IS_Free (&is);
   free (gribData);
   return 0;
}
#endif

/*****************************************************************************
 * genProbeGrib() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probe a given Cube file for all messages that match the element criteria,
 * and return the values of the given set of points inside the match
 * structure.
 *
 * ARGUMENTS
 *    filename = The name of the cube index file. (Input)
 *                      POINT FILTERING INFO
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe. (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *                      ELEMENT FILTERING INFO
 *     numElem = Number of elements in element filter list. (Input)
 *        elem = Return only data found in this list.
 *               Use NDFD_MATCHALL, to get all the elements. (Input)
 *                      TIME FILTERING INFO
 *   f_valTime = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *               3 both f_validStartTime, and f_validEndTime (Input)
 *   startTime = first valid time that we are interested in. (Input)
 *     endTime = last valid time that we are interested in. (Input)
 *                      SPECIAL FLAGS
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *      f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m (Input)
 *    majEarth = Use this to override the majEarth (< 6000 ignored) (Input)
 *    minEarth = Use this to override the minEarth (< 6000 ignored) (Input)
 *   f_wxParse = 0 => ugly string, 1 => English Translation,
 *               2 => -SimpleWx code. (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *                      OUTPUT
 *    numMatch = Number of matches found. (Output)
 *       match = Matches. (Output)
 *
 * RETURNS: int
 *   -1 = problems reading a GRIB message
 *   -2 = problems with the Grid Definition Section.
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static int genProbeCube (const char *filename, size_t numPnts,
                         const Point * pnts, sChar f_pntType, size_t numElem,
                         const genElemDescript * elem, sChar f_valTime,
                         double startTime, double endTime, uChar f_interp,
                         sChar f_unit, double majEarth, double minEarth,
                         sChar f_WxParse, sChar f_SimpleVer,
                         size_t *numMatch, genMatchType ** match)
{
   char *flxArray = NULL; /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   char *ptr;           /* A pointer to where we are in the array. */
   uShort2 numGDS;      /* # of GDS Sections. */
   uShort2 numSupPDS;   /* # of Super PDS Sections. */
   char *sPtr;          /* A pointer to the current SuperPDS. */
   sInt4 lenTotPds;     /* Length of the total PDS record */
   size_t i;            /* Loop counter over SuperPDS. */
   size_t jj;           /* Loop over the desired elements */
   char elemName[256];  /* A holder for element from meta data. */
   double refTime;      /* Reference time of this data set. */
   char unit[256];      /* A holder for unit for this data. */
   char comment[256];   /* A holder for comment from meta data. */
   uShort2 gdsNum;      /* Which GDS is associated with this data. */
   uShort2 center;      /* The center that created this data */
   uShort2 subCenter;   /* The subCenter that created this data */
   uShort2 numPDS;      /* number of PDS Sections. */
   char *pdsPtr;        /* A pointer to the current PDS in the PDS array. */
   int j;               /* Loop counter over PDS Array. */
   double validTime;    /* Valid time of this PDS. */
   char dataFile[256];  /* A holder for the Data file for this record. */
   char curFile[256];   /* A holder for the Current Data file. */
   sInt4 dataOffset;    /* An offset into dataFile for this record. */
   uChar f_bigEndian;   /* Endian'ness of the data grid. */
   uChar scan;          /* Scan mode for the data grid. */
   uShort2 numTable = 0; /* Number of strings in the table */
   char **table = NULL; /* Table of strings associated with this PDS. */
   int k;               /* Loop counter over table entries. */
   int elemEnum;        /* The NDFD element enumeration for the read grid */
   int curGdsNum;       /* Which gdsNum currently in gds. */
   gdsType gds;         /* The current grid definition section. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   Point *gridPnts = NULL; /* Holds the converted to grid point points. */
   size_t ii;           /* Loop counter over number of points. */
   genMatchType *curMatch; /* The current match */
   char *dataName = NULL; /* The name of the current opened data file. */
   char *lastSlash;     /* A pointer to last slash in the index file. */
   FILE *data = NULL;   /* A pointer to the data file. */

   if (ReadFLX (filename, &flxArray, &flxArrayLen) != 0) {
      errSprintf ("Problems Reading %s\n", filename);
      return -1;
   }

   /* Start walking through the flx array. */
   ptr = flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2 + numGDS * GDSLEN;
   MEMCPY_LIT (&numSupPDS, ptr, sizeof (uShort2));
   ptr += 2;
   sPtr = ptr;

   curGdsNum = -1;
   if (f_pntType == 0) {
      gridPnts = (Point *) malloc (numPnts * sizeof (Point));
   }
   curFile[0] = '\0';
   if ((lastSlash = strrchr (filename, '/')) == NULL) {
      lastSlash = strrchr (filename, '\\');
   }

   for (i = 0; i < numSupPDS; i++) {
      ReadSupPDSBuff (sPtr, elemName, &refTime, unit, comment, &gdsNum,
                      &center, &subCenter, &numPDS, &pdsPtr, &lenTotPds);
      if (center != 8) {
         sPtr += lenTotPds;
         continue;
      }
      elemEnum = genNdfdVar_Lookup (elemName, 1, 0);
      if (elemEnum == NDFD_UNDEF) {
         sPtr += lenTotPds;
         continue;
      }

      /* Check if we're interested in this data based on an element match. */
      for (jj = 0; jj < numElem; jj++) {
         if (elem[jj].ndfdEnum == elemEnum) {
            break;
         }
      }
      if (jj == numElem) {
         sPtr += lenTotPds;
         continue;
      }

      for (j = 0; j < numPDS; j++) {
         ReadPDSBuff (pdsPtr, &validTime, dataFile, &dataOffset,
                      &f_bigEndian, &scan, &numTable, &table, &pdsPtr);

         /* Check if we're interested in this data based on validTime. */
         if (((f_valTime & 1) && (validTime < startTime)) ||
             ((f_valTime & 2) && (validTime > endTime))) {
            if (numTable != 0) {
               for (k = 0; k < numTable; k++) {
                  free (table[k]);
               }
               free (table);
               numTable = 0;
               table = NULL;
            }
            continue;
         }

         /* Interested in data. */
         /* Set up gds. */
         if (curGdsNum != gdsNum) {
            ReadGDSBuffer (flxArray + HEADLEN + 2 + (gdsNum - 1) * GDSLEN,
                           &gds);
            /* Check that gds is valid before setting up map projection. */
            if (GDSValid (&gds) != 0) {
               errSprintf ("ERROR: Sect3 was not Valid.\n");
               goto error;
            }
            SetMapParam (&map, &gds);

            /* Get points on the grid. */
            myAssert ((f_pntType == 0) || (f_pntType == 1));
            if (f_pntType == 0) {
               for (ii = 0; ii < numPnts; ii++) {
                  myCll2xy (&map, pnts[ii].Y, pnts[ii].X, &(gridPnts[ii].X),
                            &(gridPnts[ii].Y));
               }
            }
         }

         if (strcmp (curFile, dataFile) != 0) {
            if (lastSlash == NULL) {
               dataName = (char *) realloc (dataName, strlen (dataFile) + 1);
               strcpy (dataName, dataFile);
            } else {
               dataName = (char *) realloc (dataName,
                                            (lastSlash - filename) + 1 +
                                            strlen (dataFile) + 1);
               memcpy (dataName, filename, (lastSlash - filename) + 1);
               dataName[(lastSlash - filename) + 1] = '\0';
               strcat (dataName, dataFile);
            }
            strcpy (curFile, dataFile);
            if (data != NULL) {
               fclose (data);
            }
            if ((data = fopen (dataName, "rb")) == NULL) {
               errSprintf ("Problems opening %s\n", dataName);
               goto error;
            }
         }

         /* Have determined that this is a good match, allocate memory */
         *numMatch = *numMatch + 1;
         *match = (genMatchType *) realloc (*match,
                                            (*numMatch) *
                                            sizeof (genMatchType));
         curMatch = &((*match)[*numMatch - 1]);

         /* Set Element info about the match */
         memcpy (&(curMatch->elem), &(NdfdElements[elemEnum]),
                 sizeof (genElemDescript));
         /* Set other meta info about the match. */
         curMatch->refTime = refTime;
         curMatch->validTime = validTime;
         curMatch->unit = (char *) malloc (strlen (unit) + 1);
         strcpy (curMatch->unit, unit);

         /* Fill the value structure. */
         curMatch->numValue = numPnts;
         curMatch->value = (genValueType *) malloc (numPnts *
                                                    sizeof (genValueType));
         /* Read from data, and fill in the value. */
         if (f_pntType == 0) {
            genCubeFillValue (data, dataOffset, scan, f_bigEndian, &map,
                              numPnts, gridPnts, gds.Nx, gds.Ny, f_interp,
                              elemEnum, numTable, table, f_WxParse,
                              f_SimpleVer, curMatch->value);
         } else {
            genCubeFillValue (data, dataOffset, scan, f_bigEndian, &map,
                              numPnts, pnts, gds.Nx, gds.Ny, f_interp,
                              elemEnum, numTable, table, f_WxParse,
                              f_SimpleVer, curMatch->value);
         }

         if (numTable != 0) {
            for (k = 0; k < numTable; k++) {
               free (table[k]);
            }
            free (table);
            numTable = 0;
            table = NULL;
         }
      }
      sPtr += lenTotPds;
   }

   if (data != NULL) {
      fclose (data);
   }
   if (dataName != NULL) {
      free (dataName);
   }
   if (gridPnts != NULL) {
      free (gridPnts);
   }
   free (flxArray);
   return 0;
 error:
   if (numTable != 0) {
      for (k = 0; k < numTable; k++) {
         free (table[k]);
      }
      free (table);
      numTable = 0;
      table = NULL;
   }

   if (data != NULL) {
      fclose (data);
   }
   if (dataName != NULL) {
      free (dataName);
   }
   if (gridPnts != NULL) {
      free (gridPnts);
   }
   free (flxArray);
   return -2;
}

/*****************************************************************************
 * genProbe() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probes the given files for data that matches the given elements.
 * Returns the values at the given points.
 *
 * ARGUMENTS
 *      numPnts = Number of points to look at. (Input)
 *         pnts = Points to look at. (Input)
 *    f_pntType = 0 => lat/lon pnts, 1 => cell X/Y pnts, 2 => all Cells. (In)
 *   numInFiles = Number of input files. (Input)
 *      inFiles = Input file names. (Input)
 *   f_fileType = Type of input files. (0=GRIB, 1=Data Cube index file) (Input)
 *     f_interp = 1 => interpolate to points, 0 => nearest neighbor. (Input)
 *       f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m (Input)
 *     majEarth = Use this to override the majEarth (< 6000 ignored) (Input)
 *     minEarth = Use this to override the minEarth (< 6000 ignored) (Input)
 *    f_wxParse = 0 => ugly string, 1 => English Translation,
 *                2 => -SimpleWx code. (Input)
 *  f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *      numElem = Number of elements in element filter list. (Input)
 *         elem = Return only data found in this list.
 *                Use NDFD_MATCHALL, to get all the elements. (Input)
 *    f_valTime = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *                3 both f_validStartTime, and f_validEndTime (Input)
 *    startTime = first valid time that we are interested in. (Input)
 *      endTime = last valid time that we are interested in. (Input)
 *     numMatch = Number of matches found. (Output)
 *        match = Matches. (Output)
 *    f_inTypes = File types of InFiles array. (Input)
 *   gribFilter = File filter used to find GRIB or data cube files. (Input)
 *    numSector = Number of sectors that points were found in. (Input)
 *       sector = Names of sectors that points were found in. (Input)
 * f_ndfdConven = NDFD Naming convention to use. (Input)
 *
 * RETURNS: int
 *   -1 = If numMatch or match is not 0 or NULL respectively to begin with.
 *   -2 = numInfiles < 1. (Should probably change to numInFiles = 0 => stdin)
 *   -3 = Problems opening an input file.
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *  1/2006 AAT: Modified so some matches will return values, and it will
 *         ignore bad files.
 *
 * NOTES:
 *   1) May want to add a valid time list to also match.
 *   2) Assumes that inFiles[0] = NULL implies use stdin.
 *
 *   3) with f_pntType = 1 or 2. may want to return to caller the lat/lon.
 *      Problem... lat/lon would theoretically change based on GDS.
 *****************************************************************************
 */
int genProbe (size_t numPnts, Point * pnts, sChar f_pntType,
              size_t numInFiles, char **inFiles, uChar f_fileType,
              uChar f_interp, sChar f_unit, double majEarth, double minEarth,
              sChar f_WxParse, sChar f_SimpleVer, size_t numElem,
              genElemDescript * elem, sChar f_valTime, double startTime,
              double endTime, size_t *numMatch, genMatchType ** match,
              char *f_inTypes, char *gribFilter, size_t numSector,
              char **sector, sChar f_ndfdConven)
{
#ifndef DP_ONLY
   FILE *fp;
#endif
   char f_stdin;
   size_t i;
#ifdef DEBUG
   char *msg;
#endif
   size_t numOutNames;
   char **outNames;

   myAssert (*numMatch == 0);
   myAssert (*match == NULL);
   /* Check input state. */
   if ((*numMatch != 0) || (*match != NULL))
      return -1;
   if (numInFiles < 1)
      return -2;

#ifdef DP_ONLY
   if (f_fileType == 0) {
      printf ("DP only executable doesn't handle -P option\n");
      myAssert (1 == 0);
      return -3;
   }
#endif

   myAssert (numInFiles > 0);
   f_stdin = (inFiles[0] == NULL);

   /* Assert that f_stdin does not apply to f_fileType == 1 */
   /* Actually it can, by assuming that datdName is in cur dir. */
   myAssert ((f_fileType != 1) || (!f_stdin));

   myAssert (numElem != 0);

   /* Expand the input files... */
   expandInName (numInFiles, inFiles, f_inTypes, gribFilter, numSector,
                 sector, f_ndfdConven, numElem, elem, &numOutNames,
                 &outNames);

   for (i = 0; i < numOutNames; i++) {
#ifndef DP_ONLY
      if (f_fileType == 0) {
         if ((i == 0) && f_stdin) {
            fp = stdin;
         } else {
            if ((fp = fopen (outNames[i], "rb")) == NULL) {
               continue;
            }
         }
         if (genProbeGrib (fp, numPnts, pnts, f_pntType, numElem, elem,
                           f_valTime, startTime, endTime, f_interp, f_unit,
                           majEarth, minEarth, f_WxParse, f_SimpleVer,
                           numMatch, match) != 0) {
#ifdef DEBUG
            msg = errSprintf (NULL);
            printf ("Error message was: '%s'\n", msg);
            free (msg);
#endif
            if (!f_stdin) {
#ifdef DEBUG
               printf ("\nProblems with GRIB file '%s'\n", outNames[i]);
#endif
               fclose (fp);
            }
            continue;
            /* return -3; */
         }
         if (!f_stdin) {
            fclose (fp);
         }
      } else {
#endif
         if (genProbeCube (outNames[i], numPnts, pnts, f_pntType, numElem,
                           elem, f_valTime, startTime, endTime, f_interp,
                           f_unit, majEarth, minEarth, f_WxParse,
                           f_SimpleVer, numMatch, match) != 0) {
#ifdef DEBUG
            msg = errSprintf (NULL);
            printf ("Error message was: '%s'\n", msg);
            free (msg);
            printf ("\nProblems with Index file '%s'\n", outNames[i]);
#endif
         }
         continue;
#ifndef DP_ONLY
      }
#endif
   }
#ifdef DEBUG
/*
         for (i=0; i < *numMatch; i++) {
            printf ("%d : %d : refTime %f valTime %f\n", i, (*match)[i].elem.ndfdEnum,
                    (*match)[i].refTime, (*match)[i].validTime);
         }
*/
#endif

   for (i = 0; i < numOutNames; i++) {
      free (outNames[i]);
   }
   free (outNames);
   return 0;
}

/*
Following is what fortran programmers wanted...
 * filename = The GRIB file to probe. (Input)
 *  numPnts = The number of points to probe. (Input)
 *      lat = The latitudes of the points to probe. (Input)
 *      lon = The longitudes of the points to probe. (Input)
 * f_interp = true (1) if we want to perform bi-linear interp
 *            false (0) if we want nearest neighbor (Input)
 *  lenTime = The number of messages (or validTimes) in the file (Output)
 *  valTime = The array of valid times (as strings). (Output)
 *     data = The values at the various points. (Output)
int GenericProbe (char *filename, int numPnts, double *lat, double *lon,
                  int f_interp, int *lenTime, char ***valTime,
                  double ***data)

GRIB2Probe (usr, &is, &meta, numPnts, pnts, labels, pntFiles,
                           f_pntType);
Grib2DataProbe (usr, numPnts, pnts, labels, pntFiles);
*/

/* Want to join together relevant parts of:
probe.c :: int GRIB2Probe (userType * usr, IS_dataType * is, grib_MetaData * meta)
and
cube.c :: int Grib2DataProbe (userType * usr)
*/

/*****************************************************************************
 * ReadPntFile() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Read in a set of points from pntFile, for use with the probe command.
 *
 * ARGUMENTS
 * pntFile = File to read the points in from. (Input)
 *    pnts = The points read in. (Input/Output)
 * NumPnts = The number of points read in (Input/Output)
 *  labels = The Labels for those points (Input/Output)
 *   files = The Output file for each point (Input/Output)
 *
 * FILES/DATABASES:
 *   A comma delimited file with (place, lat, lon) per line.
 *   A '#' at beginnig of line denotes line is commented out.
 *   Don't really need commas, just spaces.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = Ok.
 * -1 = Problems opening file for read.
 *
 * HISTORY
 *  12/2002 Arthur Taylor (MDL/RSIS): Created.
 *   8/2003 AAT: Found that it didn't handle "lat,lon" correctly.
 *   1/2005 AAT: Due to frequent requests no longer allow the space separator
 *               so that people's stations can have spaces.
 *   1/2005 AAT: Added an optional forth element which is what file to save a
 *               point to.
 *
 * NOTES
 *****************************************************************************
 */
static int ReadPntFile (char *pntFile, Point ** pnts, size_t *NumPnts,
                        char ***labels, char ***files)
{
   FILE *fp;            /* Ptr to point file. */
   char *buffer = NULL; /* Holds a line from the file. */
   size_t buffLen = 0;  /* Current length of buffer. */
   char *first;         /* The first phrase in buffer. */
   char *second;        /* The second phrase in buffer. */
   char *third;         /* The third phrase in buffer. */
   char *forth;         /* The forth phrase in buffer. */
   size_t numPnts;      /* Local count of number of points. */

   if ((fp = fopen (pntFile, "rt")) == NULL) {
      errSprintf ("ERROR: opening file %s for read", pntFile);
      return -1;
   }
   numPnts = *NumPnts;
   while (reallocFGets (&buffer, &buffLen, fp) > 0) {
/*      first = strtok (buffer, " ,\n"); */
      first = strtok (buffer, ",\n");
      if ((first != NULL) && (*first != '#')) {
/*         second = strtok (NULL, " ,\n"); */
         second = strtok (NULL, ",\n");
         if (second != NULL) {
            numPnts++;
            *pnts = (Point *) realloc ((void *) *pnts,
                                       numPnts * sizeof (Point));
            *labels = (char **) realloc ((void *) *labels,
                                         numPnts * sizeof (char *));
            *files = (char **) realloc ((void *) *files,
                                        numPnts * sizeof (char *));
/*            third = strtok (NULL, " ,\n"); */
            third = strtok (NULL, ",\n");
            if (third != NULL) {
               /* Assume: Name, lat, lon */
               (*pnts)[numPnts - 1].Y = atof (second);
               (*pnts)[numPnts - 1].X = atof (third);
               (*labels)[numPnts - 1] = (char *) malloc (strlen (first) + 1);
               strcpy ((*labels)[numPnts - 1], first);
               forth = strtok (NULL, ",\n");
               if (forth != NULL) {
                  strTrim (forth);
                  (*files)[numPnts - 1] =
                        (char *) malloc (strlen (forth) + 1);
                  strcpy ((*files)[numPnts - 1], forth);
               } else {
                  (*files)[numPnts - 1] = NULL;
               }
            } else {
               /* Assume: lat, lon */
               (*pnts)[numPnts - 1].Y = atof (first);
               (*pnts)[numPnts - 1].X = atof (second);
               mallocSprintf (&((*labels)[numPnts - 1]), "(%f,%f)",
                              (*pnts)[numPnts - 1].Y, (*pnts)[numPnts - 1].X);
            }
         } else {
            *NumPnts = numPnts;
            errSprintf ("ERROR: problems parsing '%s'", buffer);
            free (buffer);
            fclose (fp);
            return -1;
         }
      }
   }
   free (buffer);
   fclose (fp);
   *NumPnts = numPnts;
   return 0;
}

/*****************************************************************************
 * Grib2DataProbe() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probe an index file in a similar manner to how we probed a GRIB file.
 *
 * ARGUMENTS
 * usr = The user option structure to use while 'Probing'. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *  1 = Error.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 * May want to move some of this to a ReadPDS in database.c
 * May want to combine some of this with the probe.c stuff.
 *****************************************************************************
 */
int Grib2DataProbe (userType *usr, int numPnts, Point * pnts, char **labels,
                    char **pntFiles)
{
   char *flxArray = NULL; /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   sInt4 *grid_X = NULL; /* The nearest grid point (x coord) */
   sInt4 *grid_Y = NULL; /* The nearest grid point (x coord) */
   int grid_gdsIndex;   /* Which gdsIndex is correct for X, Y. */
   char format[20];     /* Format (# of decimals) to print the data with. */
   char *ptr;           /* A pointer to where we are in the array. */
   uShort2 numGDS;      /* # of GDS Sections. */
   uShort2 numSupPDS;   /* # of Super PDS Sections. */
   char *sPtr;          /* A pointer to the current SuperPDS. */
   int i;               /* Loop counter over SuperPDS. */
   sInt4 lenTotPDS;     /* Length of total PDS section. */
   uChar numBytes;      /* number of bytes in following string. */
   char elem[256];      /* A holder for element from meta data. */
   double refTime;      /* Reference time of this data set. */
   char unit[256];      /* A holder for unit for this data. */
   /* char comment[256]; *//* A holder for comment from meta data. */
   uShort2 gdsIndex;    /* Which GDS is associated with this data. */
   uShort2 numPDS;      /* number of PDS Sections. */
   char *PDSptr;        /* A pointer to the current PDS in the PDS array. */
   int j;               /* Loop counter over PDS Array. */
   uShort2 lenPDS;      /* The length of the current PDS. */
   double validTime;    /* Valid time of this PDS. */
   char dataFile[256];  /* A holder for the Data file for this record. */
   sInt4 dataOffset;    /* An offset into dataFile for this record. */
   uChar endian;        /* Endian'ness of the data grid. */
   uChar scan;          /* Scan mode for the data grid. */
   char *gdsPtr;        /* The location of the current GDS data. */
   gdsType gds;         /* The current grid definition section. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   int k;               /* Loop counter over number of probed points. */
   double newX, newY;   /* The grid (1..n)(1..m) value of a given lat/lon. */
   time_t tempTime;     /* Used when printing out a "double" time_t value. */
   char refBuff[21];    /* The reference time in ASCII form. */
   char validBuff[21];  /* The valid time in ASCII form. */
   FILE *data = NULL;   /* A pointer to the data file. */
   char *curDataName = NULL; /* The name of the current opened data file. */
   sInt4 offset;        /* Where the current data is in the data file. */
   float value;         /* The current cell value. */
   uShort2 numTable = 0; /* Number of strings in the table */
   uShort2 sNumBytes;   /* number of bytes in an "ugly" string. */
   char **table = NULL; /* Table of strings associated with this PDS. */
   int tableIndex;      /* 'value' cast to an integer for table lookup. */
   int jj;              /* Counter used to print "english" weather. */
   UglyStringType ugly; /* Used to 'translate' the weather keys. */
   char *lastSlash;     /* A pointer to last slash in the index file. */

   if (usr->Asc2Flx_File != NULL) {
      Asc2Flx (usr->Asc2Flx_File, usr->inNames[0]);
      return 0;
   }
   if (ReadFLX (usr->inNames[0], &flxArray, &flxArrayLen) != 0) {
      errSprintf ("Problems Reading %s\n", usr->inNames[0]);
      goto error;
   }
   if (usr->f_Print) {
      PrintFLXBuffer (flxArray, flxArrayLen);
      goto done;
   }

   /* Allocate space for grid pnts */
   grid_X = (sInt4 *) malloc (numPnts * sizeof (sInt4));
   grid_Y = (sInt4 *) malloc (numPnts * sizeof (sInt4));
   grid_gdsIndex = -1;

   /* Print labels */
   if (usr->f_pntStyle == 0) {
      printf ("element%sunit%srefTime%svalidTime%s", usr->separator,
              usr->separator, usr->separator, usr->separator);
      for (i = 0; i < numPnts; i++) {
         if (i != numPnts - 1) {
            printf ("%s%s", labels[i], usr->separator);
         } else {
            printf ("%s", labels[i]);
         }
      }
      printf ("\n");
   } else {
      printf ("Location%sElement[Unit]%srefTime%svalidTime%sValue\n",
              usr->separator, usr->separator, usr->separator, usr->separator);
   }

   /* Set up output format. */
   sprintf (format, "%%.%df", usr->decimal);

   /* Start walking through the flx array. */
   ptr = flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2 + numGDS * GDSLEN;
   MEMCPY_LIT (&numSupPDS, ptr, sizeof (uShort2));
   ptr += 2;
   sPtr = ptr;
   for (i = 0; i < numSupPDS; i++) {
      MEMCPY_LIT (&lenTotPDS, sPtr, sizeof (sInt4));
      ptr = sPtr + 4;
      /* Skip sizeof super PDS. */
      ptr += 2;
      numBytes = *ptr;
      ptr++;
      strncpy (elem, ptr, numBytes);
      elem[numBytes] = '\0';
      ptr += numBytes;
      /* Compare matchElem to elem. */
/*
      if (usr->matchElem != NULL) {
         if (strcmp (elem, usr->matchElem) != 0) {
            sPtr += lenTotPDS;
            break;
         }
      }
*/
      MEMCPY_LIT (&refTime, ptr, sizeof (double));
      ptr += 8;
/*
      if (usr->matchRefTime != -1) {
         if (refTime != usr->matchRefTime) {
            sPtr += lenTotPDS;
            break;
         }
      }
*/
      numBytes = *ptr;
      ptr++;
      strncpy (unit, ptr, numBytes);
      unit[numBytes] = '\0';
      ptr += numBytes;
      numBytes = *ptr;
      ptr++;
      /* Skip comment. */
      ptr += numBytes;
      MEMCPY_LIT (&gdsIndex, ptr, sizeof (uShort2));
      ptr += 2;
      /* Skip center / subcenter. */
      ptr += 2 + 2;
      MEMCPY_LIT (&numPDS, ptr, sizeof (uShort2));
      ptr += 2;
      PDSptr = ptr;
      for (j = 0; j < numPDS; j++) {
         MEMCPY_LIT (&lenPDS, PDSptr, sizeof (uShort2));
         ptr = PDSptr + 2;
         MEMCPY_LIT (&validTime, ptr, sizeof (double));
         ptr += 8;
/*
         if (usr->matchValidTime != -1) {
            if (validTime != usr->matchValidTime) {
               PDSptr += lenPDS;
               break;
            }
         }
*/
         numBytes = *ptr;
         ptr++;
         memcpy (dataFile, ptr, numBytes);
         ptr += numBytes;
         dataFile[numBytes] = '\0';
         MEMCPY_LIT (&dataOffset, ptr, sizeof (sInt4));
         ptr += 4;
         endian = *ptr;
         ptr++;
         scan = *ptr;
         ptr++;
         /* Check if numTable is != 0... if so free table. */
         if (numTable != 0) {
            for (k = 0; k < numTable; k++) {
               free (table[k]);
            }
            free (table);
         }
         MEMCPY_LIT (&numTable, ptr, sizeof (uShort2));
         if (numTable != 0) {
            table = (char **) malloc (numTable * sizeof (char *));
            ptr += 2;
            for (k = 0; k < numTable; k++) {
               MEMCPY_LIT (&sNumBytes, ptr, sizeof (uShort2));
               ptr += 2;
               table[k] = (char *) malloc (sNumBytes + 1);
               memcpy (table[k], ptr, sNumBytes);
               ptr += sNumBytes;
               table[k][sNumBytes] = '\0';
            }
         }
         if (grid_gdsIndex != gdsIndex) {
            gdsPtr = flxArray + HEADLEN + 2 + (gdsIndex - 1) * GDSLEN;
            ReadGDSBuffer (gdsPtr, &gds);

            /* Check that gds is valid before setting up map projection. */
            if (GDSValid (&gds) != 0) {
               preErrSprintf ("ERROR: Sect3 was not Valid.\n");
               goto error;
            }
            /* Set up the map projection. */
            SetMapParam (&map, &gds);

            for (k = 0; k < numPnts; k++) {
               myCll2xy (&map, pnts[i].Y, pnts[i].X, &newX, &newY);
#ifdef DEBUG
/*
               printf ("lat %f lon %f -> x %f y %f \n", pnts[i].Y,
                       pnts[i].X, newX, newY);
*/
#endif
               /* Find the nearest grid cell. */
               if (newX < 1) {
                  grid_X[k] = 1;
               } else if ((newX + .5) > gds.Nx) {
                  grid_X[k] = gds.Nx;
               } else {
                  grid_X[k] = (sInt4) (newX + .5);
               }
               if (newY < 1) {
                  grid_Y[k] = 1;
               } else if ((newY + .5) > gds.Ny) {
                  grid_Y[k] = gds.Ny;
               } else {
                  grid_Y[k] = (sInt4) (newY + .5);
               }
            }
            grid_gdsIndex = gdsIndex;
         }

         /* Print out what we know. */
         tempTime = (time_t) refTime;
         strftime (refBuff, 20, "%Y%m%d%H%M", gmtime (&tempTime));
         tempTime = (time_t) validTime;
         strftime (validBuff, 20, "%Y%m%d%H%M", gmtime (&tempTime));
         if ((data == NULL) || (curDataName == NULL) ||
             (strcmp (curDataName, dataFile) != 0)) {
            if (curDataName == NULL) {
               curDataName = (char *) malloc (strlen (dataFile) +
                                              strlen (usr->inNames[0]) + 1);
               strcpy (curDataName, usr->inNames[0]);
            } else {
               curDataName = (char *) realloc ((void *) curDataName,
                                               (strlen (dataFile) +
                                                strlen (usr->inNames[0]) +
                                                1));
            }
            if ((lastSlash = strrchr (curDataName, '/')) == NULL) {
               if ((lastSlash = strrchr (curDataName, '\\')) == NULL) {
                  strcpy (curDataName, dataFile);
               } else {
                  strcpy (lastSlash + 1, dataFile);
               }
            } else {
               strcpy (lastSlash + 1, dataFile);
            }
            if ((data = fopen (curDataName, "rb")) == NULL) {
               errSprintf ("Problems opening %s\n", curDataName);
               free (curDataName);
               curDataName = NULL;
               goto error;
            }
         }
         if (usr->f_pntStyle == 0) {
            printf ("%s%s%s%s%s%s%s%s", elem, usr->separator, unit,
                    usr->separator, refBuff, usr->separator, validBuff,
                    usr->separator);
            for (k = 0; k < numPnts; k++) {
               offset = dataOffset;
               myAssert (sizeof (float) == 4);
               if (scan == 0) {
                  offset += (((grid_X[k] - 1) +
                              ((gds.Ny - 1) - (grid_Y[k] - 1)) * gds.Nx) *
                             sizeof (float));
               } else {
                  offset += (((grid_X[k] - 1) + (grid_Y[k] - 1) * gds.Nx) *
                             sizeof (float));
               }
#ifdef DEBUG
               printf ("offset = %ld, gds.Nx = %ld, CurX,Y = %ld %ld\n",
                       offset, gds.Nx, grid_X[k], grid_Y[k]);
#endif
               fseek (data, offset, SEEK_SET);
               if (endian) {
                  FREAD_BIG (&value, sizeof (float), 1, data);
               } else {
                  FREAD_LIT (&value, sizeof (float), 1, data);
               }
               if (numTable != 0) {
                  tableIndex = (int) value;
                  if ((tableIndex >= 0) && (tableIndex < numTable)) {
                     if (usr->f_WxParse == 0) {
                        printf ("%s", table[tableIndex]);
                     } else if (strcmp (elem, "Weather") == 0) {
                        printf ("%s", table[tableIndex]);
                     } else if (usr->f_WxParse == 1) {
                        ParseUglyString (&ugly, table[tableIndex],
                                         usr->f_SimpleVer);
                        for (jj = 0; jj < NUM_UGLY_WORD; jj++) {
                           if (ugly.english[jj] != NULL) {
                              if (jj != 0) {
                                 printf (" and ");
                              }
                              printf ("%s", ugly.english[jj]);
                           } else {
                              if (jj == 0) {
                                 printf ("No Weather");
                              }
                              break;
                           }
                        }
                        FreeUglyString (&ugly);
                     } else if (usr->f_WxParse == 2) {
                        ParseUglyString (&ugly, table[tableIndex],
                                         usr->f_SimpleVer);
                        printf ("%d", ugly.SimpleCode);
                        FreeUglyString (&ugly);
                     }
                  } else {
                     printf ("9999");
                  }
               } else {
                  printf (format, myRound (value, usr->decimal));
               }
               if (k != numPnts - 1) {
                  printf ("%s", usr->separator);
               }
            }
            printf ("\n");
         } else {
            for (k = 0; k < numPnts; k++) {
               printf ("%s%s", labels[k], usr->separator);
               printf ("%s%s%s", elem, unit, usr->separator);
               printf ("%s%s%s%s", refBuff, usr->separator, validBuff,
                       usr->separator);
               offset = dataOffset;
               if (scan == 0) {
                  offset += (((grid_X[k] - 1) +
                              ((gds.Ny - 1) - (grid_Y[k] - 1)) * gds.Nx) *
                             sizeof (float));
               } else {
                  offset += (((grid_X[k] - 1) + (grid_Y[k] - 1) * gds.Nx) *
                             sizeof (float));
               }
               fseek (data, offset, SEEK_SET);
               if (endian) {
                  FREAD_BIG (&value, sizeof (float), 1, data);
               } else {
                  FREAD_LIT (&value, sizeof (float), 1, data);
               }
               if (numTable != 0) {
                  tableIndex = (int) value;
                  if ((tableIndex >= 0) && (tableIndex < numTable)) {
                     if (usr->f_WxParse == 0) {
                        printf ("%s", table[tableIndex]);
                     } else if (strcmp (elem, "Weather") == 0) {
                        printf ("%s", table[tableIndex]);
                     } else if (usr->f_WxParse == 1) {
                        ParseUglyString (&ugly, table[tableIndex],
                                         usr->f_SimpleVer);
                        for (jj = 0; jj < NUM_UGLY_WORD; jj++) {
                           if (ugly.english[jj] != NULL) {
                              if (jj != 0) {
                                 printf (" and ");
                              }
                              printf ("%s", ugly.english[jj]);
                           } else {
                              if (jj == 0) {
                                 printf ("No Weather");
                              }
                              break;
                           }
                        }
                        FreeUglyString (&ugly);
                     } else if (usr->f_WxParse == 2) {
                        ParseUglyString (&ugly, table[tableIndex],
                                         usr->f_SimpleVer);
                        printf ("%d", ugly.SimpleCode);
                        FreeUglyString (&ugly);
                     }
                  } else {
                     printf ("9999");
                  }
               } else {
                  printf (format, myRound (value, usr->decimal));
               }
               printf ("\n");
            }
         }
         PDSptr += lenPDS;
      }
      sPtr += lenTotPDS;
   }

 done:
   if (numTable != 0) {
      for (i = 0; i < numTable; i++) {
         free (table[i]);
      }
      free (table);
   }
   free (flxArray);
   free (grid_X);
   free (grid_Y);
   if (curDataName != NULL) {
      fclose (data);
      free (curDataName);
   }
   return 0;

 error:
   if (numTable != 0) {
      for (i = 0; i < numTable; i++) {
         free (table[i]);
      }
      free (table);
   }
   free (flxArray);
   free (grid_X);
   free (grid_Y);
   if (curDataName != NULL) {
      fclose (data);
      free (curDataName);
   }
   return 1;
}

int ProbeCmd (sChar f_Command, userType *usr)
{
   char *msg;           /* Used to print the error stack */

   size_t numPnts = 0;  /* How many points in pnts */
   Point *pnts = NULL;  /* Array of points we are interested in. */
   char **labels = NULL; /* Array of labels for the points. */
   char **pntFiles = NULL; /* Array of filenames for the points. */
   uChar f_fileType;
   PntSectInfo *pntInfo;
   size_t numSector;
   char **sector;
   size_t i;
   int ans, ans2;

#ifdef DP_ONLY
   if (f_Command == CMD_PROBE) {
      printf ("DP only executable doesn't handle -P option\n");
      myAssert (1 == 0);
      return -1;
   }
#endif

   /* Find the points we want to probe. */
   if (usr->numPnt != 0) {
      numPnts = usr->numPnt;
      pnts = (Point *) malloc (numPnts * sizeof (Point));
      labels = (char **) malloc (numPnts * sizeof (char *));
      pntFiles = (char **) malloc (numPnts * sizeof (char *));

      memcpy (pnts, usr->pnt, numPnts * sizeof (Point));
      for (i = 0; i < numPnts; i++) {
         mallocSprintf (&(labels[i]), "(%f,%f)", pnts[i].Y, pnts[i].X);
         pntFiles[i] = NULL;
      }
   }
   if (usr->pntFile != NULL) {
      if (ReadPntFile (usr->pntFile, &pnts, &numPnts, &labels, &pntFiles)
          != 0) {
         preErrSprintf ("ERROR: In call to ReadPntFile.\n");
         ans = -2;
         goto done;
      }
   } else if (usr->numPnt == 0) {
      if (usr->f_pntType != 2) {
         errSprintf ("ERROR: -pnt was not initialized.\n");
         if (numPnts > 0) {
            free (labels[0]);
            free (pntFiles[0]);
            free (pnts);
            free (labels);
            free (pntFiles);
         }
         return -2;
      }
   }

   /* Do XML Parse */
   f_fileType = 0;
   if (f_Command == CMD_DATAPROBE) {
      f_fileType = 1;
   }
   if ((usr->f_XML != 0) || (usr->f_Graph != 0) || (usr->f_MOTD != 0)) {

      /* Find out the Major sectors for all the points? */
      /* Probe geodata for all the points, and get TZ and Daylight If geoData 
       * = NULL, or can't find it, TZ = 0, dayLight = 0 */
      pntInfo = (PntSectInfo *) malloc (numPnts * sizeof (PntSectInfo));
      numSector = 0;
      sector = NULL;
      GetSectorList (usr->sectFile, numPnts, pnts, usr->f_pntType,
                     usr->geoDataDir, pntInfo, &numSector, &sector);

      /* Create File names by walking through inNames for dir types. If it is 
       * a file then keep going.  If it is a dir, tack on all relevant
       * sectors, and files that match the ndfdVars + the filter */
      if (usr->gribFilter == NULL) {
         if (usr->f_Command == CMD_DATAPROBE) {
            mallocSprintf (&(usr->gribFilter), "*.ind");
         } else {
            mallocSprintf (&(usr->gribFilter), "*.bin");
         }
      }
#ifdef OLD
      sectExpandInName (&(usr->numInNames), &(usr->inNames),
                        &(usr->f_inTypes), usr->gribFilter, numSector, sector,
                        usr->f_ndfdConven, usr->numNdfdVars, usr->ndfdVars);
      if (usr->numInNames == 0) {
         free (pntInfo);
         for (i = 0; i < numSector; i++) {
            free (sector[i]);
         }
         free (sector);
         goto done;
      }
#endif
      ans = 0;
      /* numInNames, inNames <check> f_inTypes * file type from stat (1=dir,
       * 2=file, 3=unknown). * gribFilter * filter to use to find files
       * numSecor, sector * names of all sectors that we have points in *
       * f_ndfdConven * NDFD naming convention to use. * numNdfdVars,
       * ndfdVars <check> */
      if (usr->f_XML != 0) {
#ifdef _DWML_
         ans = XMLParse (usr->f_XML, numPnts, pnts, pntInfo, usr->f_pntType,
                         labels, usr->numInNames, usr->inNames, f_fileType,
                         usr->f_interp, usr->f_unit, usr->majEarth,
                         usr->minEarth, usr->f_icon, usr->f_SimpleVer,
                         usr->f_valTime, usr->startTime, usr->endTime,
                         usr->numNdfdVars, usr->ndfdVars, usr->f_inTypes,
                         usr->gribFilter, numSector, sector,
                         usr->f_ndfdConven);
#endif
      }
      if (usr->f_Graph != 0) {
         ans2 = GraphProbe (usr->f_Graph, numPnts, pnts, pntInfo,
                            usr->f_pntType, labels, usr->numInNames,
                            usr->inNames, f_fileType, usr->f_interp,
                            usr->f_unit, usr->majEarth, usr->minEarth,
                            usr->f_WxParse, usr->f_SimpleVer, usr->f_valTime,
                            usr->startTime, usr->endTime, usr->numNdfdVars,
                            usr->ndfdVars, usr->f_inTypes, usr->gribFilter,
                            numSector, sector, usr->f_ndfdConven);
         if (ans == 0)
            ans = ans2;
      }
      if (usr->f_MOTD != 0) {
         ans2 = MOTDProbe (usr->f_MOTD, numPnts, pnts, pntInfo,
                           usr->f_pntType, labels, usr->numInNames,
                           usr->inNames, f_fileType, usr->f_interp,
                           usr->f_unit, usr->majEarth, usr->minEarth,
                           usr->f_WxParse, usr->f_SimpleVer, usr->f_valTime,
                           usr->startTime, usr->endTime, usr->numNdfdVars,
                           usr->ndfdVars, usr->f_inTypes, usr->gribFilter,
                           numSector, sector, usr->f_ndfdConven);
         if (ans == 0)
            ans = ans2;
      }

      free (pntInfo);
      for (i = 0; i < numSector; i++) {
         free (sector[i]);
      }
      free (sector);

   } else {
#ifndef DP_ONLY
      if (f_Command == CMD_PROBE) {
         ans = GRIB2Probe (usr, numPnts, pnts, labels, pntFiles);
         if (ans != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to GRIB2Probe.\n%s\n", msg);
            free (msg);
         }
      } else if (f_Command == CMD_DATAPROBE) {
#endif
         ans = Grib2DataProbe (usr, numPnts, pnts, labels, pntFiles);
         if (ans != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to Grib2DataProbe.\n%s\n", msg);
            free (msg);
         }
#ifndef DP_ONLY
      } else {
         ans = 0;
      }
#endif
   }

 done:
   for (i = 0; i < numPnts; i++) {
      free (labels[i]);
      free (pntFiles[i]);
   }
   free (pnts);
   free (labels);
   free (pntFiles);
   return ans;
}
