"""
Nicola Gigante - SM3201239

Implementazione classe Blitter, che compie le seguenti operazioni:

    - Copia un tile 32x32 indicizzato nel frame buffer 640x480 (solo copia, nessuna trasformazione)
    - Copia un sprite 64x64 indicizzato nel frame buffer 640x480:
        - applicando flip_h e/o flip_v
        - applicando rotation in {0, 90, 180, 270}
        - ignorando i pixel il cui indice è transparent_index
        - gestendo il caso in cui lo sprite esce dallo schermo (clipping)

Nota: Blitter lavora su indici di palette, non su RGB.
"""

import numpy as np

class Blitter:
    def __init__(self, transparent_index: int):
        self.transparent_index = transparent_index

    def blit_tile(self, frame: np.ndarray, tile: np.ndarray,
                  dst_x: int, dst_y: int) -> None:
        """
        Copia una tile 32x32 nel frame buffer 640x480.
        Esegue clipping se la tile esce dallo schermo.
        Non gestisce trasparenza, sovrascrive sempre i pixel.
        :param frame: array (480, 640) di indici di palette (np.uint8)
        :param tile: array (32, 32) di indici,
        :param dst_x: coordinata del pixel in alto a sinistra
        :param dst_y: coordinata del pixel in alto a sinistra
        :return:
        """
        H, W = frame.shape # 480, 640
        th, tw = tile.shape # 32, 32

        # Clipping orizzontale (x)
        src_x_start = max(0, -dst_x)
        src_x_end = min(tw, W - dst_x)
        dst_x_start = max(0, dst_x)
        dst_x_end = min(W, dst_x + tw)

        # Clipping verticale (y)
        src_y_start = max(0, -dst_y)
        src_y_end = min(th, H - dst_y)
        dst_y_start = max(0, dst_y)
        dst_y_end = min(H, dst_y + th)

        # Se non c'è intersezione, exit
        if src_x_start >= src_x_end or src_y_start >= src_y_end:
            return

        # Copia via slicing
        frame[dst_y_start:dst_y_end, dst_x_start:dst_x_end] =  tile[src_y_start:src_y_end, src_x_start:src_x_end]

    def blit_sprite(self, frame: np.ndarray, sprite: np.ndarray,
                    dst_x: int, dst_y: int,
                    flip_h: bool, flip_v: bool, rotation: int) -> None:
        """
        Disegna uno sprite 64x64 nel frame:
        - applica flip_h e/o flip_v
        - applica rotation in {0, 90, 180, 270}
        - rispetta transparent_index (non sovrascrive quei pixel)
        - esegue clipping se lo sprite esce dallo schermo

        :param frame: array (480, 640) di indici di palette (np.uint8)
        :param sprite: array (64, 64) di indici,
        :param dst_x: coordinata del pixel in alto a sinistra
        :param dst_y: coordinata del pixel in alto a sinistra
        :param flip_h: booleano, se True, inverti orizzontalmente
        :param flip_v: booleano, se True, inverti verticalmente
        :param rotation: int {0, 90, 180, 270}
        :return:
        """
        H, W = frame.shape
        sh, sw = sprite.shape # 64, 64

        # 1. Applicazione flip
        spr_t = sprite
        if flip_h:
            spr_t = spr_t[:, ::-1]
        if flip_v:
            spr_t = spr_t[::-1, :]

        # 2. Applicazione rotazione
        rot_map = {0: 0, 90: 1, 180: 2, 270: 3}
        if rotation not in rot_map:
            raise ValueError(f"Invalid rotation {rotation}")

        k = rot_map[rotation]
        if k != 0:
            spr_t = np.rot90(spr_t, k)

        # Dopo rotazione, altezza e larghezza potrebbero essere invertite
        sh, sw = spr_t.shape

        # 3. Copia di sprite
        src_x_start = max(0, -dst_x)
        src_x_end = min(sw, W - dst_x)
        dst_x_start = max(0, dst_x)
        dst_x_end = min(W, dst_x + sw)

        src_y_start = max(0, -dst_y)
        src_y_end = min(sh, H - dst_y)
        dst_y_start = max(0, dst_y)
        dst_y_end = min(H, dst_y + sh)

        if src_x_start >= src_x_end or src_y_start >= src_y_end:
            return

        # 4. Estrazione delle sotto aree interessate
        src = spr_t[src_y_start:src_y_end, src_x_start:src_x_end]
        dst = frame[dst_y_start:dst_y_end, dst_x_start:dst_x_end]

        # 5. Maschera di trasparenza
        mask = (src != self.transparent_index)

        # 6. Copia dei soli pixel non trasparenti
        dst[mask] = src[mask]