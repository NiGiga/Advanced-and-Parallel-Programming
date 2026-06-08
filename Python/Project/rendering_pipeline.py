"""
Nicola Gigante - SM3201239

Implementazione classe RenderingPipeline, la quale compie le seguenti operazioni:
    - Crea un frame buffer 640x480 inizializzato a un valore di BG (es. 0),
    - Disegna il fondale usando scene.tile_map e blit_tile,
    - Disegna tutti gli sprite in ordine con blit_sprite,
    - Converte il frame indicizzato in immagine RGB usando la palette 16x3,
    - Salva come PNG usando Pillow
"""

import numpy as np
from PIL import Image

import palette
from blitter import Blitter
from palette import Palette
from virtual_vram import VirtualVRAM
from scene_parser import SceneParser

class RenderingPipeline:
    def __init__(self, palette: Palette, vram: VirtualVRAM, scene: SceneParser):
        self.palette = palette
        self.vram = vram
        self.scene = scene

        # Blitter costruito con il transparent_index della scena
        self.blitter = Blitter(transparent_index=scene.transparent_index)

    def render_to_png(self, output_path: str) -> None:
        """
        Esegue la composizione completa:
        - crea frame indicizzato 640x480
        - disegna tile di background
        - disegna tutti gli sprite
        - converte in immagine RGB
        - salva come PNG

        :param output_path: path output PNG
        :return: None
        """
        H, W = 480, 640

        # 1. Frame indicizzato
        frame = np.zeros((H, W), dtype=np.uint8)


        # 2. Disegno background tramite tile_map (15 righe x 20 colonne)
        tile_h, tile_w = 32, 32
        for row in range(15):
            for col in range(20):
                tile_id = int(self.scene.tile_map[row, col])
                tile_img = self.vram.get_tile(tile_id)
                x = col * tile_w
                y = row * tile_h
                self.blitter.blit_tile(frame, tile_img, x, y)


        # 3. Disegna gli sprite nell'ordine del JSON
        # --------- DEBUG ------------------------------------------
        # print("Rendering ", len(self.scene.sprites), " sprites")
        # ----------------------------------------------------------
        for spr in self.scene.sprites:
            sprite_img = self.vram.get_sprite(spr.sprite_id)
            # --------- DEBUG -----------------------------------------
            # print(" sprite: ", spr)
            # print(" sprite_img shape: ", sprite_img.shape, " min/max: ",
                  # sprite_img.min(), sprite_img.max())
            # ---------------------------------------------------------
            self.blitter.blit_sprite(frame, sprite_img, spr.x, spr.y,
                                    flip_h=spr.flip_h, flip_v=spr.flip_v,
                                    rotation=spr.rotation)

        # ---------- DEBUG --------------------------------------------
        # print("frame min/max before RGB: ", frame.min(), frame.max())
        # -------------------------------------------------------------

        # 4. Conversione indicizzato -> RGB usando la palette
        # palette_array: shape (16, 3), uint8
        palette_arr = self.palette.to_numpy() # 16x3

        # frame_rgb: shape (480, 640, 3)
        # indicizzazione avanzata usando indici di frame come mappa
        # se frame[y, x] = k, allora frame_rgb[y, x] = palette_arr[k] (vettore [R, G, B])
        frame_rgb = palette_arr[frame]

        # 5. Salvataggio PNG con Pillow
        img = Image.fromarray(frame_rgb, mode="RGB")
        img.save(output_path)