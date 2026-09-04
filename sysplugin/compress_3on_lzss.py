from collections import defaultdict, deque
from pathlib import Path
import struct
import sys

TRANSIENT_MAGIC = 0x26584E33
HEADER_SIZE = 0x30
VERSION_MAGIC = b"3NXO"
COMPRESSED_MAGIC = b"3NXO"
LZ10_TYPE = 0x10
WINDOW = 4096
MIN_MATCH = 3
MAX_MATCH = 18


def align16(value):
    return (value + 0xF) & ~0xF


def read_version(data):
    if len(data) < HEADER_SIZE:
        raise SystemExit("ERROR: transient has no modern metadata header")
    fields = struct.unpack_from("<12I", data, 0)
    magic, _pid, code_size, data_size, _bss_size, reloc_size, repair_size = fields[:7]
    metadata_size = fields[11]
    if magic != TRANSIENT_MAGIC:
        raise SystemExit("ERROR: input is not a 3NX& transient")
    raw_end = HEADER_SIZE + reloc_size + code_size + data_size + repair_size
    metadata_offset = align16(raw_end)
    if metadata_size < 8 or metadata_offset + metadata_size != len(data):
        raise SystemExit("ERROR: transient metadata is missing or malformed")
    if data[metadata_offset:metadata_offset + 4] != VERSION_MAGIC:
        raise SystemExit("ERROR: transient metadata does not begin with 3NXO")
    return struct.unpack_from("<I", data, metadata_offset + 4)[0]


def compress_lz10(data):
    if len(data) > 0xFFFFFF:
        raise SystemExit("ERROR: LZ10 payload exceeds 24-bit size field")

    positions = defaultdict(deque)
    stream = bytearray()
    index = 0

    while index < len(data):
        flag_offset = len(stream)
        stream.append(0)
        flags = 0

        for slot in range(8):
            if index >= len(data):
                break

            best_length = 0
            best_distance = 0
            if index + MIN_MATCH <= len(data):
                key = data[index:index + MIN_MATCH]
                candidates = positions[key]
                while candidates and index - candidates[0] > WINDOW:
                    candidates.popleft()

                for position in reversed(list(candidates)[-64:]):
                    distance = index - position
                    maximum = min(MAX_MATCH, len(data) - index)
                    length = MIN_MATCH
                    while length < maximum and data[position + length] == data[index + length]:
                        length += 1
                    if length > best_length:
                        best_length = length
                        best_distance = distance
                        if length == maximum:
                            break

            if best_length >= MIN_MATCH:
                flags |= 0x80 >> slot
                displacement = best_distance - 1
                stream.append(((best_length - MIN_MATCH) << 4) | ((displacement >> 8) & 0x0F))
                stream.append(displacement & 0xFF)
                consumed = best_length
            else:
                stream.append(data[index])
                consumed = 1

            for step in range(consumed):
                position = index + step
                if position + MIN_MATCH <= len(data):
                    key = data[position:position + MIN_MATCH]
                    candidates = positions[key]
                    candidates.append(position)
                    while candidates and position - candidates[0] > WINDOW:
                        candidates.popleft()
            index += consumed

        stream[flag_offset] = flags

    size = len(data)
    return bytes((LZ10_TYPE, size & 0xFF, (size >> 8) & 0xFF, (size >> 16) & 0xFF)) + bytes(stream)


def decompress_lz10(blob):
    if len(blob) < 4 or blob[0] != LZ10_TYPE:
        raise SystemExit("ERROR: bad LZ10 header")
    expected_size = blob[1] | (blob[2] << 8) | (blob[3] << 16)
    cursor = 4
    output = bytearray()

    while len(output) < expected_size:
        if cursor >= len(blob):
            raise SystemExit("ERROR: compressed stream ended early")
        flags = blob[cursor]
        cursor += 1

        for slot in range(8):
            if len(output) >= expected_size:
                break
            if flags & (0x80 >> slot):
                if cursor + 2 > len(blob):
                    raise SystemExit("ERROR: truncated LZ10 match token")
                first = blob[cursor]
                second = blob[cursor + 1]
                cursor += 2
                length = (first >> 4) + MIN_MATCH
                displacement = ((first & 0x0F) << 8) | second
                distance = displacement + 1
                if distance > len(output):
                    raise SystemExit("ERROR: invalid LZ10 displacement")
                for _ in range(length):
                    if len(output) >= expected_size:
                        break
                    output.append(output[-distance])
            else:
                if cursor >= len(blob):
                    raise SystemExit("ERROR: truncated LZ10 literal")
                output.append(blob[cursor])
                cursor += 1

    if cursor != len(blob):
        raise SystemExit("ERROR: compressed LZ10 stream has trailing data")
    return bytes(output)


def main(source, destination):
    data = Path(source).read_bytes()
    version = read_version(data)
    compressed_blob = compress_lz10(data)
    if decompress_lz10(compressed_blob) != data:
        raise SystemExit("ERROR: LZ10 round-trip validation failed")

    wrapper = COMPRESSED_MAGIC + struct.pack("<II", version, len(compressed_blob))
    Path(destination).write_bytes(wrapper + compressed_blob)
    print(
        f"Wrote {destination} ({len(data)} -> {len(compressed_blob)} bytes LZ10, "
        f"3NXO version {version})"
    )


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: compress_3on_lzss.py SOURCE.3on DESTINATION.lz")
    main(sys.argv[1], sys.argv[2])
