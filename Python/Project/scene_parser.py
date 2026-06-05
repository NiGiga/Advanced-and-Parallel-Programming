"""
Nicola Gigante - SM3201239

Implementazione classe SceneParser:
    - apre scene.jeson con json.load,
    - controlla che ci siano tutte le chiavi richieste
    - valida i tipi/forme (dimensione tile_map 15x20, rotation tra i valori ammessi, ecc.)
    - espone proprietà transparent_index, tile_map come np.ndarray 15x20 int, lista di sprite in forma semplice

"""
import json
import numpy as np
from dataclasses import dataclass # per generare __init__, __repr__ e __eq__, ecc. per
                                  # per una "struttura dati" semplice

@dataclass
class SpriteDesc:
    """
    Usato come "record" per la lista di sprite, contiene solo i campi
    necessari a disegnare lo sprite nella pipeline
    """
    sprite_id: int
    x: int
    y: int
    flip_h: bool
    flip_v: bool
    rotation: int

class SceneParser:
    def __init__(self, path: str):
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)

        # 1. Parsing di transparent_index
        ti = data["transparent_index"]

        # controllo che sia un int tra 0 e 15
        if not (isinstance(ti, int) and 0 <= ti <= 15):
            raise ValueError(f"transparent_index must be an int between 0 and 15, got {ti}")
        self.transparent_index = ti # da usare poi nel Blitter

        # 2. Parsing e validazione tile_map 15x20
        tile_map = data["tile_map"]

        # controllo che sia una lista di 15 elementi
        if not(isinstance(tile_map, list) and len(tile_map) == 15):
            raise ValueError(f"tile_map must be a list of 15 elements, got {len(tile_map)}")

        # controllo che ogni riga sia una lista di 20 elementi
        for r, row in enumerate(tile_map):
            if not(isinstance(row, list) and len(row) == 20):
                raise ValueError(f"Row {r} of tile_map must have 20 entries, got {len(row)}")

            # controllo che ogni elemento sia un int tra 0 e 63
            for c, val in enumerate(row):
                if not isinstance(val, int):
                    raise ValueError(f"tile_map[{r}][{c}] must be an int, got {type(val)}")
                if val < 0 or val >= 64:
                    raise ValueError(f"tile_map[{r}][{c}] must be an int between 0 and 63, got {val}")

        self.tile_map = np.array(tile_map, dtype=np.uint8) # creo np.array 15x20 uint8

        # 3. Parsing e validazione lista di sprite
        sprites_data = data["sprites"]

        # controllo che sia una lista
        if not isinstance(sprites_data, list):
            raise ValueError(f"sprites must be a list, got {type(sprites_data)}")

        # setup temporaneo
        sprites: list[SpriteDesc] = [] # lista dove accumulare i descrittori
        allowed_rot = {0, 90, 180, 270} # valori ammessi per la rotazione

        # loop su tutti gli sprite
        for i, s in enumerate(sprites_data): # i è l'indice per i messaggi di errore
                                             # s è il dizionario che descrive lo sprite i nel JSON
            # estrazione dei campi richiesti e gestione dei KeyError
            try:
                sid = s["id"]
                x = s["x"]
                y = s["y"]
                flip_h = s["flip_h"]
                flip_v = s["flip_v"]
                rot = s["rotation"]
            except KeyError as e:
                # e.args[0] contiene il nome del campo mancante
                missing = e.args[0]
                raise ValueError(f"Sprite {i} missing field {missing}")

            # validazioni
            # controllo che ogni campo sia del tipo corretto
            if not isinstance(sid, int):
                raise ValueError(f"Sprite {i}: id must be an int, got {type(sid)}")

            # controllo che x, y siano int
            if not isinstance(x, int) or not isinstance(y, int):
                raise ValueError(f"Sprite {i}: x/y must be int, got {type(x)}/{type(y)}")

            # controllo che rotation sia un int tra i valori ammessi
            if rot not in allowed_rot:
                raise ValueError(f"Sprite {i}; invalid rotation {rot}")

            # controllo che flip_h, flip_v siano bool
            if not isinstance(flip_h, bool) or not isinstance(flip_v, bool):
                raise ValueError(f"Sprite {i}: flip_h/flip_v must be bool, got {type(flip_h)}/{type(flip_v)}")

            # costruzione del record
            sprites.append(SpriteDesc(
                sprite_id=sid, x=x, y=y,
                flip_h=flip_h, flip_v=flip_v,
                rotation=rot,
            ))

        # salvataggio finale
        self.sprites = sprites