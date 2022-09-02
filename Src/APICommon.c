// *****************************************************************************
// Helper functions for Add-On development
// API Development Kit 24; Mac/Win
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_APICOMMON_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<stdarg.h>
#include	<math.h>

#include	"GSSystem.h"

#include	"ACAPinc.h"
#include	"APICommon.h"


#define USE_DEBUG_WINDOW	1

// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------


//// =============================================================================
////
////	Messages
////
//// =============================================================================


// -----------------------------------------------------------------------------
// Return a descriptive name for an error code
// -----------------------------------------------------------------------------

const char*		ErrID_To_Name (GSErrCode err)
{
	const char	*str;

	switch (err) {
		case APIERR_GENERAL:			str = "APIERR_GENERAL";			break;
		case APIERR_MEMFULL:			str = "APIERR_MEMFULL";			break;
		case APIERR_CANCEL:				str = "APIERR_CANCEL";			break;

		case APIERR_BADID:				str = "APIERR_BADID";			break;
		case APIERR_BADINDEX:			str = "APIERR_BADINDEX";		break;
		case APIERR_BADNAME:			str = "APIERR_BADNAME";			break;
		case APIERR_BADPARS:			str = "APIERR_BADPARS";			break;
		case APIERR_BADPOLY:			str = "APIERR_BADPOLY";			break;
		case APIERR_BADDATABASE:		str = "APIERR_BADDATABASE";		break;
		case APIERR_BADWINDOW:			str = "APIERR_BADWINDOW";		break;
		case APIERR_BADKEYCODE:			str = "APIERR_BADKEYCODE";		break;
		case APIERR_BADPLATFORMSIGN:	str = "APIERR_BADPLATFORMSIGN";	break;
		case APIERR_BADPLANE:			str = "APIERR_BADPLANE";		break;
		case APIERR_BADUSERID:			str = "APIERR_BADUSERID";		break;
		case APIERR_BADVALUE:			str = "APIERR_BADVALUE";		break;
		case APIERR_BADELEMENTTYPE:		str = "APIERR_BADELEMENTTYPE";	break;
		case APIERR_IRREGULARPOLY:		str = "APIERR_IRREGULARPOLY";	break;

		case APIERR_NO3D:				str = "APIERR_NO3D";			break;
		case APIERR_NOMORE:				str = "APIERR_NOMORE";			break;
		case APIERR_NOPLAN:				str = "APIERR_NOPLAN";			break;
		case APIERR_NOLIB:				str = "APIERR_NOLIB";			break;
		case APIERR_NOLIBSECT:			str = "APIERR_NOLIBSECT";		break;
		case APIERR_NOSEL:				str = "APIERR_NOSEL";			break;
		case APIERR_NOTEDITABLE:		str = "APIERR_NOTEDITABLE";		break;
		case APIERR_NOTSUBTYPEOF:		str = "APIERR_NOTSUBTYPEOF";	break;
		case APIERR_NOTEQUALMAIN:		str = "APIERR_NOTEQUALMAIN";	break;
		case APIERR_NOTEQUALREVISION:	str = "APIERR_NOTEQUALREVISION";	break;
		case APIERR_NOTEAMWORKPROJECT:	str = "APIERR_NOTEAMWORKPROJECT";	break;

		case APIERR_NOUSERDATA:			str = "APIERR_NOUSERDATA";		break;
		case APIERR_MOREUSER:			str = "APIERR_MOREUSER";		break;
		case APIERR_LINKEXIST:			str = "APIERR_LINKEXIST";		break;
		case APIERR_LINKNOTEXIST:		str = "APIERR_LINKNOTEXIST";	break;
		case APIERR_WINDEXIST:			str = "APIERR_WINDEXIST";		break;
		case APIERR_WINDNOTEXIST:		str = "APIERR_WINDNOTEXIST";	break;
		case APIERR_UNDOEMPTY:			str = "APIERR_UNDOEMPTY";		break;
		case APIERR_REFERENCEEXIST:		str = "APIERR_REFERENCEEXIST";	break;
		case APIERR_NAMEALREADYUSED:	str = "APIERR_NAMEALREADYUSED";	break;

		case APIERR_ATTREXIST:					str = "APIERR_ATTREXIST";					break;
		case APIERR_DELETED:					str = "APIERR_DELETED";						break;
		case APIERR_LOCKEDLAY:					str = "APIERR_LOCKEDLAY";					break;
		case APIERR_HIDDENLAY:					str = "APIERR_HIDDENLAY";					break;
		case APIERR_INVALFLOOR:					str = "APIERR_INVALFLOOR";					break;
		case APIERR_NOTMINE:					str = "APIERR_NOTMINE";						break;
		case APIERR_NOACCESSRIGHT:				str = "APIERR_NOACCESSRIGHT";				break;
		case APIERR_BADPROPERTY:			str = "APIERR_BADPROPERTY";			break;
		case APIERR_BADCLASSIFICATION:	str = "APIERR_BADCLASSIFICATION";	break;

		case APIERR_MODULNOTINSTALLED:			str = "APIERR_MODULNOTINSTALLED";			break;
		case APIERR_MODULCMDMINE:				str = "APIERR_MODULCMDMINE";				break;
		case APIERR_MODULCMDNOTSUPPORTED:		str = "APIERR_MODULCMDNOTSUPPORTED";		break;
		case APIERR_MODULCMDVERSNOTSUPPORTED:	str = "APIERR_MODULCMDVERSNOTSUPPORTED";	break;
		case APIERR_NOMODULEDATA:				str = "APIERR_NOMODULEDATA";				break;

		case APIERR_PAROVERLAP:			str = "APIERR_PAROVERLAP";		break;
		case APIERR_PARMISSING:			str = "APIERR_PARMISSING";		break;
		case APIERR_PAROVERFLOW:		str = "APIERR_PAROVERFLOW";		break;
		case APIERR_PARIMPLICIT:		str = "APIERR_PARIMPLICIT";		break;

		case APIERR_RUNOVERLAP:			str = "APIERR_RUNOVERLAP";		break;
		case APIERR_RUNMISSING:			str = "APIERR_RUNMISSING";		break;
		case APIERR_RUNOVERFLOW:		str = "APIERR_RUNOVERFLOW";		break;
		case APIERR_RUNIMPLICIT:		str = "APIERR_RUNIMPLICIT";		break;
		case APIERR_RUNPROTECTED:		str = "APIERR_RUNPROTECTED";	break;

		case APIERR_EOLOVERLAP:			str = "APIERR_EOLOVERLAP";		break;

		case APIERR_TABOVERLAP:			str = "APIERR_TABOVERLAP";		break;

		case APIERR_NOTINIT:			str = "APIERR_NOTINIT";			break;
		case APIERR_NESTING:			str = "APIERR_NESTING";			break;
		case APIERR_NOTSUPPORTED:		str = "APIERR_NOTSUPPORTED";	break;
		case APIERR_REFUSEDCMD:			str = "APIERR_REFUSEDCMD";		break;
		case APIERR_REFUSEDPAR:			str = "APIERR_REFUSEDPAR";		break;
		case APIERR_READONLY:			str = "APIERR_READONLY";		break;
		case APIERR_SERVICEFAILED:		str = "APIERR_SERVICEFAILED";	break;
		case APIERR_COMMANDFAILED:		str = "APIERR_COMMANDFAILED";	break;
		case APIERR_NEEDSUNDOSCOPE:		str = "APIERR_NEEDSUNDOSCOPE";	break;

		case APIERR_MISSINGCODE:		str = "APIERR_MISSINGCODE";		break;
		case APIERR_MISSINGDEF:			str = "APIERR_MISSINGDEF";		break;
		default:						str = "???";					break;
	}

	return str;
}		// ErrID_To_Name


