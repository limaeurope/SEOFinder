// *****************************************************************************
// AddOn to test the API interface (Solid Operation Links Between Elements)
//
// Classes:        Contact person:
//
// [SG compatible]
// *****************************************************************************

#define _ELEMENT_SOLIDLINK_TRANSL_

// ------------------------------ Includes -------------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"SEOFinder.h"


// ------------------------------ Constants ------------------------------------

// -------------------------------- Types --------------------------------------

// ------------------------------ Variables ------------------------------------

// ------------------------------ Prototypes -----------------------------------
//
//static	char *GetULongStr (char		*uLStr,
//						   UInt32	uLong)
//{
//	Int32	*lp;
//#ifdef WINDOWS
//	char	t;
//#endif
//
//	lp  = (Int32 *) uLStr;
//	*lp = uLong; uLStr[4] = 0;
//
//#ifdef WINDOWS
//	t = uLStr[0]; uLStr[0] = uLStr[3]; uLStr[3] = t;
//	t = uLStr[1]; uLStr[1] = uLStr[2]; uLStr[2] = t;
//#endif
//
//	return (uLStr);
//}
//
//
//void Do_SolidOperation_Create (void)
//{
//	API_ElemTypeID		typeID;
//	API_Guid			guid_Target, guid_Operator, guid_Operator2;
//	GS::Array<API_Guid>	guid_Results;
//
//	if (!ClickAnElem ("Click on a solid operation target", API_MorphID, nullptr, &typeID, &guid_Target)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	if (!ClickAnElem ("Click on a solid operator element", API_MorphID, nullptr, &typeID, &guid_Operator)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	if (!ClickAnElem ("Click on a solid operator element", API_MorphID, nullptr, &typeID, &guid_Operator2)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	GSErrCode err = ACAPI_Element_SolidOperation_Create (guid_Target, guid_Operator, APISolid_Substract, &guid_Results);
//	if (err != NoError) {
//		if (err == APIERR_NO3D)
//			WriteReport_Alert ("Solid operation targets and operators can be freeshape elements only.");
//		else
//			WriteReport_Alert ("ACAPI_Element_SolidOperation_Create failed: %d", err);
//	} else {
//		if (!guid_Results.IsEmpty ()) {
//			err = ACAPI_Element_SolidOperation_Create (guid_Results[0], guid_Operator2, APISolid_Add);
//			if (err != NoError) {
//				if (err == APIERR_NO3D)
//					WriteReport_Alert ("Solid operation targets and operators can be freeshape elements only.");
//				else
//					WriteReport_Alert ("ACAPI_Element_SolidOperation_Create failed: %d", err);
//			}
//		}
//	}
//
//	return;
//}		/* Do_SolidOperation_Create */
//
//
//void Do_SolidLink_Create (void)
//{
//	API_ElemTypeID		typeID;
//	API_Guid			guid_Target, guid_Operator;
//
//	if (!ClickAnElem ("Click on a solid operation target", API_ZombieElemID, nullptr, &typeID, &guid_Target)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	if (!ClickAnElem ("Click on a solid operator element", API_ZombieElemID, nullptr, &typeID, &guid_Operator)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	GSErrCode err = ACAPI_Element_SolidLink_Create (guid_Target, guid_Operator, APISolid_Add, APISolidFlag_OperatorAttrib);
//	if (err != NoError) {
//		if (err == APIERR_NO3D)
//			WriteReport_Alert ("Solid operation targets and operators can be construction elements only.");
//		else
//			WriteReport_Alert ("ACAPI_Element_SolidLink_Create failed: %d", err);
//	}
//
//	return;
//}		/* Do_SolidLink_Create */
//
//
//void Do_SolidLink_Remove (void)
//{
//	API_ElemTypeID		typeID;
//	API_Guid			guid_Target, guid_Operator;
//
//	if (!ClickAnElem ("Click on first element", API_ZombieElemID, nullptr, &typeID, &guid_Target)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	if (!ClickAnElem ("Click on a second element", API_ZombieElemID, nullptr, &typeID, &guid_Operator)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	GSErrCode err = ACAPI_Element_SolidLink_Remove (guid_Target, guid_Operator);
//	if (err != NoError)
//		err = ACAPI_Element_SolidLink_Remove (guid_Operator, guid_Target);
//
//	if (err != NoError) {
//		if (err == APIERR_NO3D)
//			WriteReport_Alert ("Solid operation targets and operators can be construction elements only.");
//		else
//			WriteReport_Alert ("ACAPI_Element_SolidLink_Remove failed: %d", err);
//	}
//
//	return;
//}		/* Do_SolidLink_Remove */
//
//
//void Do_SolidLink_Targets (void)
//{
//	API_ElemTypeID			typeID;
//	GSErrCode				err;
//	API_Guid				guid_Operator;
//	GS::Array<API_Guid>		guid_Targets;
//	GSFlags					linkFlags;
//	GSTime					linkTime;
//	UInt32					linkSubTime;
//	API_SolidOperationID	operation;
//	char					str[256], opstr[16];
//
//	if (!ClickAnElem ("Click on solid operator element", API_ZombieElemID, nullptr, &typeID, &guid_Operator)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	err = ACAPI_Element_SolidLink_GetTargets (guid_Operator, &guid_Targets);
//	if (err != NoError) {
//		if (err == APIERR_NO3D)
//			WriteReport_Alert ("Solid operators can be construction elements only.");
//		else
//			WriteReport_Alert ("ACAPI_Element_SolidLink_GetTargets failed: %d", err);
//		return;
//	}
//
//	WriteReport ("-------------------");
//	WriteReport ("guid_Operator: %s", (const char *) APIGuid2GSGuid (guid_Operator).ToUniString ().ToCStr ());
//
//	for (const auto& guidTarget : guid_Targets) {
//		ACAPI_Element_SolidLink_GetOperation (guidTarget, guid_Operator, &operation);
//		ACAPI_Element_SolidLink_GetFlags (guidTarget, guid_Operator, &linkFlags);
//		ACAPI_Element_SolidLink_GetTime (guidTarget, guid_Operator, &linkTime, &linkSubTime);
//
//		char timeBuffer[256];
//		TIGetTimeString (linkTime, timeBuffer, TI_LOCAL_TIME & TI_CURRENT_TIME & TI_LONG_DATE_FORMAT & TI_LONG_TIME_FORMAT);
//		sprintf (str, "  guid_Target: %s  flags: 0x%04lx  operation: %s  linktime: %s",
//					(const char *) APIGuid2GSGuid (guidTarget).ToUniString ().ToCStr (),
//					(GS::LongForStdio) linkFlags,
//					GetULongStr (opstr, operation),
//					timeBuffer);
//		WriteReport (str);
//	}
//}		/* Do_SolidLink_Targets */
//
//
//void Do_SolidLink_Operators (void)
//{
//	API_ElemTypeID			typeID;
//	GSErrCode				err;
//	API_Guid				guid_Target;
//	GS::Array<API_Guid>		guid_Operators;
//	GSFlags					linkFlags;
//	GSTime					linkTime;
//	UInt32					linkSubTime;
//	API_SolidOperationID	operation;
//	char					str[256], opstr[16];
//
//	if (!ClickAnElem ("Click on solid operation target", API_ZombieElemID, nullptr, &typeID, &guid_Target)) {
//		WriteReport_Alert ("No element was clicked.");
//		return;
//	}
//
//	err = ACAPI_Element_SolidLink_GetOperators (guid_Target, &guid_Operators);
//	if (err != NoError) {
//		if (err == APIERR_NO3D)
//			WriteReport_Alert ("Solid operation targets can be construction elements only.");
//		else
//			WriteReport_Alert ("ACAPI_Element_SolidLink_GetOperators failed: %d", err);
//		return;
//	}
//
//	WriteReport ("-----------------");
//	WriteReport ("guid_Target: %s", (const char *) APIGuid2GSGuid (guid_Target).ToUniString ().ToCStr ());
//
//	for (const auto& guidOperator : guid_Operators) {
//		ACAPI_Element_SolidLink_GetOperation (guid_Target, guidOperator, &operation);
//		ACAPI_Element_SolidLink_GetFlags (guid_Target, guidOperator, &linkFlags);
//		ACAPI_Element_SolidLink_GetTime (guid_Target, guidOperator, &linkTime, &linkSubTime);
//
//		char timeBuffer[256];
//		TIGetTimeString (linkTime, timeBuffer, TI_LOCAL_TIME & TI_CURRENT_TIME & TI_LONG_DATE_FORMAT & TI_LONG_TIME_FORMAT);
//		sprintf (str, "  guid_Operator: %s  flags: 0x%04lx  operation: %s  linktime: %s",
//					(const char *) APIGuid2GSGuid (guidOperator).ToUniString ().ToCStr (),
//					(GS::LongForStdio) linkFlags,
//					GetULongStr (opstr, operation),
//					timeBuffer);
//		WriteReport (str);
//	}
//}		/* Do_SolidLink_Operators */

bool boundBoxesMatch(API_Element& element1, API_Element& element2)
{
	API_Box3D	bbox1, bbox2;
	GSErrCode				err;

	BNZeroMemory(&bbox1, sizeof(bbox1));
	BNZeroMemory(&bbox2, sizeof(bbox2));

	err = ACAPI_Database(APIDb_CalcBoundsID, &element1.header, &bbox1);
	err = ACAPI_Database(APIDb_CalcBoundsID, &element2.header, &bbox2);

	bool xMatch, yMatch, zMatch;
	xMatch = bbox1.xMax > bbox2.xMin && bbox2.xMax > bbox1.xMin;
	yMatch = bbox1.yMax > bbox2.yMin && bbox2.yMax > bbox1.yMin;
	zMatch = bbox1.zMax > bbox2.zMin && bbox2.zMax > bbox1.zMin;
	return xMatch && yMatch && zMatch;
}

void GetSEOElements(bool isBoundingBoxConsidered)
{
	GSErrCode				err;
	API_SelectionInfo		selectionInfo;
	GS::Array<API_Neig>		selNeigs;
	API_Element				elementThis, elementOther;
	GS::Array<API_Guid>		guid_Targets, guid_Operators;
	API_Neig				_neig;

	//bool isBoundingBoxConsidered = true;

	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
		ACAPI_WriteReport("Nothing is selected", true);
		return;
	}

	if (err != APIERR_NOSEL && err != NoError) {
		ACAPI_WriteReport("Error in ACAPI_Selection_Get: %s", true, ErrID_To_Name(err));
		return;
	}

	err = ACAPI_Element_Select(selNeigs, false);		//Empty selection 

	GS::Array<API_Neig>		neigS{};

	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		bool bAdd = false;

		BNZeroMemory(&elementThis, sizeof(API_Element));
		elementThis.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&elementThis);
		if (err != NoError) {
			break;
		}

		err = ACAPI_Element_SolidLink_GetTargets(elementThis.header.guid, &guid_Targets);

		err = ACAPI_Element_SolidLink_GetOperators(elementThis.header.guid, &guid_Operators);

		for (const auto& guidTarget : guid_Targets) {
			BNZeroMemory(&elementOther, sizeof(API_Element));
			elementOther.header.guid = guidTarget;

			err = ACAPI_Element_Get(&elementOther);

			if (!isBoundingBoxConsidered || !boundBoxesMatch(elementThis, elementOther)) {
				_neig.guid = elementOther.header.guid;

				neigS.Push(_neig);
				bAdd = true;
			}
		}

		for (const auto& guidOperator : guid_Operators) {
			BNZeroMemory(&elementOther, sizeof(API_Element));
			elementOther.header.guid = guidOperator;

			err = ACAPI_Element_Get(&elementOther);

			if (!isBoundingBoxConsidered || !boundBoxesMatch(elementThis, elementOther)) {
				_neig.guid = elementOther.header.guid;

				neigS.Push(_neig);
				bAdd = true;
			}
		}

		if (bAdd)
			neigS.Push(selNeigs[i]);
	}

	err = ACAPI_Element_Select(neigS, true);
}		/* GetSEOElements */

