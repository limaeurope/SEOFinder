// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 24; Mac/Win
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#define	_ELEMENT_MODIFY_TRANSL_
#define	_Geometry_Test_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"Element_Test.h"

#include	"GSUtilsDefs.h"

#include	"basicgeometry.h"
#include	"Point2D.hpp"
#include	"Point2DData.h"

#include	"BuiltInLibrary.hpp"
#include	"GSUnID.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------


static API_LeaderLineShapeID	GetNextLeaderLineShape (API_LeaderLineShapeID shape)
{
	switch (shape) {
		case API_Segmented:		return API_Splinear;
		case API_Splinear:		return API_SquareRoot;
		case API_SquareRoot:	return API_Segmented;
		default:				DBBREAK (); return API_Segmented;
	}
}


// =============================================================================
//
// Modify elements
//
// =============================================================================

// -----------------------------------------------------------------------------
// Change the user ID of the clicked element
//	- modify other elements in the same group also
//	- check hidden/locked layer
//	- update Automatic Labels
// -----------------------------------------------------------------------------

void		Do_Change_ElemInfo (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Element			element, mask;
	API_ElementMemo		memo;
	GSErrCode			err;

	if (!ClickAnElem ("Click an elem to modify the UserID", API_ZombieElemID, nullptr, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	if (typeID != API_WallID && typeID != API_ColumnID && typeID != API_BeamID &&
		typeID != API_SlabID && typeID != API_RoofID && typeID != API_MeshID && typeID != API_ShellID &&
		typeID != API_WindowID && typeID != API_DoorID && typeID != API_SkylightID &&
		typeID != API_ObjectID && typeID != API_LampID &&
		typeID != API_ZoneID && typeID != API_HatchID &&
		typeID != API_MorphID &&
		typeID != API_StairID && typeID != API_RailingID)
	{
		WriteReport_Alert ("The clicked element has no UserID");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = typeID;
	element.header.guid	= guid;
	err = ACAPI_Element_Get (&element);

	ACAPI_ELEMENT_MASK_CLEAR (mask);

	if (ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError) {
		if (memo.elemInfoString != nullptr) {
			delete memo.elemInfoString;
		}
		memo.elemInfoString = new GS::UniString ("Changed ID");
	}

	if (err == NoError) {
		switch (typeID) {
			case API_WallID:
			case API_ColumnID:
			case API_BeamID:
			case API_SlabID:
			case API_RoofID:
			case API_ShellID:
			case API_MorphID:
			case API_MeshID:
			case API_WindowID:
			case API_DoorID:
			case API_SkylightID:
			case API_ObjectID:
			case API_LampID:
			case API_ZoneID:
			case API_HatchID:
			case API_StairID:
			case API_RailingID:
				break;
			default:
				err = APIERR_BADID;
				break;
		}
	}

	if (err == NoError) {
		err = ACAPI_Element_ChangeParameters ({guid}, &element, &memo, &mask);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeParameters", err);
	}

	ACAPI_DisposeElemMemoHdls (&memo);
	return;
}		// Do_Change_ElemInfo


// -----------------------------------------------------------------------------
// Change the parameters of the active tool or an element instance
//	- the parameter is hardcoded, depends on the element type
//	- the layer is always changed to 'ARCHICAD'
// In case of element instance:
//	- modify other elements in the same group also
//	- check hidden/locked layer
//	- update Automatic Labels
// Check the changes in the settings dialogs or on the info palette
// -----------------------------------------------------------------------------

void		Do_Change_ElemParameters (bool defaults)
{
	API_Element			element, elementMask;
	API_ToolBoxItem		tboxInfo;
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&elementMask, sizeof (API_Element));
	BNZeroMemory (&tboxInfo, sizeof (API_ToolBoxItem));

	if (defaults) {
		err = ACAPI_Environment (APIEnv_GetToolBoxModeID, &tboxInfo, nullptr);
		if (err != NoError) {
			ErrorBeep ("APIEnv_GetToolBoxMode", err);
			return;
		}
		element.header.typeID = tboxInfo.typeID;
		ACAPI_Element_GetDefaults (&element, nullptr);
	} else {
		if (!ClickAnElem ("Click an element to change one of its parameters", API_ZombieElemID, nullptr, &element.header.typeID, &element.header.guid)) {
			WriteReport_Alert ("No element was clicked");
			return;
		}
		ACAPI_Element_Get (&element);
	}

	ACAPI_ELEMENT_MASK_SET (elementMask, API_Elem_Head, layer);
	element.header.layer = 1;		// changed to 'ARCHICAD' layer for each type

	if (element.header.hotlinkGuid != APINULLGuid) {
		element.header.guid = element.header.hotlinkGuid;
		ACAPI_Element_Get (&element);
		element.hotlink.suspendFixAngle = !element.hotlink.suspendFixAngle;
		ACAPI_ELEMENT_MASK_SET (elementMask, API_HotlinkType, suspendFixAngle);
		element.hotlink.floorDifference += 1;
		ACAPI_ELEMENT_MASK_SET (elementMask, API_HotlinkType, floorDifference);
		ACAPI_Element_Change (&element, &elementMask, nullptr, 0UL, true);
		return;
	}

	API_ElementMemo	memo;

	switch (element.header.typeID) {
		case API_WallID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_WallType, thickness);
						element.wall.thickness = 1.0;
						// WALL: thickness to 1.0
						break;
		case API_ColumnID:
						BNZeroMemory (&memo, sizeof (API_ElementMemo));

						ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ColumnSegment | APIMemoMask_AssemblySegmentCut | APIMemoMask_AssemblySegmentScheme);

						API_ElementMemo newMemo;
						BNZeroMemory (&newMemo, sizeof (API_ElementMemo));

						newMemo.columnSegments = reinterpret_cast<API_ColumnSegmentType*> (BMAllocatePtr ((element.column.nSegments + 1) * sizeof (API_ColumnSegmentType), ALLOCATE_CLEAR, 0));
						if (err == NoError && memo.columnSegments != nullptr && newMemo.columnSegments != nullptr) {
							for (USize idx = 0; idx < element.column.nSegments; ++idx)
								newMemo.columnSegments[idx] = memo.columnSegments[idx];

							newMemo.columnSegments[element.column.nSegments] = newMemo.columnSegments[element.column.nSegments - 1];
							newMemo.columnSegments[element.column.nSegments].assemblySegmentData.modelElemStructureType = API_BasicStructure;
							element.column.nSegments++;
						}

						newMemo.assemblySegmentCuts = reinterpret_cast<API_AssemblySegmentCutData*> (BMAllocatePtr ((element.column.nCuts + 1) * sizeof (API_AssemblySegmentCutData), ALLOCATE_CLEAR, 0));
						if (err == NoError && memo.assemblySegmentCuts != nullptr && newMemo.assemblySegmentCuts != nullptr) {
							for (USize idx = 0; idx < element.column.nCuts; ++idx)
								newMemo.assemblySegmentCuts[idx] = memo.assemblySegmentCuts[idx];

							newMemo.assemblySegmentCuts[element.column.nCuts] = newMemo.assemblySegmentCuts[element.column.nSegments - 1];
							newMemo.assemblySegmentCuts[element.column.nCuts].cutType = APIAssemblySegmentCut_Horizontal;
							element.column.nCuts++;
						}

						newMemo.assemblySegmentSchemes = reinterpret_cast<API_AssemblySegmentSchemeData*> (BMAllocatePtr ((element.column.nSchemes + 1) * sizeof (API_AssemblySegmentSchemeData), ALLOCATE_CLEAR, 0));
						if (err == NoError && memo.assemblySegmentSchemes != nullptr && newMemo.assemblySegmentSchemes != nullptr) {
							for (USize idx = 0; idx < element.column.nSchemes; ++idx)
								newMemo.assemblySegmentSchemes[idx] = memo.assemblySegmentSchemes[idx];

							API_AssemblySegmentSchemeData* lastScheme = &newMemo.assemblySegmentSchemes[element.column.nSchemes - 1];
							API_AssemblySegmentSchemeData* newScheme = &newMemo.assemblySegmentSchemes[element.column.nSchemes];

							newScheme->lengthType = lastScheme->lengthType;
							if (lastScheme->lengthType == APIAssemblySegment_Fixed) {
								lastScheme->fixedLength *= 0.5;
								newScheme->fixedLength = lastScheme->fixedLength;
							} else {
								lastScheme->lengthProportion *= 0.5;
								newScheme->lengthProportion = lastScheme->lengthProportion;
							}

							element.column.nSchemes++;
						}

						if (err == NoError) {
							API_Element mask;
							ACAPI_ELEMENT_MASK_CLEAR (mask);
							ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, nSegments);

							err = ACAPI_Element_Change (&element, &mask, &newMemo, APIMemoMask_ColumnSegment | APIMemoMask_AssemblySegmentCut | APIMemoMask_AssemblySegmentScheme, true);
						}

						ACAPI_DisposeElemMemoHdls (&memo);
						ACAPI_DisposeElemMemoHdls (&newMemo);

						// COLUMN: new segment
						break;
		case API_BeamID:
						BNZeroMemory (&memo, sizeof (API_ElementMemo));

						ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamSegment | APIMemoMask_AssemblySegmentCut);

						if (err == NoError && memo.beamSegments != nullptr) {
							for (USize idx = 0; idx < element.beam.nSegments; ++idx)
								memo.beamSegments[idx].assemblySegmentData.nominalHeight = 1.0;

							for (USize idx = 0; idx < element.beam.nCuts; ++idx) {
								memo.assemblySegmentCuts[idx].cutType = APIAssemblySegmentCut_Custom;
								memo.assemblySegmentCuts[idx].customAngle = 45.0 * DEGRAD;
							}

							API_Element mask;
							ACAPI_ELEMENT_MASK_CLEAR (mask);

							err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_BeamSegment | APIMemoMask_AssemblySegmentCut, true);
						}

						ACAPI_DisposeElemMemoHdls (&memo);

						// BEAM: height to 1.0, cut angles to 45 degree
						break;
		case API_WindowID:
		case API_DoorID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_WindowType, openingBase.width);
						element.window.openingBase.width = 2.34;
						// WINDOW: width to 2.34
						break;
		case API_ObjectID:
		case API_LampID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ObjectType, angle);
						element.object.angle = PI / 6.0;
						// OBJECT: angle to 60 degree
						break;
		case API_SlabID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_SlabType, topMat);
						element.slab.topMat.attributeIndex = 34;
						// SLAB: topMat to 34
						break;
		case API_RoofID:
						if (defaults)
							element.roof.roofClass = API_PolyRoofID;

						if (element.roof.roofClass == API_PlaneRoofID) {
							ACAPI_ELEMENT_MASK_SET (elementMask, API_RoofType, shellBase.ltypeInd);
							element.roof.shellBase.ltypeInd = 4;
							// ROOF: ltype to 4
						} else {
							ACAPI_ELEMENT_MASK_SET (elementMask, API_RoofType, u.polyRoof.levelNum);
							ACAPI_ELEMENT_MASK_SET (elementMask, API_RoofType, u.polyRoof.levelData);
							ACAPI_ELEMENT_MASK_SET (elementMask, API_RoofType, u.polyRoof.overHangType);
							ACAPI_ELEMENT_MASK_SET (elementMask, API_RoofType, u.polyRoof.eavesOverHang);
							element.roof.u.polyRoof.levelNum = 16;
							for (Int32 i = 0; i < element.roof.u.polyRoof.levelNum; i++) {
								element.roof.u.polyRoof.levelData[i].levelAngle = 5.0 * DEGRAD * (element.roof.u.polyRoof.levelNum - i);
								element.roof.u.polyRoof.levelData[i].levelHeight = 0.02 * (i + 1);
							}
							element.roof.u.polyRoof.overHangType = API_OffsetOverhang;
							element.roof.u.polyRoof.eavesOverHang = 0.0;
							// ROOF: roof levels
						}
						break;
		case API_ShellID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ShellType, shellBase.ltypeInd);
						element.shell.shellBase.ltypeInd = 4;
						// SHELL: ltype to 4
						break;
		case API_MorphID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_MorphType, displayOption);
						ACAPI_ELEMENT_MASK_SET (elementMask, API_MorphType, level);
						ACAPI_ELEMENT_MASK_SET (elementMask, API_MorphType, castShadow);
						element.morph.displayOption = API_Standard;
						element.morph.level = 1.0;
						element.morph.castShadow = false;
						// MORPH: Floor Plan Display to Projected, level to 1.0, do not cast shadow
						break;
		case API_MeshID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_MeshType, level);
						element.mesh.level = -1.0;
						// MESH: level to -1.0
						break;

		case API_DimensionID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_DimensionType, horizontalText);
						element.dimension.horizontalText = !element.dimension.horizontalText;
						// DIM: flip between horizontal and parallel
						ACAPI_ELEMENT_MASK_SET (elementMask, API_DimensionType, linPen);
						element.dimension.linPen = 250;
						// DIM: linPen to 250
						ACAPI_ELEMENT_MASK_SET (elementMask.dimension.markerData, API_MarkerData, markerPen);
						element.dimension.markerData.markerPen = 250;
						// DIM: linPen to 250
						break;
		case API_RadialDimensionID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_RadialDimensionType, linPen);
						element.radialDimension.linPen = 250;
						// RADDIM: linPen to 250
						ACAPI_ELEMENT_MASK_SET (elementMask.radialDimension.note, API_NoteType, notePen);
						element.radialDimension.note.notePen = 250;
						ACAPI_ELEMENT_MASK_SET (elementMask.radialDimension.note, API_NoteType, backgroundPen);
						element.radialDimension.note.backgroundPen = 250;
						ACAPI_ELEMENT_MASK_SET (elementMask.radialDimension.note, API_NoteType, leaderPen);
						element.radialDimension.note.leaderPen = 250;
						break;
		case API_LevelDimensionID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_LevelDimensionType, dimForm);
						element.levelDimension.dimForm = 4;
						// LEVDIM: dimForm to 4
						ACAPI_ELEMENT_MASK_SET (elementMask, API_LevelDimensionType, pen);
						element.levelDimension.pen = 4;
						ACAPI_ELEMENT_MASK_SET (elementMask.levelDimension.note1, API_NoteType, notePen);
						element.levelDimension.note1.notePen = 4;
						ACAPI_ELEMENT_MASK_SET (elementMask.levelDimension.note1, API_NoteType, backgroundPen);
						element.levelDimension.note1.backgroundPen = 4;
						ACAPI_ELEMENT_MASK_SET (elementMask.levelDimension.note1, API_NoteType, leaderPen);
						element.levelDimension.note1.leaderPen = 4;
						ACAPI_ELEMENT_MASK_SET (elementMask.levelDimension.note2, API_NoteType, notePen);
						element.levelDimension.note2.notePen = 4;
						ACAPI_ELEMENT_MASK_SET (elementMask.levelDimension.note2, API_NoteType, backgroundPen);
						element.levelDimension.note2.backgroundPen = 4;
						ACAPI_ELEMENT_MASK_SET (elementMask.levelDimension.note2, API_NoteType, leaderPen);
						element.levelDimension.note2.leaderPen = 4;
						break;
		case API_AngleDimensionID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_AngleDimensionType, textPos);
						element.angleDimension.textPos = APIPos_Below;
						// ANGDIM: textPos to Below
						ACAPI_ELEMENT_MASK_SET (elementMask, API_AngleDimensionType, linPen);
						element.angleDimension.linPen = 8;
						// RADDIM: linPen to 8
						ACAPI_ELEMENT_MASK_SET (elementMask.angleDimension.note, API_NoteType, notePen);
						element.angleDimension.note.notePen = 8;
						ACAPI_ELEMENT_MASK_SET (elementMask.angleDimension.note, API_NoteType, backgroundPen);
						element.angleDimension.note.backgroundPen = 8;
						ACAPI_ELEMENT_MASK_SET (elementMask.angleDimension.note, API_NoteType, leaderPen);
						element.angleDimension.note.leaderPen = 8;
						break;

		case API_TextID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_TextType, faceBits);
						element.text.faceBits |= APIFace_Bold;
						// TEXT: face to Bold
						break;
		case API_LabelID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_LabelType, leaderShape);
						element.label.leaderShape = GetNextLeaderLineShape (element.label.leaderShape);
						// LABEL: cycle through shapes
						break;
		case API_ZoneID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ZoneType, roomName);
						GS::ucscpy (element.zone.roomName, L("WC"));
						// ZONE: name to "WC"
						break;

		case API_HatchID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_HatchType, hatchType);
						element.hatch.hatchType = API_BuildingMaterialHatch;
						ACAPI_ELEMENT_MASK_SET (elementMask, API_HatchType, buildingMaterial);
						element.hatch.buildingMaterial = 10;
						// HATCH: buildingmaterial index to 10
						break;
		case API_LineID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_LineType, roomSeparator);
						element.line.roomSeparator = true;
						// LINE: roomSeparator to TRUE
						break;
		case API_ArcID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ArcType, roomSeparator);
						element.arc.roomSeparator = true;
						// ARC: roomSeparator to TRUE
						break;
		case API_SplineID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_SplineType, linePen);
						element.spline.linePen.penIndex = 100;
						element.spline.linePen.colorOverridePenIndex = 0;
						// SPLINE: linePen to 100
						break;
		case API_HotspotID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_HotspotType, pen);
						element.hotspot.pen = 10;
						// HOTSPOT: pen to 10
						break;

		case API_CutPlaneID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_CutPlaneType, segment.textSize);
						element.cutPlane.segment.textSize = 0.5;
						// CUTPLANE: textSize to 0.5
						break;
		case API_ElevationID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ElevationType, segment.vertRange);
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ElevationType, segment.vertMin);
						ACAPI_ELEMENT_MASK_SET (elementMask, API_ElevationType, segment.vertMax);
						element.elevation.segment.vertRange = APIVerRange_Limited;
						element.elevation.segment.vertMin = 1.5;
						element.elevation.segment.vertMax = 3.0;
						// ELEVATION: vertical limits to (1.5 - 3.0)
						break;
		case API_InteriorElevationID:
						if (defaults) {
							ACAPI_ELEMENT_MASK_SET (elementMask, API_InteriorElevationType, segment.shadFillPen);
							ACAPI_ELEMENT_MASK_SET (elementMask, API_InteriorElevationType, segment.shadFillBGPen);
							ACAPI_ELEMENT_MASK_SET (elementMask, API_InteriorElevationType, segment.effectBits);
							element.interiorElevation.segment.shadFillPen = 77;
							element.interiorElevation.segment.shadFillBGPen = 88;
							element.interiorElevation.segment.effectBits |= APICutPl_VectorShadow;
							// INTERIOR ELEVATION: shadow fill and background pen to 77, 88
						}
						break;

		case API_CameraID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_CameraType, perspCam.pen);
						element.camera.perspCam.pen = 200;
						// CAMERA: pen to 200
						break;

		case API_PictureID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_PictureType, mirrored);
						element.picture.mirrored = !element.picture.mirrored;
						// PICTURE: mirror the picture
						break;

		case API_DrawingID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_DrawingType, angle);
						element.drawing.angle += 0.5235987755983;
						// DRAWING: rotate by 30 degrees
						break;

		case API_StairID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_StairType, treadDepth);
						ACAPI_ELEMENT_MASK_SET (elementMask, API_StairType, treadDepthLocked);
						element.stair.treadDepth *= 2;
						element.stair.treadDepthLocked = false;
						// STAIR: unlock and increase tread depth by 20%
						break;

		case API_RailingID:
						ACAPI_ELEMENT_MASK_SET (elementMask, API_RailingType, referenceLinePen);
						element.railing.referenceLinePen = 200;
						// RAILING: change reference line pen to 200
						break;

		case API_GroupID:
		case API_SectElemID:
		case API_CamSetID:
		default:
						break;
	}

	if (element.header.guid != APINULLGuid) {
		err = ACAPI_Element_ChangeParameters ({element.header.guid}, &element, nullptr, &elementMask);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeParameters", err);
	} else {
		err = ACAPI_Element_ChangeDefaults (&element, nullptr, &elementMask);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeDefaults", err);
	}
}		// Do_Change_ElemParameters


// -----------------------------------------------------------------------------
// Drag (adjust) a node of a polygon
//	- dimensions must be updated automatically
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Helper function for poly edit examples
//
// Sets memomask for ACAPI_Element_ChangeMemo to accept the new polygon.
// When sending polygon, you have to send the edge datas as well.
// You have two choices:
//	- send a new edge data list, which is synchronous with the new polygon
//	- send nullptr to clear any edge customizations
//
// In the examples, we send nullptr as edge datas.
//
// This restriction does not apply to meshPolyZ coords, which are vertex data.
//
// -----------------------------------------------------------------------------
static UInt32 GetMemoMask (const API_Neig& neig) {
	UInt32 mask = 0;
	switch (neig.neigID) {
		case APINeig_Ceil:
		case APINeig_CeilOn:
							mask |= APIMemoMask_Polygon;
							mask |= APIMemoMask_EdgeTrims;
							mask |= APIMemoMask_SideMaterials;
							return mask;
		case APINeig_Roof:
		case APINeig_RoofOn:
							mask |= APIMemoMask_Polygon;
							mask |= APIMemoMask_EdgeTrims;
							mask |= APIMemoMask_SideMaterials;
							mask |= APIMemoMask_RoofEdgeTypes;
							return mask;
		default:
				break;
	}
	return APIMemoMask_Polygon;
}


void		Do_Poly_AdjustNode (void)
{
	API_ElementMemo		memo;
	API_Neig			neig;
	API_ElemTypeID		typeID;
	API_Coord3D			begC, endC;
	Int32				nSubPolys, i, begInd, endInd;
	GSErrCode			err = NoError;

	if (!ClickAnElem ("Click a polygon node to drag", API_ZombieElemID, &neig, &typeID, nullptr, &begC)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	switch (neig.neigID) {
		case APINeig_Ceil:
		case APINeig_Roof:
		case APINeig_Mesh:
		case APINeig_Room:
		case APINeig_Hatch:
		case APINeig_PolyLine:
		case APINeig_DetailPoly:
		case APINeig_WorksheetPoly:
		case APINeig_DrawingFrame:
		case APINeig_InteriorElevation:		break;

		default:							WriteReport_Alert ("Only polygon nodes are accepted");
											return;
	}

	if (GetEditVector (&begC, &endC, "Enter drag reference point", true)) {
		err = ACAPI_Element_GetMemo (neig.guid, &memo);
		if (err == NoError) {
			(*memo.coords)[neig.inIndex].x += endC.x - begC.x;
			(*memo.coords)[neig.inIndex].y += endC.y - begC.y;
			nSubPolys = BMGetHandleSize ((GSHandle) memo.pends) / sizeof (Int32) - 1;
			for (i = 1; i <= nSubPolys; i++) {
				begInd = (*memo.pends)[i - 1] + 1;
				endInd = (*memo.pends)[i];
				if (neig.inIndex == begInd)
					(*memo.coords)[endInd] = (*memo.coords)[neig.inIndex];
				else if (neig.inIndex == endInd)
					(*memo.coords)[begInd] = (*memo.coords)[neig.inIndex];
			}
		}
		if (err == NoError) {
			API_ElementMemo tmpMemo;
			BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
			tmpMemo.coords = memo.coords;
			tmpMemo.pends = memo.pends;
			tmpMemo.parcs = memo.parcs;
			tmpMemo.vertexIDs = memo.vertexIDs;
			tmpMemo.edgeIDs = memo.edgeIDs;
			tmpMemo.edgeTrims = memo.edgeTrims;
			tmpMemo.contourIDs = memo.contourIDs;
			tmpMemo.meshPolyZ = memo.meshPolyZ;
			err = ACAPI_Element_ChangeMemo (neig.guid, GetMemoMask (neig), &tmpMemo);
			if (err != NoError)
				ErrorBeep ("ACAPI_Element_ChangeMemo", err);
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	return;
}		// Do_Poly_AdjustNode


// -----------------------------------------------------------------------------
// Insert a new node into the clicked polygon edge
//	- dimensions must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Poly_InsertNode (void)
{
	API_Element			elem;
	API_ElementMemo		memo;
	API_Neig			neig;
	API_ElemTypeID		typeID;
	API_Coord3D			begC;
	Int32				nodeInd;
	GSErrCode			err;

	if (!ClickAnElem ("Click a polygon edge to insert a new node into", API_ZombieElemID, &neig, &typeID, nullptr, &begC)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	switch (neig.neigID) {
		case APINeig_CeilOn:
		case APINeig_RoofOn:
		case APINeig_MeshOn:
		case APINeig_RoomOn:
		case APINeig_HatchOn:
		case APINeig_PolyLineOn:
		case APINeig_DetailPolyOn:
		case APINeig_WorksheetPolyOn:
		case APINeig_DrawingFrameOn:
		case APINeig_InteriorElevationOn:	break;

		default:							WriteReport_Alert ("Only polygon edges are accepted");
											return;
	}

	// TODO: don't do it for polyroofs, only for planeroofs
	BNZeroMemory (&elem, sizeof (elem));
	elem.header.guid = neig.guid;
	err = ACAPI_Element_Get (&elem);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}
	if (elem.header.typeID == API_RoofID && elem.roof.roofClass != API_PlaneRoofID) {
		WriteReport_Alert ("Only plane roofs are accepted");
		return;
	}
	err = ACAPI_Element_GetMemo (neig.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	nodeInd = neig.inIndex + 1;
	err = ACAPI_Goodies (APIAny_InsertPolyNodeID, &memo, &nodeInd, &begC);
	if (err == NoError) {
		API_ElementMemo tmpMemo;
		BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
		tmpMemo.coords = memo.coords;
		tmpMemo.pends = memo.pends;
		tmpMemo.parcs = memo.parcs;
		tmpMemo.vertexIDs = memo.vertexIDs;
		tmpMemo.edgeIDs = memo.edgeIDs;
		tmpMemo.edgeTrims = memo.edgeTrims;
		tmpMemo.contourIDs = memo.contourIDs;
		tmpMemo.meshPolyZ = memo.meshPolyZ;
		err = ACAPI_Element_ChangeMemo (neig.guid, GetMemoMask (neig), &tmpMemo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeMemo", err);
	} else
		ErrorBeep ("APIAny_InsertPolyNodeID", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Poly_InsertNode


// -----------------------------------------------------------------------------
// Delete the clicked node from the polygon
//	- dimensions must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Poly_DeleteNode (void)
{
	API_ElementMemo		memo;
	API_Neig			neig;
	API_ElemTypeID		typeID;
	Int32				nodeInd;
	GSErrCode			err;

	if (!ClickAnElem ("Click a polygon node to delete", API_ZombieElemID, &neig, &typeID)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	switch (neig.neigID) {
		case APINeig_Ceil:
		case APINeig_Roof:
		case APINeig_Mesh:
		case APINeig_Room:
		case APINeig_Hatch:
		case APINeig_PolyLine:
		case APINeig_DetailPoly:
		case APINeig_WorksheetPoly:
		case APINeig_DrawingFrame:
		case APINeig_InteriorElevation:		break;

		default:							WriteReport_Alert ("Only polygon nodes are accepted");
											return;
	}

	err = ACAPI_Element_GetMemo (neig.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	if (BMhGetSize ((GSHandle) memo.coords) / sizeof (API_Coord) <= 5) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	nodeInd = neig.inIndex;
	err = ACAPI_Goodies (APIAny_DeletePolyNodeID, &memo, &nodeInd);
	if (err == NoError) {
		API_ElementMemo tmpMemo;
		BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
		tmpMemo.coords = memo.coords;
		tmpMemo.pends = memo.pends;
		tmpMemo.parcs = memo.parcs;
		tmpMemo.vertexIDs = memo.vertexIDs;
		tmpMemo.meshPolyZ = memo.meshPolyZ;
		tmpMemo.edgeIDs = memo.edgeIDs;
		tmpMemo.edgeTrims = memo.edgeTrims;
		tmpMemo.contourIDs = memo.contourIDs;
		err = ACAPI_Element_ChangeMemo (neig.guid, GetMemoMask (neig), &tmpMemo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeMemo", err);
	} else
		ErrorBeep ("APIAny_DeletePolyNodeID", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Poly_DeleteNode


// -----------------------------------------------------------------------------
// Delete the clicked polygon hole
//	- dimensions must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Poly_DeleteHole (void)
{
	API_ElementMemo		memo;
	API_Neig			neig;
	API_ElemTypeID		typeID;
	Int32				nSubPolys, subPolyInd, i;
	GSErrCode			err;

	if (!ClickAnElem ("Click a polygon hole to delete", API_ZombieElemID, &neig, &typeID)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	switch (typeID) {
		case API_SlabID:
		case API_RoofID:
		case API_MeshID:
		case API_ZoneID:
		case API_HatchID:	break;

		default:			WriteReport_Alert ("Only slab, roof, mesh, zone or fill nodes/edges are excepted");
							return;
	}

	err = ACAPI_Element_GetMemo (neig.guid, &memo);
	if (err != NoError)
		return;

	subPolyInd = 0;
	nSubPolys = BMGetHandleSize ((GSHandle) memo.pends) / sizeof (Int32) - 1;
	for (i = 1; i <= nSubPolys; i++) {
		if (neig.inIndex <= (*memo.pends)[i]) {
			subPolyInd = i;
			break;
		}
	}

	err = ACAPI_Goodies (APIAny_DeleteSubPolyID, &memo, &subPolyInd);
	if (err == NoError) {
		API_ElementMemo tmpMemo;
		BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
		tmpMemo.coords = memo.coords;
		tmpMemo.pends = memo.pends;
		tmpMemo.parcs = memo.parcs;
		tmpMemo.vertexIDs = memo.vertexIDs;
		tmpMemo.meshPolyZ = memo.meshPolyZ;
		tmpMemo.edgeIDs = memo.edgeIDs;
		tmpMemo.edgeTrims = memo.edgeTrims;
		tmpMemo.contourIDs = memo.contourIDs;
		err = ACAPI_Element_ChangeMemo (neig.guid, GetMemoMask (neig), &tmpMemo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeMemo", err);
	} else
		ErrorBeep ("APIAny_DeleteSubPolyID", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Poly_DeleteHole


// -----------------------------------------------------------------------------
// Add a new hole to the clicked polygon
//	- dimensions must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Poly_NewHole (void)
{
	API_GetPointType	pointInfo;
	API_GetPolyType		polyInfo;
	API_ElementMemo		memo, insMemo;
	API_Neig			neig;
	API_ElemTypeID		typeID;
	GSErrCode			err;

	if (!ClickAnElem ("Click a polygon to insert a new hole into", API_ZombieElemID, &neig, &typeID)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	switch (typeID) {
		case API_SlabID:
		case API_RoofID:
		case API_MeshID:
		case API_ZoneID:
		case API_HatchID:	break;

		default:			WriteReport_Alert ("Only slab, roof, mesh, zone or fill nodes/edges are excepted");
							return;
	}

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	BNZeroMemory (&polyInfo, sizeof (API_GetPolyType));
	strcpy (pointInfo.prompt, "Click the first node of the hole");
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
	if (err == NoError) {
		strcpy (polyInfo.prompt, "Enter the polygon nodes");
		polyInfo.startCoord = pointInfo.pos;
		err = ACAPI_Interface (APIIo_GetPolyID, &polyInfo, nullptr);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &polyInfo.coords);
		BMKillHandle ((GSHandle *) &polyInfo.parcs);
		return;
	}

	err = ACAPI_Element_GetMemo (neig.guid, &memo);
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &polyInfo.coords);
		BMKillHandle ((GSHandle *) &polyInfo.parcs);
		return;
	}

	BNZeroMemory (&insMemo, sizeof (API_ElementMemo));
	insMemo.coords = polyInfo.coords;
	insMemo.parcs	= polyInfo.parcs;

	polyInfo.coords = nullptr;
	polyInfo.parcs	= nullptr;

	err = ACAPI_Goodies (APIAny_InsertSubPolyID, &memo, &insMemo);
	if (err == NoError) {
		API_ElementMemo tmpMemo;
		BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
		tmpMemo.coords = memo.coords;
		tmpMemo.pends = memo.pends;
		tmpMemo.parcs = memo.parcs;
		tmpMemo.vertexIDs = memo.vertexIDs;
		tmpMemo.meshPolyZ = memo.meshPolyZ;
		tmpMemo.edgeIDs = memo.edgeIDs;
		tmpMemo.edgeTrims = memo.edgeTrims;
		tmpMemo.contourIDs = memo.contourIDs;
		err = ACAPI_Element_ChangeMemo (neig.guid, GetMemoMask (neig), &tmpMemo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeMemo", err);
	} else
		ErrorBeep ("APIAny_InsertSubPolyID", err);

	ACAPI_DisposeElemMemoHdls (&insMemo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Poly_NewHole


// =============================================================================
//
// Modify elements
//
// =============================================================================


// -----------------------------------------------------------------------------
// Mirror a gable list
// (Helper function to the element edit functions)
// -----------------------------------------------------------------------------

static void	ChangeGables (double			wallLength,
							API_ElementMemo	*memo)
{
	Int32	nGable, i;

	if (memo->gables != nullptr) {
		nGable = BMGetHandleSize ((GSHandle) memo->gables) / sizeof (API_Gable);

		for (i = 0; i < nGable; i++) {
			(*memo->gables)[i].xb = -(*memo->gables)[i].nx * wallLength + (*memo->gables)[i].xb;
			(*memo->gables)[i].xe = -(*memo->gables)[i].nx * wallLength + (*memo->gables)[i].xe;
			(*memo->gables)[i].nx = -(*memo->gables)[i].nx;
			(*memo->gables)[i].ny = -(*memo->gables)[i].ny;

			(*memo->gables)[i].d = (*memo->gables)[i].d - (*memo->gables)[i].a * wallLength;
			(*memo->gables)[i].a = -(*memo->gables)[i].a;
			(*memo->gables)[i].b = -(*memo->gables)[i].b;
		}
	}

	return;
}		// ChangeGables


// -----------------------------------------------------------------------------
// Update the position of an associative Label.
// -----------------------------------------------------------------------------

static GSErrCode	UpdateOneLabel (const API_Guid& labelGuid, const API_Coord& newBegC)
{
	if (labelGuid == APINULLGuid)
		return Error;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid	= labelGuid;

	if (ACAPI_Element_Get (&element) != NoError)
		return Error;

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, begC);
	ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, midC);
	ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, endC);

	const double dx = newBegC.x - element.label.begC.x;
	const double dy = newBegC.y - element.label.begC.y;

	element.label.begC = newBegC;
	element.label.midC.x += dx;
	element.label.midC.y += dy;
	element.label.endC.x += dx;
	element.label.endC.y += dy;

	if (element.label.labelClass == APILblClass_Text) {
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, u.text.loc);
		element.label.u.text.loc.x += dx;
		element.label.u.text.loc.y += dy;
	} else {
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, u.symbol.pos);
		element.label.u.symbol.pos.x += dx;
		element.label.u.symbol.pos.y += dy;
	}

	if (ACAPI_Element_Change (&element, &mask, nullptr, 0, true) != NoError)
		return Error;

	return NoError;
}		// UpdateOneLabel


// -----------------------------------------------------------------------------
// Update the position of all associative Labels of an element.
// The Labels will be positioned vertically in a row beginning at 'firstBegC'.
// -----------------------------------------------------------------------------

static GSErrCode	UpdateAllLabels (const API_Guid& elemGuid, const API_Coord& firstBegC)
{
	GS::Array<API_Guid> connectedLabels;
	if (ACAPI_Element_GetConnectedElements (elemGuid, API_LabelID, &connectedLabels) != NoError)
		return Error;

	GSErrCode err	  = NoError;
	API_Coord newBegC = firstBegC;
	for (GS::Array<API_Guid>::ConstIterator it = connectedLabels.Enumerate (); it != nullptr; ++it) {
		if (UpdateOneLabel (*it, newBegC) != NoError)
			err = Error;

		newBegC.y -= 1;
	}

	return err;
}


// -----------------------------------------------------------------------------
// Edit a window
//	 - double its width
// (Helper function to the element edit functions)
// -----------------------------------------------------------------------------

static void		UpdateOneWindow (const API_Guid& guid)
{
	API_Element		element, mask;
	API_Coord		pos;
	API_Coord3D		c3;
	GSErrCode		err;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid = guid;

	err = ACAPI_Element_Get (&element);

	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_WindowType, openingBase.width);

		element.window.openingBase.width *= 2.0;

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

		// Update LABEL (window)
		if (err == NoError) {
			API_Neig	neig;
			BNZeroMemory (&neig, sizeof (API_Neig));
			neig.neigID = APINeig_Wind;
			neig.guid = element.header.guid;
			neig.inIndex = 1;
			ACAPI_Goodies (APIAny_NeigToCoordID, &neig, &c3);
			pos.x = c3.x;
			pos.y = c3.y;
			UpdateAllLabels (element.header.guid, pos);
		}
	}

	return;
}		// UpdateOneWindow


// -----------------------------------------------------------------------------
// GSHandle windows after editing a wall
//	 - delete windows which do not fit into the wall
//	 - edit those whose position is valid
// (Helper function to the element edit functions)
// -----------------------------------------------------------------------------

static GSErrCode	UpdateOpenings (const API_Guid&	wallGuid,
									double			wallLength)
{
	GS::Array<API_Guid>	wallWindows;
	GSErrCode err = ACAPI_Element_GetConnectedElements (wallGuid, API_WindowID, &wallWindows);
	if (err == NoError) {
		GS::Array<API_Guid>	updWind;
		GS::Array<API_Guid>	delWind;
		for (UIndex i = 0; i < wallWindows.GetSize (); i++) {
			API_Element element;
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.guid = wallWindows[i];
			err = ACAPI_Element_Get (&element);
			if (err == NoError) {
				if (element.window.objLoc < wallLength) {
					updWind.Push (element.header.guid);
				} else {
					delWind.Push (element.header.guid);
				}
			} else
				break;
		}

		if (err == NoError) {
			for (UIndex i = 0; i < updWind.GetSize (); i++)
				UpdateOneWindow (updWind[i]);
		}

		ACAPI_Element_Delete (delWind);
	}

	return NoError;
}		// UpdateOpenings


// -----------------------------------------------------------------------------
// Modify the reference points of a the clicked wall
//	- dimensions/labels must be updated automatically
//	- windows most be kept
// -----------------------------------------------------------------------------

void		Do_Wall_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_Coord3D			begC, newC;
	API_Coord			c1, c2, mid;
	double				angle, wallLength;
	bool				directionChange;
	Int32				memoMask;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (!ClickAnElem ("Click a wall to modify", API_WallID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No wall was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError)
		return;

	begC.x = element.wall.begC.x;
	begC.y = element.wall.begC.y;
	begC.z = 0.0;
	if (!GetEditVector (&begC, &newC, "Enter drag reference point", true))
		return;

	angle = -30 * DEGRAD;
	directionChange = (angle < 0.0);

	c1 = element.wall.begC;
	c2.x = newC.x;
	c2.y = newC.y;

	wallLength = DistCPtr (&c1, &c2);

	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
		ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
		ACAPI_ELEMENT_MASK_SET (mask, API_WallType, type);
		ACAPI_ELEMENT_MASK_SET (mask, API_WallType, thickness);
		ACAPI_ELEMENT_MASK_SET (mask, API_WallType, thickness1);

		element.wall.begC = c1;
		element.wall.endC = c2;
		element.wall.type = APIWtyp_Trapez;
		element.wall.thickness = 0.28;
		element.wall.thickness1 = 1.28;

		if (directionChange) {
			memoMask = APIMemoMask_Gables;
			err = ACAPI_Element_GetMemo (element.header.guid, &memo, memoMask);
			if (err == NoError && memo.gables != nullptr)
				ChangeGables (wallLength, &memo);
			else
				memoMask = 0;
		} else
			memoMask = 0;

		err = ACAPI_Element_Change (&element, &mask, &memo, memoMask, true);

		if (err == NoError) {
			mid.x = (element.wall.begC.x + element.wall.endC.x) / 2.0;
			mid.y = (element.wall.begC.y + element.wall.endC.y) / 2.0;
			UpdateAllLabels (element.header.guid, mid);
		}

		if (err == NoError)
			err = UpdateOpenings (element.header.guid, wallLength);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Wall_Edit


static void TestCurtainWallPanelsOrder (const API_Guid& cwGuid)
{
    GS::Array<API_CWPanelType> panelsInOrder;

    API_Element	cwElement = {};
    cwElement.header.guid = cwGuid;

    GSErrCode err = ACAPI_Element_Get (&cwElement);
    if (DBVERIFY (err == NoError)) {
        API_ElementMemo	cwMemo = {};
        err = ACAPI_Element_GetMemo (cwElement.header.guid, &cwMemo);
        if (DBVERIFY (err == NoError && cwMemo.cWallSegments != nullptr && cwMemo.cWallPanels != nullptr && cwMemo.cWallPanelGridIDTable != nullptr)) {
            GS::HashTable<API_Guid, API_CWPanelType> cwPanelTable;
            for (UIndex ii = 0; ii < cwElement.curtainWall.nPanels; ++ii) {
                const API_CWPanelType& cwPanel = cwMemo.cWallPanels[ii];
                cwPanelTable.Add (cwPanel.head.guid, cwPanel);
            }

            using CWSegmentGridCellID = GS::Pair<UInt32, API_GridElemID>;
            GS::HashTable<CWSegmentGridCellID, GS::Array<API_CWPanelType>> panelsInSegmentGridCells;
            for (auto it = cwMemo.cWallPanelGridIDTable->EnumeratePairs (); it != nullptr; ++it) {
                const API_CWPanelType& cwPanel = cwPanelTable[*it->key];
                const GS::Array<API_GridElemID>& cwPanelGridIDs = *it->value;

                for (auto it = cwPanelGridIDs.Enumerate (); it != nullptr; ++it) {
                    const API_GridElemID& gridID = *it;

                    CWSegmentGridCellID segmentGridCellID (cwPanel.segmentID, gridID);
                    if (panelsInSegmentGridCells.ContainsKey (segmentGridCellID))
                        panelsInSegmentGridCells[segmentGridCellID].Push (cwPanel);
                    else
                        panelsInSegmentGridCells.Add (segmentGridCellID, { cwPanel });
                }
            }

            for (UIndex ii = 0; ii < cwElement.curtainWall.nSegments; ++ii) {
                const API_CWSegmentType& cwSegment = cwMemo.cWallSegments[ii];
                API_ElementMemo	cwSegmentMemo = {};
                err = ACAPI_Element_GetMemo (cwSegment.head.guid, &cwSegmentMemo);
                if (DBVERIFY (err == NoError && cwSegmentMemo.cWSegGridMesh != nullptr)) {
                    for (const API_GridMeshLine& mainLine : cwSegmentMemo.cWSegGridMesh->meshLinesMainAxis) {
                        for (const API_GridMeshLine& secLine : cwSegmentMemo.cWSegGridMesh->meshLinesSecondaryAxis) {
                            CWSegmentGridCellID segmentGridCellID (ii, mainLine.head.elemID << 16 | secLine.head.elemID);
                            if (panelsInSegmentGridCells.ContainsKey (segmentGridCellID))
                                panelsInOrder.Append (panelsInSegmentGridCells.Get (segmentGridCellID));
                        }
                    }
                }

                ACAPI_DisposeElemMemoHdls (&cwSegmentMemo);
            }
        }

        ACAPI_DisposeElemMemoHdls (&cwMemo);
    }
}


// -----------------------------------------------------------------------------
// Modify the pattern of a the clicked curtain wall
// -----------------------------------------------------------------------------

void		Do_CurtainWall_Edit (void)
{
	API_Element element, mask;
	API_ElementMemo memo;

	BNClear (element);
	BNClear (memo);

	if (!ClickAnElem ("Click a Curtain Wall to Modify", API_CurtainWallID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No curtain wall was clicked");
		return;
	}

    TestCurtainWallPanelsOrder (element.header.guid);

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError)
		return;

	const UInt64 memoMask = APIMemoMask_CWSegPrimaryPattern | APIMemoMask_CWSegSecPattern |
						    APIMemoMask_CWSegPanelPattern | APIMemoMask_CWallFrameClasses |
						    APIMemoMask_CWallPanelClasses;
	err = ACAPI_Element_GetMemo (element.header.guid, &memo, memoMask);
	if (err != NoError)
		return;

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_CurtainWallType, nFrameDefaults);
	ACAPI_ELEMENT_MASK_SET (mask, API_CurtainWallType, nPanelDefaults);

	// Add a Frame Class: "API Test Frame"
	{
		auto newFrameClasses = reinterpret_cast<API_CWFrameType*> (BMpAll ((element.curtainWall.nFrameDefaults + 1) * sizeof (API_CWFrameType)));
		for (UIndex i = 0; i < element.curtainWall.nFrameDefaults; ++i) {
			newFrameClasses[i] = memo.cWallFrameDefaults[i];
		}
		newFrameClasses[element.curtainWall.nFrameDefaults] = element.curtainWall.boundaryFrameData;

		newFrameClasses[element.curtainWall.nFrameDefaults].useOwnMaterial = false;
		newFrameClasses[element.curtainWall.nFrameDefaults].material = 48;
		GS::ucscpy (newFrameClasses[element.curtainWall.nFrameDefaults].className, GS::UniString ("API Test Frame").ToUStr ().Get ());

		BMpKill (reinterpret_cast<GSPtr*> (&memo.cWallFrameDefaults));
		memo.cWallFrameDefaults = newFrameClasses;
		element.curtainWall.nFrameDefaults++;
	}

	// Add a Panel Class: "API Test Panel"
	{
		auto newPanelClasses = reinterpret_cast<API_CWPanelType*> (BMpAll ((element.curtainWall.nPanelDefaults + 1) * sizeof (API_CWPanelType)));
		for (UIndex i = 0; i < element.curtainWall.nPanelDefaults; ++i) {
			newPanelClasses[i] = memo.cWallPanelDefaults[i];
		}
		if (element.curtainWall.nPanelDefaults > 1)
			newPanelClasses[element.curtainWall.nPanelDefaults] = memo.cWallPanelDefaults[0];

		newPanelClasses[element.curtainWall.nPanelDefaults].outerSurfaceMaterial.overridden = true;
		newPanelClasses[element.curtainWall.nPanelDefaults].outerSurfaceMaterial.attributeIndex = 100;
		newPanelClasses[element.curtainWall.nPanelDefaults].innerSurfaceMaterial.overridden = true;
		newPanelClasses[element.curtainWall.nPanelDefaults].innerSurfaceMaterial.attributeIndex = 100;
		newPanelClasses[element.curtainWall.nPanelDefaults].cutSurfaceMaterial.overridden = true;
		newPanelClasses[element.curtainWall.nPanelDefaults].cutSurfaceMaterial.attributeIndex = 100;
		GS::ucscpy (newPanelClasses[element.curtainWall.nPanelDefaults].className, GS::UniString ("API Test Panel").ToUStr ().Get ());

		BMpKill (reinterpret_cast<GSPtr*> (&memo.cWallPanelDefaults));
		memo.cWallPanelDefaults = newPanelClasses;
		element.curtainWall.nPanelDefaults++;
	}

	const double blockSize = element.curtainWall.height / memo.cWSegPrimaryPattern.nPattern;

	// Modify the Primary Pattern
	for (UIndex ii = 0; ii < memo.cWSegPrimaryPattern.nPattern; ++ii) {
		memo.cWSegPrimaryPattern.pattern[ii] = blockSize;
	}

	// Modify the Secondary Pattern
	for (UIndex ii = 0; ii < memo.cWSegSecondaryPattern.nPattern; ++ii) {
		memo.cWSegSecondaryPattern.pattern[ii] = blockSize;
	}

	// Fill the Pattern Cells
	const USize nCells = memo.cWSegPrimaryPattern.nPattern * memo.cWSegSecondaryPattern.nPattern;
	for (UIndex ii = 0; ii < nCells;  ++ii) {
		memo.cWSegPatternCells[ii].crossingFrameType = ((ii % 2) != 0) ? APICWCFT_FromTopLeftToBottomRight : APICWCFT_FromBottomLeftToTopRight;
		memo.cWSegPatternCells[ii].rightPanelID = APICWPanelClass_FirstCustomClass + element.curtainWall.nPanelDefaults - 1;
		memo.cWSegPatternCells[ii].leftPanelID = APICWPanelClass_Deleted;
		memo.cWSegPatternCells[ii].crossingFrameID = APICWFrameClass_FirstCustomClass + element.curtainWall.nFrameDefaults - 1;
	}

	err = ACAPI_Element_Change (&element, &mask, &memo, memoMask, true);
	ACAPI_DisposeElemMemoHdls (&memo);

	if (err != NoError)
		return;

	err = ACAPI_Element_Get (&element);
	if (err != NoError)
		return;

	err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_CWallPanels | APIMemoMask_CWallFrames);

	UInt32 counter = 0;
	for (UInt32 i = 0; i < element.curtainWall.nPanels; ++i) {
        API_Element panelElem = {};
		panelElem.header.guid = memo.cWallPanels[i].head.guid;

		err = ACAPI_Element_Get (&panelElem);
		if (err != NoError)
			continue;

		if (panelElem.cwPanel.classID == APICWPanelClass_Deleted)
			continue;

        API_Element panelMask = {};

		switch (counter % 4) {
			case 0: // Delete Panel
				ACAPI_ELEMENT_MASK_SET (panelMask, API_CWPanelType, classID);
				panelElem.cwPanel.classID = APICWPanelClass_Deleted;
				break;

			case 1: // Change Panel Class
				ACAPI_ELEMENT_MASK_SET (panelMask, API_CWPanelType, classID);
				panelElem.cwPanel.classID = APICWPanelClass_FirstCustomClass + 1;
				break;

			case 2: // Customize Panel
				ACAPI_ELEMENT_MASK_SET (panelMask, API_CWPanelType, classID);
				ACAPI_ELEMENT_MASK_SET (panelMask, API_CWPanelType, thickness);
				panelElem.cwPanel.classID = APICWPanelClass_Customized;
				panelElem.cwPanel.thickness *= 3;
				break;

			case 3: // Continue
			default:
				break;
		}

		if (counter % 4 < 3) {
			err = ACAPI_Element_Change (&panelElem, &panelMask, nullptr, 0UL, true);
			if (err != NoError) {
				ACAPI_WriteReport ("ACAPI_Element_Change (CW Panel) returned "
								   "with error value %ld!", false, err);
			}
		}

		++counter;
	}

	// Set Internal (Non-Boundary) Frames to the Generated Class
	for (UInt32 i = 0; i < element.curtainWall.nFrames; ++i) {
        API_Element frameElem = {};
		frameElem.header.guid = memo.cWallFrames[i].head.guid;

		err = ACAPI_Element_Get (&frameElem);
		if (err != NoError)
			continue;

		if (frameElem.cwFrame.classID == APICWFrameClass_Boundary)
			continue;

        API_Element frameMask = {};
        ACAPI_ELEMENT_MASK_SET (frameMask, API_CWFrameType, classID);
        frameElem.cwFrame.classID = APICWFrameClass_FirstCustomClass +
        							element.curtainWall.nFrameDefaults - 1;

		err = ACAPI_Element_Change (&frameElem, &frameMask, nullptr, 0UL, true);
		if (err != NoError) {
			ACAPI_WriteReport ("ACAPI_Element_Change (CW Frame) returned "
							   "with error value %ld!", false, err);
		}
    }

    TestCurtainWallPanelsOrder (element.header.guid);

	ACAPI_DisposeElemMemoHdls (&memo);
}		// Do_CurtainWall_Edit


// -----------------------------------------------------------------------------
// Change the shape of the clicked column
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Column_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a column to modify", API_ColumnID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No column was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, origoPos);
	ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, axisRotationAngle);

	element.column.origoPos.x += 0.5;
	element.column.origoPos.y += 0.5;
	element.column.axisRotationAngle = 45 * DEGRAD;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err == NoError)
		UpdateAllLabels (element.header.guid, element.column.origoPos);

	return;
}		// Do_Column_Edit


// -----------------------------------------------------------------------------
// Enlarge holes of the clicked beam
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Beam_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	Int32				nHole, i;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (!ClickAnElem ("Click a beam to enlarge its holes", API_BeamID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No beam was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err == NoError)
		err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamHole);

	if (err != NoError)
		return;

	nHole = BMGetHandleSize ((GSHandle) memo.beamHoles) / sizeof (API_Beam_Hole);
	for (i = 0; i < nHole; i++) {
		(*memo.beamHoles)[i].width *= 3.0;
		(*memo.beamHoles)[i].height *= 2.0;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, aboveViewLinePen);

	element.header.layer = 1;
	element.beam.aboveViewLinePen = 10;

	err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_BeamHole, true);

	if (err == NoError) {
		API_Coord mid;
		mid.x = (element.beam.begC.x + element.beam.endC.x) / 3.0;
		mid.y = (element.beam.begC.y + element.beam.endC.y) / 3.0;
		UpdateAllLabels (element.header.guid, mid);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Beam_Edit


// -----------------------------------------------------------------------------
// Change a window fixPoint
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Window_Edit (void)
{
	API_Element		element, mask;
	API_Coord3D		begC;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a window to modify", API_WindowID, nullptr, &element.header.typeID, &element.header.guid, &begC)) {
		WriteReport_Alert ("No window was clicked");
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);

	ACAPI_ELEMENT_MASK_SET (mask, API_WindowType, fixPoint);
	element.window.fixPoint = APIHoleAnchor_BegFix;

	GSErrCode err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	if (err == NoError) {
		API_Coord c;
		c.x = begC.x;
		c.y = begC.y;
		UpdateAllLabels (element.header.guid, c);
	}

	return;
}		// Do_Window_Edit


// -----------------------------------------------------------------------------
// Change a skylight fixPoint
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Skylight_Edit (void)
{
	API_Element			element, mask;
	API_Coord3D			begC;
	API_Coord			c;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a skylight to modify", API_SkylightID, nullptr, &element.header.typeID, &element.header.guid, &begC)) {
		WriteReport_Alert ("No skylight was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_SkylightType, anchorPoint);
	ACAPI_ELEMENT_MASK_SET (mask, API_SkylightType, anchorPosition);

	element.skylight.anchorPoint = APISkylightAnchor_TL;
	element.skylight.anchorPosition.x += 2.0;
	element.skylight.anchorPosition.y += 1.0;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	if (err == NoError) {
		c.x = begC.x;
		c.y = begC.y;
		UpdateAllLabels (element.header.guid, c);
	}

	return;
}		// Do_Skylight_Edit


// -----------------------------------------------------------------------------
// Change an array-type parameter
// -----------------------------------------------------------------------------

bool ChangeArrayParam (API_ElementMemo		*inMemo, 		// memo of the given library part element
                       const char			*paramName,
					   Int32 				inDim1,			// the new first dimension
					   Int32 				inDim2,			// the new second dimension
					   const GS::uchar_t	*inParVal,		// the new value
					   Int32 				inIndex1,		// first index of the new value
					   Int32 				inIndex2)		// second index of the new value
{
	Int32 n = BMGetHandleSize ((GSHandle) inMemo->params) / sizeof (API_AddParType);

	if (inIndex1 >= inDim1 || inIndex2 >= inDim2 || inIndex1 < 0 || inIndex2 < 0 || inParVal == nullptr)
		return false;
	// Bad parameters

	// searching for an array parameter
	for (Int32 i = 0; i < n; i++) {
		// check if it's an array parameter and the array contains strings
		if ((*inMemo->params)[i].typeMod == API_ParArray &&
				((*inMemo->params)[i].typeID == APIParT_CString || (*inMemo->params)[i].typeID == APIParT_Title) &&
				CHCompareASCII ((*inMemo->params)[i].name, paramName) == 0)
		{
			GS::uchar_t **origArrHdl = (GS::uchar_t **) (*inMemo->params)[i].value.array;
			Int32 origDim1 = (*inMemo->params)[i].dim1;
			Int32 origDim2 = (*inMemo->params)[i].dim2;

			// calculating new size of the array
			Int32 newSize = 0;
			Int32 lastPos = 0;
			for (Int32 j = 0; j < inDim1; j++) {
				for (Int32 k = 0; k < inDim2; k++) {
					Int32 size = 1;
					// 1 for the closing '\0' character
					if (j < origDim1 && k < origDim2) {
						size += GS::ucslen32 (&(*origArrHdl)[lastPos]);
						lastPos += size;
					}
					if (j == inIndex1 && k == inIndex2 && inParVal != nullptr)
						newSize += GS::ucslen32 (inParVal) + 1;
					else
						newSize += size;
				}
			}

			GS::uchar_t **newArrHdl = (GS::uchar_t **) BMAllocateHandle (newSize * sizeof (GS::uchar_t), ALLOCATE_CLEAR, 0);

			// changing array size if needed:
			if (origDim1 != inDim1 || origDim2 != inDim2) {
				(*inMemo->params)[i].dim1 = inDim1;
				(*inMemo->params)[i].dim2 = inDim2;
			}

			// changing array content:
			Int32 lastPosO = 0;
			Int32 lastPosN = 0;
			for (Int32 j = 0; j < inDim1; j++) {
				for (Int32 k = 0; k < inDim2; k++) {
					if (j == inIndex1 && k == inIndex2 && inParVal != nullptr) {
						GS::ucscpy (&(*newArrHdl)[lastPosN], inParVal);
						lastPosN += GS::ucslen32 (&(*newArrHdl)[lastPosN]) + 1;
						continue;
					}
					if (j < origDim1 && k < origDim2) {
						GS::ucscpy (&(*newArrHdl)[lastPosN], &(*origArrHdl)[lastPosO]);
						lastPosO += GS::ucslen32 (&(*origArrHdl)[lastPosO]) + 1;
						lastPosN += GS::ucslen32 (&(*newArrHdl)[lastPosN]) + 1;
					} else {
						GS::ucscpy (&(*newArrHdl)[lastPosN], L("\0"));
						// '\0' character for the empty items
						lastPosN += GS::ucslen32 (&(*newArrHdl)[lastPosN]) + 1;
					}
				}
			}

			// kill original array handle and change it to the new
			BMKillHandle ((GSHandle *) &origArrHdl);
			(*inMemo->params)[i].value.array = (GSHandle) newArrHdl;

			return true;
		}
	}

	return false;

}		// ChangeArrayParam


// -----------------------------------------------------------------------------
// Drag and rotate an object
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void		Do_Object_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click an object to modify", API_ObjectID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No object was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, angle);
	ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
	element.object.angle += PI / 2.0;	// 90 degrees
	element.object.pos.x += 2.0;
	element.object.pos.y += 1.0;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0UL, true);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Change", err);
	}

	return;
}		// Do_Object_Edit


// -----------------------------------------------------------------------------
// Delete the clicked slab node and change the pen
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void	Do_Ceil_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_Neig			neig;
	Int32				nodeInd;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a slab to modify", API_SlabID, &neig, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No slab was clicked");
		return;
	}

	if (neig.neigID != APINeig_Ceil) {
		WriteReport_Alert ("Only slab nodes are accepted");
		return;
	}

	GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	nodeInd = neig.inIndex;
	err = ACAPI_Goodies (APIAny_DeletePolyNodeID, &memo, &nodeInd);

	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, pen);

		element.slab.pen = 10;

		element.slab.poly.nCoords = BMhGetSize ((GSHandle) memo.coords) / Sizeof32 (API_Coord) - 1;
		element.slab.poly.nSubPolys = BMhGetSize ((GSHandle) memo.pends) / Sizeof32 (Int32) - 1;
		element.slab.poly.nArcs = BMhGetSize ((GSHandle) memo.parcs) / Sizeof32 (API_PolyArc);

		err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_SideMaterials | APIMemoMask_EdgeTrims, true);
		if (err == NoError) {
			API_Coord c = (*memo.coords)[1];
			UpdateAllLabels (element.header.guid, c);
		}
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Ceil_Edit


// -----------------------------------------------------------------------------
// Add a new node to the clicked roof and reset side angles to vertical
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void	Do_Roof_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_Neig			neig;
	API_Coord3D			begC, addC;
	Int32				nodeInd;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click on a roof contour edge to modify", API_RoofID, &neig, &element.header.typeID, &element.header.guid, &begC)) {
		WriteReport_Alert ("No roof was clicked");
		return;
	}

	if (neig.neigID != APINeig_RoofOn) {
		WriteReport_Alert ("Only roof edges are accepted");
		return;
	}

	if (!GetEditVector (&begC, &addC, "", true))
		return;

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	nodeInd = neig.inIndex + 1;
	err = ACAPI_Goodies (APIAny_InsertPolyNodeID, &memo, &nodeInd, &addC);
	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.pen);
		element.roof.shellBase.pen = 10;

		if (element.roof.roofClass == API_PlaneRoofID) {
			element.roof.u.planeRoof.poly.nCoords = BMhGetSize ((GSHandle) memo.coords) / Sizeof32 (API_Coord) - 1;
			element.roof.u.planeRoof.poly.nSubPolys = BMhGetSize ((GSHandle) memo.pends) / Sizeof32 (Int32) - 1;
			element.roof.u.planeRoof.poly.nArcs = BMhGetSize ((GSHandle) memo.parcs) / Sizeof32 (API_PolyArc);
		} else {
			element.roof.u.polyRoof.contourPolygon.nCoords = BMhGetSize ((GSHandle) memo.coords) / Sizeof32 (API_Coord) - 1;
			element.roof.u.polyRoof.contourPolygon.nSubPolys = BMhGetSize ((GSHandle) memo.pends) / Sizeof32 (Int32) - 1;
			element.roof.u.polyRoof.contourPolygon.nArcs = BMhGetSize ((GSHandle) memo.parcs) / Sizeof32 (API_PolyArc);
		}

		err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_SideMaterials | APIMemoMask_EdgeTrims | APIMemoMask_RoofEdgeTypes, true);		/* memo.edgeTrims = nullptr --> reset */
		if (err == NoError) {
			API_Coord c = (*memo.coords)[1];
			UpdateAllLabels (element.header.guid, c);
		}
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Roof_Edit


// -----------------------------------------------------------------------------
// Toggle bulges to the (1st) profile polygon of the clicked shell
//	- dimensions/labels must be updated automatically (???)
// -----------------------------------------------------------------------------

void	Do_Shell_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_Neig			neig;
	API_Coord3D			begC;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a shell to modify", API_ShellID, &neig, &element.header.typeID, &element.header.guid, &begC)) {
		WriteReport_Alert ("No shell was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	API_Polygon& shellShape = (element.shell.shellClass == API_ExtrudedShellID) ? element.shell.u.extrudedShell.shellShape :
								((element.shell.shellClass == API_RevolvedShellID) ? element.shell.u.revolvedShell.shellShape : element.shell.u.ruledShell.shellShape1);

	if (memo.shellShapes[0].parcs != nullptr) {
		Int32	i;
		for (i = 0; i < shellShape.nArcs; i++)
			(*memo.shellShapes[0].parcs)[i].arcAngle /= 2.0;

		err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_EdgeTrims, true);		/* memo.edgeTrims = nullptr --> reset */
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Shell_Edit


// -----------------------------------------------------------------------------
// Punch a hole to the shell
// -----------------------------------------------------------------------------

void	Do_Shell_Edit2 (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Neig			neig;
	GSErrCode			err = NoError;

	if (!ClickAnElem ("Click a shell to modify", API_ShellID, &neig, &typeID, &guid)) {
		WriteReport_Alert ("No shell was clicked");
		return;
	}

	API_GetPointType	pt;
	BNZeroMemory (&pt, sizeof (pt));
	if (ACAPI_Interface (APIIo_GetPointID, &pt, nullptr, nullptr) != NoError) {
		ErrorBeep ("ACAPI_Interface (APIIo_GetPointID, ...)", err);
		return;
	}
	API_GetPolyType	poly;
	BNZeroMemory (&poly, sizeof (poly));
	poly.startCoord = pt.pos;
	poly.method = APIPolyGetMethod_General;
	if (ACAPI_Interface (APIIo_GetPolyID, &poly, nullptr, nullptr) != NoError) {
		ErrorBeep ("ACAPI_Interface (APIIo_GetPolyID, ...)", err);
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = typeID;
	element.header.guid	= guid;
	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	API_ShellContourData* oldmemos = memo.shellContours;
	Int32	nold = BMGetPtrSize ((GSPtr) (memo.shellContours)) / sizeof (API_ShellContourData);
	memo.shellContours = (API_ShellContourData*) BMAllocatePtr ((nold + 1) * sizeof (API_ShellContourData), ALLOCATE_CLEAR, 0);
	if (memo.shellContours != nullptr) {
		Int32	i;
		for (i = 0; i < nold; i++) {
			memo.shellContours[i] = oldmemos[i];
		}
		memo.shellContours[nold].poly.nCoords = poly.nCoords;
		memo.shellContours[nold].poly.nSubPolys = 1;
		memo.shellContours[nold].poly.nArcs = poly.nArcs;
		memo.shellContours[nold].coords = poly.coords;
		memo.shellContours[nold].pends = (Int32**) BMAllocateHandle (2 * sizeof (Int32), ALLOCATE_CLEAR, 0);
		(*(memo.shellContours[nold].pends))[1] = poly.nCoords;
		memo.shellContours[nold].parcs = poly.parcs;

		memo.shellContours[nold].plane.tmx[0] = 1;
		memo.shellContours[nold].plane.tmx[5] = 1;
		memo.shellContours[nold].plane.tmx[10] = 1;

		memo.shellContours[nold].plane.tmx[3] = -element.shell.basePlane.tmx[3];
		memo.shellContours[nold].plane.tmx[7] = -element.shell.basePlane.tmx[7];
		memo.shellContours[nold].plane.tmx[11] = -element.shell.basePlane.tmx[11] - 50.0;
		memo.shellContours[nold].height = 100.0;

		memo.shellContours[nold].edgeData = (API_ContourEdgeData*) BMAllocatePtr ((poly.nCoords + 1) * sizeof (API_ContourEdgeData), ALLOCATE_CLEAR, 0);

		element.shell.numHoles++;

		err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_EdgeTrims, true);
	}

	BMKillPtr ((GSPtr*) &oldmemos);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Shell_Edit2


// -----------------------------------------------------------------------------
// Set different material to morph faces, change transformation and cover fill
// -----------------------------------------------------------------------------

void	Do_Morph_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a morph to modify", API_MorphID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No morph was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);

	// change transformation
	ACAPI_ELEMENT_MASK_SET (mask, API_MorphType, tranmat);
	double co = cos (15.0 * DEGRAD);
	double si = sin (15.0 * DEGRAD);
	API_Tranmat origTran = element.morph.tranmat;
	double* tmx = element.morph.tranmat.tmx;
	for (Int32 j = 0; j < 4; j++) {
		tmx[j]		=	co*origTran.tmx[j] + si*origTran.tmx[8 + j];
		tmx[8 + j]	= -si*origTran.tmx[j] + co*origTran.tmx[8 + j];
	}

	// change cover fill
	ACAPI_ELEMENT_MASK_SET (mask, API_MorphType, coverFillType);
	ACAPI_ELEMENT_MASK_SET (mask, API_MorphType, coverFillPen);
	ACAPI_ELEMENT_MASK_SET (mask, API_MorphType, coverFillBGPen);
	ACAPI_ELEMENT_MASK_SET (mask, API_MorphType, coverFillOrientation);
	ACAPI_ELEMENT_MASK_SET (mask, API_MorphType, useDistortedCoverFill);
	element.morph.coverFillType = 32;
	element.morph.coverFillPen = 1;
	element.morph.coverFillBGPen = 254;
	element.morph.coverFillOrientation.type = API_HatchRotated;
	element.morph.coverFillOrientation.matrix00 = cos (30.0 * DEGRAD);
	element.morph.coverFillOrientation.matrix10 = sin (30.0 * DEGRAD);
	element.morph.coverFillOrientation.matrix01 = -sin (30.0 * DEGRAD);
	element.morph.coverFillOrientation.matrix11 = cos (30.0 * DEGRAD);
	element.morph.useDistortedCoverFill = false;

	// change face materials
	if (memo.morphMaterialMapTable != nullptr) {
		USize mapSize = BMGetPtrSize ((GSPtr)memo.morphMaterialMapTable) / sizeof (API_OverriddenAttribute);
		if (mapSize > 0) {
			API_AttributeIndex materIndex = memo.morphMaterialMapTable[0].attributeIndex;
			API_AttributeIndex nMaterials;
			ACAPI_Attribute_GetNum (API_MaterialID, &nMaterials);
			for (UIndex i = 0; i < mapSize; i++) {
				memo.morphMaterialMapTable[i].attributeIndex = materIndex++;
				if (materIndex >= nMaterials)
					materIndex = 1;
			}
		}
	}

	err = ACAPI_Element_Change (&element, &mask, &memo, 0, true);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Morph_Edit


// -----------------------------------------------------------------------------
// Change width, riser and tread thickness, and slanting
// -----------------------------------------------------------------------------

void	Do_Stair_Edit (void)
{
	constexpr double Pi = 3.141592654;
	constexpr int Run = static_cast<int> (APISR_Run);

	API_Element element;
	BNClear (element);
	if (!ClickAnElem ("Click a Stair to modify.", API_StairID, nullptr, &element.header.typeID, &element.header.guid)) {
		ACAPI_WriteReport ("No Stair was clicked.", true);
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ACAPI_WriteReport ("ACAPI_Element_Get (Stair) has failed with error %s!", true, ErrID_To_Name (err));
		return;
	}

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_StairType, flightWidth);
	ACAPI_ELEMENT_MASK_SET (mask, API_StairType, riser[Run].crossSect);
	ACAPI_ELEMENT_MASK_SET (mask, API_StairType, riser[Run].angle);
	ACAPI_ELEMENT_MASK_SET (mask, API_StairType, riser[Run].thickness);
	ACAPI_ELEMENT_MASK_SET (mask, API_StairType, tread[Run].thickness);

	element.stair.flightWidth *= 2.5;
	element.stair.riser[Run].crossSect = APIRCS_Slanted;
	element.stair.riser[Run].angle = Pi / 3;
	element.stair.riser[Run].thickness *= 3.0;
	element.stair.tread[Run].thickness = element.stair.riser[Run].thickness;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ACAPI_WriteReport ("ACAPI_Element_Change (Stair) has failed with error %s!", true, ErrID_To_Name (err));
		return;
	}
}		// Do_Stair_Edit


// -----------------------------------------------------------------------------
// Change segment heights of a railing.
// -----------------------------------------------------------------------------

void	Do_Railing_Edit (void)
{
	API_Element element;
	BNClear (element);
	if (!ClickAnElem ("Click a Railing to modify.", API_RailingID, nullptr, &element.header.typeID, &element.header.guid)) {
		ACAPI_WriteReport ("No Railing was clicked.", true);
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ACAPI_WriteReport ("ACAPI_Element_Get (Railing) has failed with error %s!", true, ErrID_To_Name (err));
		return;
	}

	API_Element mask = {};
	ACAPI_ELEMENT_MASK_SET (mask, API_RailingType, defSegment.height);
	ACAPI_ELEMENT_MASK_SET (mask, API_RailingType, defNode.postOffset);
	element.railing.defSegment.height *= 1.5;
	element.railing.defNode.postOffset *= 2;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ACAPI_WriteReport ("ACAPI_Element_Change (Railing) has failed with error %s!", true, ErrID_To_Name (err));
		return;
	}
}		// Do_Railing_Edit


// -----------------------------------------------------------------------------
// Change the contour edges and ridges of a mesh
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void	Do_Mesh_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a mesh to modify", API_MeshID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No mesh was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon | APIMemoMask_MeshPolyZ | APIMemoMask_MeshLevel);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_MeshType, contPen);

	element.mesh.contPen = element.mesh.contPen % 255 + 1;

	BMKillHandle ((GSHandle *) &memo.parcs);
	if (element.mesh.poly.nArcs == 0) {
		element.mesh.poly.nArcs = element.mesh.poly.nCoords - 1;
		memo.parcs = (API_PolyArc**) BMhAllClear (element.mesh.poly.nArcs * sizeof (API_PolyArc));
		if (memo.parcs != nullptr) {
			for (Int32 i = 0; i < element.mesh.poly.nArcs; i++) {
				(*memo.parcs)[i].begIndex = i + 1;
				(*memo.parcs)[i].endIndex = i + 2;
				(*memo.parcs)[i].arcAngle = 0.5;
			}
		}
	} else {
		element.mesh.poly.nArcs = 0;
	}

	if (memo.meshLevelCoords != nullptr) {
		Int32 nMeshLevels = BMGetHandleSize ((GSHandle) memo.meshLevelCoords) / sizeof (API_MeshLevelCoord);
		for (Int32 i = 0; i < nMeshLevels; i++) {
			(*memo.meshLevelCoords)[i].c.z = 1.0 - (double)i / nMeshLevels;
		}
	}

	err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_MeshLevel, true);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Mesh_Edit


// -----------------------------------------------------------------------------
// Change the height and layer of the clicked zone
//	- dimensions must be updated automatically
// -----------------------------------------------------------------------------

void	Do_Zone_Edit (short mode)
{
	API_Element			element, mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a zone to modify", API_ZoneID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No zone was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	if (mode == 1) {					/* only setting */
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_ZoneType, pen);

		element.header.layer = 1;
		element.zone.pen = 10;

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	} else if (mode == 2) {				/* only setting, but has effect on the parameter list */
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_ZoneType, pen);
		ACAPI_ELEMENT_MASK_SET (mask, API_ZoneType, roomHeight);

		element.header.layer = 1;
		element.zone.pen = 10;
		element.zone.roomHeight /= 2.0;

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	} else if (mode == 3) {				/* only the parameter list */
		ACAPI_ELEMENT_MASK_CLEAR (mask);

		err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_AddPars);
		if (err == NoError) {
			(*memo.params)[26].value.real = 8.8888;		/* calculated area */
			err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_AddPars, true);
		}
		ACAPI_DisposeElemMemoHdls (&memo);

	} else {							/* everything: setting & parameter list */
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_ZoneType, pen);
		ACAPI_ELEMENT_MASK_SET (mask, API_ZoneType, roomHeight);

		element.header.layer = 1;
		element.zone.pen = 10;

		err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_AddPars);
		if (err == NoError) {
			(*memo.params)[26].value.real = 8.8888;		/* calculated area */
			err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_AddPars, true);
		}
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	return;
}		// Do_Zone_Edit


// -----------------------------------------------------------------------------
// Change the content or the wrapping of the clicked text
// -----------------------------------------------------------------------------

void	Do_Word_Edit (short mode)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	const char			*text = { "This word was modified by the Element Test example project.\nThis is a new line." };
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));

	if (mode == 1) {
		if (!ClickAnElem ("Click a word to change its content", API_TextID, nullptr, &element.header.typeID, &element.header.guid)) {
			WriteReport_Alert ("No word was clicked");
			return;
		}

		err = ACAPI_Element_Get (&element);
		if (err != NoError)
			return;

		ACAPI_ELEMENT_MASK_CLEAR (mask);

		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		memo.textContent = BMhAll (Strlen32 (text) + 1);
		if (memo.textContent == nullptr)
			return;

		strcpy (*memo.textContent, text);

		err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_TextContent, true);

		ACAPI_DisposeElemMemoHdls (&memo);

	} else {
		if (!ClickAnElem ("Click a word to change the nonBreaking flag", API_TextID, nullptr, &element.header.typeID, &element.header.guid)) {
			WriteReport_Alert ("No word was clicked");
			return;
		}

		err = ACAPI_Element_Get (&element);
		if (err != NoError)
			return;

		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_TextType, nonBreaking);
		element.text.nonBreaking = !element.text.nonBreaking;
		if (!element.text.nonBreaking) {
			ACAPI_ELEMENT_MASK_SET (mask, API_TextType, width);		// ignored if 'nonBreaking'
			element.text.width /= 2.0;
		}

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	}

	return;
}		// Do_Word_Edit


// -----------------------------------------------------------------------------
// Change the clicked element
// -----------------------------------------------------------------------------

void	Do_2D_Edit (bool withColorOverride)
{
	API_Element			element, mask;
	API_ArrowData		*pArrow;
	short				*pPen;
	short				*pColorOverridePen = nullptr;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a 2D element to change its pen and arrow", API_ZombieElemID, nullptr, &element.header.typeID, &element.header.guid)) {
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	switch (element.header.typeID) {
		case API_HatchID:
				ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, contPen);
				ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, fillPen);
				element.hatch.fillPen.penIndex = 10;
				pPen = &element.hatch.contPen.penIndex;
				pColorOverridePen = &element.hatch.contPen.colorOverridePenIndex;
				pArrow = nullptr;
				break;
		case API_LineID:
				ACAPI_ELEMENT_MASK_SET (mask, API_LineType, linePen);
				ACAPI_ELEMENT_MASK_SET (mask, API_LineType, arrowData.arrowType);
				ACAPI_ELEMENT_MASK_SET (mask, API_LineType, arrowData.begArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_LineType, arrowData.endArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_LineType, arrowData.arrowPen);
				ACAPI_ELEMENT_MASK_SET (mask, API_LineType, arrowData.arrowSize);
				pPen = &element.line.linePen.penIndex;
				pColorOverridePen = &element.line.linePen.colorOverridePenIndex;
				pArrow = &element.line.arrowData;
				break;
		case API_ArcID:
				ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, linePen);
				ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, arrowData.arrowType);
				ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, arrowData.begArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, arrowData.endArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, arrowData.arrowPen);
				ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, arrowData.arrowSize);
				pPen = &element.arc.linePen.penIndex;
				pColorOverridePen = &element.arc.linePen.colorOverridePenIndex;
				pArrow = &element.arc.arrowData;
				break;
		case API_SplineID:
				ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, linePen);
				ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.arrowType);
				ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.begArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.endArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.arrowPen);
				ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.arrowSize);
				pPen = &element.spline.linePen.penIndex;
				pColorOverridePen = &element.spline.linePen.colorOverridePenIndex;
				pArrow = &element.spline.arrowData;
				break;
		case API_PolyLineID:
				ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, linePen);
				ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.arrowType);
				ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.begArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.endArrow);
				ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.arrowPen);
				ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.arrowSize);
				pPen = &element.polyLine.linePen.penIndex;
				pColorOverridePen = &element.polyLine.linePen.colorOverridePenIndex;
				pArrow = &element.polyLine.arrowData;
				break;
		default:
				WriteReport_Alert ("No 2D drawing element was clicked");
				return;
	}

	if (pPen != nullptr)
		*pPen = 20;

	if (withColorOverride && pColorOverridePen != nullptr)
		*pColorOverridePen = 20;

	if (pArrow != nullptr) {
		pArrow->arrowType = APIArr_FullArrow30;
		pArrow->begArrow = false;
		pArrow->endArrow = true;
		pArrow->arrowPen = 20;
		pArrow->arrowSize = 3;
	}

	ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	return;
}		// Do_2D_Edit


// -----------------------------------------------------------------------------
// Change clicked hotspot (pen, layer, height, coords)
// -----------------------------------------------------------------------------

void	Do_Hotspot_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a hotspot to modify", API_HotspotID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No hotspot was clicked");
		return;
	}

	ACAPI_Element_Get (&element);

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_HotspotType, pen);
	ACAPI_ELEMENT_MASK_SET (mask, API_HotspotType, height);
	ACAPI_ELEMENT_MASK_SET (mask, API_HotspotType, pos);

	element.header.layer = 1;
	element.hotspot.pen = 6;
	element.hotspot.height *= 2;
	element.hotspot.pos.x += 0.5;
	element.hotspot.pos.y += 0.5;

	ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	return;
}		// Do_Hotspot_Edit


// -----------------------------------------------------------------------------
// Change clicked spline (remove arrow)
// -----------------------------------------------------------------------------

void	Do_Spline_Edit (void)
{
	API_Element		element, mask;
	API_ArrowData	myArrow;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a spline to modify", API_SplineID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No spline was clicked");
		return;
	}

	ACAPI_Element_Get (&element);

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.begArrow);
	ACAPI_ELEMENT_MASK_SET (mask, API_SplineType, arrowData.endArrow);

	myArrow.begArrow = false;
	myArrow.endArrow = false;
	element.spline.arrowData = myArrow;

	ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	return;
}		// Do_Spline_Edit

void	Do_CutPlane_Or_Elevation_Edit_Coords (void)
{
	API_Element			element, element2;
	API_ElementMemo		memo, memo2;
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));

	ClickAnElem ("Click a cut plane to modify", API_CutPlaneID, nullptr, &element.header.typeID, &element.header.guid);
	if (!(element.header.typeID == API_CutPlaneID || element.header.typeID == API_ElevationID || element.header.typeID == API_InteriorElevationID)) {
		WriteReport_Alert ("No cutplane, elevation or interior elevation was clicked");
		return;
	}

	if (ACAPI_Element_Get (&element) != NoError)
		return;

	if (ACAPI_Element_GetMemo (element.header.guid, &memo) != NoError)
		return;

	ClickAnElem ("Click a cut plane to modify", API_CutPlaneID, nullptr, &element2.header.typeID, &element2.header.guid);
	if (!(element2.header.typeID == API_CutPlaneID || element2.header.typeID == API_ElevationID)) {
		WriteReport_Alert ("No cutplane or elevation was clicked");
		return;
	}

	if (ACAPI_Element_Get (&element2) != NoError ||
		ACAPI_Element_GetMemo (element2.header.guid, &memo2) != NoError) {
		return;
	}

	UInt64 memoMask = APIMemoMask_SectionMainCoords;

	if (element.header.typeID == API_InteriorElevationID) {
		memo2.intElevSegments = memo.intElevSegments;
		err = ACAPI_Element_ChangeExt (&element, nullptr, &memo2, memoMask, 0, nullptr, true, 2);
	} else {
		err = ACAPI_Element_Change (&element, nullptr, &memo2, memoMask, true);
	}

	if (err == APIERR_BADPARS) {
		WriteReport_Alert ("Bad parameters!");
	}
	return;

}
// -----------------------------------------------------------------------------
// Change clicked cutPlane (direction and depth)
// -----------------------------------------------------------------------------

void	Do_CutPlane_Edit (void)
{
	API_Element			element;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a cut plane to modify", API_CutPlaneID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No cut plane was clicked");
		return;
	}

	if (ACAPI_Element_Get (&element) != NoError ||
		ACAPI_Element_GetMemo (element.header.guid, &memo) != NoError)
		return;

	if (memo.sectionSegmentMainCoords == nullptr ||
		memo.sectionSegmentDistCoords == nullptr ||
		memo.sectionSegmentDepthCoords == nullptr)
	{
		return;
	}

	GS::Array<Coord> coordArray;

	for (UInt32 ind = 0; ind < element.cutPlane.segment.nMainCoord; ++ind) {
		coordArray.Push (Coord (memo.sectionSegmentMainCoords[ind].x, memo.sectionSegmentMainCoords[ind].y));
	}

	if (DBERROR (coordArray.GetSize () % 2 == 1))
		return;

	Coord depthStartCoord (memo.sectionSegmentDepthCoords[0].x, memo.sectionSegmentDepthCoords[0].y);

	Vector depthVect = depthStartCoord - coordArray.GetFirst ();
	Vector dirVect = Geometry::UnitVector (depthVect);

	double newDepthVectLen = depthVect.GetLength () + 0.2;
	Coord newDepthStartCoord = Geometry::ORIGO2 + ((coordArray.GetFirst () - Geometry::ORIGO2) + dirVect * newDepthVectLen);

	GS::Array<Coord> newMainCoordArray;
	for (UIndex ind = 1; ind < coordArray.GetSize (); ind += 2) {
		const Sector s = Sector (coordArray[ind - 1], coordArray[ind]);
		Coord firstTierceCoord = Geometry::InterpolateCoord (s.c1, s.c2, 1.0/3.0);
		Coord tranlsFirstTierceCoord = Geometry::ORIGO2 + ((firstTierceCoord - Geometry::ORIGO2) + dirVect * 0.2);
		Coord secondTierceCoord = Geometry::InterpolateCoord (s.c1, s.c2, 2.0/3.0);
		Coord tranlsSecondTierceCoord = Geometry::ORIGO2 + ((secondTierceCoord - Geometry::ORIGO2) + dirVect * 0.2);

		newMainCoordArray.Push (s.c1);
		newMainCoordArray.Push (firstTierceCoord);
		newMainCoordArray.Push (tranlsFirstTierceCoord);
		newMainCoordArray.Push (tranlsSecondTierceCoord);
		newMainCoordArray.Push (secondTierceCoord);
		newMainCoordArray.Push (s.c2);
	}

	element.cutPlane.segment.nMainCoord = newMainCoordArray.GetSize ();
	API_Coord*	newMainCoords = (API_Coord *) BMpAllClear (element.cutPlane.segment.nMainCoord * sizeof (API_Coord));
	if (newMainCoords == nullptr) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}
	for (UIndex ind = 0; ind < newMainCoordArray.GetSize (); ++ind) {
		newMainCoords[ind].x = newMainCoordArray[ind].x;
		newMainCoords[ind].y = newMainCoordArray[ind].y;
	}

	element.cutPlane.segment.nDepthCoord = 1;
	API_Coord*	newDepthCoords = (API_Coord *) BMpAllClear (sizeof (API_Coord));
	newDepthCoords[0].x = newDepthStartCoord.x;
	newDepthCoords[0].y = newDepthStartCoord.y;

	ACAPI_Element_Delete ({ element.header.guid });

	BMpKill (reinterpret_cast<GSPtr *>(&memo.sectionSegmentMainCoords));
	memo.sectionSegmentMainCoords = newMainCoords;

	BMpKill (reinterpret_cast<GSPtr *>(&memo.sectionSegmentDepthCoords));
	memo.sectionSegmentDepthCoords = newDepthCoords;

	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return;

}

// -----------------------------------------------------------------------------
// Delete the clicked hatch node, change the pen, rotate reference vector and show/hide area text
//	- dimensions/labels must be updated automatically
// -----------------------------------------------------------------------------

void	Do_Hatch_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_Neig			neig;
	Int32				nodeInd;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a fill polygon to modify", API_HatchID, &neig, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No fill polygon was clicked");
		return;
	}

	if (neig.neigID != APINeig_Hatch) {
		WriteReport_Alert ("Only fill polygon nodes are accepted");
		return;
	}

	ACAPI_Element_Get (&element);

	GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	nodeInd = neig.inIndex;
	err = ACAPI_Goodies (APIAny_DeletePolyNodeID, &memo, &nodeInd);

	if (err == NoError) {
		element.hatch.poly.nCoords--;

		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, contPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, hatchType);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, fillInd);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, fillPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, fillBGPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, hatchOrientation.type);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, hatchOrientation.matrix00);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, hatchOrientation.matrix10);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, hatchOrientation.matrix01);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, hatchOrientation.matrix11);
		ACAPI_ELEMENT_MASK_SET (mask, API_HatchType, showArea);

		element.hatch.contPen.penIndex = 20;
		element.hatch.contPen.colorOverridePenIndex = 0;
		element.hatch.hatchType = API_FillHatch;
		element.hatch.fillInd = 44;
		element.hatch.fillPen.penIndex = 10;
		element.hatch.fillPen.colorOverridePenIndex = 0;
		element.hatch.fillBGPen = 6;
		element.hatch.hatchOrientation.type = API_HatchRotated;

		double	cosa = cos (30 * DEGRAD), sina = sin (30 * DEGRAD);

		element.hatch.hatchOrientation.matrix00 = cosa;
		element.hatch.hatchOrientation.matrix10 = sina;
		element.hatch.hatchOrientation.matrix01 = -sina;
		element.hatch.hatchOrientation.matrix11 = cosa;

		element.hatch.showArea = !element.hatch.showArea;

		err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_Polygon, true);
		if (err == NoError) {
			API_Coord c = (*memo.coords)[1];
			UpdateAllLabels (element.header.guid, c);
		}
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Hatch_Edit


// -----------------------------------------------------------------------------
// Change clicked PolyLine pen, layer, arrow data
// -----------------------------------------------------------------------------

void	Do_PolyLine_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_ArrowData		myArrow;
	API_Neig			neig;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a polyLine to modify", API_PolyLineID, &neig, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No polyLine was clicked");
		return;
	}

	if (neig.neigID != APINeig_PolyLine) {
		WriteReport_Alert ("Only polyLine nodes are accepted");
		return;
	}

	ACAPI_Element_Get (&element);

	GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, linePen);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, ltypeInd);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, roomSeparator);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.arrowType);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.begArrow);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.endArrow);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.arrowPen);
	ACAPI_ELEMENT_MASK_SET (mask, API_PolyLineType, arrowData.arrowSize);

	element.header.layer = 1;
	element.polyLine.linePen.penIndex = 7;
	element.polyLine.linePen.colorOverridePenIndex = 0;
	element.polyLine.ltypeInd = 6;
	element.polyLine.roomSeparator = true;

	myArrow.arrowType = APIArr_SlashLine15;
	myArrow.begArrow = true;
	myArrow.endArrow = true;
	myArrow.arrowSize = 4;
	myArrow.arrowPen = 12;

	element.polyLine.arrowData = myArrow;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_PolyLine_Edit


// -----------------------------------------------------------------------------
// Change clicked label...
// -----------------------------------------------------------------------------
void	Do_Label_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_Neig			neig;
	const char			*text = { "This word was modified by the Element Test example project." };

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (!ClickAnElem ("Click a label to modify", API_LabelID, &neig, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No label was clicked");
		return;
	}

	if (neig.neigID != APINeig_Label) {
		WriteReport_Alert ("Only label nodes are accepted");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}
	if (memo.textContent != nullptr)
		BMKillHandle (&memo.textContent);

	memo.textContent = BMhAll (Strlen32 (text) + 1);
	if (memo.textContent == nullptr)
		return;

	strcpy (*memo.textContent, text);

	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		if (element.label.labelClass == APILblClass_Text)
			ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, u.text.pen);
		if (element.label.labelClass == APILblClass_Text)
			ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, u.text.size);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, pen);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, textSize);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, textWay);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, font);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, faceBits);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, useBgFill);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, fillBgPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, effectsBits);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, ltypeInd);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, framed);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, hasLeaderLine);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, leaderShape);
		ACAPI_ELEMENT_MASK_SET (mask, API_LabelType, begC);

		if (element.label.labelClass == APILblClass_Text)
			element.label.u.text.pen = 13;
		if (element.label.labelClass == APILblClass_Text)
			element.label.u.text.size *= 2;
		element.label.textSize *= 2.0;
		element.label.textWay = APIDir_General;
		element.label.font = 20;
		element.label.faceBits = (element.label.effectsBits == APIFace_Plain) ? APIFace_Italic : APIFace_Plain;
		element.label.useBgFill = !element.label.useBgFill;
		element.label.fillBgPen = (element.label.fillBgPen + 3) % 256;
		element.label.effectsBits = ((element.label.effectsBits & APIEffect_StrikeOut) != 0) ? (element.label.effectsBits &~ APIEffect_StrikeOut) : (element.label.effectsBits | APIEffect_StrikeOut);
		element.label.ltypeInd = 1;
		element.label.framed = !element.label.framed;
		element.label.hasLeaderLine = !element.label.hasLeaderLine;
		element.label.leaderShape = GetNextLeaderLineShape (element.label.leaderShape);
		element.label.pen = 17;
		element.label.begC.x -= 1.5;
		element.label.begC.y -= 1.5;

		err = ACAPI_Element_Change (&element, &mask, &memo, 0, true);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return; // Do_Label_Edit
}


// -----------------------------------------------------------------------------
// Change clicked dimension
// -----------------------------------------------------------------------------

void	Do_Dimension_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (!ClickAnElem ("Click a dimension to modify", API_DimensionID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No dimension was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, linPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, ed_arrowAng);
		ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, textPos);
		ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, markerData.markerType);
		ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, defWitnessForm);

		element.header.layer = 1;
		element.dimension.linPen = (element.dimension.linPen + 1) % 20;
		element.dimension.ed_arrowAng = 2;
		element.dimension.textPos =	(API_TextPosID) ((element.dimension.textPos + 1) % 3);
		element.dimension.markerData.markerType =(API_MarkerID) ((element.dimension.markerData.markerType + 1) % 18);
		element.dimension.defWitnessForm =	(API_WitnessID) ((element.dimension.defWitnessForm + 1) % 4);

		for (int i = 0; i < element.dimension.nDimElem; i++) {
			int witnessID = (int) (*memo.dimElems)[i].witnessForm;
			witnessID = (witnessID + 1) % 4;
			(*memo.dimElems)[i].witnessForm = (API_WitnessID) witnessID;
			if (i == 1) {
				API_NoteType& note = (*memo.dimElems)[i].note;
				note.fixPos = true;
				if (note.useLeaderLine || element.dimension.dimAppear == APIApp_Elev) {
					note.useLeaderLine = false;
					if (element.dimension.dimAppear == APIApp_Elev) {
						note.pos.x = note.pos.x + 1.0;
						note.pos.y = note.pos.y + 2.0;
					} else {
						note.pos.x = note.endC.x;
						note.pos.y = note.endC.y;
					}
					note.noteAngle = PI / 4.0;
				} else {
					note.useLeaderLine = true;
					note.anchorAtTextEnd = false;
					note.begC.x = note.pos.x;
					note.begC.y = note.pos.y;
					note.midC.x = note.begC.x + 1.0;
					note.midC.y = note.begC.y + 2.0;
					note.endC.x = note.midC.x + 14.0;
					note.endC.y = note.midC.y;
					note.leaderPen = 9;
					note.leaderLineType = 5;
					note.leaderShape = API_Splinear;
					note.arrowData.arrowType = APIArr_FullArrow15;
					note.arrowData.arrowPen = 10;
					note.arrowData.arrowSize = 3.0;
					note.anchorPoint = APILbl_BottomAnchor;
				}
			}
		}

		err = ACAPI_Element_Change (&element, &mask, &memo, 0, true);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Dimension_Edit


// -----------------------------------------------------------------------------
// Change clicked LevelDimension
// -----------------------------------------------------------------------------

void	Do_LevelDimension_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a dimension to modify", API_LevelDimensionID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No level dimension was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, pen);
		ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, markerSize);
		ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, dimForm);
		ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, loc);
		ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, note1.contentType);
		ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, note1.contentUStr);

		element.header.layer = 1;
		element.levelDimension.pen = 6;
		element.levelDimension.markerSize *= 2;
		element.levelDimension.dimForm = (element.levelDimension.dimForm + 1) % 10;
		element.levelDimension.loc.x += 1.5;
		element.levelDimension.loc.y += 1.5;
		element.levelDimension.note1.contentType == API_NoteContent_Custom ? element.levelDimension.note1.contentType = API_NoteContent_Measured
																			: element.levelDimension.note1.contentType =	API_NoteContent_Custom;

		GS::UniString content ("Custom");
		element.levelDimension.note1.contentUStr = &content;

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	}

	return;
}		// Do_LevelDimension_Edit


// -----------------------------------------------------------------------------
// Change clicked RadialDimension
// -----------------------------------------------------------------------------

void	Do_RadialDimension_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a dimension to modify", API_RadialDimensionID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No radial dimension was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, linPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, showOrigo);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, endC);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, markerData.markerSize);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, markerData.markerType);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, note.contentType);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, note.contentUStr);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, note.noteAngle);
		ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, note.noteSize);

		element.header.layer = 1;
		element.radialDimension.linPen = 9;
		element.radialDimension.showOrigo = !element.radialDimension.showOrigo;
		element.radialDimension.endC.y += 1.0;
		element.radialDimension.markerData.markerSize *= 1.2;
		element.radialDimension.markerData.markerType =(API_MarkerID) ((element.radialDimension.markerData.markerType + 1) % 18);
		element.radialDimension.note.contentType == API_NoteContent_Custom ? element.radialDimension.note.contentType = API_NoteContent_Measured
																			: element.radialDimension.note.contentType =	API_NoteContent_Custom;
		GS::UniString content ("Custom");
		element.radialDimension.note.contentUStr = &content;
		element.radialDimension.note.noteAngle += 0.5;
		element.radialDimension.note.noteSize *= 1.5;

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	}

	return;
}		// Do_RadialDimension_Edit


// -----------------------------------------------------------------------------
// Change clicked AngleDimension
// -----------------------------------------------------------------------------

void	Do_AngleDimension_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a dimension to modify", API_AngleDimensionID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No angle dimension was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err == NoError) {
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, linPen);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, smallArc);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, markerData.markerSize);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, markerData.markerType);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, textPos);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.contentType);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.contentUStr);

		element.header.layer = (element.header.layer + 1) % 10;
		element.angleDimension.linPen = 9;
		element.angleDimension.smallArc = !element.angleDimension.smallArc;
		element.angleDimension.markerData.markerSize *= 1.2;
		element.angleDimension.markerData.markerType =(API_MarkerID) ((element.angleDimension.markerData.markerType + 1) % 18);
		element.angleDimension.textPos =	(API_TextPosID) ((element.angleDimension.textPos + 1) % 3);
		element.angleDimension.note.contentType = API_NoteContent_Custom;
		GS::UniString content ("Custom");
		element.angleDimension.note.contentUStr = &content;

		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.useLeaderLine);
		ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.fixPos);

		element.angleDimension.note.fixPos = true;
		if (element.angleDimension.note.useLeaderLine) {
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.pos);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.noteAngle);

			element.angleDimension.note.useLeaderLine = false;
			element.angleDimension.note.pos.x = element.angleDimension.note.endC.x;
			element.angleDimension.note.pos.y = element.angleDimension.note.endC.y;
			element.angleDimension.note.noteAngle = PI / 4.0;
		} else {
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.anchorAtTextEnd);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.begC);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.midC);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.endC);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.leaderPen);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.leaderLineType);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.leaderShape);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.arrowData.arrowType);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.arrowData.arrowPen);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.arrowData.arrowSize);
			ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.anchorPoint);

			element.angleDimension.note.useLeaderLine = true;
			element.angleDimension.note.anchorAtTextEnd = false;
			element.angleDimension.note.begC.x = element.angleDimension.note.pos.x;
			element.angleDimension.note.begC.y = element.angleDimension.note.pos.y;
			element.angleDimension.note.midC.x = element.angleDimension.note.begC.x + 1.0;
			element.angleDimension.note.midC.y = element.angleDimension.note.begC.y + 2.0;
			element.angleDimension.note.endC.x = element.angleDimension.note.midC.x + 14.0;
			element.angleDimension.note.endC.y = element.angleDimension.note.midC.y;
			element.angleDimension.note.leaderPen = 9;
			element.angleDimension.note.leaderLineType = 5;
			element.angleDimension.note.leaderShape = API_Segmented;
			element.angleDimension.note.arrowData.arrowType = APIArr_FullCirc;
			element.angleDimension.note.arrowData.arrowPen = 10;
			element.angleDimension.note.arrowData.arrowSize = 3.0;
			element.angleDimension.note.anchorPoint = APILbl_TopAnchor;
		}

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	}

	return;
}		// Do_AngleDimension_Edit


// -----------------------------------------------------------------------------
// Change clicked Detail Marker
// -----------------------------------------------------------------------------
void	Do_Detail_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&mask, sizeof (API_Element));

	if (!ClickAnElem ("Click a detail to modify", API_DetailID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No detail was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_DetailType, pen);
	ACAPI_ELEMENT_MASK_SET (mask, API_DetailType, ltypeInd);
	ACAPI_ELEMENT_MASK_SET (mask, API_DetailType, pos);
	ACAPI_ELEMENT_MASK_SET (mask, API_DetailType, angle);
	ACAPI_ELEMENT_MASK_SET (mask, API_DetailType, horizontalMarker);
	ACAPI_ELEMENT_MASK_SET (mask, API_DetailType, constructionElementsOnly);

	element.header.layer	= 1;
	element.detail.pen		= ++element.detail.pen % 255 + 1;
	element.detail.ltypeInd = (element.detail.ltypeInd + 3) % 10 + 1;
	element.detail.pos.x	-= 1.0;
	element.detail.pos.y	-= 1.0;
	element.detail.angle	+= 0.5;
	element.detail.horizontalMarker = false;
	element.detail.constructionElementsOnly = !element.detail.constructionElementsOnly;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	//change detail marker
	API_Guid markId = element.detail.markId;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&mask, sizeof (API_Element));

	element.header.guid = markId;

	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_AddPars);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pen);
	element.object.pen = (element.object.pen + 20) % 255 + 1;
	GS::UniString tmpUStr ("Arial Black Central European");
	GS::ucscpy (((*memo.params)+2)->value.uStr, tmpUStr.ToUStr());

	err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_AddPars, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Detail_Edit


// -----------------------------------------------------------------------------
// Change clicked Change Marker
// -----------------------------------------------------------------------------
void	Do_ChangeMarker_Edit (void)
{
	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	if (!ClickAnElem ("Click a ChangeMarker to modify", API_ChangeMarkerID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No ChangeMarker was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_ChangeMarkerType, markerComponent);

	element.header.layer = 1;
	if (element.changeMarker.markerComponent == APICMCT_HeadOnly)
		element.changeMarker.markerComponent = APICMCT_CloudOnly;
	else if (element.changeMarker.markerComponent == APICMCT_CloudOnly)
		element.changeMarker.markerComponent = APICMCT_HeadAndCloud;
	else if (element.changeMarker.markerComponent == APICMCT_HeadAndCloud)
		element.changeMarker.markerComponent = APICMCT_CloudOnly;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	// change ChangeMarker marker
	API_Element markerElement;
	BNZeroMemory (&markerElement, sizeof (API_Element));
	markerElement.header.guid = element.changeMarker.markerGuid;

	err = ACAPI_Element_Get (&markerElement);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (markerElement.header.guid, &memo, APIMemoMask_AddPars);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	API_Element markerMask;
	ACAPI_ELEMENT_MASK_CLEAR (markerMask);
	ACAPI_ELEMENT_MASK_SET (markerMask, API_ObjectType, pen);
	markerElement.object.pen = (markerElement.object.pen + 20) % 255 + 1;

	GS::ucscpy ((*memo.params)[2].value.uStr, GS::UniString ("Arial Black Central European").ToUStr ());

	err = ACAPI_Element_Change (&markerElement, &markerMask, &memo, APIMemoMask_AddPars, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	ACAPI_DisposeElemMemoHdls (&memo);
}


// -----------------------------------------------------------------------------
// Change clicked Worksheet
// -----------------------------------------------------------------------------
void	Do_Worksheet_Edit (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&mask, sizeof (API_Element));

	if (!ClickAnElem ("Click a worksheet to modify", API_WorksheetID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No worksheet was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_WorksheetType, pen);
	ACAPI_ELEMENT_MASK_SET (mask, API_WorksheetType, ltypeInd);
	ACAPI_ELEMENT_MASK_SET (mask, API_WorksheetType, pos);
	ACAPI_ELEMENT_MASK_SET (mask, API_WorksheetType, angle);
	ACAPI_ELEMENT_MASK_SET (mask, API_WorksheetType, horizontalMarker);

	element.header.layer	= ++element.header.layer % 10 + 1;
	element.worksheet.pen	= ++element.worksheet.pen % 255 + 1;
	element.worksheet.ltypeInd = (element.worksheet.ltypeInd + 3) % 10 + 1;
	element.worksheet.pos.x	-= 1.0;
	element.worksheet.pos.y	-= 1.0;
	element.worksheet.angle	+= 0.5;
	element.worksheet.horizontalMarker = false;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	//change worksheet marker
	API_Guid markId = element.worksheet.markId;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&mask, sizeof (API_Element));

	element.header.guid = markId;

	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_AddPars);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pen);
	element.object.pen = (element.object.pen + 20) % 255 + 1;
	GS::UniString tmpUStr ("Arial Black Central European");
	GS::ucscpy (((*memo.params)+2)->value.uStr, tmpUStr.ToUStr());

	err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_AddPars, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_Worksheet_Edit


// -----------------------------------------------------------------------------
// Arc editing test
// -----------------------------------------------------------------------------

void	Do_Arc_Edit (void)
{
	API_Element element {};

	if (!ClickAnElem ("Click an arc to modify.", API_ZombieElemID, nullptr, &element.header.typeID, &element.header.guid)) {
		if (element.header.typeID != API_ArcID && element.header.typeID != API_CircleID) {
			WriteReport_Alert ("No arc was clicked.");
			return;
		}
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR (mask);

	ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, whole);
	const bool newWhole = !element.arc.whole;
	element.arc.whole = newWhole;

	if (!newWhole) {
		ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, begAng);
		ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, endAng);
		ACAPI_ELEMENT_MASK_SET (mask, API_ArcType, reflected);
		element.arc.begAng = PI / 2;		//  90 degrees
		element.arc.endAng = 3 * PI / 2;	// 270 degrees
		element.arc.reflected = !element.arc.reflected;
	}

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
	}
}		// Do_Arc_Edit


// -----------------------------------------------------------------------------
// Dimension test
// -----------------------------------------------------------------------------
void	Do_Dimensions_Test (void)
{
	API_Element			element;
	API_Element			mask;
	API_ElementMemo		memo;
	GS::UniString		customText ("Custom");
	GSErrCode			err = NoError;

	for (Int32 j = 1; j <= 32; j++) {
		const API_ElemTypeID typeID = (API_ElemTypeID) j;
		if (typeID != API_DimensionID		&&
			typeID != API_RadialDimensionID	&&
			typeID != API_LevelDimensionID	&&
			typeID != API_AngleDimensionID)
				continue;

		GS::Array<API_Guid> elemList;
		ACAPI_Element_GetElemList (typeID, &elemList);

		for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate (); it != nullptr; ++it) {
			BNZeroMemory (&element, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			element.header.guid = *it;
			err = ACAPI_Element_Get (&element);
			if (err != NoError) {
				ErrorBeep ("ACAPI_Element_Get", err);
				return;
			}
			ACAPI_ELEMENT_MASK_CLEAR (mask);
			switch (typeID) {
				case API_DimensionID:
					element.dimension.defNote.contentType == API_NoteContent_Custom ? element.dimension.defNote.contentType = API_NoteContent_Measured
																					: element.dimension.defNote.contentType = API_NoteContent_Custom;
					element.dimension.defNote.contentUStr = &customText;
					ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, defNote.contentType);
					ACAPI_ELEMENT_MASK_SET (mask, API_DimensionType, defNote.content);
					break;
				case API_RadialDimensionID:
					element.radialDimension.note.contentType == API_NoteContent_Custom ? element.radialDimension.note.contentType  = API_NoteContent_Measured
																						: element.radialDimension.note.contentType = API_NoteContent_Custom;
					element.radialDimension.note.contentUStr = &customText;
					ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, note.contentType);
					ACAPI_ELEMENT_MASK_SET (mask, API_RadialDimensionType, note.content);
					break;
				case API_LevelDimensionID:
					element.levelDimension.note1.contentType == API_NoteContent_Custom ? element.levelDimension.note1.contentType  = API_NoteContent_Measured
																						: element.levelDimension.note1.contentType = API_NoteContent_Custom;
					element.levelDimension.note1.contentUStr = &customText;
					ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, note1.contentType);
					ACAPI_ELEMENT_MASK_SET (mask, API_LevelDimensionType, note1.content);
					break;
				case API_AngleDimensionID:
					element.angleDimension.note.contentType == API_NoteContent_Custom ? element.angleDimension.note.contentType   = API_NoteContent_Measured
																						: element.angleDimension.note.contentType =	API_NoteContent_Custom;
					element.angleDimension.note.contentUStr = &customText;
					ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.contentType);
					ACAPI_ELEMENT_MASK_SET (mask, API_AngleDimensionType, note.content);
					break;
				default:
					break;
			}
			if (element.header.hasMemo) {
				err = ACAPI_Element_GetMemo (element.header.guid, &memo);
				for (int k = 0; k < element.dimension.nDimElem; k++) {
					(*memo.dimElems)[k].note.contentType == API_NoteContent_Custom ? (*memo.dimElems)[k].note.contentType = API_NoteContent_Measured
																				   : (*memo.dimElems)[k].note.contentType = API_NoteContent_Custom;
					if ((*memo.dimElems)[k].note.contentUStr != nullptr)
						delete (*memo.dimElems)[k].note.contentUStr;
					(*memo.dimElems)[k].note.contentUStr = new GS::UniString (customText);
				}
				if (err != NoError) {
					ErrorBeep ("ACAPI_Element_GetMemo", err);
					return;
				}
				err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_All, true);
				if (err != NoError) {
					ErrorBeep ("ACAPI_Element_Change", err);
					return;
				}
				ACAPI_DisposeElemMemoHdls (&memo);
			} else {
				err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
				if (err != NoError) {
					ErrorBeep ("ACAPI_Element_Change", err);
					return;
				}
			}
		}
	}
}		// Do_Dimension_Test


// -----------------------------------------------------------------------------
// Return a randomly chosen drawing title library part index
// -----------------------------------------------------------------------------
typedef struct LibPart {
	GS::String		name;
	Int32			libInd;

	explicit LibPart () : libInd (-1)
		{}

	LibPart (const GS::String& inName, Int32 inLibInd) :
		name (inName), libInd (inLibInd)
		{}

	LibPart (const LibPart& rhs) :
		name (rhs.name), libInd (rhs.libInd)
		{}

	~LibPart ()
		{}

	LibPart& operator= (const LibPart& rhs)
		{
			if (&rhs != this) {
				name	= rhs.name;
				libInd = rhs.libInd;
			}
			return *this;
		}

} LibPart;


// -----------------------------------------------------------------------------
// Get a random title index for the drawing element
// -----------------------------------------------------------------------------
static Int32		GetRandomTitleIndex ()
{
	const GS::UnID unID = BL::BuiltInLibraryMainGuidContainer::GetInstance ().GetUnIDWithNullRevGuid (BL::BuiltInLibPartID::DrawingTitleLibPartID);

	GS::Array<LibPart>	titles;
	API_LibPart			lp;
	Int32				lpNum, retInd = 0;

	ACAPI_LibPart_GetNum (&lpNum);

	BNZeroMemory (&lp, sizeof (API_LibPart));
	CHCopyC (unID.ToUniString ().ToCStr (), lp.parentUnID);
	while (ACAPI_LibPart_Search (&lp, false) == NoError) {		// calls ACAPI_LibPart_Get if OK
		Int32	index = lp.index;

		delete lp.location;
		lp.location = nullptr;

		if (CHCompareASCII (lp.parentUnID, lp.ownUnID) != 0) {
			GS::UniString uName (lp.docu_UName);
			titles.Push (LibPart (GS::String (uName.ToCStr ()), index));
		}

		BNZeroMemory (&lp, sizeof (API_LibPart));
		CHCopyC (unID.ToUniString ().ToCStr (), lp.parentUnID);
		lp.index = index;					// to continue search from here
	}
	if (titles.GetSize () > 0) {
		Int32	index = (Int32) ( ((double) rand () / RAND_MAX) * (double) titles.GetSize () );
		retInd = titles[index].libInd;
		DBPrintf ("Element Test :: Do_Drawing_Edit :: chosen title for the drawing is \"%s\"\n", titles[index].name.ToCStr ());

	}

	return retInd;
}		// GetRandomTitleIndex


// -----------------------------------------------------------------------------
// Change clicked Drawing element
// -----------------------------------------------------------------------------

void	Do_Drawing_Edit (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&mask, sizeof (API_Element));

	if (!ClickAnElem ("Click a drawing to modify", API_DrawingID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No drawing was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, angle);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, hasBorderLine);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, borderLineType);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, borderPen);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, borderSize);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, isCutWithFrame);
	ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, modelOffset);

	element.header.layer	= 1;
	element.drawing.angle	+= 0.5;

	element.drawing.hasBorderLine	= !element.drawing.hasBorderLine;
	element.drawing.borderPen		= ++element.drawing.borderPen % 255 + 1;
	element.drawing.borderLineType	= (element.drawing.borderLineType + 3) % 10 + 1;
	element.drawing.borderSize		= ((Int32)((element.drawing.borderSize + 0.1) * 100) % 1000) / 100.0;
	element.drawing.isCutWithFrame  = true;
	element.drawing.modelOffset.x  += 1;
	element.drawing.modelOffset.y  += 1;
	if (element.drawing.title.libInd == 0) {
		element.drawing.title.libInd = GetRandomTitleIndex ();
		if (element.drawing.title.libInd > 0) {
			ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, title.libInd);
		}
	} else {
		ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, title.libInd);
		element.drawing.title.libInd = 0;	// remove title
		DBPrintf ("Element Test :: Do_Drawing_Edit :: removing title...\n");
	}

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Change", err);
		return;
	}

	return;
}		// Do_Drawing_Edit


// -----------------------------------------------------------------------------
// Helper function to print the section line's coordinates
// -----------------------------------------------------------------------------
static void		DumpSectionLineCoords (const API_InteriorElevationType& intElev,
										const API_SectionSegment* const segments,
										const API_Coord* const coords)
{
	for (UInt32 ii = 0, coInd = 0; ii < intElev.nSegments; ++ii) {
		DBPrintf ("Segment #%d: \n", ii + 1);
		for (UInt32 jj = 0; jj < segments[ii].nMainCoord - 1; ++jj) {
			DBPrintf ("	Line #%2d: (%5.2lf, %5.2lf) - (%5.2lf, %5.2lf)\n",
				coords[coInd].x, coords[coInd].y, coords[coInd+1].x, coords[coInd+1].y);
			coInd++;
		}
		coInd++;
	}
}	// DumpSectionLineCoords


// -----------------------------------------------------------------------------
// Change certain interior elevation segment settings to match the default
// -----------------------------------------------------------------------------
void		Do_ChangeInteriorElevation (void)
{
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_SubElemMemoMask	markers[3];
	API_SectionSegment	*segments;
	API_Coord			*slCoords;
	API_Elem_Head		head;
	API_Neig			neig;
	UInt32				nSeg;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (markers, 3 * sizeof (API_SubElemMemoMask));

	if (!ClickAnElem ("Click an element", API_ZombieElemID, &neig, &element.header.typeID, &element.header.guid) ||
		neig.neigID < APINeig_InteriorElevation || neig.neigID > APINeig_SectionSegmentMarker)
	{
		WriteReport_Alert ("No interior elevation element was clicked");
		return;
	}

	markers[0].subType = APISubElemMemoMask_MainMarker;
	markers[1].subType = APISubElemMemoMask_SHMarker;
	markers[2].subType = APISubElemMemoMask_CommonMarker;

	ACAPI_ELEMENT_MASK_SETFULL (mask);

	if (ACAPI_Element_Get (&element) != NoError)
		return;

	if (ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_SectionSegments | APIMemoMask_SectionMainCoords) != NoError)
		return;

	if (memo.intElevSegments == nullptr ||
		memo.sectionSegmentMainCoords == nullptr ||
		memo.sectionSegmentDistCoords == nullptr ||
		memo.sectionSegmentDepthCoords == nullptr)
	{
		return;
	}

	// store data for later use
	head		= element.header;
	segments	= memo.intElevSegments;
	nSeg		= element.interiorElevation.nSegments;
	slCoords	= memo.sectionSegmentMainCoords;
	DumpSectionLineCoords (element.interiorElevation, segments, slCoords);

	// modify the section line coordinates

	if (ACAPI_Element_GetDefaultsExt (&element, nullptr, 3UL, markers) == NoError) {
		element.header = head;
		memo.intElevSegments	= segments;
		element.interiorElevation.nSegments = nSeg;
		// adjust only the last segment in the chain to be the same as the default
		segments[nSeg-1] = element.interiorElevation.segment;
		ACAPI_Element_ChangeExt (&element, &element, &memo, APIMemoMask_SectionSegments, 3UL, markers, /* withDel */ true, nSeg - 1);
	}

	ACAPI_DisposeElemMemoHdls (&memo);		// also kill 'segments'
	for (UInt32 ii = 0; ii < 3; ii++)
		ACAPI_DisposeElemMemoHdls (&markers[ii].memo);

}		// Do_ChangeInteriorElevation


// -----------------------------------------------------------------------------
// Modify the selected/clicked element's renovation status
// -----------------------------------------------------------------------------
void		Do_RotateRenovationStatus (void)
{
	API_Element			element, mask;
	API_SelectionInfo 	selectionInfo;
	GS::Array<API_Neig>	selNeigs;
	Int32				index;
	GSErrCode			err;

	// Get selection
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError && err != APIERR_NOSEL) {
		ErrorBeep ("ACAPI_Selection_GetInfo", err);
		return;
	}

	if (selectionInfo.typeID == API_SelEmpty) {
		// Ask the user to click an element
		API_Neig	neig;
		if (!ClickAnElem ("Click an element", API_ZombieElemID, &neig, nullptr, nullptr) ||
			!ACAPI_Element_Filter (neig.guid, APIFilt_IsEditable)) {
			WriteReport_Alert ("No editable element was clicked");
			return;
		}

		selNeigs.Push (neig);
		selectionInfo.sel_nElemEdit = 1;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, renovationStatus);
	for (index = 0; index < selectionInfo.sel_nElemEdit; index++) {
		// Get selected element
		BNZeroMemory (&element, sizeof(element));

		element.header.guid = selNeigs[index].guid;
		if (ACAPI_Element_Get (&element) != NoError)
			continue;

		if		(element.header.renovationStatus == API_ExistingStatus)		element.header.renovationStatus = API_NewStatus;
		else if (element.header.renovationStatus == API_NewStatus)			element.header.renovationStatus = API_DemolishedStatus;
		else if (element.header.renovationStatus == API_DemolishedStatus)	element.header.renovationStatus = API_ExistingStatus;
		else continue;

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
		if (err != NoError) {
			ErrorBeep ("Can't change element's renovation status!", err);
		}
	}

	return;
}		// Do_RotateRenovationStatus


// -----------------------------------------------------------------------------
// Inverts the material chaining on the selected element.
// -----------------------------------------------------------------------------
void	Do_TestMaterialChaining (void)
{
	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	if (!ClickAnElem ("Click an Element with Material Chaining to modify.", API_ZombieElemID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("Error: No Element was clicked!");
		return;
	}

	auto err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		WriteReport_Alert ("Error: ACAPI_Element_Get failed!");
		return;
	}

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR (mask);
	bool *chainingValue = nullptr;

	API_ElementMemo memo;

	switch (element.header.typeID) {
		case API_WallID:
			ACAPI_ELEMENT_MASK_SET (mask, API_WallType, materialsChained);
			ACAPI_ELEMENT_MASK_SET (mask, API_WallType, refMat);
			chainingValue = &element.wall.materialsChained;
			element.wall.refMat.attributeIndex = 10;
			element.wall.refMat.overridden = true;
			break;

		case API_BeamID:

			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamSegment);

			if (err != NoError) {
				WriteReport_Alert ("Error: ACAPI_Element_GetMemo failed!");
				return;
			}

			if (err == NoError && memo.beamSegments != nullptr) {
				for (USize idx = 0; idx < element.beam.nSegments; ++idx) {
					chainingValue = &memo.beamSegments[idx].materialsChained;
					memo.beamSegments[idx].leftMaterial.attributeIndex = 10;
					memo.beamSegments[idx].leftMaterial.overridden = true;
					*chainingValue = !*chainingValue;
				}

				API_Element maskElem;
				ACAPI_ELEMENT_MASK_CLEAR (maskElem);
				err = ACAPI_Element_Change (&element, &maskElem, &memo, APIMemoMask_BeamSegment, true);

				if (err != NoError)
					WriteReport_Alert ("Error: ACAPI_Element_Change failed!");
			}

			return;

		case API_ColumnID:

			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ColumnSegment);

			if (err != NoError) {
				WriteReport_Alert ("Error: ACAPI_Element_GetMemo failed!");
				return;
			}

			if (err == NoError && memo.columnSegments != nullptr) {
				for (USize idx = 0; idx < element.column.nSegments; ++idx) {
					chainingValue = &memo.columnSegments[idx].materialsChained;
					memo.columnSegments[idx].extrusionSurfaceMaterial.attributeIndex = 10;
					memo.columnSegments[idx].extrusionSurfaceMaterial.overridden = true;
					*chainingValue = !*chainingValue;
				}

				API_Element maskElem;
				ACAPI_ELEMENT_MASK_CLEAR (maskElem);
				err = ACAPI_Element_Change (&element, &maskElem, &memo, APIMemoMask_ColumnSegment, true);

				if (err != NoError)
					WriteReport_Alert ("Error: ACAPI_Element_Change failed!");
			}

			return;

		case API_SlabID:
			ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, materialsChained);
			ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, sideMat);
			chainingValue = &element.slab.materialsChained;
			element.slab.sideMat.attributeIndex = 10;
			element.slab.sideMat.overridden = true;
			break;

		case API_MeshID:
			ACAPI_ELEMENT_MASK_SET (mask, API_MeshType, materialsChained);
			ACAPI_ELEMENT_MASK_SET (mask, API_MeshType, topMat);
			chainingValue = &element.mesh.materialsChained;
			element.mesh.topMat.attributeIndex = 10;
			element.mesh.topMat.overridden = true;
			break;

		case API_ShellID:
			ACAPI_ELEMENT_MASK_SET (mask, API_ShellType, shellBase.materialsChained);
			ACAPI_ELEMENT_MASK_SET (mask, API_ShellType, shellBase.sidMat);
			chainingValue = &element.shell.shellBase.materialsChained;
			element.shell.shellBase.sidMat.attributeIndex = 10;
			element.shell.shellBase.sidMat.overridden = true;
			break;

		case API_RoofID:
			ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.materialsChained);
			ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.sidMat);
			chainingValue = &element.roof.shellBase.materialsChained;
			element.roof.shellBase.sidMat.attributeIndex = 10;
			element.roof.shellBase.sidMat.overridden = true;
			break;

		default:
			WriteReport_Alert ("Error: Clicked element does not have Material Chaining!");
			return;
	}

	*chainingValue = !*chainingValue;

	err = ACAPI_Element_Change (&element, &mask, nullptr, 0U, true);
	if (err != NoError)
		WriteReport_Alert ("Error: ACAPI_Element_Change failed!");

	return;

}		// Do_TestMaterialChaining


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool Do_ModifyProfileParameter (const GS::UniString& parameterName, double newValue)
{
	API_SelectionInfo 	selectionInfo;
	GS::Array<API_Neig>	selNeigs;
	GSErrCode			err;

	// Get selection
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError && err != APIERR_NOSEL) {
		ErrorBeep ("ACAPI_Selection_GetInfo", err);
		return false;
	}

	if (selectionInfo.typeID == API_SelEmpty) {
		// Ask the user to click an element
		API_Neig	neig;
		if (!ClickAnElem ("Click a profiled beam segment", API_BeamSegmentID, &neig) ||
			!ACAPI_Element_Filter (neig.guid, APIFilt_IsEditable)) {
			WriteReport_Alert ("No editable column was clicked");
			return false;
		}

		selNeigs.Push (neig);
		selectionInfo.sel_nElemEdit = 1;
	}

	API_Guid elemGuid = selNeigs[0].guid;

	API_Element element = {};
	element.header.guid = elemGuid;
	if (DBERROR (ACAPI_Element_Get (&element) != NoError))
		return false;

	const bool isElemAttributeProfiledWall			= element.header.typeID == API_WallID &&
													element.wall.modelElemStructureType == API_ProfileStructure &&
													element.wall.profileAttr != Profile_Invalid;
	const bool isElemAttributeProfiledBeamSegment	= element.header.typeID == API_BeamSegmentID &&
													element.beamSegment.assemblySegmentData.modelElemStructureType == API_ProfileStructure &&
													element.beamSegment.assemblySegmentData.profileAttr != Profile_Invalid;
	const bool isElemAttributeProfiledColumnSegment = element.header.typeID == API_ColumnSegmentID &&
													element.columnSegment.assemblySegmentData.modelElemStructureType == API_ProfileStructure &&
													element.columnSegment.assemblySegmentData.profileAttr != Profile_Invalid;

	if (isElemAttributeProfiledWall || isElemAttributeProfiledBeamSegment || isElemAttributeProfiledColumnSegment) {
		GS::Array<API_PropertyDefinition> propertyDefinitions;
		if (DBERROR (ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_FundamentalBuiltIn, propertyDefinitions) != NoError)) {
			return false;
		}
		for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions) {
			if (propertyDefinition.definitionType != API_PropertyDynamicBuiltInDefinitionType) {
				continue;
			}
			if (propertyDefinition.name == parameterName) { // Make sure you dont name a parameter "Position" or "Renovation Status"
				API_Property property;
				if (DBERROR (ACAPI_Element_GetPropertyValue (elemGuid, propertyDefinition.guid, property) != NoError)) {
					return false;
				}
				property.value.singleVariant.variant.doubleValue = newValue;
				if (DBVERIFY (ACAPI_Element_SetProperty (elemGuid, property) == NoError)) {
					return true;
				} else {
					return false;
				}
			}
		}
	}

	return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

void		Do_Modify_Opening (void)
{
	API_Element element, mask;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click an element containing Openings", API_ZombieElemID, nullptr, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No object was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	GS::Array<API_Guid>		openingsGuid;
	ACAPI_Element_GetConnectedElements (element.header.guid, API_OpeningID, &openingsGuid);

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_OpeningID;
	err = ACAPI_Element_GetDefaults (&element, nullptr);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (opening)", err);
		return;
	}

	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_OpeningType, extrusionGeometryData.parameters.width);		// extrusionGeometryData.parameters.width

	for (UIndex i = 0; i < openingsGuid.GetSize (); i++) {
		element.header.guid = openingsGuid[i];

		err = ACAPI_Element_Change (&element, &mask, nullptr, 0UL, true);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Change", err);
		}
	}

	return;
}		// Do_Modify_Opening