// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 24; Mac/Win
//
//	Hotlink management
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#define	_ELEMENT_HOTLINK_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"Element_Test.h"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------


// =============================================================================
//
// Hotlink management
//
// =============================================================================

// -----------------------------------------------------------------------------
// Ask the user to click on an element to select a hotlink instance
// -----------------------------------------------------------------------------

static bool	SelectPlacedHotlinkByClickOnElement (API_Element* hotlinkElem)
{
	API_Guid guid = APINULLGuid;

	if (!ClickAnElem ("Click on an element to select a hotlink instance", API_ZombieElemID, nullptr, nullptr, &guid)) {
		WriteReport_Alert ("No element was selected");
		return false;
	}

	API_Guid hotlinkElemGuid = APINULLGuid;
	GSErrCode err = ACAPI_Goodies (APIAny_GetContainingHotlinkGuidID, &guid, &hotlinkElemGuid);

	if (err != NoError || hotlinkElemGuid == APINULLGuid) {
		WriteReport_Alert ("The selected element is not part of any hotlink instance");
		return false;
	}

	BNZeroMemory (hotlinkElem, sizeof (API_Element));
	hotlinkElem->header.typeID = API_HotlinkID;
	hotlinkElem->header.guid = hotlinkElemGuid;
	err = ACAPI_Element_Get (hotlinkElem);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return false;
	}

	return true;
}		// SelectPlacedHotlinkByClickOnElement


// -----------------------------------------------------------------------------
// Create a hotlink node and place hotlink element
// -----------------------------------------------------------------------------

GSErrCode	Do_CreateHotlink (void)
{
	API_Guid hotlinkNodeGuid = APINULLGuid;

	IO::Location sourceFileLoc;
	API_SpecFolderID specID = API_ApplicationFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &sourceFileLoc);
	sourceFileLoc.AppendToLocal (IO::Name ("ARCHICAD Examples"));
	sourceFileLoc.AppendToLocal (IO::Name ("Residential House"));
	sourceFileLoc.AppendToLocal (IO::Name ("Residential House.pln"));

	API_HotlinkNode hotlinkNode;
	BNZeroMemory (&hotlinkNode, sizeof (API_HotlinkNode));
	hotlinkNode.type = APIHotlink_Module;
	hotlinkNode.storyRangeType = APIHotlink_AllStories;
	hotlinkNode.sourceLocation = &sourceFileLoc;
	hotlinkNode.refFloorInd = 0;
	GS::ucsncpy (hotlinkNode.name, L("Hotlink created by test add-on"), API_UniLongNameLen - 1);

	GSErrCode err = ACAPI_Database (APIDb_CreateHotlinkNodeID, &hotlinkNode);		// create new hotlink node
	if (err == NoError) {
		char guidStr[64];
		hotlinkNodeGuid = hotlinkNode.guid;
		APIGuid2GSGuid (hotlinkNodeGuid).ConvertToString (guidStr);
		WriteReport ("Hotlink node is created successfully: {%s}", guidStr);
	}

	if (err == NoError && hotlinkNodeGuid != APINULLGuid) {
		API_Element element;														// place an instance of this hotlink
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.typeID = API_HotlinkID;
		element.header.layer = 1;
		element.hotlink.hotlinkNodeGuid = hotlinkNodeGuid;
		element.hotlink.type = APIHotlink_Module;
		element.hotlink.transformation.tmx[3] = 10;
		element.hotlink.transformation.tmx[7] = 4;
		double	co = cos (0.52359877556);
		double	si = sin (0.52359877556);
		element.hotlink.transformation.tmx[0] = element.hotlink.transformation.tmx[5] = co;
		element.hotlink.transformation.tmx[1] = -si;
		element.hotlink.transformation.tmx[4] = si;

		element.hotlink.ignoreTopFloorLinks	= true;
		element.hotlink.relinkWallOpenings	= false;
		element.hotlink.adjustLevelDiffs	= false;

		err = ACAPI_Element_Create (&element, nullptr);
		if (err == NoError) {
			char guidStr[64];
			APIGuid2GSGuid (element.header.guid).ConvertToString (guidStr);
			WriteReport ("Hotlink instance is created successfully: {%s}", guidStr);
		}
	}

	return err;
}		// Do_CreateHotlink


// -----------------------------------------------------------------------------
// Changes the settings of all hotlinks
// -----------------------------------------------------------------------------

GSErrCode	Do_ChangeHotlink (void)
{
    GSErrCode err = NoError;

    GS::Array<API_Guid> elemList;
    ACAPI_Element_GetElemList (API_HotlinkID, &elemList);

    for (const API_Guid& guid : elemList) {
        API_Element element = {};
        element.header.guid = guid;
        err = ACAPI_Element_Get (&element);

        if (err == NoError) {
            API_Element mask;

            ACAPI_ELEMENT_MASK_CLEAR (mask);

            ACAPI_ELEMENT_MASK_SET (mask, API_HotlinkType, skipNested);
            element.hotlink.skipNested = !element.hotlink.skipNested;

            err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
        }
    }

	return err;
}		// Do_ChangeHotlink


// -----------------------------------------------------------------------------
// Update the content of a hotlink
// -----------------------------------------------------------------------------

GSErrCode	Do_UpdateHotlink (void)
{
	API_Element hotlinkElem;
	GSErrCode err = NoError;
	if (SelectPlacedHotlinkByClickOnElement (&hotlinkElem)) {
		API_HotlinkNode hotlinkNode;
		BNZeroMemory (&hotlinkNode, sizeof (API_HotlinkNode));
		hotlinkNode.guid = hotlinkElem.hotlink.hotlinkNodeGuid;

		err = ACAPI_Database (APIDb_GetHotlinkNodeID, &hotlinkNode, nullptr);
		if (err == NoError) {
			GS::snuprintf (hotlinkNode.name, API_UniLongNameLen - 1, L("Hotlink updated by test add-on [%ls]"),
						   static_cast<const GS::uchar_t*> (TIGetTimeString (TIGetTime ()).ToUStr ()));
			err = ACAPI_Database (APIDb_ModifyHotlinkNodeID, &hotlinkNode);		// rename hotlink node
			if (err == NoError && hotlinkNode.guid != APINULLGuid)				// update cache content
				err = ACAPI_Database (APIDb_UpdateHotlinkCacheID, const_cast<API_Guid*> (&hotlinkNode.guid));

			if (err == NoError) {
				char guidStr[64];
				APIGuid2GSGuid (hotlinkNode.guid).ConvertToString (guidStr);
				WriteReport ("Hotlink node is updated successfully: {%s}", guidStr);
			}
		}

		delete hotlinkNode.sourceLocation;
		delete hotlinkNode.serverSourceLocation;
		BMKillPtr (&hotlinkNode.userData.data);
	}

	return err;
}		// Do_UpdateHotlink


// -----------------------------------------------------------------------------
// Delete a hotlink node with all its hotlinked elements
// -----------------------------------------------------------------------------

GSErrCode	Do_DeleteHotlink (void)
{
	API_Element hotlinkElem;
	GSErrCode err = NoError;
	if (SelectPlacedHotlinkByClickOnElement (&hotlinkElem)) {

		err = ACAPI_Database (APIDb_DeleteHotlinkNodeID, &hotlinkElem.hotlink.hotlinkNodeGuid);

		char guidStr[64];
		APIGuid2GSGuid (hotlinkElem.hotlink.hotlinkNodeGuid).ConvertToString (guidStr);
		WriteReport ("Hotlink reference deleted: {%s}", guidStr);

	}

	return err;
}		// Do_DeleteHotlink


// -----------------------------------------------------------------------------
// Delete a hotlink node, but keep the hotlinked elements in the project
// -----------------------------------------------------------------------------

GSErrCode	Do_BreakHotlink (void)
{
	API_Element hotlinkElem;
	GSErrCode err = NoError;

	if (SelectPlacedHotlinkByClickOnElement (&hotlinkElem)) {

		err = ACAPI_Database (APIDb_BreakHotlinkNodeID, &hotlinkElem.hotlink.hotlinkNodeGuid);

		char guidStr[64];
		APIGuid2GSGuid (hotlinkElem.hotlink.hotlinkNodeGuid).ConvertToString (guidStr);
		WriteReport ("Hotlink reference broken: {%s}", guidStr);
	}

	return err;
}		// Do_BreakHotlink


// -----------------------------------------------------------------------------
// List hotlink nodes and placed hotlink elements
// -----------------------------------------------------------------------------

static GSErrCode	DumpHotlinkNodeWithChildren (const API_Guid&										hotlinkNodeGuid,
												 const GS::HashTable<API_Guid, GS::Array<API_Guid> >&	hotlinkNodeTree,
												 short													nestingLevel)
{
	if (nestingLevel > 100) {
		WriteReport ("NESTING LEVEL ERROR (%d)", nestingLevel);
		return APIERR_GENERAL;
	}

	char guidStr[64];
	APIGuid2GSGuid (hotlinkNodeGuid).ConvertToString (guidStr);

	GS::UniString prefixStr = " ";
	for (short i = 0; i < nestingLevel; i++)
		prefixStr.Append ("  ");
	prefixStr.Append ("o");

	API_HotlinkNode hotlinkNode;
	BNZeroMemory (&hotlinkNode, sizeof (API_HotlinkNode));
	hotlinkNode.guid = hotlinkNodeGuid;

	GSErrCode err = ACAPI_Database (APIDb_GetHotlinkNodeID, &hotlinkNode, nullptr);
	if (err == NoError) {
		char timeString[64];
		TIGetTimeString (hotlinkNode.updateTime, timeString, TI_LONG_DATE_FORMAT | TI_SHORT_TIME_FORMAT);

		GS::UniString path;
		if (hotlinkNode.sourceLocation != nullptr)
			path = hotlinkNode.sourceLocation->ToLogText ();

		WriteReport ("%s \"%s\"  {%s} (%s)", prefixStr.ToCStr ().Get (), path.ToCStr ().Get (), guidStr, timeString);
		delete hotlinkNode.sourceLocation;
		delete hotlinkNode.serverSourceLocation;
		BMKillPtr (&hotlinkNode.userData.data);
	} else {
		WriteReport ("%s {%s} - ERROR", prefixStr.ToCStr ().Get (), guidStr);
	}


	if (hotlinkNodeTree.ContainsKey (hotlinkNodeGuid)) {
		const GS::Array<API_Guid>& nodeRefList = hotlinkNodeTree.Get (hotlinkNodeGuid);
		for (UInt32 i = 0; i < nodeRefList.GetSize () && err == NoError; i++)
			err = DumpHotlinkNodeWithChildren (nodeRefList[i], hotlinkNodeTree, nestingLevel + 1);
	}

	return err;
}		// DumpHotlinkNodeWithChildren


GSErrCode	Do_ListHotlinks (void)
{
	WriteReport ("\n=== Element Test add-on ===");

	for (short i = 1; i <= 2; i++) {
		API_HotlinkTypeID type = (i == 1) ? APIHotlink_Module : APIHotlink_XRef;
		API_Guid hotlinkRootNodeGuid = APINULLGuid;
		if (ACAPI_Database (APIDb_GetHotlinkRootNodeGuidID, &type, &hotlinkRootNodeGuid) == NoError) {
			GS::HashTable<API_Guid, GS::Array<API_Guid> > hotlinkNodeTree;
			if (ACAPI_Database (APIDb_GetHotlinkNodeTreeID, &hotlinkRootNodeGuid, &hotlinkNodeTree) == NoError) {
				if (hotlinkNodeTree.ContainsKey (hotlinkRootNodeGuid)) {
					WriteReport ("List of %s nodes:", (type == APIHotlink_Module) ? "hotlink" : "xref");
					const GS::Array<API_Guid>& nodeRefList = hotlinkNodeTree.Get (hotlinkRootNodeGuid);
					for (UInt32 i = 0; i < nodeRefList.GetSize (); i++)
						DumpHotlinkNodeWithChildren (nodeRefList[i], hotlinkNodeTree, 0);
				} else {
					WriteReport ("No %s node was found", (type == APIHotlink_Module) ? "hotlink" : "xref");
				}
			}
		}
	}

	WriteReport ("===========================\n");

	return NoError;
}		// Do_ListHotlinks
