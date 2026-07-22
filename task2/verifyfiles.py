import os
import glob
from pathlib import Path
from tiktoken import Encoding
from tiktoken.load import data_gym_to_mergeable_bpe_ranks
from tiktoken_ext.openai_public import ENDOFTEXT, r50k_pat_str
from tokenizer_wrapper import PyTokenizer


def load_local_gpt2_encoding(model_dir: Path) -> Encoding:
    """Build the GPT-2 encoding from this project's local tokenizer files."""
    mergeable_ranks = data_gym_to_mergeable_bpe_ranks(
        str(model_dir / "merges.txt"),
        str(model_dir / "vocab.json"),
    )
    return Encoding(
        name="local-gpt2",
        pat_str=r50k_pat_str,
        mergeable_ranks=mergeable_ranks,
        special_tokens={ENDOFTEXT: 50256},
    )

def main():
    project_root = Path(__file__).resolve().parent
    model_dir = project_root / "data" / "tokenizer"
    data_dir = project_root / "data" / "samples"

    # 1. Initialize custom C tokenizer via Cython and Tiktoken (gpt-2 encoding)
    tokenizer = PyTokenizer(str(model_dir / "vocab.json"), str(model_dir / "merges.txt"))
    tik_enc = load_local_gpt2_encoding(model_dir)

    # 2. Collect up to 20 text files in data directory
    txt_files = sorted(glob.glob(str(data_dir / "*.txt")))[:20]

    if not txt_files:
        print(f"No .txt files found in folder '{data_dir}'. Creating fallback files...")
        data_dir.mkdir(parents=True, exist_ok=True)
        for i in range(1, 21):
            with open(data_dir / f"sample_{i}.txt", "w", encoding="utf-8") as f:
                f.write(f"Sample test document number {i}\nHello world! Testing Cython verification.")
        txt_files = sorted(glob.glob(str(data_dir / "*.txt")))[:20]

    print(f"\n--- Running Verification across {len(txt_files)} files ---\n")

    for idx, filepath in enumerate(txt_files, 1):
        with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()

        # --- ENCODER TEST ---
        c_tokens = tokenizer.encode(text)
        tik_tokens = tik_enc.encode(text)

        enc_diff = len(c_tokens) - len(tik_tokens)
        enc_match = (c_tokens == tik_tokens)
        enc_status = "1 passed" if enc_match else f"FAILED (diff: {enc_diff})"

        # --- DECODER TEST ---
        c_decoded = tokenizer.decode(c_tokens)
        tik_decoded = tik_enc.decode(tik_tokens)

        dec_match = (c_decoded == text)
        dec_status = "1 passed" if dec_match else "FAILED"

        print(f"[{idx:02d}/20] File: {os.path.basename(filepath)}")
        print(f"  └── Encoder Status: {enc_status}")
        print(f"  └── Decoder Status: {dec_status}")

if __name__ == "__main__":
    main()