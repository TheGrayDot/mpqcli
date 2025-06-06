from pathlib import Path
import platform
import subprocess


def test_info_v1(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "Format version: 1",
        "Header offset: 0",
        "Header size: 32",
        "Archive size: 345",
        "File count: 4",
        "Signature type: None",
    }

    # Adjust the expected output for Windows due to different line endings
    if platform.system() == "Windows":
        expected_output.remove("Archive size: 345")
        expected_output.add("Archive size: 347")

    result = subprocess.run(
        [str(binary_path), "info", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_info_v2(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v2.mpq"

    expected_output = {
        "Format version: 2",
        "Header offset: 0",
        "Header size: 44",
        "Archive size: 349",
        "File count: 4",
        "Signature type: None",
    }

    # Adjust the expected output for Windows due to different line endings
    if platform.system() == "Windows":
        expected_output.remove("Archive size: 349")
        expected_output.add("Archive size: 351")

    result = subprocess.run(
        [str(binary_path), "info", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_info_v1_properties(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    test_cases = [
        ("format-version", "1"),
        ("header-offset", "0"),
        ("header-size", "32"),
        ("archive-size", "345"),
        ("file-count", "4"),
        ("signature-type", "None"),
    ]

    # Adjust archive-size for Windows
    if platform.system() == "Windows":
        test_cases = [
            (k, "347") if k == "archive-size" else (k, v)
            for (k, v) in test_cases
        ]

    for property_name, expected_output in test_cases:
        result = subprocess.run(
            [str(binary_path), "info", "-p", property_name, str(test_file)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
        assert result.stdout.strip() == expected_output, f"Unexpected output for {property_name}: {result.stdout}"


def test_info_v2_properties(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v2.mpq"

    test_cases = [
        ("format-version", "2"),
        ("header-offset", "0"),
        ("header-size", "44"),
        ("archive-size", "349"),
        ("file-count", "4"),
        ("signature-type", "None"),
    ]

    # Adjust archive-size for Windows
    if platform.system() == "Windows":
        test_cases = [
            (k, "351") if k == "archive-size" else (k, v)
            for (k, v) in test_cases
        ]

    for property_name, expected_output in test_cases:
        result = subprocess.run(
            [str(binary_path), "info", "-p", property_name, str(test_file)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
        assert result.stdout.strip() == expected_output, f"Unexpected output for {property_name}: {result.stdout}"


def test_info_v1_invalid_property(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    property_name = "not-real"

    result = subprocess.run(
        [str(binary_path), "info", "-p", property_name, str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"
