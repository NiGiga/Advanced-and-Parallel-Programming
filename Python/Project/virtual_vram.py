"""
Nicola Gigante - SM3201239

Implementazione classe VirtualVRAM:

    - Riceve in input file binari da 32768 byte (tile sheet e sprite sheet),
    - decodifica ogni byte come 2 pixel (nibble alto e nibble basso),
    - ottiene un np.ndarray 256x256 di uint8 (indici di palette),
    - applica metodi get_tile e get_sprite per ritagliare blocchi 32x32 o 64x64 dalle matrici.

"""

import numpy as np

class VirtualVRAM:
    def __init__(self, tiles_path: str, sprites_path: str):

        self._tiles = self._load_sheet(tiles_path)
        self._sprites = self._load_sheet(sprites_path)

    @staticmethod
    def _load_sheet(path: str) -> np.ndarray:
        """
        Funzione utility per decodificare i file binari.
        :param path: file binario da decodificare
        :return: matrice 256x256 di uint8
        """
        # Leggo i file binari
        with open(path, "rb") as f:
            data = f.read()

        # 32768 bytes attesi: 256*256/2
        if len(data) != (256*256) // 2:
            raise ValueError(f"Invalid sheet size for {path}: got {len(data)} bytes")

        # creo array di uint8 che guarda il buffer senza copia
        bytes_arr = np.frombuffer(data, dtype=np.uint8) # (32768,)

        # estraggo i 2 nibble per byte
        high = bytes_arr >> 4 # shift a destra di 4 bit, primi pixel
        low = bytes_arr & 0x0F # maschero e tengo solo i bit bassi, secondi pixel

        # Interleaviamo high/low: (65536,) e poi reshape 256x256
        pixels = np.empty(high.size + low.size, dtype=np.uint8)
        pixels[0::2] = high
        pixels[1::2] = low

        # prendiamo vettore 65536 -> 256x256, matrice row-major
        sheet = pixels.reshape((256, 256)) # (H,W)

        return sheet

    def get_tile(self, tile_id: int) -> np.ndarray:
        """
        Metodo per ritagliare un blocco 32x32 da una matrice di tile.
        :param tile_id: tile da ritagliare
        :return: blocco 32x32
        """
        # Check validità tile_id
        if not (0 <= tile_id < 64):
            raise ValueError(f"Invalid tile ID {tile_id}")

        tile_size = 32
        tiles_per_row = 256 // tile_size # 8

        # passo da id lineare -> coordinate griglia
        row = tile_id // tiles_per_row
        col = tile_id % tiles_per_row

        # passo da coordinate griglia (row, col) -> indice in array (posizione pixel)
        # int per evitare overflow
        y0 = int(row * tile_size)
        x0 = int(col * tile_size)

        # ritaglio blocco 32x32
        return self._tiles[y0:y0+tile_size, x0:x0+tile_size]

    def get_sprite(self, sprite_id: int) -> np.ndarray:
        """
        Metodo per ritagliare un blocco 64x64 da una matrice di sprite.
        :param sprite_id: sprite da ritagliare
        :return: blocco 64x64
        """
        # Check validità sprite_id
        if not (0 <= sprite_id < 16):
            raise ValueError(f"Invalid sprite ID {sprite_id}")

        sprite_size = 64
        sprite_per_row = 256 // sprite_size # 4

        # passo da id lineare -> coordinate griglia
        row = sprite_id // sprite_per_row
        col = sprite_id % sprite_per_row

        # passo da coordinate griglia (row, col) -> indice in array (posizione pixel)
        # int per evitare overflow
        y0 = int(row * sprite_size)
        x0 = int(col * sprite_size)

        # ritaglio blocco 64x64
        return self._sprites[y0:y0+sprite_size, x0:x0+sprite_size]