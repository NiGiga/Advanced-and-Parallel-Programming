# Retro 2D Renderer (Python)

Progetto di Programmazione Avanzata e Parallela - Parte 2 (Python)
Autore: Nicola Gigante - SM3201239

## Descrizione

Il progetto implementa un piccolo **renderer 2D in stile retro** basato su:

- **Palette indicizzata** di 16 colori (4 bit per pixel) caricata da file JSON.
- **Tilesheet** 256x256 con 64 tile 32x32 (griglia 8x8) in formato binario packed 4-bit.
- **Spritesheet** 256x256 con 16 sprites 64x64 (griglia 4x4) in formato binario packed 4-bit.
- **Tilemap** 15x20 per il fondale (640x480).
- **Sprite** sovrapposti al fondale con flip orizzontale e verticale, rotazione (0/90/180/270 gradi) e pixel trasparenti.
\
\
L'output finale è un file PNG 640x480 che compone il fondale e tutti gli sprite secondo la descrizione contenuta nel file di scena JSON.

## Requisiti
- Python 3.x
- Numpy
- Pillow (solo per la generazione del file PNG)

# Utilizzo da linea di comando

Il programma si esegue con il comando:

``bash
python main.py palette.json scene.json tiles.bin sprites.bin out.png
``

## Architettura del codice

Il progetto è suddiviso in diverse classi, ciascuna con una responsabilità chiara.

### `Palette`

- Carica la palette da `palette.json`
- Valida che il file contenga esattamente 16 colori, ognuno con terna `[R, G, B]` di interi 0-255.
- Espone:
    - `to_numpy()` $\to$ array NumPy `(16, 3)` `uint8`.
    - `resolve(index)` $\to$ tupla `(R, G, B)` per un indice 0-15.

### `VirtualVRAM`

- Carica `tiles.bin` e `sprites.bin`
- Decodifica i file packed 4-bit:
    - ogni byte contiene 2 pixel (nibble alto e nibble basso).
    - ricostruisce una matrice 256x256 di indici di palette `uint8`.
- Espone:
    - `get_tile(tile_id)` $\to$ blocco 32x32 da `self._tiles` (id 0-63).
    - `get_sprite(sprite_id)` $\to$ blocco 64x64 da `self._sprites` (id 0-15).

### `SceneParser`

- Carica `scene.json` e valida il contenuto:
    - `transparent_index`: intero 0-15 (usato solo per gli sprite).
    - `tile_map`: lista di sprite, ognuno con campi:
        - `id`, `x`, `y`, `flip_h`, `flip_v`, `rotation`
- Converte `tile_map` in array NumPy `(15, 20)`.
- Rappresenta gli sprite come una semplice `dataclass` (`SpriteDesc`).

### `Blitter`

- Lavora su frame buffer indicizzati (array `(480,640)` di `uint8`).
- Metodi principali:
    - `blit_tile(frame, tile, dst_x, dst_y)`
        - Copia una tile 32x32 nel frame, con clipping se la tile esce dallo schermo.
    - `blit_sprite(frame, sprite, dst_x, dst_y, flip_h, flip_v, rotation)`
        - Applica flip orizzontale/verticale.
        - Applica rotazione 0/90/180/270 gradi.
        - Rispetta `transparent_index` (non sovrascrive quei pixel).
        - Esegue clipping sul frame 649x480.

Tutta la logica è implementata con slicing NumPy e maschere booleane, senza doppi cicli Python espliciti.

### `RenderingPipeline`

- Riceve istanze di `Palette`, `VirtualVRAM` e `SceneParser`.
- Costruisce un `Blitter` con il `transparent_index` della scena.
- Metodo principale: `render_to_png(out_path)`:
    1. Crea il frame buffer indicizzato 640x480 (`uint8`).
    2. Disegna il fondale:
        - per ogni cella (r, c) della `tile_map`, ottiene il tile via `VirtualVRAM.get_tile` e lo copia sul frame con `blit_tile`.
    3. Disegna gli sprite nell'ordine in cui compaiono nel JSON:
        - per ogni sprite, usa `get_sprite` + `blit_sprite`.
    4. Converte il frame indicizzato in RGB:
        - `paltte_arr = palette.to_numpy()` $\to$ `(16, 3)`
        - `frame_rgb = palette_arr[frame]` $\to$ `(480, 640, 3)`
    5. Crea un oggetto `PIL.Image` e salva l'immagine in PNG.

## Note di implementazione

- In caso di input non valido (file non trovato, formati errati, valori fuori range) vengono sollevate eccezioni specifiche (`ValueError` o simili) con messaggi descrittivi.
- NumPy viene usato sistematicamente per lavorare su array di tipo `np.uint8`, in particolare:
    - decodifica dei nibble (operatori bitwise e slicing),
    - ritaglio dei tile/sprite,
    - composizione e conversione indicizzata $\to$ RGB.
- Pillow viene usato esclusivamente per la conversione da array NumPy a PNG, come richiesto dalle specifiche.

## Test e verifica

Durante lo sviluppo sono stati eseguiti alcuni test manuali (via `main.py` e REPL) per verificare i singoli componenti prima della pipeline completa:

- **Test Palette**
  - Caricamento di `palette.json` con `Palette(palette_path)`.
  - Stampa di `palette.to_numpy().shape` (atteso `(16,3)`) e del primo colore.
  - Verifica che valori e range (0–255) siano corretti.

- **Test VirtualVRAM**
  - Istanziazione di `VirtualVRAM(tiles_path, sprites_path)`.
  - `get_tile(0)` e controllo di:
    - shape `(32,32)`, 
    - `min()`/`max()` in `[0,15]`.
  - `get_sprite(id)` con vari id e controllo di:
    - shape `(64,64)`,
    - `min()`/`max()` in `[0,15]`.

- **Test SceneParser**
  - Istanza di `SceneParser(scene_path)`.
  - Stampa di:
    - `transparent_index`,
    - `tile_map.shape` (atteso `(15,20)`),
    - numero di sprite e contenuto del primo `SpriteDesc`.

- **Test Blitter**
  - Creazione di un frame vuoto `np.zeros((480,640), dtype=np.uint8)`.
  - Test di `blit_tile`:
    - copia di una tile in `(0,0)` e in posizioni quasi fuori schermo per verificare il clipping,
    - controllo di `frame.min()`/`frame.max()` per accertare che il frame venga modificato.
  - Test di `blit_sprite` su un singolo sprite:
    - posizionamento sul frame con e senza flip/rotazione,
    - verifica visiva successiva nella PNG.

- **Test end‑to‑end**
  - Esecuzione del comando:
    ```bash
    python main.py palette.json scene.json tiles.bin sprites.bin out.png
    ```
  - Confronto visivo di `out.png` con l’immagine di riferimento fornita (presenza del fondale, pinguini e pesci nelle posizioni corrette).


















