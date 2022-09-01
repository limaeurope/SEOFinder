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


// -----------------------------------------------------------------------------
// Elements: Solid Operations Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsSolidOperation (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		GetSEOElements(false);				break;
				case 2:		GetSEOElements(true);				break;

				default:														break;
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
	err = ACAPI_Register_Menu (32506, 0, MenuCode_UserDef, MenuFlag_Default);

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
