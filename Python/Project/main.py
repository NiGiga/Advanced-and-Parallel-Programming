"""
Nicola Gigante - SM3201239

main.py
"""

import sys
from palette import Palette
from virtual_vram import VirtualVRAM
from scene_parser import SceneParser
# from pipeline import RenderingPipeline

def main(argv: list[str]) -> None:
    if len(argv) != 6:
        raise SystemExit(
            "Usage: python main.py <palette.json> <scene.json> "
            "<tiles.bin> <sprites.bin> <output.png>"
        )

    _, palette_path, scene_path, tiles_path, sprites_path, output_path = argv

    #####################
    # Test Palette
    #####################
    palette = Palette(palette_path)

    arr = palette.to_numpy()
    print("Palette shape: ", arr.shape)
    print("First color: ", arr[0])

    #########################
    # Test VirtualVRAM
    #########################
    vram = VirtualVRAM(tiles_path, sprites_path)

    tile0 = vram.get_tile(0)
    print("Tile0 shape: ", tile0.shape)
    print("Tile0 min/max: ", tile0.min(), tile0.max())

    ################
    # Test Parser
    ################
    scene = SceneParser(scene_path)
    print("transparent_index: ", scene.transparent_index)
    print("title_map shape: ", scene.tile_map.shape)
    print("num sprites: ", len(scene.sprites))
    if scene.sprites:
        print("first sprite: ", scene.sprites[0])

    ###############
    # Test Render
    ###############
    # pipeline = RenderingPipeline(palette, vram, scene)
    # pipeline.render(output_path)



if __name__ == "__main__":
    main(sys.argv)