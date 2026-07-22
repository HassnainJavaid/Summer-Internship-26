import sys
from pathlib import Path
import tiktoken

def verify_full_pipeline(original_text):
    project_root = Path(__file__).resolve().parent.parent
    artifacts_dir = project_root / "artifacts"
    # 1. Test Encoder Output against Tiktoken Ground Truth
    enc = tiktoken.get_encoding("gpt2")
    expected_tokens = enc.encode(original_text)

    try:
        with open(artifacts_dir / "c_tokens.txt", "r") as f:
            c_tokens = [int(line.strip()) for line in f if line.strip()]
    except FileNotFoundError:
        print("❌ Error: c_tokens.txt missing. Run the C binary first.")
        sys.exit(1)

    print("\n=== 1. Encoder Verification ===")
    print(f"Expected (Tiktoken): {expected_tokens}")
    print(f"Actual (C Encoder):  {c_tokens}")
    
    encoder_passed = (expected_tokens == c_tokens)
    if encoder_passed:
        print("✅ Encoder Result: MATCHES HUGGINGFACE/TIKTOKEN PERFECTLY!")
    else:
        print("❌ Encoder Result: MISMATCH ENCOUNTERED!")

    # 2. Test Decoder Output against Original String
    try:
        with open(artifacts_dir / "decoded_output.txt", "r", encoding="utf-8") as f:
            decoded_text = f.read()
    except FileNotFoundError:
        print("❌ Error: decoded_output.txt missing. Verification halted.")
        sys.exit(1)

    print("\n=== 2. Decoder Verification ===")
    print(f"Original String: {repr(original_text)}")
    print(f"Decoded String:  {repr(decoded_text)}")

    decoder_passed = (original_text == decoded_text)
    if decoder_passed:
        print("✅ Decoder Result: MATCHES ORIGINAL STRING PERFECTLY!")
    else:
        print("❌ Decoder Result: MISMATCH ENCOUNTERED!")

    # Final verdict
    if encoder_passed and decoder_passed:
        print("\n🎉 CONGRATULATIONS! Complete text-token round-trip is 100% accurate!")
        sys.exit(0)
    else:
        print("\n🚨 PIPELINE FAILURE: Check the debug outputs listed above.")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 verify.py \"<text_to_test>\"")
        sys.exit(1)
    verify_full_pipeline(sys.argv[1])