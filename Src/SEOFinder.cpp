// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 24; Mac/Win
//
//	Main and common functions
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#define	_ELEMENT_TEST_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"SEOFinder.h"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------

static API_Guid	gGuid = APINULLGuid;
static API_Guid	gLastRenFiltGuid = APINULLGuid;


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Utility functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Get an edit vector for editing operations
// -----------------------------------------------------------------------------

bool	GetEditVector (API_Coord3D	*begC,
					   API_Coord3D	*endC,
					   const char	*prompt,
					   bool			begGiven)
{
	API_GetPointType	pointInfo;
	API_GetLineType		lineInfo;
	GSErrCode			err;

	err = NoError;

	if (!begGiven) {
		BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
		BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
		CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
		err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
	} else
		pointInfo.pos = *begC;

	if (err == NoError) {
		BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
		CHCopyC ("Complete the edit vector", lineInfo.prompt);
		lineInfo.startCoord = pointInfo.pos;
		err = ACAPI_Interface (APIIo_GetLineID, &lineInfo, nullptr);
		if (err == NoError) {
			*begC = lineInfo.startCoord;
			*endC = lineInfo.pos;
		}
	}

	if (err != NoError)
		ErrorBeep ("APICmd_GetLineID", err);

	return (bool) (err == NoError);
}		// GetEditVector


// -----------------------------------------------------------------------------
// Get an arc for editing operations
// -----------------------------------------------------------------------------

bool	GetEditArc (API_Coord3D	*begC,
					API_Coord3D *endC,
					API_Coord3D *origC,
					const char	*prompt)
{
	API_GetPointType	pointInfo;
	API_GetLineType		lineInfo;
	API_GetArcType		arcInfo;
	GSErrCode			err;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
	BNZeroMemory (&arcInfo, sizeof (API_GetArcType));
	CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane  = false;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);

	if (err == NoError) {
		BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
		CHCopyC ("Enter the arc start point", lineInfo.prompt);
		lineInfo.changeFilter = false;
		lineInfo.changePlane  = false;
		lineInfo.startCoord   = pointInfo.pos;
		err = ACAPI_Interface (APIIo_GetLineID, &lineInfo, nullptr);
	}

	if (err == NoError) {
		BNZeroMemory (&arcInfo, sizeof (API_GetArcType));
		CHCopyC ("Enter the arc end point", arcInfo.prompt);
		arcInfo.origo			= lineInfo.startCoord;
		arcInfo.startCoord		= lineInfo.pos;
		arcInfo.startCoordGiven = true;
		err = ACAPI_Interface (APIIo_GetArcID, &arcInfo, nullptr);
	}

	if (err != NoError) {
		ErrorBeep ("APIIo_GetArcID", err);
		return (bool) (err == NoError);
	}

	*origC	= arcInfo.origo;
	if (arcInfo.negArc) {
		*begC = arcInfo.pos;
		*endC = arcInfo.startCoord;
	} else {
		*begC = arcInfo.startCoord;
		*endC = arcInfo.pos;
	}

	return (bool) (err == NoError);
}		// GetEditArc


// -----------------------------------------------------------------------------
// Search for the active camset
//  Also search for a camset with the type of the active one.
//  If no other camsets exist, the active index is returned
// -----------------------------------------------------------------------------

void	SearchActiveCamset (API_Guid*	actGuid,
							API_Guid*	perspGuid)
{
	if (actGuid != nullptr)
		*actGuid = APINULLGuid;
	if (perspGuid != nullptr)
		*perspGuid = APINULLGuid;

	GS::Array<API_Guid>	camSetList;
	if (ACAPI_Element_GetElemList (API_CamSetID, &camSetList) != NoError)
		return;

	API_Element	element;
	for (GS::Array<API_Guid>::ConstIterator it = camSetList.Enumerate (); it != nullptr; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;

		const GSErrCode err = ACAPI_Element_Get (&element);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Get (camset)", err);
			continue;
		}

		if (element.camset.active) {
			if (actGuid != nullptr)
				*actGuid = *it;
		} else {			/* first from every type which is inactive */
			if (perspGuid != nullptr)
				*perspGuid = *it;
		}
	}

	if (*actGuid == APINULLGuid)
		WriteReport_Alert ("No camsets, please create one");
}


// -----------------------------------------------------------------------------
// Dump a polygon
// -----------------------------------------------------------------------------

bool DumpPolygon (const API_Element	*element,
				  short				lineInd,
				  const double		offset,
				  Int32				nCoords,
				  Int32				nSubPolys,
				  Int32				nArcs,
				  API_Coord			**coords,
				  Int32				**subPolys,
				  API_PolyArc		**arcs,
				  bool				createShadow,
				  bool				writeReport)
{
	API_Element	elemL, elemA;
	API_Coord	begC, endC, origC;
	double		angle;
	Int32		j,k, begInd, endInd, arcInd;
	GSErrCode	err;

	if (nCoords < 4  || coords == nullptr || subPolys == nullptr ||
			(nArcs > 0 && arcs == nullptr) || element == nullptr)
		return false;

	BNZeroMemory (&elemL, sizeof (API_Element));
	BNZeroMemory (&elemA, sizeof (API_Element));
	elemL.header.typeID = API_LineID;
	elemA.header.typeID = API_ArcID;

	err = ACAPI_Element_GetDefaults (&elemL, nullptr);
	err |= ACAPI_Element_GetDefaults (&elemA, nullptr);
	if (err != NoError)
		return false;

	elemL.line.ltypeInd   = lineInd;
	elemL.header.floorInd = element->header.floorInd;
	elemL.header.layer    = element->header.layer;

	elemL.arc.ltypeInd    = lineInd;
	elemA.header.floorInd = element->header.floorInd;
	elemA.header.layer    = element->header.layer;
	elemA.arc.ratio		  = 1.0;


	for (j = 1; j <= nSubPolys && err == NoError; j++) {
		if (writeReport && j > 1)
			WriteReport ("  ---");
		begInd = (*subPolys) [j-1] + 1;
		endInd = (*subPolys) [j];

		for (k = begInd; k < endInd && err == NoError; k++) {
			begC = (*coords) [k];
			endC = (*coords) [k+1];
			begC.x += offset;
			endC.x += offset;
			begC.y += offset;
			endC.y += offset;
			if (arcs != nullptr && nArcs > 0)
				arcInd = FindArc (*arcs, nArcs, k);
			else
				arcInd = -1;
			if (arcInd < 0) {
				elemL.line.begC = begC;
				elemL.line.endC = endC;
				if (createShadow)
					err = ACAPI_Element_Create (&elemL, nullptr);
				if (writeReport)
					WriteReport ("  (%lf, %lf)", begC.x, begC.y);
			} else {
				angle = (*arcs) [arcInd].arcAngle;
				ArcGetOrigo (&begC, &endC, angle, &origC);
				elemA.arc.origC = origC;
				elemA.arc.r = DistCPtr (&origC, &begC);
				if (angle > 0.0) {
					elemA.arc.begAng = ComputeFiPtr (&origC, &begC);
					elemA.arc.endAng = ComputeFiPtr (&origC, &endC);
				} else {
					elemA.arc.endAng = ComputeFiPtr (&origC, &begC);
					elemA.arc.begAng = ComputeFiPtr (&origC, &endC);
				}
				if (createShadow)
					err = ACAPI_Element_Create (&elemA, nullptr);
				if (writeReport)
					WriteReport ("  (%lf, %lf) with angle %lf deg", begC.x, begC.y, angle * RADDEG);
			}
		}
	}

	if (err != NoError) {
		ErrorBeep ("Unable to dump polygon", err);
		return false;
	}

	return true;
}		// DumpPolygon


// -----------------------------------------------------------------------------
// Elements: Solid Operations Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsSolidOperation (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_SolidLink_Create ();				break;
				case 2:		Do_SolidLink_Remove ();				break;
				case 3:		Do_SolidLink_Targets ();			break;
				case 4:		Do_SolidLink_Operators ();			break;
				case 5:		Do_SolidOperation_Create ();		break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsSolidOperation */


// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------
API_AddonType __ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Preload;
}		/* RegisterAddOn */


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err;

	//
	// Register menus
	//
	err = ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	err = ACAPI_Register_Menu (32501, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32502, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32503, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32504, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32505, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32506, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32507, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32508, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32509, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32511, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_FileType (1, 'TEXT', '    ', "txt;", 0, 32510, 1, SaveAs2DSupported);
	err = ACAPI_Register_FileType (2, 'TEXT', '    ', "txt;", 0, 32510, 2, Import2DDrawingSupported);

	return err;
}		/* RegisterInterface */


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = NoError;

	//
	// Install menu handler callbacks
	//

	err = ACAPI_Install_MenuHandler (32506, ElementsSolidOperation);

	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		/* FreeData */
