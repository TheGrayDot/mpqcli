import subprocess
from pathlib import Path


def test_verify_no_signature(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "[!] Verify failed",
    }

    result = subprocess.run(
        [str(binary_path), "verify", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 1, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_verify_weak_signature(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_weak_signature.mpq"

    expected_output = {
        "[+] Verify success",
    }

    result = subprocess.run(
        [str(binary_path), "verify", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_verify_weak_signature_patch(binary_path, download_test_files):
    _ = download_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "v1_weak_signature_patch.mpq"

    expected_output = {
        "[+] Verify success",
        "[+] Signature content:",
        (
            "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\xb6\\x35\\x12\\x78\\xb3\\x88\\xb2\\x82"
            "\\x7a\\xcf\\x83\\x60\\x98\\x48\\x79\\x68\\x87\\x92\\x9f\\xd7\\x03\\xd6\\xde\\x18"
            "\\xd0\\x17\\x49\\x30\\x12\\x23\\x9a\\xf4\\xa4\\x11\\x82\\x5b\\xe7\\x52\\x4a\\x95"
            "\\xd8\\x7d\\x48\\xad\\xb5\\x30\\xf9\\x13\\x7b\\xff\\x06\\x32\\x6d\\x24\\xc9\\x68"
            "\\x56\\xac\\x20\\x09\\x44\\xc6\\x96\\x69"
        )
    }

    result = subprocess.run(
        [str(binary_path), "verify", "-p", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_verify_strong_signature_patch(binary_path, download_test_files):
    _ = download_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "v2_strong_signature_patch.mpq"

    expected_output = {
        "[+] Verify success",
        "[+] Signature content:",
        (
            "\\x4e\\x47\\x49\\x53\\x49\\x41\\x3f\\x6f\\x29\\x07\\xfc\\xd0\\x72\\x12\\xf2\\x8b"
            "\\x77\\x05\\x26\\x28\\xff\\xdf\\x6f\\x9f\\x13\\x7a\\x80\\x58\\xcf\\x4f\\x0b\\xd8"
            "\\xa5\\x8a\\x24\\x26\\x61\\xdf\\x98\\x39\\x62\\x40\\xde\\x99\\x2e\\xf6\\x6e\\xe8"
            "\\x21\\xbc\\x7b\\xba\\x2e\\xcf\\xb7\\x59\\x8d\\x61\\xf7\\xfb\\x75\\x9d\\x0e\\xe7"
            "\\xc5\\x6d\\x02\\x0f\\x07\\x09\\x2e\\x5b\\x7a\\x4f\\x70\\x22\\x97\\xde\\x62\\xde"
            "\\x26\\x46\\xc9\\x89\\xa1\\xcf\\xa2\\xa9\\x78\\xf6\\x43\\x49\\xd2\\x5d\\x00\\x9c"
            "\\x92\\x14\\x8f\\x16\\xbb\\xe3\\xd4\\xae\\x34\\x41\\x52\\xf9\\xd6\\x48\\xe8\\x49"
            "\\x94\\x92\\x89\\xd7\\xda\\x3c\\x4f\\xa0\\xc0\\xdb\\x36\\x56\\x46\\x0b\\x02\\x52"
            "\\xd0\\xad\\x3b\\x90\\x5f\\x7f\\xc9\\xcd\\xe4\\x55\\x95\\x9f\\x84\\xb9\\xaa\\xbb"
            "\\x2e\\x24\\x29\\x5b\\x1d\\x6c\\xee\\x51\\x1f\\x62\\xf3\\x0a\\x91\\xdb\\x02\\x20"
            "\\xb0\\x6c\\x4b\\x8a\\x14\\x31\\x1f\\xba\\x4a\\x51\\x11\\x30\\xa9\\x2d\\xcb\\xc1"
            "\\x9b\\x37\\xbc\\xa2\\x1a\\x2c\\xbb\\x0a\\x4a\\x6d\\xab\\x5e\\xdb\\xfa\\xb4\\x0e"
            "\\xad\\x9d\\xd2\\xbc\\x1e\\x41\\xa4\\x03\\x4c\\xbe\\x15\\x96\\xa1\\xb1\\x02\\xc7"
            "\\x8a\\xcb\\xdf\\xc3\\xb1\\x4e\\xf9\\x55\\x55\\x71\\xbd\\xd0\\xc9\\xe7\\x82\\x62"
            "\\x06\\x73\\xa7\\xb1\\xb7\\x22\\x42\\xed\\x32\\x75\\x72\\x6c\\x91\\x80\\xfd\\x83"
            "\\xae\\xf8\\x93\\x79\\x84\\x20\\x06\\xda\\xc5\\x53\\x67\\x71\\x7e\\x47\\x94\\xba"
            "\\x24\\x1a\\x11\\xbb"
        )
    }

    result = subprocess.run(
        [str(binary_path), "verify", "-p", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
