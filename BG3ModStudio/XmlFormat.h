#pragma once

#include "XmlToken.h"

enum XmlColor {
    COLOR_TEXT = 1,       // Default text
    COLOR_TAG,            // <tag>
    COLOR_ATTRIBUTE,      // attribute_name=
    COLOR_VALUE,          // "attribute_value"
    COLOR_COMMENT,        // <!-- comment -->
    COLOR_CDATA,          // <![CDATA[...]]>
    COLOR_DOCTYPE         // <!DOCTYPE ...>
};

XmlColor XmlGetColor(XmlTokenType type);
COLORREF XmlToColorRef(XmlColor color);
