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

typedef GS::Pair<API_Guid, API_Guid> SelectionPair;

GS::HashSet<SelectionPair> seoPairs;
GS::HashSet<SelectionPair>::ConstIterator selectThis;
bool isIterationStarted = false;

// ------------------------------ Prototypes -----------------------------------

void SetMenuClickability()
{
	API_MenuItemRef itemRef;
	GSFlags         itemFlags;

	BNZeroMemory(&itemRef, sizeof(API_MenuItemRef));
	itemRef.menuResID = 32506;
	itemRef.itemIndex = 3;

	itemFlags = 0;
	ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
	if (seoPairs.GetSize() > 0)
		itemFlags &= ~API_MenuItemDisabled;
	else
		itemFlags |= API_MenuItemDisabled;
	ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);

	return;
}

void SelectNextSEOElement()
{
	if (seoPairs.GetSize() == 0) return;	//TODO

	if (!isIterationStarted)
	{
		selectThis = seoPairs.Begin();
		isIterationStarted = true;
	}

	API_Neig neig1{}, neig2{};

	neig1.guid = selectThis->first;
	neig2.guid = selectThis->second;

	GS::Array<API_Neig> neigS;
	neigS.Push(neig1);
	neigS.Push(neig2);
	
	GSErrCode err;

	err = ACAPI_Element_DeselectAll();
	err = ACAPI_Element_Select(neigS, true);

	if (++selectThis == seoPairs.End())
	{
		seoPairs.Clear();
		SetMenuClickability();
	}
}

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
	seoPairs.Clear();

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

				SelectionPair _newPair{ elementThis.header.guid, elementOther.header.guid };
				SelectionPair _oppositePair{ elementOther.header.guid, elementThis.header.guid };
				if (!seoPairs.Contains(_newPair)
					&& !seoPairs.Contains(_oppositePair))
					seoPairs.Add(_newPair);
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

				SelectionPair _newPair{ elementThis.header.guid, elementOther.header.guid };
				SelectionPair _oppositePair{ elementOther.header.guid, elementThis.header.guid };
				if (!seoPairs.Contains(_newPair)
					&& !seoPairs.Contains(_oppositePair))
					seoPairs.Add(_newPair);
			}
		}

		if (bAdd)
			neigS.Push(selNeigs[i]);

		isIterationStarted = false;
	}

	err = ACAPI_Element_Select(neigS, true);
	SetMenuClickability();

	ACAPI_KeepInMemory(true);
}		/* GetSEOElements */

