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


def test_verify_weak_signature_print(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_weak_signature.mpq"

    expected_output = {
        "[+] Verify success",
        "[+] Signature content:",
        "[+] Weak signature file size: 72",
        (
            "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\xad\\xec\\x2d\\xe5\\xfb\\x0f\\x58\\x28"
            "\\x35\\x01\\xec\\x59\\x90\\xee\\x36\\x41\\x04\\xa3\\x99\\xc0\\x84\\xe8\\xff\\x11"
            "\\x23\\xb8\\xe4\\xc9\\xc8\\x11\\xcc\\xd5\\xb8\\x4c\\xac\\x45\\xde\\x0f\\x5a\\x32"
            "\\x56\\xdf\\x89\\x08\\x0e\\xd4\\x78\\xfa\\xc6\\x9f\\x76\\xe2\\xea\\x03\\x86\\xcf"
            "\\xbd\\xf7\\xbd\\xb6\\x23\\x6e\\x7d\\x85"
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
