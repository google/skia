// Copyright 2026 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! General-purpose TIFF Image File Directory (IFD) parser.
//!
//! Used for MPF (Multi-Picture Format) and other TIFF-based structures
//! embedded in JPEG segments.

/// Parse the TIFF header at the start of `data`, returning
/// `(little_endian, ifd_offset)`.
pub fn parse_header(data: &[u8]) -> Option<(bool, u32)> {
    if data.len() < 8 {
        return None;
    }
    let little_endian = match (data[0], data[1]) {
        (b'I', b'I') => true,
        (b'M', b'M') => false,
        _ => return None,
    };
    let magic = read_u16(data, 2, little_endian)?;
    if magic != 42 {
        return None;
    }
    let ifd_offset = read_u32(data, 4, little_endian)?;
    Some((little_endian, ifd_offset))
}

pub fn read_u16(data: &[u8], off: usize, little_endian: bool) -> Option<u16> {
    if off.checked_add(2)? > data.len() {
        return None;
    }
    Some(if little_endian {
        u16::from_le_bytes([data[off], data[off + 1]])
    } else {
        u16::from_be_bytes([data[off], data[off + 1]])
    })
}

pub fn read_u32(data: &[u8], off: usize, little_endian: bool) -> Option<u32> {
    if off.checked_add(4)? > data.len() {
        return None;
    }
    Some(if little_endian {
        u32::from_le_bytes([data[off], data[off + 1], data[off + 2], data[off + 3]])
    } else {
        u32::from_be_bytes([data[off], data[off + 1], data[off + 2], data[off + 3]])
    })
}

/// A parsed TIFF IFD entry.
pub struct IfdEntry {
    pub tag: u16,
    pub entry_type: u16,
    pub count: u32,
    /// Position of the 4-byte value/offset field within the data.
    pub value_offset_pos: usize,
}

// TIFF type constants.
#[allow(dead_code)]
pub const TYPE_UNSIGNED_SHORT: u16 = 3;
pub const TYPE_UNSIGNED_LONG: u16 = 4;
#[allow(dead_code)]
pub const TYPE_UNDEFINED: u16 = 7;

/// Size in bytes of a single element of the given TIFF type.
fn type_size(entry_type: u16) -> Option<usize> {
    match entry_type {
        1 | 2 | 6 | 7 => Some(1), // BYTE, ASCII, SBYTE, UNDEFINED
        3 | 8 => Some(2),         // SHORT, SSHORT
        4 | 9 => Some(4),         // LONG, SLONG
        5 | 10 => Some(8),        // RATIONAL, SRATIONAL
        11 => Some(4),            // FLOAT
        12 => Some(8),            // DOUBLE
        _ => None,
    }
}

/// Parse the IFD at `ifd_offset` within `data`, returning the list of
/// entries.  `data` starts at the TIFF header (endian bytes).
pub fn parse_ifd(data: &[u8], ifd_offset: u32, little_endian: bool) -> Option<Vec<IfdEntry>> {
    let off = ifd_offset as usize;
    let num_entries = read_u16(data, off, little_endian)? as usize;
    let entries_start = off.checked_add(2)?;
    let entries_size = num_entries.checked_mul(12)?;
    if entries_start.checked_add(entries_size)? > data.len() {
        return None;
    }
    let mut entries = Vec::with_capacity(num_entries);
    for i in 0..num_entries {
        let entry_off = entries_start.checked_add(i.checked_mul(12)?)?;
        entries.push(IfdEntry {
            tag: read_u16(data, entry_off, little_endian)?,
            entry_type: read_u16(data, entry_off + 2, little_endian)?,
            count: read_u32(data, entry_off + 4, little_endian)?,
            value_offset_pos: entry_off + 8,
        });
    }
    Some(entries)
}

/// Read the raw bytes for an IFD entry.
///
/// If the total data fits in 4 bytes it is stored inline at
/// `value_offset_pos`; otherwise `value_offset_pos` contains an
/// offset (relative to `data` start) pointing to the actual data.
pub fn get_entry_data<'a>(
    data: &'a [u8],
    entry: &IfdEntry,
    little_endian: bool,
) -> Option<&'a [u8]> {
    let elem_size = type_size(entry.entry_type)?;
    let total_size = elem_size.checked_mul(entry.count as usize)?;
    if total_size <= 4 {
        let end = entry.value_offset_pos.checked_add(total_size)?;
        if end > data.len() {
            return None;
        }
        Some(&data[entry.value_offset_pos..end])
    } else {
        let offset = read_u32(data, entry.value_offset_pos, little_endian)? as usize;
        let end = offset.checked_add(total_size)?;
        if end > data.len() {
            return None;
        }
        Some(&data[offset..end])
    }
}

/// Read a single unsigned-long value from an IFD entry.
pub fn read_entry_unsigned_long(data: &[u8], entry: &IfdEntry, little_endian: bool) -> Option<u32> {
    if entry.entry_type != TYPE_UNSIGNED_LONG || entry.count != 1 {
        return None;
    }
    read_u32(data, entry.value_offset_pos, little_endian)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_header_big_endian() {
        let data = [b'M', b'M', 0x00, 0x2A, 0x00, 0x00, 0x00, 0x08];
        let (le, off) = parse_header(&data).unwrap();
        assert!(!le);
        assert_eq!(off, 8);
    }

    #[test]
    fn test_parse_header_little_endian() {
        let data = [b'I', b'I', 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00];
        let (le, off) = parse_header(&data).unwrap();
        assert!(le);
        assert_eq!(off, 8);
    }

    #[test]
    fn test_parse_header_truncated() {
        assert!(parse_header(&[b'M', b'M']).is_none());
    }

    #[test]
    fn test_parse_header_bad_magic() {
        let data = [b'M', b'M', 0x00, 0x43, 0x00, 0x00, 0x00, 0x08];
        assert!(parse_header(&data).is_none());
    }

    #[test]
    fn test_parse_ifd_basic() {
        let mut data = vec![
            b'M', b'M', 0x00, 0x2A, 0x00, 0x00, 0x00, 0x08, // IFD at offset 8: 1 entry
            0x00, 0x01, // Entry: tag=0x1234, type=LONG(4), count=1, value=42
            0x12, 0x34, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2A,
        ];
        data.extend_from_slice(&[0u8; 4]);

        let entries = parse_ifd(&data, 8, false).unwrap();
        assert_eq!(entries.len(), 1);
        assert_eq!(entries[0].tag, 0x1234);
        assert_eq!(entries[0].entry_type, TYPE_UNSIGNED_LONG);
        assert_eq!(entries[0].count, 1);

        let val = read_entry_unsigned_long(&data, &entries[0], false).unwrap();
        assert_eq!(val, 42);
    }

    #[test]
    fn test_parse_ifd_rejects_truncated_entries() {
        let data = vec![
            b'M', b'M', 0x00, 0x2A, 0x00, 0x00, 0x00, 0x08, // IFD at offset 8: 2 entries
            0x00, 0x02, // Entry count
            0x12, 0x34, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2A,
        ];

        assert!(parse_ifd(&data, 8, false).is_none());
    }
}
