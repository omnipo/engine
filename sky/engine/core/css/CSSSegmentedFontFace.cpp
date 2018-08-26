/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sky/engine/core/css/CSSSegmentedFontFace.h"

#include "gen/sky/platform/RuntimeEnabledFeatures.h"
#include "sky/engine/core/css/CSSFontSelector.h"
#include "sky/engine/platform/fonts/FontCache.h"
#include "sky/engine/platform/fonts/FontDescription.h"
#include "sky/engine/platform/fonts/FontFaceCreationParams.h"
#include "sky/engine/platform/fonts/SegmentedFontData.h"
#include "sky/engine/platform/fonts/SimpleFontData.h"

namespace blink {

CSSSegmentedFontFace::CSSSegmentedFontFace(CSSFontSelector* fontSelector, FontTraits traits)
    : m_fontSelector(fontSelector)
    , m_traits(traits)
{
}

CSSSegmentedFontFace::~CSSSegmentedFontFace()
{
    pruneTable();
}

void CSSSegmentedFontFace::pruneTable()
{
    // Make sure the glyph page tree prunes out all uses of this custom font.
    if (m_fontDataTable.isEmpty())
        return;

    m_fontDataTable.clear();
}

bool CSSSegmentedFontFace::isValid() const
{
    return false;
}

PassRefPtr<FontData> CSSSegmentedFontFace::getFontData(const FontDescription& fontDescription)
{
    if (!isValid())
        return nullptr;

    FontTraits desiredTraits = fontDescription.traits();
    FontCacheKey key = fontDescription.cacheKey(FontFaceCreationParams(), desiredTraits);

    RefPtr<SegmentedFontData>& fontData = m_fontDataTable.add(key.hash(), nullptr).storedValue->value;
    if (fontData && fontData->numRanges())
        return fontData; // No release, we have a reference to an object in the cache which should retain the ref count it has.

    if (!fontData)
        fontData = SegmentedFontData::create();

    FontDescription requestedFontDescription(fontDescription);
    requestedFontDescription.setTraits(m_traits);
    requestedFontDescription.setSyntheticBold(m_traits.weight() < FontWeight600 && desiredTraits.weight() >= FontWeight600);
    requestedFontDescription.setSyntheticItalic(m_traits.style() == FontStyleNormal && desiredTraits.style() == FontStyleItalic);

    if (fontData->numRanges())
        return fontData; // No release, we have a reference to an object in the cache which should retain the ref count it has.

    return nullptr;
}

bool CSSSegmentedFontFace::isLoading() const
{
    return false;
}

bool CSSSegmentedFontFace::isLoaded() const
{
    return true;
}

void CSSSegmentedFontFace::willUseFontData(const FontDescription& fontDescription, UChar32 character)
{
}

bool CSSSegmentedFontFace::checkFont(const String& text) const
{
    return true;
}

}
