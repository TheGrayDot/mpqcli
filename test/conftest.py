import platform
from pathlib import Path
import shutil

import pytest


@pytest.fixture(scope="session")
def binary_path():
    script_dir = Path(__file__).parent

    if platform.system() == "Windows":
        binary = script_dir.parent / "build" / "bin" / "mpqcli.exe"
    else:
        binary = script_dir.parent / "build" / "bin" / "mpqcli"

    if not binary.exists():
        pytest.fail(f"Binary not found at {binary}")

    return binary


@pytest.fixture(scope="session")
def test_files():
    script_dir = Path(__file__).parent

    data_dir = script_dir / "data"
    data_dir.mkdir(parents=True, exist_ok=True)

    files_dir = data_dir / "files"
    files_dir.mkdir(parents=True, exist_ok=True)

    files = {
        "cats.txt": "This is a file about cats.\n",
        "dogs.txt": "This is a file about dogs.\n",
        "bytes": b"\x00\x01\x02\x03\x04\x05\x06\x07",
    }

    created_files = []
    for filename, content in files.items():
        file_path = files_dir / filename
        if isinstance(content, str):
            file_path.write_text(content, newline="\r\n")
        if isinstance(content, bytes):
            file_path.write_bytes(content)
        created_files.append(file_path)

    yield created_files

    shutil.rmtree(data_dir)
