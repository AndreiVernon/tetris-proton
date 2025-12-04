import wave
import struct
import os
import argparse

def wav_to_header(wav_path, header_path):
    with wave.open(wav_path, "rb") as wf:
        num_channels = wf.getnchannels()
        sampwidth = wf.getsampwidth()
        sample_rate = wf.getframerate()
        num_frames = wf.getnframes()
        raw_data = wf.readframes(num_frames)

    if sampwidth == 1:
        c_type = "uint8_t"
        fmt = "<{}B".format(num_frames * num_channels)
        samples = list(struct.unpack(fmt, raw_data))

    elif sampwidth == 2:
        c_type = "uint16_t"
        fmt = "<{}h".format(num_frames * num_channels)
        samples_signed = struct.unpack(fmt, raw_data)
        samples = [s + 32768 for s in samples_signed]

    else:
        raise ValueError("Only 8-bit or 16-bit PCM WAV files are supported.")

    base_name = os.path.basename(wav_path)
    sanitized = os.path.splitext(base_name)[0].replace(" ", "_")
    symbol_name = sanitized.upper()

    with open(header_path, "w") as out:
        out.write(f"/*    File {base_name}\n")
        out.write(f" *    Sample rate {sample_rate} Hz\n")
        out.write(" */\n")
        out.write("#include <stdint.h>\n")
        out.write(f"#define {symbol_name}_DATA_LENGTH {len(samples)}\n\n")
        out.write(f"const {c_type} {symbol_name}_DATA[] = {{\n    ")

        for i, s in enumerate(samples):
            out.write(str(s))
            if i != len(samples) - 1:
                out.write(",")
            if (i + 1) % 16 == 0:
                out.write("\n    ")

        out.write("\n};\n")


def main():
    parser = argparse.ArgumentParser(
        description="Convert WAV PCM (8-bit or 16-bit) to a C header file."
    )
    parser.add_argument("input_wav", help="Input WAV file")
    parser.add_argument("output_h", help="Output C header file")

    args = parser.parse_args()
    wav_to_header(args.input_wav, args.output_h)


if __name__ == "__main__":
    main()
