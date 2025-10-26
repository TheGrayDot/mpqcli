from datetime import datetime
import os
import platform
from pathlib import Path
import urllib.request
import subprocess

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

    # Hard-coded timestamp: July 29, 2025, 14:31:00
    # This is needed because archive size changes based on the timestamp
    # and we need to have a consistent test output for "detailed" listing
    dt = datetime(2025, 7, 29, 14, 31, 0)
    ts = dt.timestamp()

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
        os.utime(file_path, (ts, ts))
        created_files.append(file_path)

    yield created_files


@pytest.fixture(scope="session")
def generate_locales_mpq_test_files(binary_path):
    script_dir = Path(__file__).parent

    data_dir = script_dir / "data"
    data_dir.mkdir(parents=True, exist_ok=True)

    locales_files_dir = data_dir / "locale_files"
    locales_files_dir.mkdir(parents=True, exist_ok=True)

    mpq_many_locales_file_name = data_dir / "many_locales.mpq"
    mpq_one_locale_file_name = data_dir / "one_locale.mpq"
    text_file_name = "cats.txt"

    locale_files = {
        "": "This is a file about cats.", # Default locale
        "deDE": "Dies ist eine Datei über Katzen.",
        "esES": "Este es un archivo sobre gatos.",
    }

    # Put all items into mpq_many_locales_file_name with their locale
    created_files = []
    for locale, content in locale_files.items():
        file_path = locales_files_dir / text_file_name
        file_path.write_text(content, newline="\n")
        created_files.append(file_path)

        if locale == "": # Default locale - create a new MPQ file
            result = subprocess.run(
                [str(binary_path), "create", "-v", "1", "-o", str(mpq_many_locales_file_name), str(file_path)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

        else: # Explicit locale - add to existing MPQ file
            result = subprocess.run(
                [str(binary_path), "add", str(file_path), str(mpq_many_locales_file_name), "--locale", locale],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    # Put the last item into mpq_one_locale_file_name with its locale
    locale, content = list(locale_files.items())[-1]
    file_path = locales_files_dir / text_file_name
    file_path.write_text(content, newline="\n")
    created_files.append(file_path)

    result = subprocess.run(
        [str(binary_path), "create", "-v", "1", "-o", str(mpq_one_locale_file_name), str(file_path), "--locale", locale],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

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
