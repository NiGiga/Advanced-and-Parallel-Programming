"""
Nicola Gigante - SM3201239

main.py
"""

import sys

import numpy as np

from palette import Palette
from virtual_vram import VirtualVRAM
from scene_parser import SceneParser
from blitter import Blitter
from rendering_pipeline import RenderingPipeline

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

    ###################
    # Test Blitter 1
    ##################
    frame = np.zeros((480, 640), dtype=np.uint8) # tutto indice 0
    blitter = Blitter(transparent_index=0)

    tile0 = vram.get_tile(0)
    blitter.blit_tile(frame, tile0, 0, 0) # in alto a sx
    blitter.blit_tile(frame, tile0, 620, 460) # quasi fuori schermo, test del clipping
    print("frame min/max: ", frame.min(), frame.max())

    ###################
    # Test Blitter 2
    ##################
    frame = np.zeros((480, 640), dtype=np.uint8) # tutto indice 0
    blitter = Blitter(transparent_index=scene.transparent_index)

    # Disegno background molto semplice
    for row in range(15):
        for col in range(20):
            tile = vram.get_tile(scene.tile_map[row, col])
            blitter.blit_tile(frame, tile, col*32, row*32)

    # Disegno solo il primo sprite della scena
    spr_desc = scene.sprites[0]
    sprite_img = vram.get_sprite(spr_desc.sprite_id)
    blitter.blit_sprite(frame, sprite_img, spr_desc.x, spr_desc.y,
                        flip_h=spr_desc.flip_h, flip_v=spr_desc.flip_v,
                        rotation=spr_desc.rotation)

    print("frame min/max: ", frame.min(), frame.max())


    ###############
    # Test Render
    ###############
    pipeline = RenderingPipeline(palette, vram, scene)
    pipeline.render_to_png(output_path)



if __name__ == "__main__":
    main(sys.argv)