// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use crate::base::BridgeFontRef;
use crate::ffi::BridgeLocalizedName;
use read_fonts::{tables::os2::SelectionFlags, TableProvider};
use skrifa::{
    string::{LocalizedStrings, StringId},
    MetadataProvider,
};

pub struct BridgeLocalizedStrings<'a> {
    #[allow(dead_code)]
    localized_strings: LocalizedStrings<'a>,
}

pub fn get_localized_strings<'a>(
    font_ref: &'a BridgeFontRef<'a>,
) -> Box<BridgeLocalizedStrings<'a>> {
    Box::new(BridgeLocalizedStrings {
        localized_strings: font_ref
            .with_font(|f| Some(f.localized_strings(StringId::FAMILY_NAME)))
            .unwrap_or_default(),
    })
}

pub fn localized_name_next(
    bridge_localized_strings: &mut BridgeLocalizedStrings,
    out_localized_name: &mut BridgeLocalizedName,
) -> bool {
    match bridge_localized_strings.localized_strings.next() {
        Some(localized_string) => {
            out_localized_name.string = localized_string.to_string();
            // TODO(b/307906051): Remove the suffix before shipping.
            out_localized_name.string.push_str(" (Fontations)");
            out_localized_name.language = localized_string
                .language()
                .map(|l| l.to_string())
                .unwrap_or_default();
            true
        }
        _ => false,
    }
}

pub fn english_or_first_font_name(font_ref: &BridgeFontRef, name_id: StringId) -> Option<String> {
    font_ref.with_font(|f| {
        f.localized_strings(name_id)
            .english_or_first()
            .map(|localized_string| localized_string.to_string())
    })
}

pub fn family_name(font_ref: &BridgeFontRef) -> String {
    font_ref
        .with_font(|f| {
            // https://learn.microsoft.com/en-us/typography/opentype/spec/os2#fsselection
            // Bit 8 of the `fsSelection' field in the `OS/2' table indicates a WWS-only font face.
            // When this bit is set it means *do not* use the WWS strings.
            let use_wws = !f
                .os2()
                .map(|t| t.fs_selection().contains(SelectionFlags::WWS))
                .unwrap_or_default();
            use_wws
                .then(|| english_or_first_font_name(font_ref, StringId::WWS_FAMILY_NAME))
                .flatten()
                .or_else(|| english_or_first_font_name(font_ref, StringId::TYPOGRAPHIC_FAMILY_NAME))
                .or_else(|| english_or_first_font_name(font_ref, StringId::FAMILY_NAME))
        })
        .unwrap_or_default()
}

pub fn postscript_name(font_ref: &BridgeFontRef, out_string: &mut String) -> bool {
    let postscript_name = english_or_first_font_name(font_ref, StringId::POSTSCRIPT_NAME);
    match postscript_name {
        Some(name) => {
            *out_string = name;
            true
        }
        _ => false,
    }
}
