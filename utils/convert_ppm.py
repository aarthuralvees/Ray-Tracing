from pathlib import Path
import sys
import time

from PIL import Image


def main() -> int:
    input_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("image.ppm")

    if len(sys.argv) > 2:
        output_path = Path(sys.argv[2])
    else:
        output_dir = Path("renders")
        output_dir.mkdir(parents=True, exist_ok=True)
        output_path = output_dir / f"{int(time.time())}.png"

    with Image.open(input_path) as image:
        image.save(output_path)

    print(output_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
