// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 24; Mac/Win
//
//	Tool operations on elements
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#define	_ELEMENT_TOOLS_TRANSL_


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
// Apply tools on elements
//
// =============================================================================

// -----------------------------------------------------------------------------
// Do the command on the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_ElemTool (bool withInput, const char* inputStr, API_ToolCmdID typeID)
{
	GS::Array<API_Guid> elemGuids;
	GSErrCode			err;

	if (withInput)
		elemGuids = ClickElements_Guid (inputStr, API_ZombieElemID);

	err = ACAPI_Element_Tool (elemGuids, typeID, nullptr);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool", err);
}		// Do_ElemTool


// -----------------------------------------------------------------------------
// Toggle the Suspend Groups switch
//   - selection dots must be updated automatically
//   - menu must be updated
// -----------------------------------------------------------------------------

void		Do_SuspendGroups (void)
{
	GSErrCode	err;

	err = ACAPI_Element_Tool ({}, APITool_SuspendGroups, nullptr);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (suspend group)", err);
}		// Do_SuspendGroups
