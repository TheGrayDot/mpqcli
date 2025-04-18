import pytest
from pathlib import Path


@pytest.fixture(scope="session")
def binary_path():
    script_dir = Path(__file__).parent
    binary = script_dir.parent / "build" / "bin" / "mpqcli"

    if not binary.exists():
        pytest.fail(f"Binary not found at {binary}")

    return binary


@pytest.fixture(scope="function")
def test_files():
    script_dir = Path(__file__).parent

    data_dir = script_dir / "data"
    data_dir.mkdir(parents=True, exist_ok=True)

    files_dir = data_dir / "files"
    files_dir.mkdir(parents=True, exist_ok=True)

    files = {
        "cats.txt": "This is a file about cats.\n",
        "dogs.txt": "This is a file about dogs.\n",
    }

    created_files = []
    for filename, content in files.items():
        file_path = files_dir / filename
        file_path.write_text(content)
        created_files.append(file_path)

    yield created_files
