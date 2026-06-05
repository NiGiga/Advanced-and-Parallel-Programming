"""
Nicola Gigante - SM3201239

Implementazione classe Palette:

Input: path JSON.
Responsabilità: caricare e validare un array di 16 colori, ciascuno [R, G, B] tra 0 e 255;
esporre un metodo tipo resolve(index: int) -> (R, G, B).

"""
import json
import numpy as np

class PaletteError(ValueError):
    pass

class Palette:
    def __init__(self, path):
        """
        Carica e controlla:
        che ci siano 16 entry,
        che ogni entri sia una lista/tupla di 3 int ra 0 e 255,
        in caso di errore, solleva un'eccezione specifica
        :param path: path JSON
        """
        # 1. Leggo il JSON
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)

        # 2, Converto in ndarray uint8
        arr = np.array(data, dtype=np.uint8)

        # 3. Controllo forma (16 colori, ognuno con 3 componenti RGB)
        if arr.shape != (16, 3):
            raise PaletteError(f"Palette must be 16x3, got shape {arr.shape}")

        # 4. Controllo che valori siano interi tra 0 e 255
        # (Se c'erano float fuori range, il cast a unit8 li cambia in modo silente)
        if not isinstance(data, list) or len(data) != 16:
            raise PaletteError("Palette must be a list of 16 colors")

        for i, color in enumerate(data):
            if not (isinstance(color, (list, tuple)) and len(color) == 3):
                raise PaletteError(f"Color {i} must be a list/tuple of 3 components")

            for c in color:
                if not (isinstance(c, int) and 0 <= c <= 255):
                    raise PaletteError(f"Invalid component {c} in color {i}")

        self._arr = arr # (16, 3) unit8

    def to_numpy(self) -> np.ndarray:
        """
        :return: array np.ndarrat shape (16, 3), dtype np.uint8
        """
        return self._arr.copy()

    def resolve(self, idx: int) -> tuple[int, int, int]:
        """
        :param idx: indice colore
        :return: Tupla (R, G, B) dopo aver controllato che l'indice sia valido.
        """
        if not (0 <= idx <= 16):
            raise PaletteError(f"Invalid palette index {idx}")

        r, g, b = self._arr[idx]
        return int(r), int(g), int(b)