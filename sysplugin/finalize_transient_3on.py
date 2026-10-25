"""Turn a self-contained .3nx into a .3on after checking its references."""

import hashlib
import os
from pathlib import Path
import re
import struct
import sys
from tempfile import NamedTemporaryFile


HEADER_SIZE = 0x30
TRANSIENT_MAGIC = 0x26584E33
PLUGIN_MAGICS = (0x24584E33, 0x25584E33)


def align(value, size):
    return (value + size - 1) & ~(size - 1)


def check(data, expected_id, magics, require_self_group):
    if len(data) < HEADER_SIZE:
        raise ValueError("truncated plugin header")

    magic, pid, code_size, data_size, bss_size, reloc_size, repair_size, abi, env, metadata_size = struct.unpack_from(
        "<7IQQI", data
    )
    if magic not in magics or data[4:8] != expected_id.encode("ascii") or not code_size or env:
        raise ValueError("wrong magic/ID, empty code, or nonzero expected environment")

    raw_end = HEADER_SIZE + reloc_size + code_size + data_size + repair_size
    if metadata_size & 15 or align(raw_end, 16) + metadata_size != len(data):
        raise ValueError("plugin length or metadata is malformed")

    if not reloc_size:
        if require_self_group:
            raise ValueError("missing self-relocation group")
    else:
        if reloc_size < 8 or (reloc_size - 8) % 8:
            raise ValueError("malformed self-relocation group")
        provider, count = struct.unpack_from("<II", data, HEADER_SIZE)
        if provider != pid or 8 + count * 8 != reloc_size:
            raise ValueError("external or malformed relocation group")
        runtime_size = align(code_size, 0x1000) + align(data_size + bss_size, 0x1000)
        for i in range(count):
            patch, target = struct.unpack_from("<II", data, HEADER_SIZE + 8 + 8 * i)
            if patch & 3 or patch + 4 > runtime_size or target >= runtime_size:
                raise ValueError(f"invalid self-relocation #{i}")

    repair_offset = HEADER_SIZE + reloc_size + code_size + data_size
    if repair_size < 4:
        raise ValueError("missing repair export count")
    export_count = struct.unpack_from("<I", data, repair_offset)[0]
    if repair_size != 4 + export_count * 12:
        raise ValueError("HOST/plugin repair group or malformed export table")
    exports = data[repair_offset + 4:repair_offset + repair_size]
    payload = b"NEXUS_3NX_ABI\0" + struct.pack("<II", magic, pid) + exports
    expected_abi = int.from_bytes(hashlib.sha256(payload).digest()[:8], "little") & ~1
    if abi != expected_abi:
        raise ValueError("ABI hash does not match the export table")
    return pid, reloc_size, raw_end, metadata_size, exports


def finalize(source, expected_id):
    if re.fullmatch(r"[A-Za-z0-9_]{4}", expected_id) is None:
        raise ValueError("ID must be exactly four ASCII letters, digits, or underscores")
    source = Path(source)
    match = re.fullmatch(r"(.+)\.([0-9]+)\.3nx", source.name)
    if match is None:
        raise ValueError("expected a name.priority.3nx input")
    target = source.with_name(match.group(1) + ".3on")
    if target.exists():
        raise ValueError(f"output already exists: {target}")

    original = source.read_bytes()
    pid, reloc_size, raw_end, metadata_size, exports = check(original, expected_id, PLUGIN_MAGICS, False)
    self_group = b"" if reloc_size else struct.pack("<II", pid, 0)
    result = bytearray(original[:raw_end])
    if self_group:
        result[HEADER_SIZE:HEADER_SIZE] = self_group
    struct.pack_into("<I", result, 0, TRANSIENT_MAGIC)
    struct.pack_into("<I", result, 0x14, reloc_size + len(self_group))
    payload = b"NEXUS_3NX_ABI\0" + struct.pack("<II", TRANSIENT_MAGIC, pid) + exports
    abi = int.from_bytes(hashlib.sha256(payload).digest()[:8], "little") & ~1
    struct.pack_into("<Q", result, 0x1C, abi)
    result += b"\0" * (align(len(result), 16) - len(result))
    if metadata_size:
        result += original[align(raw_end, 16):]
    check(result, expected_id, (TRANSIENT_MAGIC,), True)

    temporary = None
    try:
        with NamedTemporaryFile(dir=source.parent, prefix=f".{target.name}.", delete=False) as handle:
            temporary = Path(handle.name)
            handle.write(result)
        os.replace(temporary, target)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    source.unlink()
    print(f"Finalized {target} ({expected_id})")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: finalize_transient_3on.py NAME.priority.3nx ID")
    try:
        finalize(sys.argv[1], sys.argv[2])
    except (OSError, struct.error, ValueError) as exc:
        raise SystemExit(f"ERROR: {sys.argv[1]}: {exc}") from exc
