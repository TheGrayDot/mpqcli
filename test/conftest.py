import platform
from pathlib import Path
import urllib.request

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
def generate_test_files():
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
            if platform.system() == "Windows":
                file_path.write_text(content, newline="\r\n")
            else:
                file_path.write_text(content, newline="\n")
        if isinstance(content, bytes):
            file_path.write_bytes(content)
        created_files.append(file_path)

    yield created_files


@pytest.fixture(scope="session")
def download_test_files():
    script_dir = Path(__file__).parent

    data_dir = script_dir / "data"
    data_dir.mkdir(parents=True, exist_ok=True)

    file_urls = {
        "v1_weak_signature_patch.mpq": (
            "https://archive.org/download/World_of_Warcraft_Client_and_Installation_Archive/"
            "Patches/1.x/WoW-1.10.0-to-1.10.1-enGB-patch.zip/wow-patch.mpq"
        ),
        "v2_strong_signature_patch.mpq": (
            "https://archive.org/download/World_of_Warcraft_Client_and_Installation_Archive/"
            "Patches/3.x/wow-3.2.2-to-3.3.0-enGB-Win-patch.zip/"
            "wow-3.2.2-to-3.3.0-enGB-Win-patch%2Fwow-final.MPQ"
        )
    }

    downloaded_files = []

    for name, url in file_urls.items():
        file_path = data_dir / name
        if file_path.exists():
            downloaded_files.append(file_path)
            continue

        try:
            urllib.request.urlretrieve(url, file_path)
            downloaded_files.append(file_path)
        except Exception:
            exit(1)

    return downloaded_files
