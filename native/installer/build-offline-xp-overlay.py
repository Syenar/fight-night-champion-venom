from argparse import ArgumentParser
from hashlib import sha256
from pathlib import Path
import sys


SOURCE_SHA256 = "2a464fadcf5e4a07de5a0eba2c16892b47656bacf96856b01323023057443330"
PATCH_OFFSET = 0x2460
ORIGINAL = bytes.fromhex("B9 01 AF 0E B9 01 B5 02 AE 15 AF 16 AF 17 A2 18 52 5A B9 02 B2 6C")


def main():
    parser = ArgumentParser(description="Build the native TU2 Legacy offline XP Boosts overlay.")
    parser.add_argument("tools_dir", type=Path, help="Reborn tools/display_tab directory")
    parser.add_argument("source", type=Path, help="Stock TU2 boosts.big")
    parser.add_argument("destination", type=Path, help="New private boosts.big")
    args = parser.parse_args()

    sys.path.insert(0, str(args.tools_dir.resolve()))
    import aptpatch_display as pack

    original_archive = args.source.read_bytes()
    if sha256(original_archive).hexdigest() != SOURCE_SHA256:
        raise ValueError("Unrecognized game archive. Select the exact stock TU2 boosts.big.")
    members, footer = pack.bigf_read(original_archive)
    if members[0][0] != "0":
        raise ValueError("Unexpected APT member order")

    apt = bytearray(pack.unrefpack(members[0][1]))
    end = PATCH_OFFSET + len(ORIGINAL)
    if apt[PATCH_OFFSET:end] != ORIGINAL:
        raise ValueError("Unexpected Boosts.animateIn instructions")

    # Call this.initSubScreenAfterLogin() directly. Fill the remaining bytes
    # with balanced pushes and pops to retain the original function boundaries.
    replacement = bytes.fromhex("59 B9 01 B2 0E B5 00 17") + bytes.fromhex("59 17") * 7
    if len(replacement) != len(ORIGINAL):
        raise ValueError("Unexpected replacement length")
    apt[PATCH_OFFSET:end] = replacement

    compressed = pack.refpack(bytes(apt))
    if len(compressed) >= len(apt):
        raise ValueError("APT member must fit the game's in-place decompression buffer")
    members[0][1] = compressed
    result = pack.bigf_write(members, footer)
    if pack.bigf_read(result) != (members, footer):
        raise ValueError("Archive round-trip failed")

    with args.destination.open("xb") as output:
        output.write(result)
    print(sha256(result).hexdigest(), args.destination)


if __name__ == "__main__":
    main()

