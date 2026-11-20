"""Read and stage files from a local Xbox XDVDFS disc image."""

import os
from pathlib import Path
import shutil
import struct
from tempfile import TemporaryDirectory
from uuid import uuid4

from save_upgrade import check_path


SECTOR = 2048
MAGIC = b"MICROSOFT*XBOX*MEDIA"
PARTITIONS = (0, 0xFD90000, 0x18300000, 0x2080000)
ENTRY = struct.Struct("<HHIIBB")
BUFFER = 1024 * 1024
MAX_TABLE = 16 * BUFFER
MAX_ENTRIES = 100_000


def component(raw):
    name = raw.decode("utf-8")
    if (not name or name[-1] in " ." or Path(name).is_reserved()
            or any(ord(char) < 32 or char in '<>:"/\\|?*' for char in name)):
        raise ValueError("Invalid Windows filename in the disc image.")
    return name


class XboxDisc:
    def __init__(self, source):
        self.source = source
        self.size = os.fstat(source.fileno()).st_size
        self.base = 0
        for base in PARTITIONS:
            offset = base + 32 * SECTOR
            if offset + SECTOR <= self.size:
                volume = self.read(offset, SECTOR)
                if volume.startswith(MAGIC) and volume.endswith(MAGIC):
                    self.base = base
                    sector, size = struct.unpack_from("<II", volume, len(MAGIC))
                    self.files = {}
                    self.paths = set()
                    self.collect(sector, size, Path(), 0)
                    if not self.files or sum(size for _, size in self.files.values()) > self.size:
                        raise ValueError("Disc image file inventory is invalid.")
                    return
        raise ValueError("This .iso does not contain a supported Xbox disc partition.")

    def read(self, offset, size):
        if offset + size > self.size:
            raise ValueError("File data extends beyond the Xbox disc image.")
        self.source.seek(offset)
        data = self.source.read(size)
        if len(data) != size:
            raise ValueError("The Xbox disc image ended unexpectedly.")
        return data

    def collect(self, sector, size, directory, depth):
        if depth > 32 or size > MAX_TABLE:
            raise ValueError("Xbox disc directory exceeds supported limits.")
        if not size:
            return
        table = self.read(self.base + sector * SECTOR, size)
        pending = [0]
        seen = set()
        while pending:
            position = pending.pop()
            if position in seen or position + ENTRY.size > size:
                raise ValueError("Invalid directory link in the Xbox disc image.")
            seen.add(position)
            left, right, sector, length, attributes, name_size = ENTRY.unpack_from(table, position)
            name_start = position + ENTRY.size
            if name_start + name_size > size:
                raise ValueError("Truncated directory entry in the Xbox disc image.")
            name = component(table[name_start:name_start + name_size])
            target = directory / name
            key = target.as_posix().casefold()
            if key in self.paths or len(self.paths) >= MAX_ENTRIES:
                raise ValueError("Duplicate or excessive entries in the Xbox disc image.")
            self.paths.add(key)
            pending.extend(child * 4 for child in (left, right) if child not in (0, 0xFFFF))
            if attributes & 0x10:
                self.collect(sector, length, target, depth + 1)
            else:
                offset = self.base + sector * SECTOR
                if offset + length > self.size:
                    raise ValueError("File data extends beyond the Xbox disc image.")
                self.files[target] = (offset, length)

    def chunks(self, relative):
        offset, left = self.files[relative]
        while left:
            data = self.read(offset, min(left, BUFFER))
            yield data
            offset += len(data)
            left -= len(data)

    def extract(self, directory):
        needed = sum(size for _, size in self.files.values())
        if shutil.disk_usage(directory).free < needed:
            raise ValueError("There is not enough free space to extract the Xbox disc image.")
        for relative in self.files:
            destination = directory / relative
            check_path(destination)
            destination.parent.mkdir(parents=True, exist_ok=True)
            with destination.open("xb") as output:
                for block in self.chunks(relative):
                    output.write(block)


def stage_disc(disc, cache):
    check_path(cache)
    cache.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix=".disc-import-", dir=cache) as temporary:
        staging = Path(temporary) / "game"
        staging.mkdir()
        disc.extract(staging)
        destination = cache / ("disc-" + uuid4().hex)
        staging.rename(destination)
    return destination