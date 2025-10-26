import subprocess
from pathlib import Path
import platform


def test_list_mpq_with_output_v1(binary_path):
    """
    Test MPQ file listing with no parameters.

    This test checks:
    - That running the list command with no parameters renders a list of the files inside, with no details.
    """
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
    }

    result = subprocess.run(
        [str(binary_path), "list", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_list_mpq_with_standard_details(binary_path):
    """
    Test MPQ file listing with the standard details.

    This test checks:
    - That the standard long listing parameters work as expected.
    """
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "      27 enUS                      (listfile)",
        "     148 enUS                      (attributes)",
        "      27 enUS 2025-07-29 14:31:00  dogs.txt",
        "       8 enUS 2025-07-29 14:31:00  bytes",
        "      27 enUS 2025-07-29 14:31:00  cats.txt",
    }

    # Adjust filesize for Windows
    if platform.system() == "Windows":
        expected_output.remove("      27 enUS 2025-07-29 14:31:00  dogs.txt")
        expected_output.add("      28 enUS 2025-07-29 14:31:00  dogs.txt")
        expected_output.remove("      27 enUS 2025-07-29 14:31:00  cats.txt")
        expected_output.add("      28 enUS 2025-07-29 14:31:00  cats.txt")

    result = subprocess.run(
        [str(binary_path), "list", "-a", "-d", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_list_mpq_with_specified_details(binary_path):
    """
    Test MPQ file listing with specified details.

    This test checks:
    - That providing parameters to be listed works as expected.
    """
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "   44 0fd58937 70ab788e 0000000000000000       35    cexmn 935a7772  cats.txt",
        "    0 eb30456b 48345fbb 0000000000000000       35    cexmn a073c614  dogs.txt",
        "   35 147178ed c99b9ee2 0000000000000000       16    cexmn eaa753f9  bytes",
        "   25 fd657910 4e9b98a7 0000000000000000       35  ce2xmnf 2d2f0a94  (listfile)",
        "   14 d38437cb 07dfeaec 0000000000000000      123  ce2xmnf 50e314af  (attributes)",
    }
    # Adjust filesize for Windows
    if platform.system() == "Windows":
        expected_output.remove("   44 0fd58937 70ab788e 0000000000000000       35    cexmn 935a7772  cats.txt")
        expected_output.add("   44 0fd58937 70ab788e 0000000000000000       36    cexmn 935a7772  cats.txt")
        expected_output.remove("    0 eb30456b 48345fbb 0000000000000000       35    cexmn a073c614  dogs.txt")
        expected_output.add("    0 eb30456b 48345fbb 0000000000000000       36    cexmn a073c614  dogs.txt")

    result = subprocess.run(
        [str(binary_path), "list", "-a", "-d", str(test_file),
         "-p", "hash-index", "-p", "name-hash1", "-p", "name-hash2", "-p", "name-hash3",
         "-p", "compressed-size", "-p", "flags", "-p", "encryption-key-raw"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_list_mpq_with_specified_details_but_no_detailed_flag(binary_path):
    """
    Test MPQ file listing.

    This test checks:
    - That providing parameters to be listed does nothing if the -d/--detailed flag is not provided.
    """
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
        "(listfile)",
        "(attributes)",
    }

    result = subprocess.run(
        [str(binary_path), "list", "-a", str(test_file),
         "-p", "hash-index", "-p", "flags", "-p", "encryption-key-raw", "-p", "encryption-key"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_list_mpq_with_weak_signature(binary_path):
    """
    Test MPQ file listing of MPQ with weak signature.

    This test checks:
    - That handling MPQs with weak signatures generates the expected output.
    """
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_weak_signature.mpq"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
        "(listfile)",  # Only included as "-a" specified
        "(signature)",  # Only included as "-a" specified
        "(attributes)",  # Only included as "-a" specified
    }

    result = subprocess.run(
        [str(binary_path), "list", "-a", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
