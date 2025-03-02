#pragma once

template <typename T = HGDIOBJ>
class CSelectObject
{
public:
    CSelectObject(HDC hdc, const T& hObject);
    ~CSelectObject();
    T Select(T hObject);
    T Unselect();
    operator T() const;

private:
    HDC m_hdc;
    T m_hObject, m_hOldObject;
};

template <typename T>
CSelectObject<T>::CSelectObject(HDC hdc, const T& hObject) : m_hdc(hdc), m_hObject(hObject), m_hOldObject(Select(hObject))
{
}

template <typename T>
CSelectObject<T>::~CSelectObject()
{
    Unselect();
}

template <typename T>
T CSelectObject<T>::Select(T hObject)
{
    return m_hOldObject = static_cast<T>(SelectObject(m_hdc, hObject));
}

template <typename T>
T CSelectObject<T>::Unselect()
{
    return static_cast<T>(SelectObject(m_hdc, m_hOldObject));
}

template <typename T>
CSelectObject<T>::operator T() const
{
    return m_hObject;
}
