#include "stdafx.h"
#include "XmlFormat.h"

XmlColor XmlGetColor(XmlTokenType type)
{
    switch (type) {
    case XmlTokenType::TT_TAG_NAME:
        return COLOR_TAG;
    case XmlTokenType::TT_ATTRIBUTE_NAME:
        return COLOR_ATTRIBUTE;
    case XmlTokenType::TT_ATTRIBUTE_VALUE:
        return COLOR_VALUE;
    case XmlTokenType::TT_COMMENT:
        return COLOR_COMMENT;
    case XmlTokenType::TT_CDATA:
        return COLOR_CDATA;
    case XmlTokenType::TT_DOCTYPE:
        return COLOR_DOCTYPE;
    default:
        return COLOR_TEXT;
    }
};

COLORREF XmlToColorRef(XmlColor color)
{
    switch (color) {
    case COLOR_TEXT:
        return RGB(0, 0, 0);
    case COLOR_TAG:
        return RGB(0, 0, 200);
    case COLOR_ATTRIBUTE:
        return RGB(200, 0, 0);
    case COLOR_VALUE:
        return RGB(0, 128, 0);
    case COLOR_COMMENT:
        return RGB(0, 128, 0);
    case COLOR_CDATA:
        return RGB(200, 0, 200);
    case COLOR_DOCTYPE:
        return RGB(128, 0, 0);
    }

    return RGB(0, 0, 0);
}
