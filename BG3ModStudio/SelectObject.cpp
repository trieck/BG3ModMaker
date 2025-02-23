#include "stdafx.h"
#include "SelectObject.h"

CSelectObject::CSelectObject(HDC hdc, HGDIOBJ hObject) : m_hdc(hdc), m_hObject(hObject), m_hOldObject(Select(hObject))
{
}

HGDIOBJ CSelectObject::Select(HGDIOBJ hObject)
{
    return m_hOldObject = SelectObject(m_hdc, hObject);
}

HGDIOBJ CSelectObject::Unselect()
{
    return SelectObject(m_hdc, m_hOldObject);
}

CSelectObject::operator HGDIOBJ() const
{
    return m_hObject;
}

CSelectObject::~CSelectObject()
{
    Unselect();
}
