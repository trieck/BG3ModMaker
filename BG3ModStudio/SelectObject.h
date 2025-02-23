#pragma once

class CSelectObject
{
public:
    CSelectObject(HDC hdc, HGDIOBJ hObject);
    ~CSelectObject();
    HGDIOBJ Select(HGDIOBJ hObject);
    HGDIOBJ Unselect();
    operator HGDIOBJ() const;

private:
    HDC m_hdc;
    HGDIOBJ m_hObject, m_hOldObject;
};
