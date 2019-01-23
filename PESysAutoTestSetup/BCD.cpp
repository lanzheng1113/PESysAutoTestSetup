#include "BCD.h"

BCD::BCDStore BCD::BCDStore::OpenStore( LPCWSTR lpStoreName /*= L""*/, BOOL *pStatus /*= NULL*/ )
{
	WMIServices services(L"root\\WMI");
	if (!services.Valid())
	{
		if (pStatus) *pStatus = FALSE;
		return BCDStore();
	}
		
	WMIObject storeClass = services.GetObject(L"BCDStore");
	if (!storeClass.Valid())
	{
		if (pStatus) *pStatus = FALSE;
		return BCDStore();
	}
	CComVariant res, objStore;
	BOOL Bxx = storeClass.ExecMethod(L"OpenStore", &res, &objStore, L"File", L"");
	if (!Bxx)
	{
		if (pStatus) *pStatus = FALSE;
		return BCDStore();
	}
	if ((objStore.vt != VT_UNKNOWN) || !objStore.punkVal)
	{
		if (pStatus) *pStatus = FALSE;
		return BCDStore();
	}

	if (pStatus) *pStatus = TRUE;
	return BCDStore(objStore.punkVal, services);
}

BOOL BCD::BCDStore::ProvideBcdObjects()
{
	if (!m_BcdObjects.empty())
		return TRUE;

	CComVariant res, objects;
	BOOL bxx = ExecMethod(L"EnumerateObjects", &res, &objects, L"Type", CComVariant(0, VT_I4));
	if (!bxx)
		return FALSE;

	if (objects.vt != (VT_ARRAY | VT_UNKNOWN))
		return FALSE;

	m_BcdObjects.clear();

	size_t nElements = objects.parray->rgsabound[0].cElements;

	m_BcdObjects.reserve(nElements);

	for (ULONG i = 0; i < nElements; i++)
	{
		IUnknown *pUnknown = ((IUnknown **)objects.parray->pvData)[i];
		CComPtr<IWbemClassObject> pBcdObject;

		BCDObject obj(pUnknown, m_pServices);
		if (!obj.Valid())
			return FALSE;

		m_BcdObjects.push_back(obj);
	}

	return TRUE;
}

BCD::BCDObject BCD::BCDStore::CopyObject( const BCDObject &Obj, BOOL *pStatus )
{
	if (!Obj.Valid())
	{
		if (pStatus) *pStatus = FALSE;
		return BCDObject();
	}
	wstring objID = Obj.GetID();
	if (objID.empty())
	{
		if (pStatus) *pStatus = FALSE;
		return BCDObject();
	}

	CComVariant res, newobj;
	BOOL bxx = ExecMethod(L"CopyObject", &res, &newobj, L"SourceStoreFile", L"", L"SourceId", objID.c_str(), L"Flags", 1);
	if (!bxx)
	{
		if (pStatus) *pStatus = FALSE;
		return BCDObject();
	}
	if ((newobj.vt != VT_UNKNOWN) || !newobj.punkVal)
	{
		if (pStatus) *pStatus = FALSE;
		return BCDObject();
	}
	if (pStatus) *pStatus = TRUE;
	return BCDObject(newobj.punkVal, m_pServices);
}

BCD::BCDElement BCD::BCDObject::GetElement( BCDElementType type )
{
	if (!Valid())
		return BCDElement();

	CComVariant res, bcdElement;
	BOOL bxx = ExecMethod(L"GetElement", &res, &bcdElement, L"Type", (long)type);
	if (!bxx)
		return BCDElement();

	if ((bcdElement.vt != VT_UNKNOWN) || !bcdElement.punkVal)
		return BCDElement();

	return BCDElement(bcdElement.punkVal, m_pServices, *this, type);
}

BOOL BCD::BCDObject::SetElementHelper( BCDElementType type, LPCWSTR pFunctionName, LPCWSTR pParamName, const CComVariant &Value )
{
	assert(pFunctionName && pParamName);
	if (!Valid())
		return FALSE;

	CComVariant res;
	if (!ExecMethod(pFunctionName, &res, L"Type", (long)type, pParamName, Value))
		return FALSE;

	if ((res.vt != VT_BOOL) && (res.boolVal != TRUE))
		return FALSE;

	return TRUE;
}

BOOL BCD::BCDObjectList::ApplyChanges()
{
	if (!Valid())
		return FALSE;
	
	SAFEARRAY *pArray = SafeArrayCreateVector(VT_BSTR, 0, (ULONG)m_Ids.size());
	if (!pArray)
		return FALSE;

	for (size_t i = 0; i < m_Ids.size(); i++)
		((BSTR *)pArray->pvData)[i] = SysAllocString(m_Ids[i].c_str());

	CComVariant objList(pArray);
	SafeArrayDestroy(pArray);
	
	return m_Element.SetElementHelper(L"SetObjectListElement", L"Ids", objList);
}

